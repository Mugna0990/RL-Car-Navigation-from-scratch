#include "NeuralNetwork.h"
#include "ReplayBuffer.h"
#include <random>
#include <string>
#include <vector>
#include <algorithm>

class Agent {
public:
    NeuralNetwork q_network;
    NeuralNetwork target_q_network;
    ReplayBuffer replay_buffer;

    double epsilon;
    double epsilon_decay;
    double min_epsilon;
    double gamma;
    int action_space_size;

    int maxX;
    int maxY;

    std::mt19937 rng;

    Agent(std::vector<int> layerSizes,
          size_t buffer_capacity,
          double initial_epsilon,
          double decay,
          double min_eps,
          double discount_factor,
          int num_actions,
          std::string path)
        : q_network(layerSizes, initial_epsilon, 0.001, path + "/target_q_network"),
          target_q_network(layerSizes, initial_epsilon, 0.001, path + "/target_q_network"),
          replay_buffer(buffer_capacity),
          epsilon(initial_epsilon),
          epsilon_decay(decay),
          min_epsilon(min_eps),
          gamma(discount_factor),
          action_space_size(num_actions),
          rng(std::random_device{}())
    {
        update_target_network();
    }

    // Epsilon-greedy action selection
    int select_action(const State& current_state) {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        if (dist(rng) < epsilon) {
            std::uniform_int_distribution<int> action_dist(0, action_space_size - 1);
            return action_dist(rng);
        } else {
            std::vector<double> q_values = q_network.forward(current_state.toVector());
            return std::distance(q_values.begin(), std::max_element(q_values.begin(), q_values.end()));
        }
    }

    void store_transition(const State& s, int a, double r, const State& s_prime, bool done) {
        Transition t = { s, a, r, s_prime, done };
        replay_buffer.add(t);
    }

    // Learning from past experience
    void experience_replay(size_t batch_size) {
        if (replay_buffer.size() < batch_size) return;

        std::vector<Transition> batch = replay_buffer.sample(batch_size);
        std::vector<std::tuple<ReplayRecord, double>> training_batch;

        for (const auto& trans : batch) {
            std::vector<double> next_q_values = target_q_network.forward(trans.nextState.toVector());
            double max_next_q = trans.done ? 0.0 : *std::max_element(next_q_values.begin(), next_q_values.end());

            double target_q = trans.reward + gamma * max_next_q;

            ReplayRecord record(
                trans.state,
                static_cast<Action>(trans.action),
                trans.reward,
                trans.nextState,
                trans.done
            );

            training_batch.emplace_back(record, target_q);
        }

        if (!training_batch.empty()) {
            q_network.learn(training_batch);
        }
        
    }

    void update_target_network() {
        target_q_network = q_network;
    }

};
