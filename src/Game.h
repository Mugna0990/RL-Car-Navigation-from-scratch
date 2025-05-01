#pragma once
#include "Map.h"
#include "Car.h"

class Game {
public:
    Game();
    void run();

private:
    Map track;
    Car car;
    void processInput(char input);
    void render();
    bool gameOver;
};
