#include "Game.h"
#include <iostream>
#include <chrono>
#include <thread>


Game::Game() : car(-1, -1), gameOver(false) {
    std::cout << "Loading track...\n";
    if (!track.loadFromFile("/Users/matteomugnai/Desktop/RL/assets/track.txt")) {
        std::cerr << "Failed to load track.\n";
        return;
    }
    std::cout << "Track loaded successfully.\n";

    int startX = -99, startY = -99;
    if (track.find('S', startX, startY)) {
        std::cout << "Found start at: (" << startX << ", " << startY << ")\n";
        car = Car(startX, startY);
    } else {
        std::cerr << "Start position 'S' not found on map.\n";
    }
    movementFile.open("/Users/matteomugnai/Desktop/RL/assets/movements.txt");
    if (!movementFile.is_open()) {
        std::cerr << "Failed to open movement file: " << "/Users/matteomugnai/Desktop/RL/assets/movements.txt" << std::endl;
    } else {
        std::cout << "Movement file opened successfully: " << "/Users/matteomugnai/Desktop/RL/assets/movements.txt" << std::endl;
    }
}

void Game::processInput(char input) {
    switch (input) {
        case 'w': car.setDirection(UP); break;
        case 'd': car.setDirection(RIGHT); break;
        case 's': car.setDirection(DOWN); break;
        case 'a': car.setDirection(LEFT); break;
        case 'm': car.accelerate(); break;
        case 'n': car.decelerate(); break;
    }
}

void Game::render() {
    if (movementFile.is_open()) {
        movementFile << car.getX() << " " << car.getY() << "\n";
        movementFile.flush();
    } 
    
    track.display(car.getX(), car.getY());
    std::cout << "Car at: (" << car.getX() << ", " << car.getY() << ")\n";
    
}

void Game::run() {
    while (!gameOver) {
        render();

        char input;
        std::cin >> input;
        processInput(input);
        int dist = car.minDotsToGoal(track);
        std::cout<<"current distance is"<<dist<<std::endl;
        UpdateStatus status = car.update(track);
        if (status == UpdateStatus::GOAL) {
            std::cout << "🏁 You reached the goal!\n";
            gameOver = true;
        } else if (status == UpdateStatus::COLLISION) {
            std::cout << "💥 You crashed into a wall!\n";
            gameOver = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (movementFile.is_open()) {
        movementFile.close();
        std::cout << "Movement file closed.\n";
    }

    std::cout << "Game over. Displaying track and movement...\n";
    displayTrackWithMovement("/Users/matteomugnai/Desktop/RL/assets/track.txt", "/Users/matteomugnai/Desktop/RL/assets/movements.txt");
}
