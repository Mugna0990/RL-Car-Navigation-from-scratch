#include "Agent.h"
#include "State.h"
#include "game/Map.h"
#include "game/Car.h"
#include "game/Game.h"
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <filesystem> 
#include <functional> 

struct pair_hash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

void displayTrackWithMovement(const std::string& trackFilePath, const std::string& movementFilePath);

void train(Agent& agent, Map& map, int episodes, const std::string& save_path) {

    std::vector<std::pair<int, int>> freeCells;
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (map.getTile(x, y) != '#' && map.getTile(x, y) != 'G') {
                freeCells.emplace_back(x, y);
            }
        }
    }
    
    if (!std::filesystem::exists(save_path)) {
        if (std::filesystem::create_directories(save_path)) {
            std::cout << "Created save directory: " << save_path << std::endl;
        } else {
            std::cerr << "❌ Error: Could not create save directory: " << save_path << std::endl;
            return; // Exit if directory cannot be created
        }
    }

    // movement log file
    std::ofstream movementFile("/Users/matteomugnai/Desktop/RL/assets/movements.txt", std::ios::app);
    if (!movementFile.is_open()) {
        std::cerr << "❌ Failed to open movements.txt\n";
        return;
    }

    // reward log file
    std::ofstream rewardFile("/Users/matteomugnai/Desktop/RL/assets/episode_rewards.txt", std::ios::app);
    if (!rewardFile.is_open()) {
        std::cerr << "❌ Failed to open episode_rewards.txt\n";
        movementFile.close(); // Close movements file if it's open
        return;
    }

    // episode and distance
    std::ofstream distanceFile("/Users/matteomugnai/Desktop/RL/assets/episode_distance.txt", std::ios::app);
    if (!distanceFile.is_open()) {
        std::cerr << "❌ Failed to open episode_distance.txt\n";
        movementFile.close();
        rewardFile.close();
        return;
    }
    // Write header if the file is new or empty
    if (distanceFile.tellp() == 0) {
        distanceFile << "Episode,Distance\n";
    }

    int startX = -1, startY = -1;
    if (!map.find('S', startX, startY)) {
        std::cerr << "Start position not found in the map!\n";
        movementFile.close();
        rewardFile.close();
        distanceFile.close();
        return;
    }

    Car car(startX, startY);
    int prevDist = car.minDotsToGoal(map);
    int bestDist = prevDist;
    int maxSteps = prevDist * 2;

    int save_frequency = 1000;
    int random_start_frequency = 5;
    int display_movements_frequency = 35000;
    int save_movements_frequency = 500;

    for (int episode = 0; episode < episodes; ++episode) {

        std::vector<std::pair<int, int>> episodeMovements;

        
        if (episode > 0 && episode % display_movements_frequency == 0) {
            if (movementFile.is_open()) movementFile.close();
            displayTrackWithMovement("/Users/matteomugnai/Desktop/RL/assets/track.txt", "/Users/matteomugnai/Desktop/RL/assets/movements.txt");
            movementFile.open("/Users/matteomugnai/Desktop/RL/assets/movements.txt", std::ios::app);
            if (!movementFile.is_open()) {
                std::cerr << "❌ Failed to re-open training_movements.txt after display\n";
            }
        }
        int carStartX = startX;
        int carStartY = startY;

        // Choose a new random position every 10 episodes (but not every 200)
        if (episode % random_start_frequency == 0 && episode % save_movements_frequency != 0) {
            if (!freeCells.empty()) {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, freeCells.size() - 1);

                auto randomCell = freeCells[dis(gen)];
                carStartX = randomCell.first;
                carStartY = randomCell.second;

            }
        }

        // Now instantiate the car once, in one place
        Car car(carStartX, carStartY);

       
        prevDist = car.minDotsToGoal(map);
        bool done = false;
        double episodeReward = 0.0;

        bool logThisEpisode = (episode % save_movements_frequency == 0); 
        std::unordered_set<std::pair<int, int>, pair_hash> visited;

        for (int step = 0; step < maxSteps && !done; ++step) {

            episodeMovements.emplace_back(car.getX(), car.getY());

            int x = car.getX();
            int y = car.getY();

            // Compute distances to wall in four directions
            int distU = 0, distR = 0, distD = 0, distL = 0;

            // UP
            for (int i = y - 1; i >= 0; --i) {
                if (map.getTile(x, i) == '#') break;
                distU++;
            }

            // RIGHT
            for (int i = x + 1; i < MAP_WIDTH; ++i) {
                if (map.getTile(i, y) == '#') break;
                distR++;
            }

            // DOWN
            for (int i = y + 1; i < MAP_HEIGHT; ++i) {
                if (map.getTile(x, i) == '#') break;
                distD++;
            }

            // LEFT
            for (int i = x - 1; i >= 0; --i) {
                if (map.getTile(i, y) == '#') break;
                distL++;
            }

            // Create current state with distances
            State currentState(x, y, car.getDirection(), car.getVelocity());
            currentState.distU = distU;
            currentState.distR = distR;
            currentState.distD = distD;
            currentState.distL = distL;
            int action = agent.select_action(currentState);

            switch (action) {
                case 0: car.accelerate(); break;
                case 1: car.decelerate(); break;
                case 2:
                    if (car.getDirection() == RIGHT) car.decelerate();
                    else car.setDirection(LEFT);
                    break;
                case 3:
                    if (car.getDirection() == LEFT) car.decelerate();
                    else car.setDirection(RIGHT);
                    break;
                case 4:
                    if (car.getDirection() == DOWN) car.decelerate();
                    else car.setDirection(UP);
                    break;
                case 5:
                    if (car.getDirection() == UP) car.decelerate();
                    else car.setDirection(DOWN);
                    break;
                default:
                    std::cerr << "Unknown action index: " << action << std::endl;
                    break;
            }

            UpdateStatus status = car.update(map);
            int newDist = car.minDotsToGoal(map);

            double reward = 0.0;
            if (newDist != -1 && prevDist != -1) {
                double improvement = prevDist - newDist;
                reward += 5 * improvement;
                if (newDist < bestDist && !(episode % random_start_frequency == 0 && episode % save_movements_frequency != 0)) {
                    reward += 5 * improvement;
                }
            }
            reward -= 0.5; // Penalty for each step

            if (visited.count({car.getX(), car.getY()})) {
                reward -= 5.0; // Penalize loops
            }
            visited.insert({car.getX(), car.getY()});

            bool newBestPath = (prevDist < bestDist);

            if ((newDist < bestDist && !(episode % random_start_frequency == 0) )|| logThisEpisode) {
                for (const auto& pos : episodeMovements) {
                    if (movementFile.is_open()) {
                        movementFile << pos.first << " " << pos.second << "\n";
                    }
                }
            }

            if (newBestPath) {
                bestDist = prevDist;
            }

            prevDist = newDist;

            if (status == UpdateStatus::GOAL) {
                reward += 1000.0;
                done = true;
            } else if (status == UpdateStatus::COLLISION) {
                reward -= 100.0;
                done = true;
            }

            State nextState(car.getX(), car.getY(), car.getDirection(), car.getVelocity());
            agent.store_transition(currentState, action, reward, nextState, done);
            agent.experience_replay(64);

            episodeReward += reward;
        }

        // Log episode reward to file
        if (rewardFile.is_open()) {
            rewardFile << episode << "," << episodeReward << "\n";
        }

        // Log episode distance to file
        if (distanceFile.is_open()) {
            distanceFile << episode << "," << prevDist << "\n";  // Log the final distance for this episode
        }

        std::cout << "📘 Episode " << episode
                  << " | Total reward: " << episodeReward
                  << " | Distance: " << prevDist
                  << " | Epsilon: " << agent.epsilon << "\n";

        if (agent.epsilon > agent.min_epsilon) {
            agent.epsilon *= agent.epsilon_decay;
            if (agent.epsilon < agent.min_epsilon) {
                agent.epsilon = agent.min_epsilon;
            }
        }

        // Save networks at specified frequency
        if (episode % save_frequency == 0) {
            std::string episode_save_path = save_path + "/episode_" + std::to_string(episode);
            if (!std::filesystem::exists(episode_save_path)) {
                std::filesystem::create_directories(episode_save_path);
            }

            std::string q_network_dir = episode_save_path + "/q_network";
            if (!std::filesystem::exists(q_network_dir)) {
                if (!std::filesystem::create_directories(q_network_dir)) {
                    std::cerr << "❌ Error: Could not create Q-network save directory: " << q_network_dir << std::endl;
                }
            }

            std::string target_q_network_dir = episode_save_path + "/target_q_network";
            if (!std::filesystem::exists(target_q_network_dir)) {
                if (!std::filesystem::create_directories(target_q_network_dir)) {
                    std::cerr << "❌ Error: Could not create Target Q-network save directory: " << target_q_network_dir << std::endl;
                }
            }

            agent.q_network.save(q_network_dir);
            agent.target_q_network.save(target_q_network_dir);
            std::cout << "💾 Saved networks at episode " << episode << " to " << episode_save_path << std::endl;
        }

        if (episode % 500 == 0) {
            agent.update_target_network();
        }
    }

    // Save final networks
    std::string final_save_path = save_path + "/final";
    if (!std::filesystem::exists(final_save_path)) {
        std::filesystem::create_directories(final_save_path);
    }

    std::string final_q_network_dir = final_save_path + "/q_network";
    if (!std::filesystem::exists(final_q_network_dir)) {
        if (!std::filesystem::create_directories(final_q_network_dir)) {
            std::cerr << "❌ Error: Could not create final Q-network save directory: " << final_q_network_dir << std::endl;
        }
    }

    std::string final_target_q_network_dir = final_save_path + "/target_q_network";
    if (!std::filesystem::exists(final_target_q_network_dir)) {
        if (!std::filesystem::create_directories(final_target_q_network_dir)) {
            std::cerr << "❌ Error: Could not create final Target Q-network save directory: " << final_target_q_network_dir << std::endl;
        }
    }

    agent.q_network.save(final_q_network_dir);
    agent.target_q_network.save(final_target_q_network_dir);
    std::cout << "💾 Saved final networks to " << final_save_path << std::endl;

    // Close all files
    if (movementFile.is_open()) movementFile.close();
    if (rewardFile.is_open()) rewardFile.close();
    if (distanceFile.is_open()) distanceFile.close();
}


int main() {
    std::vector<int> layerSizes = {8, 128, 128, 6};
    size_t buffer_capacity = 100000;
    double initial_epsilon = 1.0;
    double epsilon_decay = 0.9999;
    double min_epsilon = 0.1;
    double discount_factor = 0.97;
    int num_actions = 6;
    std::string save_directory = "/Users/matteomugnai/Desktop/RL/trained_agent";

    bool load_agent = false;
    std::string load_path = save_directory + "/episode";

    if (load_agent) {
        
        Agent agent(layerSizes,
                    buffer_capacity,
                    initial_epsilon, 
                    epsilon_decay,
                    min_epsilon,
                    discount_factor,
                    num_actions,
                    load_path
                );
        Game game;
        train(agent, game.track, 1000000, save_directory);

    } else {
        // This block is for starting a fresh training session
        Agent agent_train(layerSizes,
                    buffer_capacity,
                    initial_epsilon, 
                    epsilon_decay,
                    min_epsilon,
                    discount_factor,
                    num_actions,
                    "no_load"
                );
        Game game;
        train(agent_train, game.track, 1000000, save_directory);
    }

    return 0;
}