#pragma once
#include <vector>
#include <iostream>
#include "../Utils.h"

enum class Action {
    ACCELERATE = 0,
    DECELERATE = 1,
    LEFT  = 2,
    RIGHT = 3,
    UP = 4,
    DOWN = 5
};

class State {
public:
    int x; // position X
    int y; // position Y
    Direction direction; // 0-3
    int speed; // 1–5
    int distU; // up distance car-wall
    int distR; // right distance car-wall
    int distD; // down distance car-wall
    int distL; // left distance car-wall
    int distG; // distance from goal

    State(int x, int y, Direction dir, int spd,
        int distU = 0, int distR = 0, int distD = 0, int distL = 0, int distG = 0)
      : x(x), y(y), direction(dir), speed(spd),
        distU(distU), distR(distR), distD(distD), distL(distL), distG(distG) {}

      std::vector<double> toVector() const {
        double normX = static_cast<double>(x) / MAP_WIDTH;
        double normY = static_cast<double>(y) / MAP_HEIGHT;
        double normDirection = static_cast<double>(static_cast<int>(direction)) / 3.0;
        double normSpeed = static_cast<double>(speed - 1) / 4.0;
    
       // Normalize distances to range [0, 1]
        double normDistU = std::min(1.00, static_cast<double>(distU) / 15);
        double normDistR = std::min(1.00, static_cast<double>(distR) / 15);
        double normDistD = std::min(1.00, static_cast<double>(distD) / 15);
        double normDistL = std::min(1.00, static_cast<double>(distL) / 15);
        double normDistG = std::min(1.00, static_cast<double>(distG) / std::max(MAP_HEIGHT, MAP_WIDTH));
    
        return {normX, normY, normDirection, normSpeed, normDistU, normDistR, normDistD, normDistL, normDistG};
    }
    


    static State fromVector(const std::vector<double>& vec, int maxX, int maxY) {
        if (vec.size() != 4) throw std::invalid_argument("State vector must have 4 elements.");
        int x = static_cast<int>(vec[0] * maxX);
        int y = static_cast<int>(vec[1] * maxY);
        Direction dir = static_cast<Direction>(static_cast<int>(vec[2] * 3.0 + 0.5)); // round to nearest
        int speed = static_cast<int>(vec[3] * 4.0 + 1.0);
        return State(x, y, dir, speed);
    }

    void print() const {
        std::cout << "State(x=" << x << ", y=" << y
                  << ", direction=" << static_cast<int>(direction)
                  << ", speed=" << speed << ")\n";
    }

    bool isValid(int maxX, int maxY) const {
        return (x >= 0 && x < maxX &&
                y >= 0 && y < maxY &&
                static_cast<int>(direction) >= 0 && static_cast<int>(direction) <= 3 &&
                speed >= 1 && speed <= 5);
    }
};

struct ReplayRecord {
    State state;
    Action action;
    double reward;
    State next_state;
    bool done;

    ReplayRecord(State s, Action a, double r, State ns, bool d)
        : state(s), action(a), reward(r), next_state(ns), done(d) {}
};