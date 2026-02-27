#include "core/world.h"
#include "core/world_palette.h"
#include "utypes.h"
#include "raycast.h"
#include "math/gmath.h"

bool ray_hits_door
(WorldTile *tile, Vec2 pos, Vec2 ray_dir, u32 map_x, u32 map_y, u32 side, float *dist, float *hit_x, float *hit_y, float *u)
{
    float half = tile->thickness * 0.5;
    float t_best = 1e30;
    bool hit = false;

    if (side == 0) {
        float center = map_x + 0.5;
        float x1 = center - half;
        float x2 = center + half;
        float max_y = map_y + 1.0;
        float min_y = max_y - tile->open_state;

        _raycast_try_plane(
            x1, min_y, max_y, true,
            pos, ray_dir,
            &t_best, hit_x, hit_y, u, &hit
        );
        _raycast_try_plane(
            x2, min_y, max_y, true,
            pos, ray_dir,
            &t_best, hit_x, hit_y, u, &hit
        );
        _raycast_try_plane(
            min_y, x1, x2, false, 
            pos, ray_dir,
            &t_best, hit_x, hit_y, u, &hit
        );
        _raycast_try_plane(
            max_y, x1, x2, false, 
            pos, ray_dir,
            &t_best, hit_x, hit_y, u, &hit
        );
    }
    
    if (side == 1) {
        float center = map_y + 0.5f;
        float y1 = center - half;
        float y2 = center + half;
        float max_x = map_x + 1.0f;
        float min_x = max_x - tile->open_state;

        _raycast_try_plane(
            y1, min_x, max_x, false,
            pos, ray_dir,
            &t_best, hit_x, hit_y, u, &hit
        );
        _raycast_try_plane(
            y2, min_x, max_x, false,
            pos, ray_dir,
            &t_best, hit_x, hit_y, u, &hit
        );
        _raycast_try_plane(
            min_x, y1, y2, true, 
            pos, ray_dir,
            &t_best, hit_x, hit_y, u, &hit
        );
        _raycast_try_plane(
            max_x, y1, y2, true, 
            pos, ray_dir,
            &t_best, hit_x, hit_y, u, &hit
        );
    }
    
    if (hit) *dist = t_best;
    return hit;
}

RayResult raycast_ray(World *world, Vec2 pos, Vec2 ray_dir)
{
    int map_x = (int)floorf(pos.x);
    int map_y = (int)floorf(pos.y);

    float delta_dist_x = fabsf(ray_dir.x == 0 ? 1e30 : 1.0f / ray_dir.x);
    float delta_dist_y = fabsf(ray_dir.y == 0 ? 1e30 : 1.0f / ray_dir.y);

    int step_x, step_y;
    float side_dist_x, side_dist_y;

    if (ray_dir.x < 0) {
        step_x = -1;
        side_dist_x = (pos.x - map_x) * delta_dist_x;
    } else {
        step_x = 1;
        side_dist_x = (map_x + 1.0f - pos.x) * delta_dist_x;
    }

    if (ray_dir.y < 0) {
        step_y = -1;
        side_dist_y = (pos.y - map_y) * delta_dist_y;
    } else {
        step_y = 1;
        side_dist_y = (map_y + 1.0f - pos.y) * delta_dist_y;
    }

    int side = 0;
    float u = 0.0f;
    float dist = 0.0f;
    float hit_x = 0.0f;
    float hit_y = 0.0f;
    bool hit = false;

    while (!hit) {
        if (side_dist_x < side_dist_y) {
            side_dist_x += delta_dist_x;
            map_x += step_x;
            side = 0;
        } else {
            side_dist_y += delta_dist_y;
            map_y += step_y;
            side = 1;
        }

        WorldTile *tile = world_tile(world, map_x, map_y);
        if (NULL == tile) return RAYCAST_RAY_EMPTY;

        if (tile->type == WORLD_TILE_TYPE_WALL) {
            dist = (side == 0)
                ? (map_x - pos.x + (float)(1 - step_x) * 0.5f) / ray_dir.x
                : (map_y - pos.y + (float)(1 - step_y) * 0.5f) / ray_dir.y;

            hit_x = pos.x + dist * ray_dir.x;
            hit_y = pos.y + dist * ray_dir.y;

            if (side == 0) {
                u = hit_y - floorf(hit_y);
            } else {
                u = hit_x - floorf(hit_x);
            }

            hit = true;
        } else if (tile->type == WORLD_TILE_TYPE_DOOR) {
            if (ray_hits_door(
                tile, pos, ray_dir,
                map_x, map_y, side,
                &dist, &hit_x, &hit_y, &u
            )) {
                hit = true;
            }
        }
    }

    return (RayResult){
        .x = hit_x, 
        .y = hit_y,
        .dx = ray_dir.x,
        .dy = ray_dir.y,
        .u = gmath_clamp(u, 0.0f, 1.0f),
        .dist = dist, 
        .side = side, 
        .map_x = map_x,
        .map_y = map_y,
        .hit = hit,
    };
}