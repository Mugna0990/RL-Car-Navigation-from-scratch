#include "NeuralNetwork.h"


NeuralNetwork::NeuralNetwork(std::vector<int> layerSizes, double eps, double lr, std::string p) {

    learnRate = lr;
    epsilon = eps;
    path = p;

    for(int i = 1; i < layerSizes.size()-1; i++) {
        if(i == layerSizes.size()-1){
            layers.push_back(Layer(layerSizes[i-1], layerSizes[i], 1, i-1));
        } else {
            layers.push_back(Layer(layerSizes[i-1], layerSizes[i], 0, i-1));
        }
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
    // 1. Reset gradients for the batch
    for(auto& layer : layers) {
        layer.reset();
    }

    double totalLoss = 0.0; // Initialize total loss for the batch

    // Process each experience in the batch
    for(const auto& experience_tuple : batch) {
        // Extract data from the tuple
        const auto& record = std::get<0>(experience_tuple);
        double target_q_value = std::get<1>(experience_tuple);

       

        const auto& input_state = record.state; // Get the state from the replay record
        Action action_taken = record.action;   // Get the action taken in this state

        // 2. Forward pass to get current Q-value predictions for the state
        std::vector<double> predicted_q_values = forward(input_state); // Assuming State has a member 'data' that is a vector<double>

   
        size_t action_idx = static_cast<size_t>(action_taken); // Cast Action to size_t index

        if (action_idx >= predicted_q_values.size()) {
             std::cerr << "Error: Action index out of bounds!" << std::endl;
             // Handle error appropriately - skip this sample or throw
             continue; // Skip this sample
        }

        double predicted_q_for_action = predicted_q_values[action_idx];
        double lossDerivative = predicted_q_for_action - target_q_value;

        // Accumulate squared error for tracking total loss
        totalLoss += 0.5 * lossDerivative * lossDerivative; // Standard MSE / 2

        // 4. Backward pass to calculate gradients
        std::vector<double> nodeValues = layers.back().outputLayerNodeValues(lossDerivative, action_idx);
        layers.back().update(); // Accumulate gradients for the output layer

        // Propagate the error backwards through the hidden layers
        for (int layerIdx = layers.size() - 2; layerIdx >= 0; --layerIdx) {
             nodeValues = layers[layerIdx].hiddenLayerNodeValues(layers[layerIdx+1], nodeValues);

            layers[layerIdx].update();
        }
    } // End of batch loop

    // 5. Apply the accumulated gradients using the optimizer
    for(auto& layer : layers) {
        layer.update();
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
