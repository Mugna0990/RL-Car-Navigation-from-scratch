#pragma once

#include <vector>
#include <string>
#include <random>
#include "Optimizer.h"

class Layer {
public:
    // Constructor
    Layer(int n_inputs, int n_outputs, int layer_idx, bool type);

    // Forward pass
    std::vector<double> forward(const std::vector<double>& input);

    // Backward pass (returns gradients to propagate to previous layer)
    std::vector<double> backward(const std::vector<double>& dLoss_dOutput);

    // Apply gradients using the optimizer
    void update();

    // Save and load layer weights, biases, and optimizer state
    void save(const std::string& path) const;
    void load(const std::string& path);

    // Set input for backpropagation
    void setInput(const std::vector<double>& input);

private:
    int n_inputs;
    int n_outputs;
    int layer_idx;
    bool type;

    // Layer parameters
    std::vector<std::vector<double>> weights;
    std::vector<double> biases;

    // Gradients
    std::vector<std::vector<double>> grad_weights;
    std::vector<double> grad_biases;

    std::vector<double> input;
    std::vector<double> output;
    std::vector<double> node_values;

    // Optimizer
    AdamOptimizer optimizer;
};
