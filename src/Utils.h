#pragma once

const int MAP_WIDTH = 250;
const int MAP_HEIGHT = 250;

enum Direction { UP, RIGHT, DOWN, LEFT };
enum TileType { EMPTY, WALL, ROAD, START, GOAL };
enum class UpdateStatus { OK, COLLISION, GOAL };

