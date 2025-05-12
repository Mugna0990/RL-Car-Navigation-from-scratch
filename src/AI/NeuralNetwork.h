#include <vector>
#include <string>
#include "Layer.h"
#include "Optimizer.h"
#include "State.h"

class NeuralNetwork {
    public:
        NeuralNetwork(std::vector<int> layerSizes, double eps, double lr, std::string p);
        std::vector<double> forward(const std::vector<double>& input) ;
        void backward(const std::vector<double>& expected_output);
        void trainStep(const std::vector<double>& input, const std::vector<double>& expected_output);
        void learn(const std::vector<std::tuple<ReplayRecord, double>>& batch); 
        void save(const std::string& directory_path);
        void load(const std::string& directory_path);
    private:
        std::vector<Layer> layers;
        double learnRate;
        double epsilon;
        std::string path;
};
    