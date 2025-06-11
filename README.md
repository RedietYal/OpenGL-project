# Tetris OpenGL Game

A fully functional Tetris clone implemented in C++ using OpenGL and GLFW.

### Core Features
- 10x20 grid-based playing field
- All 7 standard tetromino shapes (I, O, T, S, Z, J, L)
- Smooth keyboard controls for movement and rotation
- Automatic row clearing when lines are completed
- Game over detection when blocks reach the top
- Proper collision detection

### Extended Features
- **Next Piece Preview**: See the upcoming tetromino
- **Progressive Difficulty**: Speed increases with level
- **Scoring System**: Points awarded for cleared lines
- **Level System**: Advance levels every 10 cleared lines
- **Pause Functionality**: Pause and resume gameplay
- **Game Restart**: Restart after game over

### OpenGL Implementation
- Modern OpenGL 3.3 Core Profile
- Vertex and Fragment shaders
- Efficient block rendering using instancing
- Smooth transformations and animations
- Time-based game logic using glfwGetTime()

## Controls

- "A/D" or "Left/Right Arrow": Move piece horizontally
- **S** or **Down Arrow**: Soft drop (faster fall)
- **W** or **Up Arrow**: Rotate piece clockwise
- **Space**: Hard drop (instant drop)
- **P**: Pause/Resume game
- **R**: Restart game (when game over)
- **ESC**: Exit game


### Compilation

Using the provided Makefile:
\`\`\`bash
make install-deps  # Install dependencies (Ubuntu/Debian)
make              # Build the game
make run          # Run the game
\`\`\`

Manual compilation:
\`\`\`bash
g++ -std=c++17 -Wall -Wextra -O2 -o tetris tetrisFinal.cpp glad.c -lglfw -lGL -ldl
./tetrisFinal
\`\`\`

## Game Mechanics

### Scoring System
- 1 line: 40 × level points
- 2 lines: 100 × level points  
- 3 lines: 300 × level points
- 4 lines (Tetris): 1200 × level points

### Level Progression
- Start at Level 1
- Advance one level every 10 lines cleared
- Fall speed increases with each level
- Formula: speed = base_speed / (1 + (level-1) × 0.1)

### Tetromino Colors
- I-piece: Cyan
- O-piece: Yellow
- T-piece: Purple
- S-piece: Green
- Z-piece: Red
- J-piece: Blue
- L-piece: Orange

## Code Structure

The game is implemented as a single-file C++ application with the following key components:

- **TetrisGame Class**: Main game logic and state management
- **Tetromino Struct**: Represents individual game pieces
- **OpenGL Rendering**: Modern shader-based rendering system
- **Input System**: Robust keyboard input handling with key state tracking
- **Game Loop**: Fixed timestep game loop with smooth animations

## Technical Details

### OpenGL Features Used
- Vertex Array Objects (VAO) and Vertex Buffer Objects (VBO)
- Vertex and Fragment shaders with uniform variables
- 2D transformations for block positioning
- Alpha blending for visual effects

### Performance Optimizations
- Efficient collision detection
- Minimal OpenGL state changes
- Optimized rendering with instanced drawing
- Smart memory management for game state


