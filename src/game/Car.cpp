#include "Car.h"
#include "../Utils.h"

Car::Car(int startX, int startY) : x(startX), y(startY), velocity(1), dir(UP) {}

void Car::accelerate() {
    velocity = std::min(velocity + 1, 5); // Max speed 5
}

void Car::turnLeft() {
    dir = static_cast<Direction>((dir + 3) % 4);
}

void Car::turnRight() {
    dir = static_cast<Direction>((dir + 1) % 4);
}

bool Car::checkCollision(const Map& map, int nextX, int nextY) {
    char tile = map.getTile(nextX, nextY);
    if (tile == '#') {
        return true;  // Wall collision
    }
    return false;
}

int Car::getVelocity() {
    return this->velocity;
}

Direction Car::getDirection() {
    return this->dir;
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
        std::cout<<"collision detected\n";
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

int Car::minDotsToGoal(const Map& map) {
    const int rows = MAP_HEIGHT, cols = MAP_WIDTH;
    int gX = -1, gY = -1;

    // Try to find 'G'
    map.find('G', gX, gY);
    std::pair<int, int> goal = {gY, gX};  // row = y, col = x
    std::queue<std::tuple<int, int, int>> q;  // row, col, dot count
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    q.push({y, x, 0});
    visited[y][x] = true;

    int dr[] = {-1, 1, 0, 0};  // N, S
    int dc[] = {0, 0, -1, 1};  // W, E

    while (!q.empty()) {
        auto [r, c, dots] = q.front(); q.pop();

        if (r == goal.first && c == goal.second) {
            std::cout << "Reached goal at (" << r << "," << c << ") with " << dots << " dots.\n";
            return dots;
        }

        for (int d = 0; d < 4; ++d) {
            int nr = r + dr[d];
            int nc = c + dc[d];

            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && !visited[nr][nc]) {
                char tile = map.getTile(nc, nr);  // Remember: x = col, y = row
                if (tile == '.' || tile == 'G') {
                    visited[nr][nc] = true;
                    int newDots = dots + (tile == '.' ? 1 : 0);
                    q.push({nr, nc, newDots});
                }
            }
        }
    }

    return -1;
}

