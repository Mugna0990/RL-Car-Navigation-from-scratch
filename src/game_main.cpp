#include "Agent.h"
#include "State.h"
#include "game/Map.h"
#include "game/Car.h"
#include <iostream>

#include <fstream>

void train(Agent& agent, Map& map, int episodes, int maxStepsPerEpisode) {
    std::ofstream movementFile("training_movements.txt");

    if (!movementFile.is_open()) {
        std::cerr << "❌ Failed to open training_movements.txt\n";
        return;
    }

    for (int episode = 0; episode < episodes; ++episode) {
        // Reset car at 'S'
        int startX = -1, startY = -1;
        if (!map.find('S', startX, startY)) {
            std::cerr << "Start position not found in the map!\n";
            return;
        }

        Car car(startX, startY);
        int prevDist = car.minDotsToGoal(map);
        bool done = false;
        double episodeReward = 0.0;

        // Determine if this is a logging episode
        bool logThisEpisode = (episode % 10 == 0); // Log every 10th episode (0, 10, 20, ...)

        for (int step = 0; step < maxStepsPerEpisode && !done; ++step) {
            // Log car position for the designated episodes
            if (logThisEpisode) {
                movementFile << car.getX() << " " << car.getY() << "\n";
            }

            // Build current state
            State currentState(car.getX(), car.getY(), car.getDirection(), car.getVelocity());

            // Agent selects an action
            int action = agent.select_action(currentState);

            // Apply action
            switch (action) {
                case 0: car.setDirection(static_cast<Direction>(UP)); break;
                case 1: car.setDirection(static_cast<Direction>(RIGHT)); break;
                case 2: car.setDirection(static_cast<Direction>(DOWN)); break;
                case 3: car.setDirection(static_cast<Direction>(LEFT)); break;
                case 4: car.accelerate(); break;
                case 5: car.decelerate(); break;
                default: break;
            }

            // Step environment
            UpdateStatus status = car.update(map);
            int newDist = car.minDotsToGoal(map);

            // Reward: shaped by distance + time penalty
            double reward = (prevDist == -1 || newDist == -1) ? 0.0 : static_cast<double>(prevDist - newDist);
            reward -= 0.1; // ✅ Time penalty
            prevDist = newDist;

            if (status == UpdateStatus::GOAL) {
                reward += 100.0;
                done = true;
                std::cout << "✅ Goal reached in episode " << episode << "\n";
            } else if (status == UpdateStatus::COLLISION) {
                reward = -100.0;
                episodeReward = 0.0; // ✅ Total points reset to 0 on crash
                done = true;
                std::cout << "💥 Collision in episode " << episode << "\n";
            }

            // Get next state
            State nextState(car.getX(), car.getY(), car.getDirection(), car.getVelocity());

            // Store and learn
            agent.store_transition(currentState, action, reward, nextState, done);
            agent.experience_replay(32);

            episodeReward += reward;
        }

        agent.update_target_network(); // Optional: every N episodes

        std::cout << "📘 Episode " << episode
                  << " | Total reward: " << episodeReward
                  << " | Epsilon: " << agent.epsilon << "\n";
    }

    movementFile.close();
    std::cout << "✅ Movement log saved to training_movements.txt\n";
}