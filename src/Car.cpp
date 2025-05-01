#include "Car.h"
#include "Utils.h"

Car::Car(int startX, int startY) : x(startX), y(startY), velocity(0), dir(UP) {}

void Car::accelerate() {
    velocity = std::min(velocity + 1, 2); // Max speed 2
}

void Car::turnLeft() {
    dir = static_cast<Direction>((dir + 3) % 4);
}

void Car::turnRight() {
    dir = static_cast<Direction>((dir + 1) % 4);
}

bool Car::checkCollision(const Map& map, int nx, int ny) {
    return map.getTile(nx, ny) == WALL;
}

UpdateStatus Car::update(const Map& map) {
    int nx = x, ny = y;
    switch (dir) {
        case UP:    ny -= velocity; break;
        case RIGHT: nx += velocity; break;
        case DOWN:  ny += velocity; break;
        case LEFT:  nx -= velocity; break;
    }

    if (checkCollision(map, nx, ny)) {
        velocity = 0;
        return UpdateStatus::COLLISION;
    }

    if (map.getTile(nx, ny) == 'G') {
        x = nx;
        y = ny;
        return UpdateStatus::GOAL;
    }

    x = nx;
    y = ny;
    return UpdateStatus::OK;
}

int Car::getX() const { return x; }
int Car::getY() const { return y; }

void Car::setDirection(Direction newDir) {
    dir = newDir;
}

void Car::decelerate() {
    velocity = std::max(velocity - 1, 0);
}

char Car::getDirectionChar() const {
    switch (dir) {
        case UP: return '^';
        case RIGHT: return '>';
        case DOWN: return 'v';
        case LEFT: return '<';
    }
    return '?';
}
