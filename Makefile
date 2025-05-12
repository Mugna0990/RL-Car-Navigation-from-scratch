# Define the compiler
CXX = g++

# Options for compilation
CXXFLAGS = -std=c++17 -Wall -Wextra

# Add include paths for your project's header directories
# These tell the compiler where to find header files like "Agent.h", "Car.h", etc.
CXXFLAGS += -Isrc/
CXXFLAGS += -Isrc/game/
CXXFLAGS += -Isrc/UI/
CXXFLAGS += -Isrc/AI/

# SFML path via Homebrew
# !! IMPORTANT: Verify this path is correct for your system !!
SFML_HOME = /opt/homebrew/opt/sfml

# SFML includes and libs
CXXFLAGS += -I$(SFML_HOME)/include
LDFLAGS = -L$(SFML_HOME)/lib -lsfml-graphics -lsfml-window -lsfml-system

# --- Source Files by Directory ---
# List the .cpp files located directly in src/ (if any)
# Assuming src/main.cpp is the entry point for the editor
SRC_ROOT = src/main.cpp

# List the .cpp files in the src/game/ directory
SRC_GAME = src/game/Game.cpp \
           src/game/Car.cpp \
           src/game/Map.cpp

# List the .cpp files in the src/UI/ directory
SRC_UI = src/UI/MapEditor.cpp \
         src/UI/DisplayMovement.cpp

# List the .cpp files in the src/AI/ directory
# !! IMPORTANT: Verify these paths match your actual file locations !!
SRC_AI = src/AI/Agent.cpp \
         src/AI/NeuralNetwork.cpp \
         src/AI/Layer.cpp \
         src/AI/Optimizer.cpp \
         src/AI/ReplayBuffer.cpp \
         src/AI/State.cpp # Assuming State.cpp is also in src/AI/

# Assuming src/game_main.cpp is the entry point for the RL training
# !! IMPORTANT: Verify this path matches your actual file location !!
SRC_RL_MAIN = src/game_main.cpp

# --- Object Files by Directory ---
# Generate the corresponding .o file names for each source file list
OBJ_ROOT = $(SRC_ROOT:.cpp=.o)
OBJ_GAME = $(SRC_GAME:.cpp=.o)
OBJ_UI = $(SRC_UI:.cpp=.o)
OBJ_AI = $(SRC_AI:.cpp=.o)
OBJ_RL_MAIN = $(SRC_RL_MAIN:.cpp=.o)


# --- Executable Targets ---

# Define the object files needed to build the 'editor' executable
# Assuming the editor needs main.cpp, UI files, and Game files
OBJ_EDITOR = $(OBJ_ROOT) $(OBJ_UI) $(OBJ_GAME)

# Define the object files needed to build the 'rl_trainer' executable
# Assuming the trainer needs game_main.cpp, Game files, and AI files
OBJ_RL_TRAINER = $(OBJ_RL_MAIN) $(OBJ_GAME) $(OBJ_AI)

# Default target: what happens when you just type 'make'
# We set the default to build the RL trainer
all: rl_trainer

# Rule to build the editor executable
# Depends on all object files listed in OBJ_EDITOR
editor: $(OBJ_EDITOR)
	$(CXX) $(OBJ_EDITOR) -o editor $(LDFLAGS)

# Rule to build the rl_trainer executable
# Depends on all object files listed in OBJ_RL_TRAINER
rl_trainer: $(OBJ_RL_TRAINER)
	$(CXX) $(OBJ_RL_TRAINER) -o rl_trainer $(LDFLAGS)

# --- Compilation Rules ---
# General pattern rule for compiling any .cpp to .o
# This rule applies to any .cpp file whose .o file is needed by a target.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Explicit rule for compiling .cpp files in src/AI/ to .o files in src/AI/
# This rule is added to help make resolve dependencies specifically for this directory,
# in case the general rule isn't sufficient on some systems/configurations.
src/AI/%.o: src/AI/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


# --- Clean Rule ---
# Removes generated object files and executables
clean:
	# Remove executables
	rm -f editor rl_trainer
	# Remove object files from source directories
	rm -f $(OBJ_ROOT) $(OBJ_GAME) $(OBJ_UI) $(OBJ_AI) $(OBJ_RL_MAIN)
	# Optional: Uncomment the line below to use find for cleaning,
	# which can be more robust but depends on 'find' being available.
	# find src/ -name "*.o" -delete
