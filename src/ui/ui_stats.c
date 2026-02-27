#include "ctype.h"
#include "ui_stats.h"
#include "render/render.h"
#include "game/game_state.h"
#include "utypes.h"
#include "assets/assets.h"
#include "math/gmath.h"

bool ui_draw_debug(GameState* gs)
{
	if (!gs) return false;
	if (gs->debug) {
		u32 font_size = 16;
		char fps[32] = { 0 };
		SDL_snprintf(fps, sizeof(fps), "fps %u", gs->timer->fps);
		render_draw_font(
			gmath_vec2(0, font_size),
			gs->game_w,
			gs->game_h, 
			font_size, 0xFFFFFFFF,
			TEXTURE_FONT_MINECRAFT,
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

	char hp[32] = { 0 };
	char armor[32] = { 0 };
	SDL_snprintf(hp, 32, "Health %u", player->health);
	SDL_snprintf(armor, 32, "Armor  %u", player->armor);

	Vec2 hp_pos = gmath_vec2(gs->ui_w * 0.1, gs->game_h + (gs->ui_h * 0.5));
	Vec2 armor_pos = gmath_vec2(hp_pos.x, hp_pos.y * 1.05f);

	render_draw_font(
		hp_pos,
		gs->window_w, gs->window_h, 
		gs->ui_h * 0.25, 0xFFFFFFFF, 
		TEXTURE_FONT_MINECRAFT, 
		hp
	);

	render_draw_font(
		armor_pos,
		gs->window_w, gs->window_h,
		gs->ui_h * 0.25, 0xFFFFFFFF,
		TEXTURE_FONT_MINECRAFT,
		armor
	);

	return true;
}