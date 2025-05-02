#pragma once

enum class Action {
    ACCELERATE = 0,
    DECELERATE = 1,
    TURN_LEFT  = 2,
    TURN_RIGHT = 3,
    NOOP       = 4
};

inline int toIndex(Action a) {
    return static_cast<int>(a);
}

inline Action fromIndex(int i) {
    return static_cast<Action>(i);
}

const int ACTION_COUNT = 5;
