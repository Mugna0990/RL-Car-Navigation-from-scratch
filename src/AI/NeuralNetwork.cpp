#include "NeuralNetwork.h"


NeuralNetwork::NeuralNetwork(std::vector<int> layerSizes, double eps, double lr, std::string p) {

    learnRate = lr;
    epsilon = eps;
    path = p;

    for(size_t i = 0; i < layerSizes.size() - 1; ++i) {
        int layer_type = (i == layerSizes.size() - 2) ? 1 : 0;
        layers.push_back(Layer(layerSizes[i], layerSizes[i+1], layer_type, i));
    }
    
    load(path);

}

void NeuralNetwork::save(const std::string& path){

    std::cout << "Saving network to directory: " << path << std::endl;
    for (size_t i = 0; i < layers.size(); ++i) {
        layers[i].save(path);
        if(typeid(layers[i].optimizer) == typeid(AdamOptimizer)) {
            layers[i].optimizer.save(path);
        }
    }
    std::cout << "Network saved." << std::endl;
}

void NeuralNetwork::load(const std::string& path) {

    std::cout << "Loading network from directory: " << path << std::endl;
    for (size_t i = 0; i < layers.size(); ++i) {
        layers[i].load(path);
        if(typeid(layers[i].optimizer) == typeid(AdamOptimizer)) {
            layers[i].optimizer.load(path);
        }
    }
    std::cout << "Network loaded." << std::endl;
}

void NeuralNetwork::learn(const std::vector<std::tuple<ReplayRecord, double>>& batch) {
    // 1. Reset gradients for the batch (Correct, now that Layer::reset() is fixed)
    for(auto& layer : layers) {
        layer.reset();
    }

    double totalLoss = 0.0;

    // Process each experience in the batch
    for(const auto& experience_tuple : batch) {
        const auto& record = std::get<0>(experience_tuple);
        double target_q_value = std::get<1>(experience_tuple);
        const auto& input_state = record.state;
        Action action_taken = record.action;

        // 2. Forward pass to get current Q-value predictions for the state
        std::vector<double> predicted_q_values = forward(input_state.toVector());

        size_t action_idx = static_cast<size_t>(action_taken);
        if (action_idx >= predicted_q_values.size()) {
             std::cerr << "Error: Action index out of bounds!" << std::endl;
             continue;
        }

        double predicted_q_for_action = predicted_q_values[action_idx];
        double lossDerivative = predicted_q_for_action - target_q_value;

        // Accumulate squared error for tracking total loss
        totalLoss += 0.5 * lossDerivative * lossDerivative;

        std::vector<double> error_signals_to_propagate = layers.back().outputLayerNodeValues(lossDerivative, action_idx);

        // Propagate the error backwards through the hidden layers
        for (int layerIdx = layers.size() - 2; layerIdx >= 0; --layerIdx) {
             error_signals_to_propagate = layers[layerIdx].hiddenLayerNodeValues(layers[layerIdx+1], error_signals_to_propagate);
        }
    } // End of batch loop

    // 5. Apply the accumulated gradients using the optimizer (ONLY ONCE PER BATCH)
    for(auto& layer : layers) {
        layer.update(); // This call is correct here. It applies optimizer and clears gradients.
    }

    // 6. Log the average loss
    if (!batch.empty()) {
        double avgLoss = totalLoss / batch.size();
        std::cout << "\nAvg loss: " << avgLoss << "\n";

        std::ofstream outFile("training_loss.txt", std::ios::app);
        if (outFile.is_open()) {
            outFile << avgLoss << std::endl;
        } else {
            std::cerr << "Warning: Unable to open training_loss.txt for logging." << std::endl;
        }
    } else {
         std::cout << "\nAttempted to learn with an empty batch.\n";
    }
}
