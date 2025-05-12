#include "Optimizer.h"

// Default constructor implementation
AdamOptimizer::AdamOptimizer()
    : alpha(0.001), beta_one(0.9), beta_two(0.999), epsilon_stable(1e-8),
      training_steps(0), beta_one_power(1.0), beta_two_power(1.0),
      input_features(0), output_features(0), layer_identifier(-1)
{
    // Moment vectors will be resized later when dimensions are known
}

// Parameterized constructor implementation
AdamOptimizer::AdamOptimizer(int num_input, int num_output, double b1, double b2, double eps, double learning_r, int layer_id)
    : alpha(learning_r), beta_one(b1), beta_two(b2), epsilon_stable(eps),
      training_steps(0), beta_one_power(1.0), beta_two_power(1.0),
      input_features(num_input), output_features(num_output), layer_identifier(layer_id)
{
    weight_first_moment.assign(input_features, std::vector<double>(output_features, 0.0));
    weight_second_moment.assign(input_features, std::vector<double>(output_features, 0.0));
    bias_first_moment.assign(output_features, 0.0);
    bias_second_moment.assign(output_features, 0.0);
}


// Perform one optimization step implementation
void AdamOptimizer::optimize(std::vector<std::vector<double>>& layer_weights, std::vector<double>& layer_biases,
                                    const std::vector<std::vector<double>>& weight_gradients, const std::vector<double>& bias_gradients)
{
    training_steps++; // Increment the timestep

    // Update bias correction terms
    beta_one_power *= beta_one;
    beta_two_power *= beta_two;

    // Update moment estimates and apply updates for weights
    for (int i = 0; i < input_features; ++i) {
        for (int j = 0; j < output_features; ++j) {
            // Update biased first and second moment estimates
            weight_first_moment[i][j] = weight_first_moment[i][j] * beta_one + (1 - beta_one) * weight_gradients[i][j];
            weight_second_moment[i][j] = weight_second_moment[i][j] * beta_two + (1 - beta_two) * weight_gradients[i][j] * weight_gradients[i][j];

            // Compute bias-corrected first and second moment estimates
            double corrected_m = weight_first_moment[i][j] / (1 - beta_one_power);
            double corrected_v = weight_second_moment[i][j] / (1 - beta_two_power);

            // Update weights
            layer_weights[i][j] -= alpha * corrected_m / (std::sqrt(corrected_v) + epsilon_stable);
        }
    }

    // Update moment estimates and apply updates for biases
    for (int i = 0; i < output_features; ++i) {
        // Update biased first and second moment estimates
        bias_first_moment[i] = bias_first_moment[i] * beta_one + (1 - beta_one) * bias_gradients[i];
        bias_second_moment[i] = bias_second_moment[i] * beta_two + (1 - beta_two) * bias_gradients[i] * bias_gradients[i];

        // Compute bias-corrected first and second moment estimates
        double corrected_m = bias_first_moment[i] / (1 - beta_one_power);
        double corrected_v = bias_second_moment[i] / (1 - beta_two_power);

        // Update biases
        layer_biases[i] -= alpha * corrected_m / (std::sqrt(corrected_v) + epsilon_stable);
    }
}

// Save the optimizer's state to a file implementation
void AdamOptimizer::save(const std::string& file_path) const {
    std::ofstream outFile(file_path + "/layer" + std::to_string(layer_identifier) + "_adam_state.txt");

    if (!outFile) {
        std::cerr << "Error: Could not open file " << file_path << "/layer" << layer_identifier << "_adam_state.txt for writing." << std::endl;
        return;
    }

    // Save hyperparameters and state
    outFile << alpha << " " << beta_one << " " << beta_two << " " << epsilon_stable << " "
            << training_steps << " " << beta_one_power << " " << beta_two_power << std::endl;

    // Save moment estimates for weights
    for (int i = 0; i < input_features; ++i) {
        for (int j = 0; j < output_features; ++j) {
            outFile << weight_first_moment[i][j] << (j == output_features - 1 ? "" : " ");
        }
        outFile << std::endl;
    }

    // Save moment estimates for weight_second_moment
    for (int i = 0; i < input_features; ++i) {
        for (int j = 0; j < output_features; ++j) {
            outFile << weight_second_moment[i][j] << (j == output_features - 1 ? "" : " ");
        }
        outFile << std::endl;
    }

    // Save moment estimates for biases
    for (int i = 0; i < output_features; ++i) {
        outFile << bias_first_moment[i] << (i == output_features - 1 ? "" : " ");
    }
    outFile << std::endl;

    // Save moment estimates for bias_second_moment
    for (int i = 0; i < output_features; ++i) {
        outFile << bias_second_moment[i] << (i == output_features - 1 ? "" : " ");
    }
    outFile << std::endl;

    outFile.close();
    // std::cout << "Adam state saved to file: " << file_path << "/layer" << layer_identifier << "_adam_state.txt" << std::endl;
}

// Load the optimizer's state from a file implementation
void AdamOptimizer::load(const std::string& file_path) {
    std::ifstream inFile(file_path + "/layer" + std::to_string(layer_identifier) + "_adam_state.txt");

    if (!inFile) {
        std::cerr << "Error: Could not open file " << file_path << "/layer" << layer_identifier << "_adam_state.txt for reading. Initializing new Adam configuration..." << std::endl;
        // Depending on desired behavior, you might want to initialize with default values here
        // or signal an error more forcefully. Returning does nothing in this case.
        return;
    }

    std::string line;
    std::istringstream iss;

    // Read hyperparameters and state
    if (std::getline(inFile, line)) {
        iss.str(line);
        if (!(iss >> alpha >> beta_one >> beta_two >> epsilon_stable >> training_steps >> beta_one_power >> beta_two_power)) {
            std::cerr << "Error: Failed to read Adam hyperparameters and state from file." << std::endl;
            inFile.close();
            return;
        }
    } else {
        std::cerr << "Error: File is empty or malformed (hyperparameter line missing)." << std::endl;
        inFile.close();
        return;
    }

    // Resize moment vectors based on loaded dimensions if not already set (e.g., using default constructor)
    // If using the parameterized constructor, dimensions are already set, so this might not be strictly needed,
    // but it adds robustness if loading into a default-constructed object.
    if (weight_first_moment.empty() || weight_first_moment.size() != static_cast<size_t>(input_features) ||
    (input_features > 0 && weight_first_moment[0].size() != static_cast<size_t>(output_features))){ 
         weight_first_moment.resize(input_features, std::vector<double>(output_features));
         weight_second_moment.resize(input_features, std::vector<double>(output_features));
         bias_first_moment.resize(output_features);
         bias_second_moment.resize(output_features);
    }


    // Load moment estimates for weights
    for (int i = 0; i < input_features; ++i) {
        if (std::getline(inFile, line)) {
            iss.clear(); // Clear previous state
            iss.str(line);
            for (int j = 0; j < output_features; ++j) {
                if (!(iss >> weight_first_moment[i][j])) {
                    std::cerr << "Error: Failed to read weight_first_moment value at position [" << i << "][" << j << "]." << std::endl;
                    inFile.close();
                    return;
                }
            }
        } else {
            std::cerr << "Error: Not enough lines to read weight_first_moment values." << std::endl;
            inFile.close();
            return;
        }
    }

    // Load moment estimates for weight_second_moment
    for (int i = 0; i < input_features; ++i) {
        if (std::getline(inFile, line)) {
            iss.clear(); // Clear previous state
            iss.str(line);
            for (int j = 0; j < output_features; ++j) {
                if (!(iss >> weight_second_moment[i][j])) {
                    std::cerr << "Error: Failed to read weight_second_moment value at position [" << i << "][" << j << "]." << std::endl;
                    inFile.close();
                    return;
                }
            }
        } else {
            std::cerr << "Error: Not enough lines to read weight_second_moment values." << std::endl;
            inFile.close();
            return;
        }
    }

    // Load moment estimates for biases
    if (std::getline(inFile, line)) {
        iss.clear(); // Clear previous state
        iss.str(line);
        for (int i = 0; i < output_features; ++i) {
            if (!(iss >> bias_first_moment[i])) {
                std::cerr << "Error: Failed to read bias_first_moment value at position [" << i << "]." << std::endl;
                inFile.close();
                return;
            }
        }
    } else {
        std::cerr << "Error: Failed to read bias_first_moment values for the biases." << std::endl;
        inFile.close();
        return;
    }

    // Load moment estimates for bias_second_moment
    if (std::getline(inFile, line)) {
        iss.clear(); // Clear previous state
        iss.str(line);
        for (int i = 0; i < output_features; ++i) {
            if (!(iss >> bias_second_moment[i])) {
                std::cerr << "Error: Failed to read bias_second_moment value at position [" << i << "]." << std::endl;
                inFile.close();
                return;
            }
        }
    } else {
        std::cerr << "Error: Failed to read bias_second_moment values for the biases." << std::endl;
        inFile.close();
        return;
    }

    inFile.close();
    // std::cout << "Adam state loaded from file: " << file_path << "/layer" << layer_identifier << "_adam_state.txt" << std::endl;
}