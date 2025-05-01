#pragma once
#include <vector>
#include <SFML/Graphics.hpp> // Include for sf::RenderWindow and sf::Color/Vector2u/Vector2i/RectangleShape/Event etc.
#include <string>
#include "Utils.h" // Assuming TileType is defined here

class MapEditor {
public:
    MapEditor(unsigned int width, unsigned int height, unsigned int tileSize);
    void run();

private:
    void processEvents();
    void update();
    void render();
    void saveMap(const std::string& filename);
    void setTile(int x, int y, TileType type);
    void printLegend(); // Declaration for the printLegend function

    unsigned int width, height, tileSize;
    sf::RenderWindow window;
    std::vector<std::vector<TileType>> grid;
    TileType currentDrawType = ROAD;
    bool isDrawing = false;

    // Variables to track start and goal positions and if they've been placed
    bool startPlaced = false;
    int startX = -1, startY = -1;
    bool goalPlaced = false;
    int goalX = -1, goalY = -1;

    // Legend texts stored as strings (as used in the .cpp)
    std::vector<std::string> legendTexts;
};