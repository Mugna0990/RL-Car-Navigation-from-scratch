#include "Map.h"
#include <fstream>
#include <iostream>

Map::Map() {}

bool Map::loadFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile) return false;

    grid.clear();
    std::string line;
    while (std::getline(inFile, line)) {
        grid.push_back(line);
    }

    return true;
}

char Map::getTile(int x, int y) const {
    if (y < 0 || y >= static_cast<int>(grid.size()) ||
        x < 0 || x >= static_cast<int>(grid[y].size())) {
        return '#';  // Treat out-of-bounds as wall
    }
    return grid[y][x];
}

void Map::setTile(int x, int y, char tile) {
    if (y >= 0 && y < static_cast<int>(grid.size()) && x >= 0 && x < static_cast<int>(grid[y].size())) {
        grid[y][x] = tile;
    }
}

void Map::display() const {
    for (const auto& row : grid) {
        std::cout << row << "\n";
    }
}

void Map::display(int xC, int yC) const {
    for (int y = 0; y < getHeight(); y++) {
        for (int x = 0; x < getWidth(); x++) {
            if (y == yC && x == xC) {
                std::cout << "C";
            }
            else{
                std::cout << grid[y][x];
            }
        }
        std::cout<<"\n";
    }
}

bool Map::findStart(int& startX, int& startY) const {
    for (int y = 0; y < getHeight(); y++) {
        for (int x = 0; x < getWidth(); x++) {
            if (grid[y][x] == 'S') {
                startX = x;
                startY = y;
                return true;
            }
        }
    }
    return false; 
}
