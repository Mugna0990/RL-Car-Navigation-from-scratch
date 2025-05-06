#include <vector>
#include <string>
#include "Layer.h"
#include "Optimizer.h"

class NeuralNetwork {
    public:
        NeuralNetwork(std::vector<int> layerSizes, double eps, double lr, std::string p);
        void save(const std::string& directory_path);
        void load(const std::string& directory_path);
    private:
        std::vector<Layer> layers;
        double learnRate;
        double epsilon;
        std::string path;
};
    