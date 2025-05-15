#pragma once
#include <vector>
#include <SFML/Graphics.hpp> 
#include <string>
#include <fstream>
#include <iostream>
#include <optional> 
#include <variant>  
#include "../Utils.h"

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
    void printLegend(); 

    unsigned int width, height, tileSize;
    sf::RenderWindow window;
    std::vector<std::vector<TileType>> grid;
    TileType currentDrawType = ROAD;
    bool isDrawing = false;

    bool startPlaced = false;
    int startX = -1, startY = -1;
    bool goalPlaced = false;
    int goalX = -1, goalY = -1;

    std::vector<std::string> legendTexts;
};