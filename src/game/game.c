#define SDL_MAIN_USE_CALLBACKS 1
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "utypes.h"
#include "core/player.h"
#include "core/world.h"
#include "render/render.h"
#include "ui/ui.h"
#include "game_state.h"
#include "game_systems.h"
#include "core/gtimer.h"
#include "assets/assets.h"
#include "ui/ui_stats.h"
#include "math/gmath.h"

#define WINDOW_W     1920
#define WINDOW_H     1080
#define WINDOW_TITLE "theDODIK3D"

static GameState game_state = { 0 };

static SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
    if (!game_init(&game_state, WINDOW_TITLE, WINDOW_W, WINDOW_H, 0))
        return SDL_APP_FAILURE;

    SDL_Log(
        "Window size: %ux%u\n"
        "Game size:   %ux%u\n"
        "UI size:     %ux%u\n",
        game_state.window_w,
        game_state.window_h,
        game_state.game_w,
        game_state.game_h,
        game_state.ui_w,
        game_state.ui_h
    );

    game_vsync(&game_state, GAME_VSYNC_ENABLED);

    if (!game_load_assets()) {
        SDL_Log("Failed to load game assets");
        return SDL_APP_FAILURE;
    }

    if (!game_load_worlds(&game_state)) {
        SDL_Log("Failed to load game assets");
        return SDL_APP_FAILURE;
    }
    
    if (!game_init_render(&game_state, WINDOW_W, WINDOW_H)) {
        SDL_Log("Failed to initialize game render");
        return SDL_APP_FAILURE;
    }

    if (!game_init_timer(&game_state)) {
        SDL_Log("Failed to initialize game timer");
        return SDL_APP_FAILURE;
    }

    if (!game_init_entities(&game_state)) { // TODO
        SDL_Log("Failed to initialize game entities");
        return SDL_APP_FAILURE;
    }

    game_relative_mouse_mode(&game_state, true);

    game_init_player(&game_state, 2.5f, 2.5f, 0.0f);
    game_set_active_world(&game_state, WORLD_TEST_ARENA);
    
    SDL_Log("Ready. Set. Go.");
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    return game_handle_event(event, &game_state);
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    if (!game_state.active) {
        SDL_Delay(256);
        return SDL_APP_CONTINUE;
    }

    GameTimer* timer = game_state.timer;
    Player* player = &game_state.player;
    WorldId current_world = game_state.current_world;
    EntityPool* entities = game_state.entities;

    render_clear();
        gtimer_update(timer);

        world_update(current_world, timer->delta_scaled);
        player_update(&game_state, timer->delta_scaled);
        entity_pool_update(entities, timer->delta_scaled);
        render_update(&game_state, 65);

        ui_update(&game_state, 0, game_state.game_h, timer->delta_ms_raw);
        ui_draw_debug(&game_state);
        ui_draw_stats(&game_state);
    render_present();

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    game_free(&game_state);
}