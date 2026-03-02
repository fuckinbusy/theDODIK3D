#include <string.h>
#include "game_systems.h"
#include "assets/assets.h"
#include "assets/font.h"
#include "game_state.h"
#include "core/world.h"
#include "render/render.h"
#include "core/input.h"
#include "core/gtimer.h"
#include "core/entity.h"

#define GAME_UI_HEIGHT_RATIO 0.15f

void game_relative_mouse_mode(GameState* gs, bool enabled)
{
    if (!SDL_SetWindowRelativeMouseMode(gs->window, enabled))
        SDL_Log("Failed to set relative mouse mode");
    gs->cursor_visible = false;
}

bool game_init(GameState* gs, const char* title, u32 window_w, u32 window_h, u64 window_flags)
{
    if (!gs) return false;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL");
        return false;
    }
    SDL_Log("SDL initialized");

    if (!SDL_CreateWindowAndRenderer(title, (int)window_w, (int)window_h,
        window_flags, &gs->window, &gs->renderer)) {
        SDL_Log("Failed to create window and renderer");
        return false;
    }
    SDL_Log("Created window and renderer (%p | %p)", gs->window, gs->renderer);

    if (!SDL_SetRenderLogicalPresentation(gs->renderer, (int)window_w, (int)window_h,
        SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        SDL_Log("Failed to set window logic representation");
        return false;
    }

    gs->zoom = 1.0f;
    gs->zoom_speed = 1.15f;
    gs->window_w = window_w;
    gs->window_h = window_h;
    gs->ui_w = window_w;
    gs->ui_h = (u32)((float)window_h * GAME_UI_HEIGHT_RATIO);
    gs->game_w = window_w;
    gs->game_h = window_h - gs->ui_h;
    gs->active = true;
    gs->debug = true;
    gs->cursor_visible = true;
    gs->timer = NULL;

    return true;
}

bool game_vsync(GameState* gs, int vsync)
{
    if (!SDL_SetRenderVSync(gs->renderer, vsync)) {
        SDL_Log("Failed to turn on VSync");
        return false;
    }
    gs->vsync = true;
    return true;
}

bool game_load_assets()
{
    assets_load_texmap("assets/textures/tiles"   ASSETS_EXT_TEXTURE, TEXTURE_MAP_TILES);
    assets_load_texmap("assets/textures/dodik"   ASSETS_EXT_TEXTURE, TEXTURE_MAP_DODIK);
    assets_load_texmap("assets/textures/weapons" ASSETS_EXT_TEXTURE, TEXTURE_MAP_WEAPON);
    assets_load_texmap("assets/textures/ui_bg"   ASSETS_EXT_TEXTURE, TEXTURE_MAP_UI_BG);
    assets_load_texmap("assets/textures/enemy"   ASSETS_EXT_TEXTURE, TEXTURE_MAP_ENEMY);

    assets_load_fontmap("assets/textures/font/default"   ASSETS_EXT_FONT, FONT_MAP_DEFAULT);
    assets_load_fontmap("assets/textures/font/minecraft" ASSETS_EXT_FONT, FONT_MAP_MINECRAFT);

    return true;
}

bool game_load_worlds(GameState* gs)
{
    (void)gs;
    world_load("assets/worlds/world_0"    ASSETS_EXT_TEXTURE, WORLD_0);
    world_load("assets/worlds/world_1"    ASSETS_EXT_TEXTURE, WORLD_1);
    world_load("assets/worlds/test_door"  ASSETS_EXT_TEXTURE, WORLD_TEST_DOOR);
    world_load("assets/worlds/test_arena" ASSETS_EXT_TEXTURE, WORLD_TEST_ARENA);
    return true;
}

void game_init_player(GameState* gs, float start_x, float start_y, float start_angle)
{
    gs->player = player_create(start_x, start_y, start_angle);
    gs->input = (Input){ 0 };
}

bool game_init_render(GameState* gs, u32 screen_w, u32 screen_h)
{
    (void)screen_w; (void)screen_h;
    return gs->renderer ? render_init(gs) : false;
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
    return gs->timer != NULL;
}

bool game_init_entities(GameState* gs)
{
    gs->entities = entity_pool_init();
    if (!gs->entities) return false;

    Entity entity_test = entity_create(
        gmath_vec2(5, 5),
        0,
        100,
        ENTITY_TEST_0
    );
    entity_pool_push(gs->entities, &entity_test);
    return true;
}