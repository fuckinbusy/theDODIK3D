# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

**Primary IDE:** Visual Studio 2022 (MSVC v145 toolset, C17 standard)
- Solution file: `theDODIK3D.sln`
- Project file: `theDODIK3D.vcxproj`
- Build from VS: open `.sln`, select configuration (`Debug|x64` or `Release|x64`), press Build

**Build output:** `game/<Platform>/<Configuration>/` (e.g., `game/x64/Debug/`)

**Post-build steps** (automated in `.vcxproj`):
- Copies `assets/` folder into the output directory
- Copies `lib/<Platform>/SDL3.dll` into the output directory

**Include paths:** `src/` and `include/` (SDL3 headers live under `include/SDL3/`)
**Linked library:** `SDL3.lib` from `lib/<Platform>/`

There is no CMake or Makefile — this is a Windows-only MSVC project.

## Architecture Overview

A Wolfenstein-style raycasting 3D game written in C17, using SDL3 with its callback-based main loop (`SDL_MAIN_USE_CALLBACKS`).

### Main Loop (SDL3 Callbacks)
`src/game/game.c` is the entry point. SDL3's app callbacks drive the loop:
- `SDL_AppInit` — initialises subsystems in order: window/renderer → assets → worlds → render → timer → entities → player
- `SDL_AppEvent` — delegates to `game_handle_event()`
- `SDL_AppIterate` — per-frame: clear → update timer → update world/player/entities → render 3D view → render UI → present
- `SDL_AppQuit` — calls `game_free()`

### Core Data Structures
- **`GameState`** (`src/game/game_state.h`) — singleton struct holding all top-level game state: window, renderer, timer, entity pool, player, input, and current world ID.
- **`World`** / **`WorldTile`** (`src/core/world.h`) — tile-based 2D map loaded from `.DIHT` binary files. Each tile stores color, type, flags (collidable, door), and door animation state.
- **`Player`** (`src/core/player.h`) — position (Vec2), angle, velocity, health/armor, and a bob_timer for view bobbing.
- **`EntityPool`** (`src/core/entity.h`) — fixed-size array of up to 20 `Entity` structs. Entities have position, direction, type, health, and basic AI flags.
- **`GameTimer`** (`src/core/gtimer.h`) — tracks `delta_raw`, `delta_scaled` (by `time_scale`), FPS, and SDL performance counter.

### Module Breakdown
| Directory | Responsibility |
|-----------|---------------|
| `src/game/` | Top-level orchestration: init (`game_bootstrap.c`), event handling (`game_input.c`), door/player interactions (`game_interactions.c`) |
| `src/engine/` | `raycast.c` — DDA raycasting against the tile map, returns `RayResult` (hit pos, distance, texture U coord, side) |
| `src/render/` | Software `FrameBuffer` rendered to an SDL texture; `render_update()` casts one ray per screen column, draws walls, floor/ceiling; `render_draw_texture()` and `render_draw_font()` for sprites/UI |
| `src/core/` | World, player, entity, timer, input state |
| `src/assets/` | Loads custom `.DIHT` texture format and `.DIHA` animation format into `Texture`/`TextureAnim` structs indexed by `TextureId` enum |
| `src/ui/` | HUD: `ui_dodik.c` (animated face), `ui_stats.c` (health/armor display), `ui.c` (layout coordinator) |
| `src/math/` | `Vec2`, trig helpers, `MATH_DEG_TO_RAD`/`MATH_RAD_TO_DEG` macros |
| `include/` | `utypes.h` (u8/u16/u32/u64 typedefs), SDL3 headers |

### World / Palette System
World maps are stored as `.DIHT` binary files in `assets/worlds/`. The `WorldId` enum enumerates available maps. `world_palette.h` maps tile pixel colors to `TextureId` and `WorldTileType` — so the map image's colors directly determine wall textures and tile behavior (air, wall, door).

### Asset Format
Custom binary format `.DIHT` (textures) and `.DIHA` (animations). PNG → `.DIHT` conversion is done by the `PNGConverter/` tool: a Python script (`PNGConverter.py`) that calls `conv.dll` (a pre-built C DLL using SDL3) via ctypes. Run it from within the `PNGConverter/` directory.

### Rendering Pipeline
1. `render_clear()` — zeroes the `FrameBuffer`
2. `render_update()` — for each screen column, fires a ray via `raycast_ray()`, computes wall slice height from hit distance, samples the wall texture column, applies distance-based shading (`render_apply_shade()`), and writes pixels into the software framebuffer
3. `ui_update()` — draws HUD elements (face, stats) into the same framebuffer below the 3D view
4. `render_present()` — uploads framebuffer to an SDL texture and calls `SDL_RenderPresent`

### Game Systems Flow
```
game_bootstrap.c → game_init() calls:
  game_load_assets()       → assets.c (loads all DIHT textures)
  game_load_worlds()       → world.c  (loads all DIHT world maps)
  game_init_render()       → render.c (allocates FrameBuffer, SDL texture)
  game_init_timer()        → gtimer.c
  game_init_entities()     → entity.c (allocates EntityPool)
  game_init_player()       → player.c
  game_set_active_world()  → world.c
```

### Key Conventions
- All pixel colors use `ARGB8888` format (`ASSETS_PIXELFORMAT`).
- Tile world coordinates use float units where 1.0 = one tile (`WORLD_TILE_SIZE`); pixel size is tracked separately (`WORLD_TILE_SIZE_PX = 64`).
- `WorldTile.flags` uses a bitmask (`WorldTileFlag` enum) — use the `world_tile_set/clear/toggle/has_flag` inline helpers.
- Input state is polled each frame via `src/core/input.h` (not shown; referenced by `GameState.input`).
- `game_state.active` is false when the window is not focused; the iterate loop sleeps 256 ms in that case.
