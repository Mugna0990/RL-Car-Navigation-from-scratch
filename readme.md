# Reinforcement Learning Agent for Autonomous Car Navigation

This project implements a Reinforcement Learning (RL) agent that learns to control and navigate a car to a goal on a 2D map. **Notably, all core Artificial Intelligence components, including the Deep Q-Network, neural network layers, and the Adam optimizer, are implemented entirely from scratch in C++, without reliance on external machine learning libraries.** 

The agent utilizes its custom-built Deep Q-Network (DQN) to make decisions. The project also includes a map editor for creating custom tracks and an SFML-based visualizer to display the agent's learned path.


## Project Overview

The primary goal of this project is to train an autonomous car agent to navigate a predefined track from a start ('S') position to a goal ('G') position. The agent learns through trial and error, interacting with the environment and receiving rewards or penalties based on its actions.

A key aspect of this project is that the **Reinforcement Learning algorithms and neural network components are built from the ground up in C++**. This includes the Deep Q-Network, the individual network layers (handling forward and backward propagation), and the Adam optimization algorithm.

The agent perceives its environment using a `State` representation, which includes its position, direction, speed, and distances to nearby walls. It uses its Deep Q-Network (DQN) to approximate the optimal action-value function, guiding its decision-making process. Actions include accelerating, decelerating, and changing direction (Up, Down, Left, Right).

Training is facilitated by an epsilon-greedy exploration strategy and experience replay, ensuring stable and efficient learning. The project also provides tools for creating custom game maps and visualizing the agent's performance.

## Project Structure

The project is organized into a `src` directory with several subdirectories:

* `src/`
    * `main.cpp`: Main entry point for the Map Editor and normal game (the game is intended for testing the enviroment).
    * `game_main.cpp`: Main entry point for training the RL agent.
    * `visualize.cpp`: Main entry point for the Movement Visualizer.
    * `AI/`
        * `Agent.h` encapsulates the RL logic, including action selection and learning from experience using the NN.
        * `NeuralNetwork.h` / `NeuralNetwork.cpp`: Implements the neural network **from scratch**.
        * `Layer.h` / `Layer.cpp`: Defines individual neural network layers **from scratch**.
        * `Optimizer.h` / `Optimizer.cpp`: Implements the Adam optimizer **from scratch**.
        * `ReplayBuffer.h`: Provides the experience replay buffer.
        * `State.h`: Defines the agent's state representation.
    * `game/`
        * `Game.h` / `Game.cpp`: Manages the game simulation.
        * `Car.h` / `Car.cpp`: Defines the car's attributes and behavior.
        * `Map.h` / `Map.cpp`: Handles the game map.
    * `UI/`
        * `MapEditor.h` (Assumed) / `MapEditor.cpp`: Implements the SFML-based map editor.
        * `DisplayMovement.cpp`: Contains SFML logic to visualize movement.
* `Utils.h` (Inferred): Likely contains common utilities like `Direction`, `MAP_WIDTH`, `MAP_HEIGHT`.
* `Makefile`: Used to compile the project.
* `assets/` (Assumed, path should be made relative, e.g., `./assets/`)
    * `track.txt`: Default file for saving/loading the game map.
    * `movements.txt`: Logs car movements.
    * `episode_rewards.txt`: Logs rewards per episode.
    * `episode_distance.txt`: Logs distance to goal per episode.
* `trained_agent/` 
    * Subdirectories for saved model weights and optimizer states.

## Core Components

### Agent

The `Agent` class is central to the RL process:
* Initializes and manages the main Q-network and the target Q-network.
* Selects actions using an epsilon-greedy strategy: random action (exploration),or action with the highest Q-value predicted by the Q-network (exploitation).
* Stores experiences (state, action, reward, next state, done flag) into the `ReplayBuffer`.
* Performs experience replay by sampling batches of transitions from the buffer to train the Q-network.
* Periodically updates the weights of the target Q-network with the weights from the main Q-network.
* Manages the exploration rate (epsilon) and its decay over time.

### Neural Network

The `NeuralNetwork` class provides the Q-function approximation:
* Constructs a feedforward neural network based on a vector of layer sizes.
* Performs forward propagation to calculate Q-values for all possible actions given an input state.
* Implements the `learn` method, which performs backpropagation. This involves:
    * Calculating the loss between predicted Q-values and target Q-values (derived from the Bellman equation).
    * Propagating the error signals backward through the network.
    * Accumulating gradients for each layer.
    * Instructing each layer to update its weights and biases using its optimizer.
* Handles saving and loading of the entire network state (all layers' weights, biases, and optimizer states).

### Layer

Each `Layer` object in the `NeuralNetwork`:
* Manages its own weights and biases, initialized using a normal distribution (He initialization implied).
* Computes its output during the forward pass, applying a ReLU activation function for hidden layers and a linear activation for the output layer.
* Calculates and accumulates gradients for its weights and biases during backpropagation (`outputLayerNodeValues` for the output layer, `hiddenLayerNodeValues` for hidden layers).
* Contains an `AdamOptimizer` instance to update its parameters.
* Supports saving and loading its state.

### Optimizer

The `AdamOptimizer` class:
* Implements the Adam (Adaptive Moment Estimation) optimization algorithm.
* Maintains biased first moment (mean) and second moment (uncentered variance) estimates for gradients.
* Calculates bias-corrected moment estimates.
* Updates the layer's weights and biases using these corrected estimates, the learning rate (`alpha`), and other Adam hyperparameters (`beta_one`, `beta_two`, `epsilon_stable`).
* Supports saving and loading its internal state (moment estimates, training steps, power terms for betas).

### Replay Buffer

The `ReplayBuffer` (`ReplayBuffer.h`):
* A fixed-size circular buffer (implemented with `std::deque`) that stores `Transition` structs.
* Each `Transition` captures a single step of interaction: `state`, `action`, `reward`, `nextState`, `done`.
* Provides an `add` method to store new transitions and a `sample` method to retrieve a random batch of transitions for training the agent.

### State

The `State` class (`State.h`) defines the agent's perception of the environment:
* `x`, `y`: Car's current coordinates.
* `direction`: Car's current orientation (UP, RIGHT, DOWN, LEFT).
* `speed`: Car's current velocity (1-5).
* `distU`, `distR`, `distD`, `distL`: Calculated distances from the car to the nearest wall in the up, right, down, and left directions, respectively. This is crucial for obstacle avoidance.
* `toVector()`: Normalizes these attributes into a vector of doubles, suitable as input for the neural network. Distances to walls are capped to avoid excessively large normalized values.

### Game Environment

#### Game

The `Game` class (`game/Game.h`, `game/Game.cpp`):
* Initializes the game by loading the `track` (map) from a file.
* Creates a `Car` object and places it at the 'S' (start) position found on the map.
* The `run()` method in `Game.cpp` seems to be for manual play, processing character inputs ('w', 'a', 's', 'd', 'm', 'n') to control the car, rendering the state to the console, and checking for game over conditions (goal or collision).
* It also logs movements to `movements.txt` during manual play and can call `displayTrackWithMovement` upon game over.
* In the RL context (`game_main.cpp`), an instance of `Game` is primarily used to provide the `track` (map) to the training function.

#### Car

The `Car` class (`game/Car.h`, `game/Car.cpp`):
* Represents the agent's avatar in the game.
* Attributes: `x`, `y` coordinates, `velocity` (1-5), `dir` (Direction: UP, RIGHT, DOWN, LEFT).
* Actions:
    * `accelerate()`: Increases velocity (max 5).
    * `decelerate()`: Decreases velocity (min 1).
    * `setDirection(Direction)`: Changes the car's orientation. (Note: `turnLeft` and `turnRight` exist but might not be directly used by the RL agent's discrete action space).
* `update(const Map& map)`: Moves the car based on its current direction and velocity. Checks for collisions with walls ('#') or reaching the goal ('G'). Returns an `UpdateStatus` (COLLISION, GOAL, OK).
* `checkCollision(const Map& map, int nextX, int nextY)`: Determines if a move to `(nextX, nextY)` results in a collision.
* `minDotsToGoal(const Map& map)`: Performs a Breadth-First Search (BFS) to find the minimum number of '.' (road) tiles to reach the 'G' (goal) tile. This is used for reward shaping in the training loop, encouraging the agent to make progress.

#### Map

The `Map` class (`game/Map.h`, `game/Map.cpp`):
* `loadFromFile(const std::string& filename)`: Loads the track layout from a text file.
* `grid`: A `std::vector<std::string>` storing the map, where characters represent different tiles (e.g., '#' for wall, '.' for road, 'S' for start, 'G' for goal).
* `getTile(int x, int y)`: Returns the character at the specified coordinates. Treats out-of-bounds as a wall ('#').
* `find(char c, int& startX, int& startY)`: Locates the first occurrence of a character `c` on the map.
* `display()` / `display(int xC, int yC)`: Renders the map to the console, optionally highlighting the car's position.

### Map Editor

The `MapEditor` class (`MapEditor.cpp`):
* An SFML-based graphical tool for creating and editing game tracks.
* Allows users to place Walls, Roads, Start (S), and Goal (G) tiles on a grid.
* Key controls:
    * Mouse Click: Draw selected tile type.
    * 'S': Select Start tile.
    * 'G': Select Goal tile.
    * 'L': Select Road tile (draws a 5x5 square of road).
    * 'Backspace': Select Empty/Wall tile (likely to erase or draw walls).
    * 'Enter': Save the current map to `/Users/matteomugnai/Desktop/RL/assets/track.txt`.
* Ensures only one Start and one Goal position are active on the map.

### Movement Display

The `displayTrackWithMovement` function (in `DisplayMovement.cpp` and declared in `game_main.cpp`):
* Uses SFML to visualize the game track and the car's movement path.
* `loadTrack`: Loads the map from a file.
* `loadMovement`: Loads a sequence of (x, y) coordinates from `movements.txt`.
* Renders the track tiles with different colors.
* Animates a red circle along the loaded movement path, effectively replaying the car's trajectory.
* This visualization is called periodically during training in `game_main.cpp` and at the end of a manual game session in `Game::run()`.

## Training Process

The RL agent is trained in the `train` function within `game_main.cpp`:

1.  **Initialization:**
    * The game map is accessed via a `Game` object.
    * An `Agent` is instantiated with its neural networks, replay buffer, and learning parameters.
    * Log files for movements, rewards, and distances are prepared.
    * Free cells (non-wall, non-goal) on the map are identified for potential random starting positions.

2.  **Episodic Training:** The agent is trained for a specified number of `episodes`.
    * **Episode Start:**
        * The car is reset. Its starting position is the default 'S' on the map, or, periodically (every 3 episodes, but not every 200), a random free cell is chosen to encourage broader exploration.
        * The initial `minDotsToGoal` is calculated as `prevDist`.
        * `maxSteps` for the episode is set (twice the initial distance to goal).
    * **Step-by-Step Interaction:** For each `step` within an episode (up to `maxSteps` or until `done`):
        * **State Perception:** The current `State` of the car is constructed, including its position, direction, speed, and distances to walls in four cardinal directions (calculated explicitly by checking tiles).
        * **Action Selection:** The `Agent` selects an `action` using its epsilon-greedy policy based on the `currentState`.
        * **Action Execution & Environment Update:** The chosen action is translated into car controls (e.g., `accelerate`, `setDirection`). The `car.update(map)` method is called, which moves the car and returns its `UpdateStatus` (OK, GOAL, COLLISION).
        * **Reward Calculation:** A `reward` is computed:
            * `+5 * improvement` for reducing `minDotsToGoal`.
            * Additional `+5 * improvement` if a new best distance is achieved (under certain conditions).
            * `-0.5` penalty for each step taken (encourages efficiency).
            * `-5.0` penalty for visiting an already visited cell in the current episode (discourages loops).
            * `+1000.0` for reaching the GOAL.
            * `-100.0` for a COLLISION.
        * **Store Transition:** The experience (`currentState`, `action`, `reward`, `nextState`, `done` flag) is stored in the `Agent`'s `replay_buffer`.
        * **Experience Replay:** The `agent.experience_replay(batch_size)` method is called to sample experiences and train the Q-network.
        * `episodeReward` is accumulated.
        * `prevDist` is updated to the new `minDotsToGoal`.
    * **Episode End:**
        * Episode statistics (total reward, final distance, epsilon) are printed to the console.
        * Rewards and final distances are logged to their respective files.
        * `epsilon` is decayed.
        * The `target_q_network` is updated with the weights from the `q_network` every 500 episodes.
        * The Q-network and target Q-network models are saved to disk at a specified `save_frequency`.
        * Periodically (every 5000 episodes), the `displayTrackWithMovement` function is called to visualize the agent's progress using the `movements.txt` log (which is populated for episodes divisible by 200).


## Getting Started

### Prerequisites

* A C++ compiler supporting C++17 (e.g., g++). The Makefile is configured for `g++`.
* SFML (Simple and Fast Multimedia Library). The Makefile assumes SFML headers are in `/opt/homebrew/opt/sfml/include` and libraries in `/opt/homebrew/opt/sfml/lib` (typical for Homebrew on macOS). Adjust these paths in the `Makefile` (`CXXFLAGS` and `LDFLAGS`) if SFML is installed elsewhere.
* `make` utility.

### Directory Structure Setup

Ensure your project follows the directory structure outlined in the "Project Structure" section and referenced in the `Makefile` (e.g., source files in `src/`, `src/AI/`, `src/game/`, `src/UI/`). It's highly recommended to create an `assets` directory in the project root for map files and logs, and a `trained_agent` directory for saved models.

### Compilation

A `Makefile` is provided to simplify the compilation process. Open your terminal in the root directory of the project (where the `Makefile` is located).

You can build specific components or all default targets:

* **Build the RL Trainer (default):**
    ```bash
    make rl_trainer 
    ```
    or simply:
    ```bash
    make
    ```
    This will create an executable named `rl_trainer`.

* **Build the Map Editor:**
    ```bash
    make editor
    ```
    This will create an executable named `editor`.

* **Build the Movement Visualizer:**
    ```bash
    make visualizer
    ```
    This will create an executable named `visualizer`.

* **Clean Build Files:**
    To remove all compiled object files (`.o`) and the executables:
    ```bash
    make clean
    ```

### Running

After successful compilation, you can run the executables from your terminal (from the project root):

#### Training the Agent

To start training the RL agent:
```bash
./rl_trainer

This runs the src/game_main.cpp program. Behavior (fresh train vs. load) is controlled by the load_agent boolean and load_path string within src/game_main.cpp.

Running the Map Editor

To create or edit game tracks:

./editor

This runs the src/main.cpp program, which should launch the MapEditor interface.

Running the Movement Visualizer

To view a replay of logged movements:

./visualizer

This runs the src/visualize.cpp program. Ensure assets/track.txt and assets/movements.txt exist and are populated.

