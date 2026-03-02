# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

Run from the project root (Windows only, requires Visual Studio 2022):

```bat
build.bat
```

This calls MSBuild for `Debug|x64`. Output: `game/x64/Debug/theDODIK3D.exe`.

To open in IDE: `theDODIK3D.sln` in Visual Studio 2022. No CMake or Makefile.

MSBuild path (this machine): `E:\VisualStudio\Professional\MSBuild\Current\Bin\MSBuild.exe`

Assets (`lib/x64/`) are **not tracked by git** and must be present locally for linking.

## Asset Pipeline

All textures/fonts/animations are custom binary formats converted from PNGs:
- `.DIHT` — texture / tilemap / world map
- `.DIHF` — font bitmap
- `.DIHA` — animation strip

To convert PNGs: run `PNGConverter/PNGConverter.py` (uses `PNGConverter/lib/conv.dll`). Select format, source PNGs, and output folder.

World maps are `.DIHT` image files in `assets/worlds/`. Each pixel encodes a tile via `world_palette.h`:
- color → tile type (air/wall/door) via `world_palette_coltotype()`
- color → texture index via `world_palette_coltotex()`

## Architecture

### Entry Point & Game Loop (`src/game/`)

SDL3 app-callback model — no `main()`:
- `SDL_AppInit` → calls `game_init`, loads assets/worlds/render/timer/entities
- `SDL_AppIterate` → per-frame: `gtimer_update` → `world_update` / `player_update` / `entity_pool_update` → `render_update` → `ui_update` → `render_present`
- `SDL_AppEvent` → `game_handle_event`
- `SDL_AppQuit` → `game_free`

`GameState` (`src/game/game_state.h`) is the root struct holding all live state: window/renderer, player, timer, entities, input, active world ID.

`game_bootstrap.c` — all `game_init_*` and `game_load_*` functions.
`game_input.c` — keyboard/mouse event dispatch into `gs->input`.
`game_interactions.c` — player interaction logic (e.g. door activation).

### Renderer (`src/render/render.c`)

Software renderer: all drawing goes into a global `FrameBuffer* frame_buffer` (ARGB8888 pixel array), which is then uploaded to an SDL texture and presented each frame.

`render_update()` drives the full 3D frame:
1. Cast one ray per screen column using `raycast_ray()` (DDA in `src/engine/raycast.c`)
2. Draw wall column with distance-based shading (`render_draw_wall`)
3. Draw floor/ceiling gradient planes (`render_draw_plane` with precomputed `shade_table`)
4. Draw sprites/entities sorted by depth, clipped by z-buffer (`render_draw_entities`)
5. Draw weapon bob overlay (`render_draw_weapon`)

Pixel format is `ARGB8888` everywhere. Helper macros: `render_buffer_put_pixel`, `render_buffer_get_pixel`, `render_color(r,g,b,a)`.

### Raycaster (`src/engine/raycast.c`)

`raycast_ray(world, pos, ray_dir)` — standard DDA grid traversal. Returns `RayResult` with hit position, distance, wall side, texture U coordinate. Door tiles use `_raycast_try_plane()` for thin-slab intersection.

### World (`src/core/world.c`)

Up to `WORLD_BUFFER_MAX_SIZE` (32) worlds loaded simultaneously into a static buffer. Each world is a flat `WorldTile` array (row-major). Access tiles via `world_tile(world, tx, ty)`. Doors animate via `open_state` [0..1] in `world_update()`.

### Assets (`src/assets/`)

`TextureMap` — array of `Texture*` pointers. Loaded by `assets_load_texmap(path, TextureMapId)`. Access via `assets_get_texture(TextureMapId, TextureId)`.

`TextureMapId` enum: `TEXTURE_MAP_TILES`, `TEXTURE_MAP_DODIK`, `TEXTURE_MAP_UI_BG`, `TEXTURE_MAP_WEAPON`, `TEXTURE_MAP_ENEMY`.

Adding a new texture: add entry to `TextureId` enum in `assets.h`, add corresponding PNG to the appropriate atlas PNG, re-run PNGConverter, update `game_load_assets()` if a new texmap file is needed.

### Entities (`src/core/entity.c`)

Fixed-size `EntityPool` (max 20). Entities are plain `Entity` structs with position, direction, health, type. `entity_pool_update()` runs AI/movement each frame.

### UI (`src/ui/`)

Rendered into the bottom strip of the framebuffer (`game_h` to `window_h`):
- `ui_dodik.c` — the face widget (reacts to player state/damage)
- `ui_stats.c` — health/armor numbers
- `ui.c` — layout, debug overlay

### Math (`src/math/gmath.h`)

Header-only `Vec2` helpers: `gmath_vec2_*`, `gmath_direction(angle)`, `gmath_bob(...)` for weapon/view bobbing, `gmath_clamp`. Angles are in radians throughout.

## Key Constants

| Constant | Value | Location |
|---|---|---|
| Window size | 1920×1080 | `game.c` |
| UI strip height | 15% of window_h | `game_bootstrap.c` |
| Default FOV | 65° | `render_update()` call |
| Tile size (world units) | 1.0 | `world.h` |
| Tile size (pixels) | 64px | `world.h` |
| Player radius | 0.1 | `player.h` |
| Max entities | 20 | `entity.h` |
| Max worlds loaded | 32 | `world.h` |
