#include "Game.h"
#include <iostream>
#include <chrono>
#include <thread>

Game::Game() : car(-1, -1), gameOver(false) {
    std::cout << "Loading track...\n";
    if (!track.loadFromFile("assets/track1.txt")) {
        std::cerr << "Failed to load track.\n";
        return;
    }
    std::cout << "Track loaded successfully.\n";

    int startX = -99, startY = -99;
    if (track.findStart(startX, startY)) {
        std::cout << "Found start at: (" << startX << ", " << startY << ")\n";
        car = Car(startX, startY);
    } else {
        std::cerr << "Start position 'S' not found on map.\n";
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
    track.display();
    std::cout << "Car at: (" << car.getX() << ", " << car.getY() << ")\n";
}

void Game::run() {
    while (!gameOver) {
        render();

        char input;
        std::cin >> input;
        processInput(input);

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
}
