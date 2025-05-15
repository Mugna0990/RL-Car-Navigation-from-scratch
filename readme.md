# RL Agent for Autonomous Car Navigation

This project implements a Reinforcement Learning (RL) agent that learns to control and navigate a car to a goal on a 2D map. **Notably, all core Artificial Intelligence components, including the Deep Q-Network, neural network layers, and the Adam optimizer, are implemented entirely from scratch in C++, without reliance on external machine learning libraries.** 


## Project Overview

The primary goal of this project is to train an autonomous car to navigate a predefined track from a start ('S') position to a goal ('G') position. The agent learns through trial and error, interacting with the environment and receiving rewards or penalties based on its actions.

The agent perceives its environment using a `State` representation, which includes its position, direction, speed, and distances to walls and goal. It uses its Deep Q-Network (DQN) to approximate the optimal action-value function. Actions include accelerating, decelerating, and changing direction (Up, Down, Left, Right).

The project also includes a map editor for creating custom tracks and an SFML-based visualizer to display the agent's learned path.

## Project Structure

The project is organized into a `src` directory with several subdirectories:

* `src/`
    * `main.cpp`: Main entry point for the Map Editor and classic game (the game is intended for testing the enviroment).
    * `game_main.cpp`: Main entry point for training the RL agent.
    * `visualize.cpp`: Main entry point for the Movement Visualizer.
    * `AI/`
        * `Agent.h`RL logic, including action selection and learning from experience using the NN.
        * `NeuralNetwork.h` / `NeuralNetwork.cpp`: Implements the neural network*.
        * `Layer.h` / `Layer.cpp`: Defines individual neural network layers.
        * `Optimizer.h` / `Optimizer.cpp`: Implements the Adam optimizer.
        * `ReplayBuffer.h`: Provides the experience replay buffer.
        * `State.h`: Defines the agent's state representation.
    * `game/`
        * `Game.h` / `Game.cpp`: Manages the game simulation.
        * `Car.h` / `Car.cpp`: Defines the car's attributes and behavior.
        * `Map.h` / `Map.cpp`: Handles the game map.
    * `UI/`
        * `MapEditor.h` / `MapEditor.cpp`: Implements the SFML-based map editor.
        * `DisplayMovement.h` / `DisplayMovement.cpp`: Contains SFML logic to visualize movement.
* `Utils.h`: Contains common utilities like `Direction`, `MAP_WIDTH`, `MAP_HEIGHT`.
* `Makefile`: Used to compile the project.
* `assets/`
    * `track.txt`: Default file for saving/loading the game map.
    * `movements.txt`: Logs car movements.
    * `episode_rewards.txt`: Logs rewards per episode.
    * `episode_distance.txt`: Logs distance to goal per episode.
* `trained_agent/` 
    * Subdirectories for saved model weights and optimizer states.

## Core Components

### Agent

* Initializes and manages the main Q-network and the target Q-network.
* Selects actions using an epsilon-greedy strategy: random action (exploration),or action with the highest Q-value predicted (exploitation).
* Stores experiences (state, action, reward, next state, done flag) into the `ReplayBuffer`.
* Performs experience replay by sampling batches of transitions from the buffer to train the Q-network.
* Periodically updates the weights of the target Q-network with the weights from the main Q-network.
* Manages the exploration rate (epsilon) and its decay over time.

### Neural Network

* Constructs a feedforward neural network based on a vector of layer sizes.
* Performs forward propagation to calculate Q-values for all possible actions given an input state.
* Implements the `learn` method, which performs backpropagation. This involves:
    * Calculating the loss between predicted Q-values and target Q-values (derived from the Bellman equation).
    * Propagating the error signals backward through the network.
    * Accumulating gradients for each layer.
    * Instructing each layer to update its weights and biases using its optimizer.
* Handles saving and loading of the entire network state.

### Layer

* Manages its own weights and biases, initialized using a normal distribution.
* Computes its output during the forward pass, applying a ReLU activation function for hidden layers and a linear activation for the output layer.
* Calculates and accumulates gradients for its weights and biases during backpropagation (`outputLayerNodeValues` for the output layer, `hiddenLayerNodeValues` for hidden layers).
* Contains an `AdamOptimizer` instance to update its parameters.
* Supports saving and loading its state.

### Optimizer

* Implements the Adam (Adaptive Moment Estimation) optimization algorithm.
* Maintains biased first moment (mean) and second moment (uncentered variance) estimates for gradients.
* Calculates bias-corrected moment estimates.
* Supports saving and loading its internal state (moment estimates, training steps, power terms for betas).

### Replay Buffer

* A fixed-size circular buffer (implemented with `std::deque`) that stores `Transition` structs.
* Each `Transition` captures a single step of interaction: `state`, `action`, `reward`, `nextState`, `done`.
* Provides a `sample` method to retrieve a random batch of transitions for training the agent.

### State

* `x`, `y`: Car's current coordinates.
* `direction`: Car's current orientation (UP, RIGHT, DOWN, LEFT).
* `speed`: Car's current velocity (1-5).
* `distU`, `distR`, `distD`, `distL`: Calculated distances from the car to the nearest wall in the up, right, down, and left directions, respectively. This is crucial for obstacle avoidance.
* `toVector()`: Normalizes these attributes into a vector of doubles, suitable as input for the neural network. Distances to walls are capped to avoid excessively large normalized values.

### Game Environment

#### Game

* Initializes the game by loading the `track` (map) from a file.
* Creates a `Car` object and places it at the 'S' (start) position found on the map.
* The `run()` method in `Game.cpp` is "manual play mode", processing character inputs ('w', 'a', 's', 'd', 'm', 'n') to control the car, rendering the state to the console, and checking for game conditions (goal or collision).
* It also logs movements to `movements.txt` during manual play and can call `displayTrackWithMovement` upon game over.
* In the project context (`game_main.cpp`) is primarily used to test the enviroment.

#### Car

* Represents the agent's avatar in the game.
* Attributes: `x`, `y` coordinates, `velocity` (1-5), `dir` (Direction: UP, RIGHT, DOWN, LEFT).
* Actions:
    * `accelerate()`: Increases velocity (max 5).
    * `decelerate()`: Decreases velocity (min 1).
    * `setDirection(Direction)`: Changes the car's orientation.
* `update(const Map& map)`: Moves the car based on its current direction and velocity. Checks for collisions with walls ('#') or reaching the goal ('G'). Returns an `UpdateStatus` (COLLISION, GOAL, OK).
* `checkCollision(const Map& map, int nextX, int nextY)`: Determines if a move to `(nextX, nextY)` results in a collision.
* `minDotsToGoal(const Map& map)`: Performs a Breadth-First Search (BFS) to find the minimum number of '.' (road) tiles to reach the 'G' (goal) tile. This is used for reward shaping in the training loop, encouraging the agent to make progress.

#### Map

* `loadFromFile(const std::string& filename)`: Loads the track layout from a text file.
* `grid`: A `std::vector<std::string>` storing the map, where characters represent different tiles (e.g., '#' for wall, '.' for road, 'S' for start, 'G' for goal).
* `getTile(int x, int y)`: Returns the character at the specified coordinates.
* `find(char c, int& startX, int& startY)`: Locates the first occurrence of a character `c` on the map.
* `display()` / `display(int xC, int yC)`: Renders the map to the console, optionally highlighting the car's position.

### Map Editor

* An SFML-based graphical tool for creating and editing game tracks.
* Allows users to place Walls, Roads, Start (S), and Goal (G) tiles on a grid.
* Key controls:
    * Mouse Click: Draw selected tile type.
    * 'S': Select Start tile.
    * 'G': Select Goal tile.
    * 'L': Select Road tile (draws a 5x5 square of road).
    * 'Enter': Save the current map to `/assets/track.txt`.
* Ensures only one Start and one Goal position are active on the map.

### Movement Display

* Uses SFML to visualize the game track and the car's movement path.
* `loadTrack`: Loads the map from a file.
* `loadMovement`: Loads a sequence of (x, y) coordinates from `movements.txt`.
* Animates a red circle along the loaded movement path, effectively replaying the car's trajectory.

## Training Process

The RL agent is trained in the `train` function within `game_main.cpp`:

1.  **Initialization:**
    * The game map is accessed via a `Game` object.
    * An `Agent` is instantiated with its neural networks, replay buffer, and learning parameters.
    * Log files for movements, rewards, and distances are prepared.
    * Free cells (non-wall, non-goal) on the map are identified for potential random starting positions.

2.  **Episodic Training:** 
    * **Episode Start:**
        * The car is reset. Its starting position is the default 'S' on the map, or, periodically a random free cell is chosen to encourage broader exploration.
    * **Step-by-Step Interaction:** 
        * **State Perception:** The current `State` of the car is constructed, including its position, direction, speed, and distances to walls in four cardinal directions (calculated explicitly by checking tiles).
        * **Action Selection:** The `Agent` selects an `action` using its epsilon-greedy policy based on the `currentState`.
        * **Action Execution & Environment Update:** The chosen action is translated into car controls 
        * **Reward Calculation:** A `reward` is computed:
            * `+5 * improvement` for reducing `minDotsToGoal`.
            * Additional `+5 * improvement` if a new best distance is achieved (under certain conditions).
            * `-0.5` penalty for each step taken (encourages efficiency).
            * `-5.0` penalty for visiting an already visited cell in the current episode (discourages loops).
            * `+1000.0` for reaching the GOAL.
            * `-100.0` for a COLLISION.
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
```

This runs the src/game_main.cpp program. Behavior (fresh train vs. load) is controlled by the load_agent boolean and load_path string within src/game_main.cpp.

Running the Map Editor

To create or edit game tracks:
```bash
./editor
```

This runs the src/main.cpp program, which should launch the MapEditor interface.

Running the Movement Visualizer

To view a replay of logged movements:

```bash
./visualizer
```

This runs the src/visualize.cpp program. 

