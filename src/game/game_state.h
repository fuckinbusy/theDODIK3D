#ifndef _GAME_STATE_H
#define _GAME_STATE_H

#include <stdbool.h>
#include "core/world.h"
#include "core/player.h"
#include "core/input.h"
#include "SDL3/SDL.h"
#include "utypes.h"
#include "core/gtimer.h"
#include "core/entity.h"

typedef SDL_Window   Window;
typedef SDL_Renderer Renderer;

typedef struct GameState {
    u32   window_w, window_h;
    u32   game_w, game_h;
    u32   ui_w, ui_h;
    float zoom, zoom_speed;

    Window*     window;
    GameTimer*  timer;
    Renderer*   renderer;
    EntityPool* entities;
    Player      player;
    Input       input;
    WorldId     current_world;

    bool vsync;
    bool active;
    bool cursor_visible;
    bool debug;
} GameState;

#endif // _GAME_STATE_H