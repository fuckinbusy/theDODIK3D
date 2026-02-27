#include <malloc.h>
#include <stdio.h>
#include "world_palette.h"
#include "world.h"
#include "math/gmath.h"
#include "SDL3/SDL.h"

static World* worlds[WORLD_BUFFER_MAX_SIZE];
static u32 worlds_count = 0;

static bool world_add(World* world, WorldId id)
{
    worlds[id] = world;
    worlds_count++;
    return true;
}

World* world_get(WorldId id)
{
    if (id < 0 || id >= WORLD_BUFFER_MAX_SIZE)
        return NULL;

    return worlds[id];
}

bool world_load(const char *path, WorldId id)
{
    if (!path) return false;

    if (id < 0 || id >= WORLD_BUFFER_MAX_SIZE) {
        SDL_Log("Failed to load world `%s` (worlds buffer is full)", path);
        return false;
    }

    if (world_get(id) != NULL) {
        SDL_Log("World `%s` is already loaded with id %d", path, id);
        return false;
    }

    FILE* f = fopen(path, "rb");
    if (!f) {
        SDL_Log("Failed to open world file `%s`", path);
        return false;
    }

    size_t w = 0;
    size_t h = 0;

    if (fread(&w, sizeof(u32), 1, f) != 1 ||
        fread(&h, sizeof(u32), 1, f) != 1) {
        SDL_Log("Failed to read world size `%s` (world file may be corrupted)", path);
        goto exit_failure;
    }

    size_t map_size = w * h * sizeof(WorldTile);
    size_t map_pixels_size = w * h * sizeof(u32);
    size_t world_struct_size = map_size + sizeof(World);

    World* world = (World*)malloc(world_struct_size);
    if (!world) {
        SDL_Log("Failed to allocate world in memory `%s`", path);
        goto exit_failure;
    }

    world->w = w;
    world->h = h;

    u32* map_pixels = (u32*)malloc(map_pixels_size);
    if (!map_pixels) {
        SDL_Log("Failed to allocate temporary pixels buffer `%s`", path);
        goto exit_failure;
    }

    size_t result = fread(map_pixels, 1, map_pixels_size, f);
    if (result < map_pixels_size) {
        SDL_Log("Failed to read world map `%s`", path);
        goto exit_failure;
    }

    for (u32 y = 0; y < h; ++y) {
        for (u32 x = 0; x < w; ++x) {
            u32 idx = y * w + x;
            u32 color = map_pixels[idx];

            WorldTile tile = {
                .color      = color,
                .type       = world_palette_coltotype(color),
                .thickness  = WORLD_TILE_DOOR_THICKNESS,
                .state      = WORLD_TILE_STATE_DOOR_CLOSED,
                .open_speed = WORLD_TILE_DOOR_SPEED,
                .open_state = 1.0,
                .flags      = WORLD_TILE_FLAG_COLLIDABLE,
            };

            if (tile.type == WORLD_TILE_TYPE_AIR) 
                world_tile_clear_flag(&tile, WORLD_TILE_FLAG_COLLIDABLE);
            
            world->map[idx] = tile;
        }
    }

    world_add(world, id);

    free(map_pixels);
    fclose(f);
    SDL_Log("World loaded `%s`", path);
    return true;

exit_failure:
    SDL_Log("Failed to load world from file `%s` (%s)", path, SDL_GetError());
    fclose(f);
    return false;
}

bool world_can_collide(World* world, int tile_x, int tile_y)
{
    if (!world)
        return true;

    if (tile_x < 0 || tile_x >= world->w || 
        tile_y < 0 || tile_y >= world->h)
        return true;

    WorldTile *tile = world_tile(world, tile_x, tile_y);

    return tile ? world_tile_has_flag(tile, WORLD_TILE_FLAG_COLLIDABLE) : true;
}

bool world_is_colliding(WorldId wid, Vec2 pos, float radius)
{
    World* world = world_get(wid);
    if (!world) return true;

    int min_tx = (int)floorf(pos.x - radius);
    int max_tx = (int)floorf(pos.x + radius);
    int min_ty = (int)floorf(pos.y - radius);
    int max_ty = (int)floorf(pos.y + radius);

    for (int ty = min_ty; ty <= max_ty; ++ty) {
        for (int tx = min_tx; tx <= max_tx; ++tx) {
            if (ty < 0 || ty >= world->h ||
                tx < 0 || tx >= world->w)
                return true;

            if (!world_can_collide(world, tx, ty))
                continue;

            if (gmath_circle_vs_aabb(
                pos, radius,
                (Vec2) {
                (float)tx, (float)ty
            },
                (Vec2) {
                (float)tx + WORLD_TILE_SIZE, (float)ty + WORLD_TILE_SIZE
            }
            )) return true;
        }
    }

    return false;
}

void world_update(WorldId wid, float dt)
{
    World* world = world_get(wid);
    if (!world) return;

    u32 world_size = world->h * world->w;

    for (u32 i = 0; i < world_size; ++i) {
        WorldTile *tile = &world->map[i];

        if (!world_tile_has_flag(tile, WORLD_TILE_FLAG_ACTIVATED))
            continue;

        if (tile->type != WORLD_TILE_TYPE_DOOR)
            continue;

        if (tile->open_state > 0.0f) {
            world_tile_set_flag(tile, WORLD_TILE_FLAG_COLLIDABLE);
        }

        float dir = (tile->state == WORLD_TILE_STATE_DOOR_OPENED) ? -1.0f : 1.0f;
        tile->open_state += dir * tile->open_speed * dt;

        if (tile->open_state <= 0.0f) {
            tile->open_state = 0.0f;
            world_tile_set_state(tile, WORLD_TILE_STATE_DOOR_OPENED);
            world_tile_clear_flag(tile, WORLD_TILE_FLAG_ACTIVATED);
            world_tile_clear_flag(tile, WORLD_TILE_FLAG_COLLIDABLE);
        }
        else if (tile->open_state >= 1.0f) {
            tile->open_state = 1.0f;
            world_tile_set_state(tile, WORLD_TILE_STATE_DOOR_CLOSED);
            world_tile_clear_flag(tile, WORLD_TILE_FLAG_ACTIVATED);
        }
    }
}

void world_free()
{
    for (int i = 0; i < worlds_count; ++i) {
        if (worlds[i]) {
            free(worlds[i]);
        }
    }

    worlds_count = 0;
}
