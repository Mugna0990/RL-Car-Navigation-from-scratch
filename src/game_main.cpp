#include "Agent.h"
#include "State.h"
#include "game/Map.h"
#include "game/Car.h"
#include "game/Game.h"
#include <iostream>
#include <fstream>
#include <filesystem>

void displayTrackWithMovement(const std::string& trackFilePath, const std::string& movementFilePath);

void train(Agent& agent, Map& map, int episodes, const std::string& save_path) {
    if (!std::filesystem::exists(save_path)) {
        if (std::filesystem::create_directories(save_path)) {
            std::cout << "Created save directory: " << save_path << std::endl;
        } else {
            std::cerr << "❌ Error: Could not create save directory: " << save_path << std::endl;
        }
    }

    std::ofstream movementFile("/Users/matteomugnai/Desktop/RL/assets/movements.txt", std::ios::app);

    if (!movementFile.is_open()) {
        std::cerr << "❌ Failed to open training_movements.txt\n";
    }

    int startX = -1, startY = -1;
    if (!map.find('S', startX, startY)) {
        std::cerr << "Start position not found in the map!\n";
        if (movementFile.is_open()) movementFile.close();
        return;
    }

    Car car(startX, startY);
    int prevDist = car.minDotsToGoal(map);
    int maxSteps = prevDist * 2;

    int save_frequency = 1000;

    for (int episode = 0; episode < episodes; ++episode) {
        if (episode > 0 && episode % 1500 == 0) {
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

        bool logThisEpisode = (episode % 100 == 0);

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
                reward += 5.0 * improvement;
            }
            reward -= 1;

            prevDist = newDist;

            if (status == UpdateStatus::GOAL) {
                reward += 100.0;
                done = true;
                std::cout << "✅ Goal reached in episode " << episode << "\n";
            } else if (status == UpdateStatus::COLLISION) {
                reward -= 100.0;
                done = true;
                std::cout << "💥 Collision in episode " << episode << "\n";
            }

            State nextState(car.getX(), car.getY(), car.getDirection(), car.getVelocity());

            agent.store_transition(currentState, action, reward, nextState, done);
            agent.experience_replay(32);

            episodeReward += reward;
        }

        if (episode > 0 && episode % save_frequency == 0) {
             std::string episode_save_path = save_path + "/episode_" + std::to_string(episode);
             if (!std::filesystem::exists(episode_save_path)) {
                 std::filesystem::create_directories(episode_save_path);
             }
             agent.q_network.save(episode_save_path + "/q_network");
             agent.target_q_network.save(episode_save_path + "/target_q_network");
             std::cout << "💾 Saved networks at episode " << episode << " to " << episode_save_path << std::endl;
        }

        if(episode%500 == 0){
            agent.update_target_network();
            }

        std::cout << "📘 Episode " << episode
                  << " | Total reward: " << episodeReward
                  << " | Epsilon: " << agent.epsilon << "\n";
    }

    std::string final_save_path = save_path + "/final";
     if (!std::filesystem::exists(final_save_path)) {
         std::filesystem::create_directories(final_save_path);
     }
    agent.q_network.save(final_save_path + "/q_network");
    agent.target_q_network.save(final_save_path + "/target_q_network");
    std::cout << "💾 Saved final networks to " << final_save_path << std::endl;

    if (movementFile.is_open()) {
        movementFile.close();
        std::cout << "✅ Movement log saved to training_movements.txt\n";
    }
}

int main() {
    std::vector<int> layerSizes = {4, 128, 64, 6};
    size_t buffer_capacity = 10000;
    double initial_epsilon = 1.0;
    double epsilon_decay = 0.999;
    double min_epsilon = 0.01;
    double discount_factor = 0.99;
    int num_actions = 6;
    std::string save_directory = "/Users/matteomugnai/Desktop/RL/trained_agent";

    bool load_agent = false;
    std::string load_path = save_directory + "/final";

    if (load_agent) {
        Agent agent(layerSizes,
                    buffer_capacity,
                    min_epsilon,
                    epsilon_decay,
                    min_epsilon,
                    discount_factor,
                    num_actions);

        std::cout << "Attempting to load agent from: " << load_path << std::endl;
        if (std::filesystem::exists(load_path + "/q_network/layer0.txt")) {
             agent.q_network.load(load_path + "/q_network");
             agent.target_q_network.load(load_path + "/target_q_network");
             std::cout << "✅ Agent loaded successfully." << std::endl;

             Game game;
             std::cout << "Agent loaded. Ready for evaluation or deployment." << std::endl;

        } else {
            std::cerr << "❌ Error: Could not find saved agent files at " << load_path << ". Starting training instead." << std::endl;
            Agent agent_train(layerSizes,
                        buffer_capacity,
                        initial_epsilon,
                        epsilon_decay,
                        min_epsilon,
                        discount_factor,
                        num_actions);
             Game game;
             train(agent_train, game.track, 100000, save_directory);
        }

    } else {
        Agent agent_train(layerSizes,
                    buffer_capacity,
                    initial_epsilon,
                    epsilon_decay,
                    min_epsilon,
                    discount_factor,
                    num_actions);

        Game game;
        train(agent_train, game.track, 100000, save_directory);
    }

    return 0;
}