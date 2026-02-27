#ifndef _RAYCAST_H
#define _RAYCAST_H

#include "math/gmath_types.h"

typedef struct RayResult {
    float x, y;
    float dx, dy;
    float u;
    float dist;
    int side;
    int map_x, map_y;
    bool hit;
} RayResult;

#define RAYCAST_RAY_EMPTY (RayResult){ 0 };

typedef struct World World;

RayResult raycast_ray(World *world, Vec2 pos, Vec2 ray_dir);

static inline void _raycast_try_plane(
    float plane_coord, float min, float max,
    bool is_vertical,
    Vec2 pos, Vec2 dir,
    float *t_best,
    float *hit_x, float *hit_y,
    float *u,
    bool *hit
)
{
    float d = is_vertical ? dir.x : dir.y;
    if (fabsf(d) < 1e-8) return;

    float t = (plane_coord - (is_vertical ? pos.x : pos.y)) / d;
    if (t <= 0 || t >= *t_best) return;

    float plane_hit_pos = is_vertical 
        ? pos.y + t * dir.y 
        : pos.x + t * dir.x;

    if (plane_hit_pos < min || plane_hit_pos > max)
        return;

    *t_best = t;

    if (is_vertical) {
        *hit_x = plane_coord;
        *hit_y = plane_hit_pos;
        *u = plane_hit_pos - min;
    } else {
        *hit_x = plane_hit_pos;
        *hit_y = plane_coord;
        *u = plane_hit_pos - min;
    }

    *hit = true;
}

#endif // _RAYCAST_H