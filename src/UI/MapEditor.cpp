#include "MapEditor.h"

MapEditor::MapEditor(unsigned int w, unsigned int h, unsigned tSize)
    : width(w), height(h), tileSize(tSize),
      window(sf::VideoMode(sf::Vector2u(w * tSize, h * tSize), 32U), "Track Editor"),
      grid(h, std::vector<TileType>(w, WALL))  // Initialize grid with WALLs by default
{
    // legend text
    legendTexts.push_back("S: Set Start");
    legendTexts.push_back("G: Set Goal");
    legendTexts.push_back("L: Set Road (5x5 square)");
    legendTexts.push_back("Enter: Save Map");

    printLegend();
}

void MapEditor::printLegend() {
    std::cout << "=== Map Editor Legend ===\n";
    for (const auto& text : legendTexts) {
        std::cout << text << "\n";
    }
}

void MapEditor::run() {
    while (window.isOpen()) {
        processEvents();
        update();
        render();
    }
}

void MapEditor::processEvents() {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        } else if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            isDrawing = true;
        } else if (const auto* mouseButtonReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
            isDrawing = false;
        } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::S) {
                currentDrawType = START;
            }
            else if (keyPressed->code == sf::Keyboard::Key::G) {
                currentDrawType = GOAL;
            }
            else if (keyPressed->code == sf::Keyboard::Key::L) {
                currentDrawType = ROAD;
            }
            else if (keyPressed->code == sf::Keyboard::Key::Backspace) {
                currentDrawType = EMPTY;
            }
            else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                saveMap("./assets/track.txt");
            }
        }
    }

    if (isDrawing) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        int x = mousePos.x / tileSize;
        int y = mousePos.y / tileSize;
        setTile(x, y, currentDrawType);
    }
}

void MapEditor::setTile(int x, int y, TileType type) {

    auto isInBounds = [&](int ix, int iy) {
        return ix >= 0 && ix < static_cast<int>(width) && iy >= 0 && iy < static_cast<int>(height);
    };

    if (!isInBounds(x, y)) {
        return;
    }

    if (type == START) {
        // If a start is placed, remove the previous one if it exists
        if (startPlaced) {

            if (isInBounds(startX, startY)) {
                grid[startY][startX] = WALL;
            }
        }
        grid[y][x] = START;
        startX = x;
        startY = y;
        startPlaced = true;
    }
    else if (type == GOAL) {
        // If a goal is placed, remove the previous one if it exists
        if (goalPlaced) {

             if (isInBounds(goalX, goalY)) {
                grid[goalY][goalX] = WALL; 
             }
        }
        grid[y][x] = GOAL;
        goalX = x;
        goalY = y;
        goalPlaced = true;
    }
    else if (type == ROAD) {
        // Draw a 5x5 area around the clicked tile, avoiding START/GOAL
        for (int dy = -2; dy <= 2; ++dy) {
            for (int dx = -2; dx <= 2; ++dx) {
        
                int nx = x + dx;
                int ny = y + dy;

                if (isInBounds(nx, ny)) {
                    if (grid[ny][nx] != START && grid[ny][nx] != GOAL) {
                        grid[ny][nx] = ROAD;
                    }
                }
            }
        }
    }
}

void MapEditor::update() {}

void MapEditor::render() {
    window.clear(sf::Color::Black);
    sf::RectangleShape cell(sf::Vector2f(static_cast<float>(tileSize - 1), static_cast<float>(tileSize - 1)));

    for (int y = 0; y < static_cast<int>(height); ++y) {
        for (int x = 0; x < static_cast<int>(width); ++x) {
            switch (grid[y][x]) {
                case EMPTY: cell.setFillColor(sf::Color::Black); break;
                case WALL:  cell.setFillColor(sf::Color(100, 100, 100)); break;
                case ROAD:  cell.setFillColor(sf::Color::White); break;
                case START: cell.setFillColor(sf::Color::Green); break;
                case GOAL:  cell.setFillColor(sf::Color::Red); break;
                default: break; 
            }
            cell.setPosition(sf::Vector2f(static_cast<float>(x * tileSize), static_cast<float>(y * tileSize)));
            window.draw(cell);
        }
    }

    window.display();
}

void MapEditor::saveMap(const std::string& filename) {
    std::ofstream out(filename);
    for (int y = 0; y < static_cast<int>(height); ++y) {
        for (int x = 0; x < static_cast<int>(width); ++x) {
            char c = '#';
            switch (grid[y][x]) {
                case EMPTY: c = ' '; break;
                case WALL:  c = '#'; break;
                case ROAD:  c = '.'; break;
                case START: c = 'S'; break;
                case GOAL:  c = 'G'; break;
                default: break; 
            }
            out << c;
        }
        out << "\n";
    }
    out.close();
    std::cout << "Map saved to " << filename << "\n";
}