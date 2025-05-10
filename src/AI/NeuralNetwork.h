#include <vector>
#include <string>
#include "Layer.h"
#include "Optimizer.h"
#include "Action.h"

class NeuralNetwork {
    public:
        NeuralNetwork(std::vector<int> layerSizes, double eps, double lr, std::string p);
        std::vector<double> NeuralNetwork::forward(const std::vector<double>& input) ;
        void NeuralNetwork::backward(const std::vector<double>& expected_output);
        void NeuralNetwork::trainStep(const std::vector<double>& input, const std::vector<double>& expected_output);
        void NeuralNetwork::learn(const std::vector<std::tuple<ReplayRecord, double>>& batch); 
        void save(const std::string& directory_path);
        void load(const std::string& directory_path);
    private:
        std::vector<Layer> layers;
        double learnRate;
        double epsilon;
        std::string path;
};
    