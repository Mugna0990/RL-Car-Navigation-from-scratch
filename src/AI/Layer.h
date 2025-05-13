#pragma once

#include <vector>
#include <string>
#include <random>
#include "Optimizer.h"

class Layer {
public:
    // Constructors
    Layer(int n_inputs, int n_outputs, int layer_idx, bool is_output);
    Layer(const Layer& other);                      // Copy constructor
    Layer& operator=(const Layer& other);           // Copy assignment
    Layer clone() const;                            // Clone method

    // Forward and backward passes
    std::vector<double> forward(const std::vector<double>& input);
    std::vector<double> backward(const std::vector<double>& dLoss_dOutput);

    // Backpropagation helper methods
    void setInput(const std::vector<double>& input);
    std::vector<double>& outputLayerNodeValues(double lossDerivative, int action);
    std::vector<double>& hiddenLayerNodeValues(const Layer& nextLayer, const std::vector<double>& nextLayerNodeValues);

    // Update methods
    void update();
    void reset();

    // Save/load
    void save(const std::string& path);
    void load(const std::string& path);

    // Parameters and gradients
    std::vector<std::vector<double>> weights;
    std::vector<double> biases;
    std::vector<std::vector<double>> grad_weights;
    std::vector<double> grad_biases;

    // Optimizer
    AdamOptimizer optimizer;

private:
    int n_inputs;
    int n_outputs;
    int layer_idx;
    bool isOut;

    std::vector<double> input;
    std::vector<double> output;
    std::vector<double> node_values;
};
