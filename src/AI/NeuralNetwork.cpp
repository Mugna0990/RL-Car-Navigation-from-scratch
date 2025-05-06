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