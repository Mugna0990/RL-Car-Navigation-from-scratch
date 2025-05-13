#pragma once

#include <vector>
#include <cmath>   
#include <iostream> 
#include <fstream> 
#include <sstream> 
#include <numeric> 


class AdamOptimizer {
public:

    double alpha; // Learning rate 
    double beta_one; // Exponential decay rate for the first moment estimates
    double beta_two; // Exponential decay rate for the second moment estimates
    double epsilon_stable; // Small constant for numerical stability

    int training_steps; // Number of optimization steps taken
    double beta_one_power; // beta_one raised to the power of training_steps
    double beta_two_power; // beta_two raised to the power of training_steps

    std::vector<std::vector<double>> weight_first_moment;  // First moment for weights
    std::vector<std::vector<double>> weight_second_moment; // Second moment for weights
    std::vector<double> bias_first_moment;   // First moment for biases
    std::vector<double> bias_second_moment;  // Second moment for biases

    int input_features;
    int output_features;
    int layer_identifier;

    // Default constructor
    AdamOptimizer();

    // Parameterized constructor
    AdamOptimizer(int num_input, int num_output, double b1, double b2, double eps, double learning_r, int layer_id);

    AdamOptimizer(const AdamOptimizer& other) = default;
    AdamOptimizer& operator=(const AdamOptimizer& other) = default;

    // Perform one optimization step
    void optimize(std::vector<std::vector<double>>& layer_weights, std::vector<double>& layer_biases,
                         const std::vector<std::vector<double>>& weight_gradients, const std::vector<double>& bias_gradients);

    void save(const std::string& file_path) const;

    void load(const std::string& file_path);
};
