#ifndef _GAME_SYSTEMS_H
#define _GAME_SYSTEMS_H
#include <stdbool.h>
#include "SDL3/SDL.h"
#include "utypes.h"

#define GAME_VSYNC_ADAPTIVE SDL_RENDERER_VSYNC_ADAPTIVE
#define GAME_VSYNC_DISABLED SDL_RENDERER_VSYNC_DISABLED
#define GAME_VSYNC_ENABLED  1

typedef struct GameState GameState;
typedef enum WorldId WorldId;

bool game_init(GameState* gs, const char* title, u32 window_w, u32 window_h, u64 window_flags);
void game_relative_mouse_mode(GameState* gs, bool enabled);
bool game_vsync(GameState* gs, int vsync);
bool game_load_assets();
bool game_load_worlds(GameState *gs);
void game_init_player(GameState* gs, float start_x, float start_y, float start_angle);
bool game_init_render(GameState* gs, u32 screen_w, u32 screen_h);
void game_free(GameState* gs);
bool game_set_active_world(GameState* gs, WorldId wid);
bool game_init_timer(GameState* gs);
bool game_init_entities(GameState* gs);

SDL_AppResult game_handle_event(SDL_Event *event, GameState *gs);
void game_handle_interactions(GameState* gs);

#endif // _GAME_SYSTEMS_H