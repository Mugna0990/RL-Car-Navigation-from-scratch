# Makefile

# Define the compiler
CXX = g++

# Options for compilation
CXXFLAGS = -std=c++17 -Wall -Wextra

# Define the path to your SFML installation via Homebrew
SFML_HOME = /opt/homebrew/opt/sfml

# Add the include path for SFML headers
CXXFLAGS += -I$(SFML_HOME)/include

# Add the library path and the SFML libraries to link
# Ensure you link all necessary SFML modules (graphics, window, system are common)
LDFLAGS = -L$(SFML_HOME)/lib -lsfml-graphics -lsfml-window -lsfml-system

# Source files - Make sure ALL your .cpp files are listed here
SRC = src/main.cpp src/MapEditor.cpp src/Game.cpp src/Car.cpp src/Map.cpp src/DisplayMovement.cpp

# Object files corresponding to source files
OBJ = $(SRC:.cpp=.o)

# Default target: build the editor executable
all: editor

# Rule to build the executable from object files
editor: $(OBJ)
	$(CXX) $(OBJ) -o editor $(LDFLAGS)

# Generic rule to compile .cpp files into .o files
# This uses the CXX and CXXFLAGS defined above
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to clean generated files
clean:
	rm -f $(OBJ) editor