# Flappy Bird — raylib

A Flappy Bird clone built in C++ using the [raylib](https://www.raylib.com/) library. Features parallax scrolling backgrounds, animated sprites, bird rotation, persistent high scores, and a clean game state system.

---

## Features

- Animated bird sprite with flapping animation
- Classic Flappy Bird rotation — snaps up on jump, smoothly nosedives on fall
- Parallax scrolling city background with 6 layers
- Procedurally generated pipes with variable gap positions
- Circle-based hitbox for fair and accurate collisions
- Persistent high score saved to file across sessions
- Start screen, gameplay, and game over states
- Live best score display — updates instantly when beaten

---

## Controls

| Key | Action |
|-----|--------|
| `ENTER` | Start the game from the start screen |
| `SPACE` | Flap / jump |
| `R` | Restart after game over |
| `ESC` | Quit the game |

---

## Requirements

- [raylib](https://www.raylib.com/) (any recent version)
- A C++ compiler (g++, clang++, MSVC)

---

## Building

### Windows (MinGW)
```bash
g++ main.cpp -o flappybird -lraylib -lopengl32 -lgdi32 -lwinmm
```

### Linux
```bash
g++ main.cpp -o flappybird -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

### macOS
```bash
g++ main.cpp -o flappybird -lraylib -framework OpenGL -framework Cocoa -framework IOKit
```

---

## Assets Used


| File | Description |
|------|-------------|
| `assets/opSprite.png` | Bird sprite sheet (3 frames, horizontal) |
| `assets/pipe.png` | Pipe texture (cap at top, body below) |
| `assets/1.png` | Background layer — sky (far, slowest) |
| `assets/2.png` | Background layer — far buildings |
| `assets/3.png` | Background layer — mid buildings |
| `assets/4.png` | Background layer — mid buildings |
| `assets/5.png` | Background layer — near buildings |
| `assets/6.png` | Background layer — near buildings (fastest) |

---

## High Score

The best score is saved to `bestscore.txt` in the same directory as the executable. It is written automatically when:

- The bird hits a pipe
- The bird hits the ground or ceiling
- The game window is closed

---

## Project Structure

```
flappybird/
├── main.cpp          # All game code
├── Makefile          # Build script
├── assets/
│   ├── opSprite.png  # Bird sprite sheet
│   ├── pipe.png      # Pipe texture
│   └── 1.png – 6.png # Parallax background layers
└── bestscore.txt     # Auto-generated high score file
```

---

## How It Works

### Game States
The game uses a simple enum-based state machine with three states:
- `STATE_START` — title screen with animated bird
- `STATE_PLAYING` — active gameplay
- `STATE_GAMEOVER` — shows score and restart prompt

### Pipes
Pipes are drawn using a 2-slice technique — the cap is drawn at a fixed size and only the body is stretched. The top pipe uses a vertically pre-flipped texture loaded at startup via `ImageFlipVertical`.

### Parallax Background
Six background layers scroll at different speeds (5–80 px/s) to create a depth effect. Each layer is drawn twice side by side and loops seamlessly.

### Collision
A circle hitbox centered on the bird is used with `CheckCollisionCircleRec` for more forgiving and natural-feeling collisions compared to a rectangle hitbox.

---

## Releases

Pre-built releases are available on the [GitHub Releases](../../releases) page. Download the latest version for your platform and run it directly — no build required.

---

## License

Free to use and modify for personal and educational projects.
