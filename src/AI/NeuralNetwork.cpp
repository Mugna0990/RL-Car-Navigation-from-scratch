#include "NeuralNetwork.h"


NeuralNetwork::NeuralNetwork(std::vector<int> layerSizes, double eps, double lr, std::string p) {

    learnRate = lr;
    epsilon = eps;
    path = p;

    for(size_t i = 0; i < layerSizes.size() - 1; ++i) {
        int layer_type = (i == layerSizes.size() - 2) ? 1 : 0;
        layers.push_back(Layer(layerSizes[i], layerSizes[i+1], i, layer_type));
    }
    
    load(path);

}

NeuralNetwork::NeuralNetwork(const NeuralNetwork& other)
    : learnRate(other.learnRate),
      epsilon(other.epsilon),
      path(other.path)
{
    layers.clear();
    for (const Layer& layer : other.layers) {
        layers.push_back(layer); // calls Layer copy constructor
    }
}

NeuralNetwork& NeuralNetwork::operator=(const NeuralNetwork& other) {
    if (this != &other) {
        learnRate = other.learnRate;
        epsilon = other.epsilon;
        path = other.path;

        layers.clear();
        for (const Layer& layer : other.layers) {
            layers.push_back(layer);
        }
    }
    return *this;
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

std::vector<double> NeuralNetwork::forward(const std::vector<double>& input) {
    std::vector<double> current_output = input; 
    for (auto& layer : layers) {
        current_output = layer.forward(current_output); // Pass output of current layer as input to the next
    }
    return current_output; 
}

void NeuralNetwork::learn(const std::vector<std::tuple<ReplayRecord, double>>& batch) {

    for(auto& layer : layers) {
        layer.reset();
    }

    for(const auto& experience_tuple : batch) {
        const auto& record = std::get<0>(experience_tuple);
        double target_q_value = std::get<1>(experience_tuple);
        const auto& input_state = record.state;
        Action action_taken = record.action;

        std::vector<double> predicted_q_values = NeuralNetwork::forward(input_state.toVector());

        size_t action_idx = static_cast<size_t>(action_taken);
        if (action_idx >= predicted_q_values.size()) {
             std::cerr << "Error: Action index out of bounds!  size:  " << predicted_q_values.size() << "idx: " << action_idx<< std::endl;
             continue;
        }

        double predicted_q_for_action = predicted_q_values[action_idx];
        double lossDerivative = predicted_q_for_action - target_q_value;

        std::vector<double> error_signals_to_propagate = layers.back().outputLayerNodeValues(lossDerivative, action_idx);

        for (int layerIdx = static_cast<int>(layers.size()) - 2; layerIdx >= 0; --layerIdx) {
             error_signals_to_propagate = layers[layerIdx].hiddenLayerNodeValues(layers[layerIdx+1], error_signals_to_propagate);
        }
    } 

    //Apply the accumulated gradients 
    for(auto& layer : layers) {
        layer.update(); 
    }
}
