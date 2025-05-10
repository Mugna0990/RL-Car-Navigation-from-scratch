#pragma once
#include <vector>
#include <string>

class Map {
public:
    Map();
    bool loadFromFile(const std::string& filename);
    char getTile(int x, int y) const;
    void setTile(int x, int y, char tile);
    void display() const;
    void display(int x, int y) const;

    bool find(char c, int& startX, int& startY) const;
    int getWidth() const { return grid.empty() ? 0 : grid[0].size(); }
    int getHeight() const { return grid.size(); }

private:
    std::vector<std::string> grid;
};
