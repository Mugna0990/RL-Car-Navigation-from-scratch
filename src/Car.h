#pragma once
#include "Map.h"
#include "Utils.h"
#include<iostream>
#include <fstream>

enum class UpdateStatus { OK, COLLISION, GOAL };

class Car {
public:
    Car(int startX, int startY);
    
    UpdateStatus update(const Map& map); 

    void accelerate();
    void decelerate();
    void turnLeft();
    void turnRight();
    void reset();
    void setDirection(Direction newDir);

    int getX() const;
    int getY() const;
    char getDirectionChar() const;
    int minDotsToGoal(const Map& map);

private:
    int x, y;
    int velocity;
    Direction dir;

    bool checkCollision(const Map& map, int nextX, int nextY);
};
