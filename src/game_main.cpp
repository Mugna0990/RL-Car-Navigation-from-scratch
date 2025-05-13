#include "Agent.h"
#include "State.h"
#include "game/Map.h"
#include "game/Car.h"
#include "game/Game.h"
#include <iostream>
#include <fstream>
#include <filesystem> // Make sure this is included for std::filesystem

void displayTrackWithMovement(const std::string& trackFilePath, const std::string& movementFilePath);

void train(Agent& agent, Map& map, int episodes, const std::string& save_path) {
    if (!std::filesystem::exists(save_path)) {
        if (std::filesystem::create_directories(save_path)) {
            std::cout << "Created save directory: " << save_path << std::endl;
        } else {
            std::cerr << "❌ Error: Could not create save directory: " << save_path << std::endl;
            return; // Exit if directory cannot be created
        }
    }

    // Open the movement log file
    std::ofstream movementFile("/Users/matteomugnai/Desktop/RL/assets/movements.txt", std::ios::app);
    if (!movementFile.is_open()) {
        std::cerr << "❌ Failed to open movements.txt\n";
        return;
    }

    // Open the reward log file
    std::ofstream rewardFile("/Users/matteomugnai/Desktop/RL/assets/episode_rewards.txt", std::ios::app);
    if (!rewardFile.is_open()) {
        std::cerr << "❌ Failed to open episode_rewards.txt\n";
        movementFile.close(); // Close movements file if it's open
        return;
    }
    // Write header if the file is new or empty (optional, good for clarity)
    if (rewardFile.tellp() == 0) {
        rewardFile << "Episode,Reward\n";
    }

    // New file to record episode and distance
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

    for (int episode = 0; episode < episodes; ++episode) {
        // Optionally log every 100th episode for tracking movements
        if (episode > 0 && episode % 3000 == 0) {
            if (movementFile.is_open()) movementFile.close();
            displayTrackWithMovement("/Users/matteomugnai/Desktop/RL/assets/track.txt", "/Users/matteomugnai/Desktop/RL/assets/movements.txt");
            movementFile.open("/Users/matteomugnai/Desktop/RL/assets/movements.txt", std::ios::app);
            if (!movementFile.is_open()) {
                std::cerr << "❌ Failed to re-open training_movements.txt after display\n";
            }
        }

        Car car(startX, startY);
        prevDist = car.minDotsToGoal(map);
        bool done = false;
        double episodeReward = 0.0;

        bool logThisEpisode = (episode % 200 == 0); // Log car movements for every 100th episode

        for (int step = 0; step < maxSteps && !done; ++step) {
            if (logThisEpisode && movementFile.is_open()) {
                movementFile << car.getX() << " " << car.getY() << "\n";
            }

            State currentState(car.getX(), car.getY(), car.getDirection(), car.getVelocity());
            int action = agent.select_action(currentState);
            Direction currentDir = car.getDirection();

            switch (action) {
                case 0: // UP
                    if (currentDir == DOWN) car.decelerate();
                    else car.setDirection(static_cast<Direction>(UP));
                    break;
                case 1: // RIGHT
                    if (currentDir == LEFT) car.decelerate();
                    else car.setDirection(static_cast<Direction>(RIGHT));
                    break;
                case 2: // DOWN
                    if (currentDir == UP) car.decelerate();
                    else car.setDirection(static_cast<Direction>(DOWN));
                    break;
                case 3: // LEFT
                    if (currentDir == RIGHT) car.decelerate();
                    else car.setDirection(static_cast<Direction>(LEFT));
                    break;
                case 4: // Accelerate
                    car.accelerate();
                    break;
                case 5: // Decelerate
                    car.decelerate();
                    break;
                default:
                    std::cerr << "Warning: Unknown action selected: " << action << std::endl;
                    break;
            }

            UpdateStatus status = car.update(map);
            int newDist = car.minDotsToGoal(map);

            double reward = 0.0;
            if (newDist != -1 && prevDist != -1) {
                double improvement = prevDist - newDist;
                reward += 5 * improvement;
                if (newDist < bestDist) {
                    reward += 5 * improvement;
                    bestDist = newDist;
                }
            }
            reward -= 0.5; // Penalty for each step

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
    std::vector<int> layerSizes = {4, 128, 128, 6};
    size_t buffer_capacity = 100000;
    double initial_epsilon = 1.0;
    double epsilon_decay = 0.9997;
    double min_epsilon = 0.05;
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