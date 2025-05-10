# Define the compiler
CXX = g++

# Options for compilation
CXXFLAGS = -std=c++17 -Wall -Wextra

# SFML path via Homebrew
SFML_HOME = /opt/homebrew/opt/sfml

# SFML includes and libs
CXXFLAGS += -I$(SFML_HOME)/include
LDFLAGS = -L$(SFML_HOME)/lib -lsfml-graphics -lsfml-window -lsfml-system

# Common source files
SRC_COMMON = src/Game.cpp src/Car.cpp src/Map.cpp src/DisplayMovement.cpp

# Main editor target
SRC_EDITOR = src/main.cpp src/MapEditor.cpp $(SRC_COMMON)
OBJ_EDITOR = $(SRC_EDITOR:.cpp=.o)

# Game-only target
SRC_GAME = src/game_main.cpp $(SRC_COMMON)
OBJ_GAME = $(SRC_GAME:.cpp=.o)

# Default target
all: editor

# Editor build
editor: $(OBJ_EDITOR)
	$(CXX) $(OBJ_EDITOR) -o editor $(LDFLAGS)

# Game-only build
game: $(OBJ_GAME)
	$(CXX) $(OBJ_GAME) -o game $(LDFLAGS)

# Compile rule
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJ_EDITOR) $(OBJ_GAME) editor game
