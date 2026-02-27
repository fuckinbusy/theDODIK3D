#include "game_systems.h"
#include "game_state.h"
#include "core/player.h"
#include "core/world.h"
#include "math/gmath.h"

void game_handle_interactions(GameState* gs)
{
    Player* player = &gs->player;
    World* world = world_get(gs->current_world);
    if (!world) return;
    Vec2 d = gmath_direction(player->angle);

    int tile_x = (int)(player->pos.x + d.x * 0.85);
    int tile_y = (int)(player->pos.y + d.y * 0.85);

    if (gmath_circle_vs_aabb(gs->player.pos, gs->player.radius,
        (Vec2) {
        tile_x, tile_y
    },
        (Vec2) {
        tile_x + WORLD_TILE_SIZE, tile_y + WORLD_TILE_SIZE
    }
    )) return;

    WorldTile* tile = world_tile(world, tile_x, tile_y);
    if (!tile) return;

    if (!world_tile_has_flag(tile, WORLD_TILE_FLAG_ACTIVATED)
        && tile->type == WORLD_TILE_TYPE_DOOR) {
        if (tile->state == WORLD_TILE_STATE_DOOR_CLOSED) {
            tile->state = WORLD_TILE_STATE_DOOR_OPENED;
        }
        else {
            tile->state = WORLD_TILE_STATE_DOOR_CLOSED;
        }
        world_tile_toggle_flag(tile, WORLD_TILE_FLAG_ACTIVATED);
    }
}