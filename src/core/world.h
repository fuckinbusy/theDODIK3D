#ifndef _WORLD_H
#define _WORLD_H
#include <stdbool.h>
#include "utypes.h"
#include "world_palette.h"
#include "math/gmath_types.h"

#define WORLD_TILE_SIZE_PX 64.0f
#define WORLD_TILE_SIZE    1.0f
#define WORLD_BUFFER_MAX_SIZE 32

#define WORLD_TILE_DOOR_THICKNESS 0.15
#define WORLD_TILE_DOOR_SPEED     2.0

typedef enum WorldTileFlag {
    WORLD_TILE_FLAG_NONE       = 0,
    WORLD_TILE_FLAG_COLLIDABLE = 1u << 0,
    WORLD_TILE_FLAG_ACTIVATED  = 1u << 1,
    WORLD_TILE_FLAG_DOOR       = 1u << 2,
} WorldTileFlag;

typedef struct WorldTile {
    u32   color;
    u32   type;
    u16   state;      // for doors
    u16   flags;
    float open_speed;
    float open_state; // for doors [0..1]
    float thickness;  // for doors 
} WorldTile;

typedef struct World {
    u32 w, h;
    WorldTile map[];
} World;

typedef enum WorldId {
    WORLD_0 = 0,
    WORLD_1,
    WORLD_TEST_DOOR,
    WORLD_TEST_ARENA,
} WorldId;

typedef enum WorldTileState {
    WORLD_TILE_STATE_DOOR_CLOSED = 0,
    WORLD_TILE_STATE_DOOR_OPENED,
} WorldTileState;

World* world_get(WorldId id);
bool world_load(const char* path, WorldId id);
bool world_can_collide(World* world, int tile_x, int tile_y);
bool world_is_colliding(WorldId wid, Vec2 pos, float radius);
void world_update(WorldId wid, float dt);
void world_free();

static inline WorldTile *world_tile(World* world, int tile_x, int tile_y)
{
    if (world == NULL ||
        tile_x >= world->w ||
        tile_y >= world->h ||
        tile_x < 0 ||
        tile_y < 0) return 0;

    return &world->map[tile_y * world->w + tile_x];
}

static inline u32 world_tile_type(World* world, int tile_x, int tile_y)
{
    if (world == NULL ||
        tile_x >= world->w ||
        tile_y >= world->h ||
        tile_x < 0 ||
        tile_y < 0) return 0;

    WorldTile *tile = world_tile(world, tile_x, tile_y);

    return tile ? tile->type : 0;
}

static inline u32 world_tile_texture(World* world, int tile_x, int tile_y)
{
    if (world == NULL      ||
        tile_x >= world->w || 
        tile_y >= world->h || 
        tile_x < 0         || 
        tile_y < 0) return 0;

    WorldTile *tile = world_tile(world, tile_x, tile_y);

    return tile ? world_palette_coltotex(tile->color) : 0;
}

static inline u32 world_tile_color(World* world, int tile_x, int tile_y)
{
    if (world == NULL ||
        tile_x >= world->w ||
        tile_y >= world->h ||
        tile_x < 0 ||
        tile_y < 0) return 0;

    WorldTile *tile = world_tile(world, tile_x, tile_y);

    return tile ? tile->color : 0;
}

static inline void
world_tile_set_flag(WorldTile *tile, WorldTileFlag flag)
{
    tile->flags |= (u16)flag;
}

static inline void
world_tile_clear_flag(WorldTile *tile, WorldTileFlag flag)
{
    tile->flags &= ~((u16)flag);
}

static inline void
world_tile_toggle_flag(WorldTile* tile, WorldTileFlag flag)
{
    tile->flags ^= (u16)flag;
}

static inline bool
world_tile_has_flag(WorldTile *tile, WorldTileFlag flag)
{
    return (tile->flags & (u16)flag) != 0;
}

static inline bool
world_tile_has_flags(WorldTile* tile, WorldTileFlag flags)
{
    return (tile->flags & (u16)flags) == (u16)flags;
}

static inline void 
world_tile_set_state(WorldTile* tile, WorldTileState state)
{
    tile->state = (u16)state;
}

#endif // _WORLD_H