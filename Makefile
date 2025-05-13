# Compiler
CXX := g++

# Compiler and linker flags
CXXFLAGS := -std=c++17 -Wall -Wextra
CXXFLAGS += -Isrc/ -Isrc/game/ -Isrc/UI/ -Isrc/AI/ -I/opt/homebrew/opt/sfml/include
LDFLAGS := -L/opt/homebrew/opt/sfml/lib -lsfml-graphics -lsfml-window -lsfml-system

# Source files
SRC_ROOT := src/main.cpp
SRC_RL_MAIN := src/game_main.cpp

SRC_GAME := \
    src/game/Game.cpp \
    src/game/Car.cpp \
    src/game/Map.cpp

SRC_UI := \
    src/UI/MapEditor.cpp \
    src/UI/DisplayMovement.cpp \

SRC_AI := \
    src/AI/NeuralNetwork.cpp \
    src/AI/Layer.cpp \
    src/AI/Optimizer.cpp

# Object files
OBJ_ROOT := $(SRC_ROOT:.cpp=.o)
OBJ_RL_MAIN := $(SRC_RL_MAIN:.cpp=.o)
OBJ_GAME := $(SRC_GAME:.cpp=.o)
OBJ_UI := $(SRC_UI:.cpp=.o)
OBJ_AI := $(SRC_AI:.cpp=.o)

# Targets
OBJ_EDITOR := $(OBJ_ROOT) $(OBJ_UI) $(OBJ_GAME)
OBJ_RL_TRAINER := $(OBJ_RL_MAIN) $(OBJ_GAME) $(OBJ_AI) $(OBJ_UI)

# Default target
all: rl_trainer

# Build the editor executable
editor: $(OBJ_EDITOR)
	$(CXX) $^ -o $@ $(LDFLAGS)

SRC_VISUALIZER := src/visualize.cpp
OBJ_VISUALIZER := $(SRC_VISUALIZER:.cpp=.o)

visualizer: $(OBJ_VISUALIZER) src/UI/DisplayMovement.o
	$(CXX) $^ -o $@ $(LDFLAGS)


# Build the RL trainer executable
rl_trainer: $(OBJ_RL_TRAINER)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Compilation rule (applies to all .cpp files)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f editor rl_trainer
	find src/ -name '*.o' -delete
