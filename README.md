# Midnight Highway 3D

A real-time 3D endless racing game built with C++ and OpenGL. Drive through a nighttime highway, avoid traffic, and survive dynamic weather conditions.

## Features
- Endless highway with traffic (cars, buses, trucks)
- Dynamic weather (Clear, Rain, Storm)
- Vehicle headlights and street lamp lighting
- HUD (Speed, Fuel, Distance, Score)
- Sound effects (Engine, Rain, Crash, Thunder, Horn)
- Game states (Countdown, Playing, Paused, Game Over)

## Controls
| Action | Key |
|--------|-----|
| Accelerate | `W` or `↑` |
| Reverse | `Z` or `↓` |
| Steer Left | `A` or `←` |
| Steer Right | `D` or `→` |
| Brake | `S` |
| Headlights ON/OFF | `O` / `F` |
| Horn | `H` |
| Pause/Resume | `P` |
| Restart (Game Over) | `R` |
| Quit | `Esc` |

## How to Run
1. Clone the repository.
2. Open the `.sln` file in **Microsoft Visual Studio**.
3. Link the required libraries: `opengl32.lib`, `glu32.lib`, `freeglut.lib`, `winmm.lib`.
4. Place the `.wav` files (`engine.wav`, `rain.wav`, `crash.wav`, `thunder.wav`, `horn.wav`) in the executable's folder.
5. Build and run (`F5`).

> **Note:** If audio does not work, set the Visual Studio **Working Directory** to `$(OutDir)` in Project Properties → Debugging.

## Technologies Used
- **Language:** C++
- **Graphics:** OpenGL (GLUT/freeglut)
- **Audio:** Windows Multimedia API (`mmsystem.h`)


