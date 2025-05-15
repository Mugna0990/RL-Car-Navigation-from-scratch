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

// training loop
void train(Agent& agent, Map& map, int episodes, const std::string& save_path) {
    
    // free cells used as random starting points
    std::vector<std::pair<int, int>> freeCells;
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (map.getTile(x, y) != '#' && map.getTile(x, y) != 'G') {
                freeCells.emplace_back(x, y);
            }
        }
    }

    if (!std::filesystem::exists(save_path)) {
        if (!std::filesystem::create_directories(save_path)) {
            std::cerr << " Could not create save directory: " << save_path << std::endl;
            return;
        }
    }

    std::ofstream movementFile("/assets/movements.txt", std::ios::app);
    if (!movementFile.is_open()) {
        std::cerr << "Failed to open movements.txt\n";
        return;
    }

    int startX = -1, startY = -1;
    if (!map.find('S', startX, startY)) {
        std::cerr << "Start position not found in the map!\n";
        movementFile.close();
        return;
    }

    Car car(startX, startY);
    int prevDist = car.minDotsToGoal(map);
    int maxSteps = prevDist * 2;

    int save_frequency = 5000;
    int random_start_frequency = 5;
    int display_movements_frequency = 15000;
    int save_movements_frequency = 1000;
    int bestDist = MAP_HEIGHT * MAP_WIDTH;

    for (int episode = 0; episode < episodes; ++episode) {
        std::vector<std::pair<int, int>> episodeMovements;

        if (episode > 0 && episode % display_movements_frequency == 0) {
            if (movementFile.is_open()) movementFile.close();
            displayTrackWithMovement("/assets/track.txt", "/assets/movements.txt");
            movementFile.open("/assets/movements.txt", std::ios::app);
        }

        int carStartX = startX;
        int carStartY = startY;

        bool logEpisode = episode%save_movements_frequency == 0;
        bool randomStartEpisode = episode % random_start_frequency == 0;
        
        // random start
        if ( randomStartEpisode && !logEpisode) {
            if (!freeCells.empty()) {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, freeCells.size() - 1);
                auto randomCell = freeCells[dis(gen)];
                carStartX = randomCell.first;
                carStartY = randomCell.second;
            }
        }

        Car car(carStartX, carStartY);
        prevDist = car.minDotsToGoal(map);
        bool done = false;
        double episodeReward = 0.0;

        std::unordered_set<std::pair<int, int>, pair_hash> visited;

        for (int step = 0; step < maxSteps && !done; ++step) {
            episodeMovements.emplace_back(car.getX(), car.getY());

            int x = car.getX();
            int y = car.getY();

            int distU = 0, distR = 0, distD = 0, distL = 0;
            int newDist = car.minDotsToGoal(map);
            
            // calculate distance from wall
            for (int i = y - 1; i >= 0; --i) if (map.getTile(x, i) == '#') break; else distU++;
            for (int i = x + 1; i < MAP_WIDTH; ++i) if (map.getTile(i, y) == '#') break; else distR++;
            for (int i = y + 1; i < MAP_HEIGHT; ++i) if (map.getTile(x, i) == '#') break; else distD++;
            for (int i = x - 1; i >= 0; --i) if (map.getTile(i, y) == '#') break; else distL++;

            // create the state
            State currentState(x, y, car.getDirection(), car.getVelocity());
            currentState.distU = distU;
            currentState.distR = distR;
            currentState.distD = distD;
            currentState.distL = distL;
            currentState.distG = newDist;

            int action = agent.select_action(currentState);

            // action selected by the network
            switch (action) {
                case 0: car.accelerate(); break;
                case 1: car.decelerate(); break;
                case 2: car.getDirection() == RIGHT ? car.decelerate() : car.setDirection(LEFT); break;
                case 3: car.getDirection() == LEFT ? car.decelerate() : car.setDirection(RIGHT); break;
                case 4: car.getDirection() == DOWN ? car.decelerate() : car.setDirection(UP); break;
                case 5: car.getDirection() == UP ? car.decelerate() : car.setDirection(DOWN); break;
                default: std::cerr << "Unknown action: " << action << "\n"; break;
            }

            UpdateStatus status = car.update(map);
            
            double reward = 0.0;
            if (newDist != -1 && prevDist != -1) {
                double improvement = prevDist - newDist;
                reward += 5.0 * improvement;

                // more reward for best distance
                if (newDist < bestDist && !(randomStartEpisode)) {
                    reward += 3.0*improvement;
                }
            }

            // time penalty
            reward -= 1;

            // prevents loop
            if (visited.count({car.getX(), car.getY()})) reward -= 5.0;
            visited.insert({car.getX(), car.getY()});

            // conditions that end the episode
            if (status == UpdateStatus::GOAL || newDist < 5) {
                reward += 500.0;
                done = true;
            } else if (status == UpdateStatus::COLLISION) {
                reward -= 100.0;
                done = true;
            }

            State nextState(car.getX(), car.getY(), car.getDirection(), car.getVelocity());
            agent.store_transition(currentState, action, reward/1000, nextState, done);
            agent.experience_replay(64);

            episodeReward += reward;
            prevDist = newDist;
        }

        // update and log if new best distance
        bool newBestPath = (prevDist < bestDist);
        if (newBestPath && !(randomStartEpisode)) {
            bestDist = prevDist;
        }

        if (logEpisode || (newBestPath && !randomStartEpisode)) {
            for (const auto& pos : episodeMovements) {
                if (movementFile.is_open()) {
                    movementFile << pos.first << " " << pos.second << "\n";
                }
            }
        }
        

        std::cout << "📘 Episode " << episode
                  << " | Total reward: " << episodeReward
                  << " | Current Distance: " << prevDist
                  << " | Best Distance: " << bestDist
                  << " | Epsilon: " << agent.epsilon << "\n";

        // epsilon decay
        if (agent.epsilon > agent.min_epsilon) {
            agent.epsilon *= agent.epsilon_decay;
            if (agent.epsilon < agent.min_epsilon) agent.epsilon = agent.min_epsilon;
        }

        if (episode % save_frequency == 0) {
            std::string episode_save_path = save_path + "/episode_" + std::to_string(episode);
            std::filesystem::create_directories(episode_save_path + "/q_network");
            std::filesystem::create_directories(episode_save_path + "/target_q_network");

            agent.q_network.save(episode_save_path + "/q_network");
            agent.target_q_network.save(episode_save_path + "/target_q_network");
            std::cout << "💾 Saved networks at episode " << episode << "\n";
        }

        if (episode % 100 == 0) {
            agent.update_target_network();
        }
    }

    std::string final_save_path = save_path + "/final";
    std::filesystem::create_directories(final_save_path + "/q_network");
    std::filesystem::create_directories(final_save_path + "/target_q_network");

    agent.q_network.save(final_save_path + "/q_network");
    agent.target_q_network.save(final_save_path + "/target_q_network");
    std::cout << "Saved final networks to " << final_save_path << std::endl;

    if (movementFile.is_open()) movementFile.close();
}


int main() {
    std::vector<int> layerSizes = {9, 128, 128, 6};
    size_t buffer_capacity = 100000;
    double initial_epsilon = 1.0;
    double epsilon_decay = 0.9998;
    double min_epsilon = 0.05;
    double discount_factor = 0.95;
    int num_actions = 6;
    std::string save_directory = "/trained_agent";

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
        train(agent_train, game.track, 100000, save_directory);
    }

    return 0;
}