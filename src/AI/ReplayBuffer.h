#pragma once
#include <vector>
#include <deque>
#include <random>
#include "State.h"

struct Transition {
    State state;
    int action;
    double reward;
    State nextState;
    bool done;
};

class ReplayBuffer {
public:
    ReplayBuffer(size_t capacity)
        : capacity(capacity), rng(rd()) {}

    void add(const Transition& t) {
        if (buffer.size() >= capacity) {
            buffer.pop_front();
        }
        buffer.push_back(t);
    }

    std::vector<Transition> sample(size_t batchSize) {
        std::vector<Transition> batch;
        std::uniform_int_distribution<size_t> dist(0, buffer.size() - 1);

        for (size_t i = 0; i < batchSize; ++i) {
            size_t index = dist(rng);
            batch.push_back(buffer[index]);
        }
        return batch;
    }

    size_t size() const {
        return buffer.size();
    }

private:
    std::deque<Transition> buffer;
    size_t capacity;
    std::random_device rd;
    std::mt19937 rng;
};
