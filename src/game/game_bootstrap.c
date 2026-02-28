#include <string.h>
#include "game_systems.h"
#include "assets/assets.h"
#include "game_state.h"
#include "core/world.h"
#include "render/render.h"
#include "core/input.h"
#include "core/gtimer.h"
#include "core/entity.h"

void game_relative_mouse_mode(GameState *gs, bool enabled)
{
    if (!SDL_SetWindowRelativeMouseMode(gs->window, enabled))
        SDL_Log("Failed to set window relative mouse mode");

    gs->cursor_visible = false;
}

bool game_init(GameState* gs, const char* title, u32 window_w, u32 window_h, u64 window_flags)
{
    if (!gs) return false;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL");
        return false;
    }
    SDL_Log("Game launched");

    if (!SDL_CreateWindowAndRenderer(title, window_w, window_h, window_flags, &gs->window, &gs->renderer)) {
        SDL_Log("Failed to initialize window and renderer");
        return false;
    }
    SDL_Log("Created window and renderer (%p | %p)", gs->window, gs->renderer);

    if (SDL_SetRenderLogicalPresentation(gs->renderer, window_w, window_h, SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        SDL_Log("Render logical representation is set to SDL_LOGICAL_PRESENTATION_LETTERBOX");
    }
    else {
        SDL_Log("Failed to set render logical representation to SDL_LOGICAL_PRESENTATION_LETTERBOX");
        return false;
    }

    gs->zoom           = 1.0f;
    gs->zoom_speed     = 1.15f;
    gs->window_w       = window_w;
    gs->window_h       = window_h;
    gs->ui_w           = window_w;
    gs->ui_h           = (u32)(window_h * 0.15f);
    gs->game_w         = window_w;
    gs->game_h         = window_h - gs->ui_h;
    gs->active         = true;
    gs->debug          = true;
    gs->cursor_visible = true;
    gs->timer          = NULL;

    return true;
}

bool game_vsync(GameState *gs, int vsync)
{
    if (!SDL_SetRenderVSync(gs->renderer, vsync)) {
        SDL_Log("Failed to enable VSync");
        return false;
    }

    gs->vsync = true;
    return true;
}

bool game_load_assets()
{
    if (false == assets_load_texture("assets/textures/tiles/default_64"ASSETS_EXT_TEX, TEXTURE_TILE_DEFAULT)) {
        SDL_Log("Failed to load default texture. Game closed");
        return false;
    }
    assets_load_texmap("assets/textures/tiles"ASSETS_EXT_TEX, TEXTURE_MAP_TILES, TEXTURE_SIZE_128);
    assets_load_texmap("assets/textures/dodik"ASSETS_EXT_TEX, TEXTURE_MAP_DODIK, TEXTURE_SIZE_128);
    // todo

    return true;
}

bool game_load_worlds(GameState* gs)
{
    world_load("assets/worlds/world_0"ASSETS_EXT_TEX, WORLD_0);
    world_load("assets/worlds/world_1"ASSETS_EXT_TEX, WORLD_1);
    world_load("assets/worlds/test_door"ASSETS_EXT_TEX, WORLD_TEST_DOOR);
    world_load("assets/worlds/test_arena"ASSETS_EXT_TEX, WORLD_TEST_ARENA);

    return true;
}

void game_init_player(GameState *gs, float start_x, float start_y, float start_angle)
{
    gs->player = player_create(start_x, start_y, start_angle);
    gs->input = (Input) { 0 };
}

bool game_init_render(GameState* gs, u32 screen_w, u32 screen_h)
{
    return gs->renderer 
        ? render_init(gs)
        : false;
}

void game_free(GameState* gs)
{
    gtimer_free(gs->timer);
    entity_pool_free(gs->entities);
    world_free();
    assets_free();
    render_free();
}

bool game_set_active_world(GameState* gs, WorldId wid)
{
    gs->current_world = wid;
    return true;
}

bool game_init_timer(GameState* gs)
{
    gs->timer = gtimer_init();
    return gs->timer ? true : false;
}

bool game_init_entities(GameState* gs)
{
    gs->entities = entity_pool_init();
    if (!gs->entities) return false;

    Entity entity_test = entity_create(
        gmath_vec2(5, 5),
        gmath_vec2(0, 0),
        100,
        ENTITY_TEST_0
    );

    entity_pool_push(gs->entities, &entity_test);

    return true;
}
