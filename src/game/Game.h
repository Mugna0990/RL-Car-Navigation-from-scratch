#pragma once
#include "Map.h"
#include "Car.h"
#include "util.h"
#include "../UI/DisplayMovement.h"
#include <queue>
#include <fstream>
#include <string>

class Game {
public:
    Game();
    void run();
    void processInput(char input);
    Map track;

private:
    Car car;
    void render();
    bool gameOver;
    std::ofstream movementFile;
};
