#include <stdbool.h>
#include "ui.h"
#include "ui_dodik.h"
#include "SDL3/SDL.h"
#include "assets/assets.h"
#include "render/render.h"
#include "core/player.h"
#include "game/game_state.h"
#include "math/gmath.h"

static void ui_draw_bg(int x_start, int y_start, int ui_w, int ui_h, u32 color)
{
    for (int y = y_start; y < y_start+ui_h; ++y) {
        for (int x = x_start; x < ui_w; ++x) {
            render_buffer_put_pixel(x, y, color);
        }
    }
}

static void ui_draw_health(float x, float y, u32 value)
{
    (void)x;
    (void)y;
    (void)value;
}

void ui_update(GameState *gs, u32 x_start, u32 y_start, float delta_ms)
{
    Dodik* dodik = ui_dodik_get();

    int dodik_size = gs->ui_h;
    int dodik_x = (gs->ui_w * 0.5f) - (dodik_size * 0.5f);
    
    ui_dodik_update(&gs->player, delta_ms);

    Vec2 gun_bob = gmath_bob(
        gs->player.bob_timer,
        8.0f,
        gs->player.velocity,
        4.0f,
        false, true
    );
    u32 gun_size = gs->window_h * 0.5f - gs->ui_h;
    int gun_x = (int)((float)gs->ui_w * 0.5f + gun_bob.x);
    int gun_y = (int)((float)gs->game_h - (float)gun_size + gun_bob.y) * 1.1f;

    render_draw_texture(
        gun_x,
        gun_y,
        gun_size, gun_size,
        TEXTURE_UI_DODIK_HAND_TEST,
        false
    );

    render_draw_texture(
        x_start, 
        y_start, 
        gs->ui_w, gs->ui_h,
        TEXTURE_UI_BACKGROUND,
        false
    );
    
    render_draw_texture(
        dodik_x,
        y_start, 
        dodik_size, dodik_size, 
        dodik_get_texture(),
        dodik->mirrored
    );

    render_draw_texture(
        dodik_x,
        y_start,
        dodik_size, dodik_size,
        TEXTURE_UI_DODIK_FRAME,
        false
    );
}