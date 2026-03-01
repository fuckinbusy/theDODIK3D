# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

Open `theDODIK3D.sln` in **Visual Studio 2022** (MSVC v145). There is no CMake or Makefile.

- **Build shortcut:** `Ctrl+Shift+B` in Visual Studio
- **Output:** `game/<Platform>/<Configuration>/` (e.g. `game/x64/Debug/`)
- **Configurations:** Debug|x64 and Release|x64 are the primary targets

The project uses C17 (`/std:c17`) and is **Windows-only**.

## Asset Pipeline

Custom binary formats are used for all game assets:
- `.DIHT` — textures and world maps
- `.DIHA` — animations

Convert PNGs to these formats using `PNGConverter/PNGConverter.py` (requires `PNGConverter/lib/conv.dll`). World maps are authored as PNG files in `assets/worlds/png/` then converted. Pixel colors in world PNGs map to tile types via `src/core/world_palette.h`:
- `0xFF000000` (black) → WALL
- `0xFFFF0000` (red) → DOOR
- `0x00000000` (transparent) → AIR

## Architecture

The codebase follows a strict layered architecture. **Do not create circular dependencies between layers.**

```
game/    → orchestration, SDL3 callbacks (entry point)
engine/  → DDA raycasting
render/  → software framebuffer, wall/sprite/font drawing
core/    → world, player, entity, timer, input
assets/  → texture/animation loading
ui/      → HUD (dodik face, stats)
math/    → Vec2, angle helpers
```

### Entry Point & Frame Loop

`src/game/game.c` uses **SDL3 app callbacks** (`SDL_MAIN_USE_CALLBACKS`):
- `SDL_AppInit` — one-time initialization
- `SDL_AppEvent` — SDL event handling
- `SDL_AppIterate` — called every frame

Frame sequence in `SDL_AppIterate`:
1. `render_clear()` — clear framebuffer
2. `gtimer_update()` — update delta time
3. `world_update()` — animate doors
4. `player_update()` — movement + collision
5. `entity_pool_update()` — enemy AI
6. `render_update()` — **main raycasting + drawing**
7. `ui_update()` / `ui_draw_stats()` — HUD
8. `render_present()` — upload framebuffer to SDL texture and flip

### Rendering Pipeline

`render_update()` in `src/render/render.c` does per-column raycasting:
- FOV: 65° (configurable via `gs->zoom`)
- For each screen column: cast a ray via `raycast_ray()`, compute wall height, draw ceiling/wall/floor
- Z-buffer stored per column for sprite occlusion
- After walls: draw all sprites sorted by distance
- Pixel format throughout: **ARGB8888**

### Raycasting Engine

`src/engine/raycast.c` implements DDA raycasting:
- Returns `RayResult` with hit distance, texture U coordinate, side (0=vertical/1=horizontal), tile coords
- Handles **door collision** via plane intersection (`ray_hits_door`), respecting `tile->open_state` for animated doors

### Key Data Structures

**`GameState`** (`src/game/game_state.h`) — singleton root of all game state; passed as pointer to nearly every function.

**`World`** (`src/core/world.h`) — flat array of `WorldTile` structs; each tile has type, door open state/speed/thickness, and collision flags.

**`EntityPool`** (`src/core/entity.h`) — fixed pool of max 20 enemies.

**`FrameBuffer`** (`src/render/render.h`) — heap-allocated with flexible array member; pixels are ARGB8888.

**`TextureMap`** (`src/assets/assets.h`) — per-category texture collections indexed by `TextureMapId` enum (TILES, DODIK, UI, WEAPON, FONT_DEFAULT, FONT_MINECRAFT, ENEMY).

### Coordinate System

- World space: 1.0 unit = 1 tile
- Player radius: 0.1 units, velocity: 3.5 u/s, rotation: 2.5 rad/s
- Window: 1920×1080; game area and UI area are separate regions

### Time System

`src/core/gtimer.c` — high-resolution timer with time scaling (`time_scale`). Always use `timer->dt` (scaled delta) for game logic; `timer->raw_dt` for frame diagnostics.
