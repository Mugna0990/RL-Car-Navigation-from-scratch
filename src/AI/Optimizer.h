#pragma once

#include <vector>
#include <cmath>   // For std::sqrt
#include <iostream> // For std::cerr, std::cout
#include <fstream> // For std::ifstream, std::ofstream
#include <sstream> // For std::istringstream
#include <numeric> // For std::fill (though resize with value is often used)

// Assuming types.h contains necessary type definitions if any.
// If not, you might not need this include.
// #include "types.h"

class AdamOptimizer {
public:
    // Hyperparameters
    double alpha; // Learning rate (often denoted as alpha in Adam literature)
    double beta_one; // Exponential decay rate for the first moment estimates
    double beta_two; // Exponential decay rate for the second moment estimates
    double epsilon_stable; // Small constant for numerical stability

    // Internal state variables
    int training_steps; // Number of optimization steps taken
    double beta_one_power; // beta_one raised to the power of training_steps
    double beta_two_power; // beta_two raised to the power of training_steps

    // Moment vectors
    std::vector<std::vector<double>> weight_first_moment;  // First moment for weights
    std::vector<std::vector<double>> weight_second_moment; // Second moment for weights
    std::vector<double> bias_first_moment;   // First moment for biases
    std::vector<double> bias_second_moment;  // Second moment for biases

    // Layer dimensions and identifier
    int input_features;
    int output_features;
    int layer_identifier;

    // Default constructor
    AdamOptimizer();

    // Parameterized constructor
    AdamOptimizer(int num_input, int num_output, double b1, double b2, double eps, double learning_r, int layer_id);

    // Perform one optimization step
    // Takes references to the layer's weights and biases to update them in place.
    // Takes constant references to the gradients calculated during the backward pass.
    void optimize(std::vector<std::vector<double>>& layer_weights, std::vector<double>& layer_biases,
                         const std::vector<std::vector<double>>& weight_gradients, const std::vector<double>& bias_gradients);

    // Save the optimizer's state to a file
    void save(const std::string& file_path) const;

    // Load the optimizer's state from a file
    void load(const std::string& file_path);
};
