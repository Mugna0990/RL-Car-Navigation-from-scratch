#pragma once
#include "Map.h"
#include "Utils.h"

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

private:
    int x, y;
    int velocity;
    Direction dir;

    bool checkCollision(const Map& map, int nextX, int nextY);
};
