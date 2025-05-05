#pragma once
#include <vector>
#include <iostream>

class State {
public:
    int x; // X position on the road/grid
    int y; // Y position on the road/grid
    int direction; // 0: Up (North), 1: Right (East), 2: Down (South), 3: Left (West)
    int speed;     // 1–5

    // Constructor
    State(int x, int y, int dir, int spd)
        : x(x), y(y), direction(dir), speed(spd) {}

    // Converts the state to a normalized vector for neural network input
    std::vector<double> toVector(int maxX, int maxY) const {
        // Normalize position (assuming x ranges from 0 to maxX-1, y from 0 to maxY-1)
        double normX = static_cast<double>(x) / maxX;
        double normY = static_cast<double>(y) / maxY;

        // Normalize direction (0-3 mapped to 0-1)
        // Dividing by 3.0 maps 0->0, 1->1/3, 2->2/3, 3->1
        double normDirection = static_cast<double>(direction) / 3.0;

        // Normalize speed (1-5 mapped to 0-1)
        // Speed 1 maps to 0, Speed 5 maps to 1
        double normSpeed = static_cast<double>(speed - 1) / 4.0;

        return {normX, normY, normDirection, normSpeed};
    }

    // Prints the state to the console
    void print() const {
        std::cout << "State(x=" << x << ", y=" << y
                  << ", direction=" << direction
                  << ", speed=" << speed << ")\n";
    }

    // Checks if the state is within valid bounds
    bool isValid(int maxX, int maxY) const {
        return (x >= 0 && x < maxX &&
                y >= 0 && y < maxY &&
                direction >= 0 && direction <= 3 && // Updated to check for 0-3
                speed >= 1 && speed <= 5);
    }
};
