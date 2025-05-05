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

    optimizer = AdamOptimizer(n_inputs, n_outputs, 0.9, 0.999, 1e-8, 0, layer_idx);

    //He weight initialization
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::normal_distribution<double> distribution(0, sqrt(2/(double)n_inputs));
    weights.resize(n_inputs);
    grad_weights.resize(n_inputs);
    for(int i = 0; i < n_inputs; i++) {
        weights[i].resize(n_outputs);
        grad_weights[i].resize(n_outputs);
        for(int j = 0; j < n_outputs; j++) {
            //Initialize random weights
            weights[i][j] = distribution(generator);
        }
    }
}

void Layer::resetGradients() {
    // Reset gradients for weights
    for (int i = 0; i < n_inputs; i++) {
        std::fill(grad_weights[i].begin(), grad_weights[i].end(), 0.0);
    }

    std::fill(grad_biases.begin(), grad_biases.end(), 0.0);
}

std::vector<double> Layer::forward(const std::vector<double>& in) {
            
    input = in;
    std::fill(output.begin(), output.end(), 0.01);

    for(int i = 0; i < input.size(); i++) {
        for(int j = 0; j < output.size(); j++) {
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
