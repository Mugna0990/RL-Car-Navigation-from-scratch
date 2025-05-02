#pragma once
#include <vector>

class State {
public:
    int x;
    int y;
    int direction; // 0–3 (up, right, down, left)
    int speed;     // 1–5

    State(int x, int y, int dir, int spd)
        : x(x), y(y), direction(dir), speed(spd) {}

    // Normalize state to [0, 1] for neural net input
    std::vector<double> toVector(int maxX, int maxY) const {
        return {
            static_cast<double>(x) / maxX,
            static_cast<double>(y) / maxY,
            static_cast<double>(direction) / 3.0,
            static_cast<double>(speed - 1) / 4.0 
        };
    }
};
