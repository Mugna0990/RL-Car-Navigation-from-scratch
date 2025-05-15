#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

const int TILE_SIZE = 3; // Adjust to fix the display dimension

// load track
std::vector<std::string> loadTrack(const std::string& filePath, int& width, int& height) {
    std::vector<std::string> track;
    std::ifstream file(filePath);
    std::string line;

    width = 0;
    height = 0;

    if (file.is_open()) {
        while (getline(file, line)) {
            if (!line.empty()) {
                track.push_back(line);
                if (width == 0) {
                    width = line.length();
                } else if (static_cast<int>(line.length()) != width) {
                    std::cerr << "Inconsistent line length in track file.\n";
                }
                height++;
            }
        }
        file.close();
        std::cout << "Track loaded: " << width << "x" << height << std::endl;
    } else {
        std::cerr << "Unable to open track file: " << filePath << std::endl;
    }

    return track;
}

// load movement coordinates from file
std::vector<std::pair<int, int>> loadMovement(const std::string& filePath) {
    std::vector<std::pair<int, int>> movement;
    std::ifstream file(filePath);
    int x, y;

    if (file.is_open()) {
        while (file >> x >> y) {
            movement.push_back({x, y});
        }
        file.close();
        std::cout << "Loaded " << movement.size() << " movement points." << std::endl;
    } else {
        std::cerr << "Unable to open movement file: " << filePath << std::endl;
    }

    return movement;
}

// display the track and movement using SFML
void displayTrackWithMovement(const std::string& trackFilePath, const std::string& movementFilePath) {
    int trackWidth = 0;
    int trackHeight = 0;
    std::vector<std::string> track = loadTrack(trackFilePath, trackWidth, trackHeight);

    if (track.empty() || trackWidth == 0 || trackHeight == 0) {
        std::cerr << "Failed to load track. Exiting display.\n";
        return;
    }

    std::vector<std::pair<int, int>> movement = loadMovement(movementFilePath);

    // Create the SFML window
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u( trackWidth * TILE_SIZE, trackHeight * TILE_SIZE)), "Track and Movement Display");
    window.setFramerateLimit(60);

    // Main SFML loop
    static bool useRed = true;
    while (window.isOpen()) {
        // Process events
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear(sf::Color::Black); // Background color

        // Draw the track grid
        
        sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));
        for (int y = 0; y < trackHeight; ++y) {
            for (int x = 0; x < trackWidth; ++x) {
                char cellChar = track[y][x]; 

                sf::Color tileColor;
                switch (cellChar) {
                    case '#': tileColor = sf::Color(100, 100, 100); break; // Wall (Dark Grey)
                    case '.': tileColor = sf::Color(50, 50, 50); break;   // Path (Grey)
                    case 'S': tileColor = sf::Color::Green; break;      // Start (Green)
                    case 'G': tileColor = sf::Color::Blue; break;        // Goal (Blue)
                    default:  tileColor = sf::Color::Black; break;      // Empty (Black)
                }

                tile.setFillColor(tileColor);
                tile.setPosition(sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE));
                window.draw(tile);
            }
        }

        // Draw the movement path (red / cyan to understand when all movements have been seen)
        sf::CircleShape movementPoint(TILE_SIZE / 1.5f); 
        movementPoint.setFillColor(useRed ? sf::Color::Red : sf::Color(0, 255, 255)); 

        static int startDelayCounter = 0;
        const int startDelayFrames = 60 * 2;

        if (startDelayCounter < startDelayFrames) {
            startDelayCounter++;
        } else {
            static int currentMovementIndex = 0;
            static int frameCounter = 0;
            const int framesPerStep = 1;

            if (!movement.empty()) {
                float pixelX = movement[currentMovementIndex].first * TILE_SIZE + (TILE_SIZE - movementPoint.getRadius() * 2) / 2.0f;
                float pixelY = movement[currentMovementIndex].second * TILE_SIZE + (TILE_SIZE - movementPoint.getRadius() * 2) / 2.0f;

                movementPoint.setPosition(sf::Vector2f(pixelX, pixelY));
                window.draw(movementPoint);

                frameCounter++;
            if (frameCounter >= framesPerStep) {
                
                if (currentMovementIndex >= static_cast<int>(movement.size()) - 1) {
                    currentMovementIndex = 0;
                    useRed = !useRed;  // Toggle color when cycle completes
                } else {
                    currentMovementIndex++;
                }
                frameCounter = 0;
            }
            }
        }

        window.display();
    }
}