# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```bat
build.bat
```

Output: `game\x64\Debug\theDODIK3D.exe`

To run, launch the exe from `game\x64\Debug\` (SDL3.dll must be present there). There are no tests.

To change build config, edit `CONFIG`/`PLATFORM` in `build.bat`. Default is `Debug|x64`.

## Asset Pipeline

Custom binary formats (all ARGB8888 pixel data):
- **`.DIHT`** — texture or world map: `u32 width, u32 height, u32 pixels[w*h]`
- **`.DIHA`** — animation sequence
- **`.DIHF`** — font atlas

Convert PNG → `.DIHT` using `PNGConverter/PNGConverter.py`.

**World maps** are images where each pixel color maps to a tile type (see `src/core/world_palette.h`):
- `0x00000000` transparent → AIR
- `0xFF000000` black → WALL
- `0xFF00FF00` green → DOOR

Texture atlases are split into sub-textures at load time by `assets.c` based on per-`TextureMapId` grid dimensions.

## Architecture

### Entry Point & Loop

`src/game/game.c` uses SDL3 app callbacks (`SDL_MAIN_USE_CALLBACKS`):
- `SDL_AppInit` — creates window/renderer, loads all assets + worlds, inits subsystems
- `SDL_AppEvent` — routes SDL events to input system
- `SDL_AppIterate` — main loop body (called ~60/s)
- `SDL_AppQuit` — cleanup

Loop order each frame:
1. `gtimer_update` — compute `delta_scaled` / `delta_raw`
2. `world_update` — animate doors
3. `player_update` — input → velocity → collision → position
4. `entity_pool_update` — enemy AI/movement
5. `render_update` — raycasting + wall/sprite draw into FrameBuffer
6. `ui_update` — HUD overlay (Dodik face, stats, font)
7. Present FrameBuffer to SDL texture

### GameState

Single global struct (`src/game/game_state.h`) is the root of all state — passed by pointer to every subsystem. Key fields:
- `window_w/h`, `game_w/h` (rendering area), `ui_w/h` (HUD strip at bottom ~15%)
- `Player player`, `Input input`, `EntityPool* entities`
- `WorldId current_world`, `GameTimer* timer`

### Raycasting Engine (`src/engine/raycast.c`)

DDA (Digital Differential Analyzer) per screen column. `raycast_ray()` takes player pos + ray direction, steps through tile grid, handles wall/door hits, returns `RayResult` with: hit coords, wall side (0=vertical/1=horizontal), texture U (0..1), and perpendicular distance.

Doors have special logic: tests 4 planes around the door opening position; `open_state` [0..1] shifts the hit test point.

### Rendering (`src/render/render.c`)

Pure software renderer writing into `FrameBuffer` (heap-allocated `u32[]` ARGB8888), then blitted to an SDL texture each frame.

- **Walls**: per-column projected height = `game_h / (1.5 + side + dist)`. Texture column sampled by U from `RayResult`. Shading: `shade = 1.0 / (1.5 + side + dist)`.
- **Sprites/entities**: projected via 2D camera-space transform, depth-tested against per-column zbuffer, alpha-blended.
- **Font/UI**: `render_draw_font()` and `render_draw_texture()` draw into the same framebuffer.

### World & Collision (`src/core/world.c`)

`WorldTile` stores: color, type (AIR/WALL/DOOR), flags (`COLLIDABLE`, `DOOR`, `ACTIVATED`), and `open_state`/`open_speed` for doors.

Collision: `world_is_colliding()` checks a 3×3 tile grid around the player using `gmath_circle_vs_aabb()` (circle radius = 0.1). Player movement resolves X and Y axes independently.

### Key Constants

- Player speed: 3.5 units/s, rotation: 2.5 rad/s, radius: 0.1
- Default FOV: 65°, modifiable by `zoom`
- Max entities: 20 (`ENTITY_POOL_MAX_SIZE`)
- Max worlds buffered: 32
- Tile world-unit = 64 display pixels
- Pixel format everywhere: ARGB8888
