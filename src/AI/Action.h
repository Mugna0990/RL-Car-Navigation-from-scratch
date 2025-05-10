#pragma once
#include <vector>

enum class Action {
    ACCELERATE = 0,
    DECELERATE = 1,
    LEFT  = 2,
    RIGHT = 3,
    UP = 4,
    DOWN = 5
};

inline int toIndex(Action a) {
    return static_cast<int>(a);
}

inline Action fromIndex(int i) {
    return static_cast<Action>(i);
}

const int ACTION_COUNT = 6;

using State = std::vector<double>;

struct ReplayRecord {
    State state;
    Action action; // Using the Action enum from Action.h
    double reward;
    State next_state;
    bool done;

    // Constructor
    ReplayRecord(State s, Action a, double r, State ns, bool d)
        : state(s), action(a), reward(r), next_state(ns), done(d) {}
};