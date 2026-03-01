#include "ctype.h"
#include "ui_stats.h"
#include "render/render.h"
#include "game/game_state.h"
#include "utypes.h"
#include "assets/assets.h"
#include "assets/font.h"
#include "math/gmath.h"

bool ui_draw_debug(GameState* gs)
{
    if (!gs) return false;
    if (gs->debug) {
        int   font_size = 16;
        char  fps[32]   = {0};
        SDL_snprintf(fps, sizeof(fps), "fps %u", gs->timer->fps);
        render_draw_font(
            gmath_vec2(0, (float)font_size),
            (int)gs->game_w,
            (int)gs->game_h,
            font_size, 0xFFFFFFFF,
            FONT_MAP_MINECRAFT,
            fps
        );
        return true;
    }
    return false;
}

bool ui_draw_stats(GameState* gs)
{
    if (!gs) return false;

    Player* player = &gs->player;

    char hp[32]    = {0};
    char armor[32] = {0};
    SDL_snprintf(hp,    sizeof(hp),    "Health %u", player->health);
    SDL_snprintf(armor, sizeof(armor), "Armor  %u", player->armor);

    Vec2 hp_pos    = gmath_vec2((float)gs->ui_w * 0.1f,
                                (float)gs->game_h + (float)gs->ui_h * 0.5f);
    Vec2 armor_pos = gmath_vec2(hp_pos.x, hp_pos.y * 1.05f);

    int font_size = (int)((float)gs->ui_h * 0.25f);

    render_draw_font(hp_pos,    (int)gs->window_w, (int)gs->window_h,
                     font_size, 0xFFFFFFFF, FONT_MAP_MINECRAFT, hp);

    render_draw_font(armor_pos, (int)gs->window_w, (int)gs->window_h,
                     font_size, 0xFFFFFFFF, FONT_MAP_MINECRAFT, armor);

    return true;
}
