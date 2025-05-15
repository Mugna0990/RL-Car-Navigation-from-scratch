#include "Layer.h"

Layer::Layer(int n_in, int n_out, int idx, bool isOut) {
    n_inputs  = n_in;
    n_outputs = n_out;
    this->isOut = isOut;
    layer_idx = idx;
    output.resize(n_outputs);
    input.resize(n_inputs);

    biases.resize(n_outputs);
    grad_biases.resize(n_outputs);
    std::fill(biases.begin(), biases.end(), 0); 

    node_values.resize(n_outputs);

    optimizer = AdamOptimizer(n_inputs, n_outputs, 0.9, 0.999, 1e-8, 0.001, layer_idx);

    std::random_device rd;
    std::default_random_engine generator(rd());
    std::normal_distribution<double> distribution(0, sqrt(2/(double)n_inputs));
    weights.resize(n_inputs);
    grad_weights.resize(n_inputs);
    for(int i = 0; i < n_inputs; i++) {
        weights[i].resize(n_outputs);
        grad_weights[i].resize(n_outputs);
        for(int j = 0; j < n_outputs; j++) {
            weights[i][j] = distribution(generator);
        }
    }
}

Layer::Layer(const Layer& other)
    : weights(other.weights),          
      biases(other.biases),            
      grad_weights(other.grad_weights),
      grad_biases(other.grad_biases),  
      optimizer(other.optimizer),       
      n_inputs(other.n_inputs),        
      n_outputs(other.n_outputs),      
      layer_idx(other.layer_idx),      
      isOut(other.isOut),              
      input(other.input),              
      output(other.output),            
      node_values(other.node_values)   
{}

//  copy operator for target network
Layer& Layer::operator=(const Layer& other) {
    if (this != &other) {
        weights = other.weights;
        biases = other.biases;
        grad_weights = other.grad_weights;
        grad_biases = other.grad_biases;
        optimizer = other.optimizer;

        n_inputs = other.n_inputs;
        n_outputs = other.n_outputs;
        layer_idx = other.layer_idx;
        isOut = other.isOut;

        input = other.input;
        output = other.output;
        node_values = other.node_values;
    }
    return *this;
}

std::vector<double> Layer::forward(const std::vector<double>& in) {
            
    input = in;
    std::fill(output.begin(), output.end(), 0.0);

    for(int i = 0; i < static_cast<int>(input.size()); i++) {
        for(int j = 0; j < static_cast<int>(output.size()); j++) {
            output[j] += input[i] * weights[i][j];   
        }
    }

    for(int i = 0; i < n_outputs; i++) {
        output[i] += biases[i];
    }

    if(!isOut) {
        for(double& n : output)  {
            if (n < 0) n = 0; 
        }
    }

    return output;
}

void Layer::save(const std::string& path) {
    std::ofstream outFile(path + "/layer" + std::to_string(layer_idx) + ".txt");

    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file for saving: " << path << "/layer" << layer_idx << ".txt" << std::endl;
        return;
    }

    // Save weights
    for (int i = 0; i < n_inputs; ++i) {
        for (int j = 0; j < n_outputs; ++j) {
            outFile << weights[i][j] << " ";
        }
        outFile << "\n";
    }

    // Save biases
    for (int j = 0; j < n_outputs; ++j) {
        outFile << biases[j] << " ";
    }
    outFile << "\n";

    outFile.close();
    std::cout << "Layer " << layer_idx << " saved successfully to " << path << "\n";

    optimizer.save(path);
}

void Layer::load(const std::string& path) {
    std::ifstream inFile(path + "/layer" + std::to_string(layer_idx) + ".txt");

    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open file for loading: " << path << "/layer" << layer_idx << ".txt" << std::endl;
        return;
    }

    std::string line;
    weights.resize(n_inputs);
    for (int i = 0; i < n_inputs; ++i) {
        weights[i].resize(n_outputs);
        if (!std::getline(inFile, line)) {
            std::cerr << "Error: Failed to read weights row " << i << std::endl;
            return;
        }

        std::istringstream iss(line);
        for (int j = 0; j < n_outputs; ++j) {
            if (!(iss >> weights[i][j])) {
                std::cerr << "Error: Failed to read weight [" << i << "][" << j << "]" << std::endl;
                return;
            }
        }
    }

    if (!std::getline(inFile, line)) {
        std::cerr << "Error: Missing biases line in file." << std::endl;
        return;
    }

    std::istringstream biasStream(line);
    for (int j = 0; j < n_outputs; ++j) {
        if (!(biasStream >> biases[j])) {
            std::cerr << "Error: Failed to read bias[" << j << "]" << std::endl;
            return;
        }
    }

    inFile.close();
    std::cout << "Successfully loaded layer " << layer_idx << " from file.\n";

    optimizer.load(path);
}




void Layer::reset() {
    for (auto& row : grad_weights) {
        std::fill(row.begin(), row.end(), 0.0);
    }
    std::fill(grad_biases.begin(), grad_biases.end(), 0.0);
}

std::vector<double> Layer::backward(const std::vector<double>& dLoss_dOutput) {
    std::vector<double> dLoss_dInput(n_inputs, 0.0);

    for (int i = 0; i < n_outputs; i++) {
        double delta = dLoss_dOutput[i];
        if (!isOut && output[i] <= 0) {
            delta = 0;
        }
        grad_biases[i] += delta;

        for (int j = 0; j < n_inputs; j++) {
            grad_weights[j][i] += input[j] * delta;
            dLoss_dInput[j] += weights[j][i] * delta;
        }
    }

    return dLoss_dInput;
}

void Layer::update() {
    optimizer.optimize(weights, biases, grad_weights, grad_biases);

    // Clear gradients after applying them
    for (auto& row : grad_weights) {
        std::fill(row.begin(), row.end(), 0.0);
    }
    std::fill(grad_biases.begin(), grad_biases.end(), 0.0);
}

void Layer::setInput(const std::vector<double>& in) {
    input = in;
}

std::vector<double>& Layer::outputLayerNodeValues(double lossDerivative, int action) {
    std::fill(node_values.begin(), node_values.end(), 0.0);
    if (action >= 0 && action < static_cast<int>(node_values.size())) {
        node_values[action] = lossDerivative;

        grad_biases[action] += node_values[action]; 
        
        for (int i = 0; i < n_inputs; ++i) {
            grad_weights[i][action] += input[i] * node_values[action];
        }


    } else {
        std::cerr << "Invalid action index in outputLayerNodeValues: " << action << std::endl;
    }
    return node_values;
}

std::vector<double>& Layer::hiddenLayerNodeValues(const Layer& nextLayer, const std::vector<double>& nextLayerNodeValues) {
    std::vector<double> activationDerivatives(output.size());
    for (size_t i = 0; i < output.size(); ++i) {
        activationDerivatives[i] = (output[i] > 0.0) ? 1.0 : 0.0; // ReLU derivative
    }

    for (int i = 0; i < n_outputs; ++i) {
        double sum = 0.0;
        for (int j = 0; j < nextLayer.n_outputs; ++j) {
            // Error propagated from the next layer multiplied by connection weight
            sum += nextLayer.weights[i][j] * nextLayerNodeValues[j];
        }
        node_values[i] = sum * activationDerivatives[i]; // Apply activation derivative
    }

    for (int i = 0; i < n_outputs; ++i) { 
        grad_biases[i] += node_values[i]; // Accumulate bias gradient

        for (int j = 0; j < n_inputs; ++j) { 
            // Accumulate weight gradient
            grad_weights[j][i] += input[j] * node_values[i];
        }
    }

    return node_values;
}



