#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

const int TILE_SIZE = 7; // Adjust as needed

// Function to load the track from a file
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
                    // Handle inconsistent line lengths if necessary
                    std::cerr << "Warning: Inconsistent line length in track file.\n";
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

// Function to load movement coordinates from a file
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

// Function to display the track and movement using SFML
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
    window.setFramerateLimit(60); // Limit frame rate for smoother display

    // Main SFML loop
    while (window.isOpen()) {
        // Process events
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            // Add other event handling (like keyboard input if needed) here
        }

        // Clear the window
        window.clear(sf::Color::Black); // Background color

        // Draw the track grid
        sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));
        for (int y = 0; y < trackHeight; ++y) {
            for (int x = 0; x < trackWidth; ++x) {
                char cellChar = track[y][x]; // Assuming track is indexed [row][column]

                sf::Color tileColor;
                switch (cellChar) {
                    case '#': tileColor = sf::Color(100, 100, 100); break; // Wall (Dark Grey)
                    case '.': tileColor = sf::Color(50, 50, 50); break;   // Path (Grey)
                    case 'S': tileColor = sf::Color::Green; break;      // Start (Green)
                    case 'G': tileColor = sf::Color::Red; break;        // Goal (Red)
                    default:  tileColor = sf::Color::Black; break;      // Empty/Unknown (Black)
                }

                tile.setFillColor(tileColor);
                tile.setPosition(sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE));
                window.draw(tile);
            }
        }

        // Draw the movement path (blue points)
        sf::CircleShape movementPoint(TILE_SIZE / 1.5f); // Smaller circle for the point
        movementPoint.setFillColor(sf::Color::Blue);     // Blue color for movement
        static int startDelayCounter = 0;
        // Adjust the number of frames for the delay (e.g., 60 frames/sec * 2 sec = 120 frames)
        const int startDelayFrames = 60 * 2; 

        if (startDelayCounter < startDelayFrames) {
            startDelayCounter++;
        } else {
            // --- Place the existing code for drawing the single blue square here ---
            // Draw the movement path (blue points)
            sf::CircleShape movementPoint(TILE_SIZE / 1.5f); // Smaller circle for the point
            movementPoint.setFillColor(sf::Color::Blue);     // Blue color for movement

            static int currentMovementIndex = 0;
            static int frameCounter = 0;
            const int framesPerStep = 8; // Adjust to control speed

            if (!movement.empty()) {
                // Calculate position in pixels, centering the circle in the cell
                float pixelX = movement[currentMovementIndex].first * TILE_SIZE + (TILE_SIZE - movementPoint.getRadius() * 2) / 2.0f;
                float pixelY = movement[currentMovementIndex].second * TILE_SIZE + (TILE_SIZE - movementPoint.getRadius() * 2) / 2.0f;

                movementPoint.setPosition(sf::Vector2f(pixelX, pixelY));
                window.draw(movementPoint);

                frameCounter++;
                if (frameCounter >= framesPerStep) {
                    currentMovementIndex++;
                    if (currentMovementIndex >= static_cast<int>(movement.size())) {
                        currentMovementIndex = 0; // Loop the movement
                    }
                    frameCounter = 0;
                }
            }
            // --- End of the code for drawing the single blue square ---
        }

        // Display the drawn content
        window.display();
    }
}