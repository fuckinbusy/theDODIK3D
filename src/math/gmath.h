#ifndef _GAME_MATH_H
#define _GAME_MATH_H
#include <stdbool.h>
#include <math.h>

#define MATH_PI ((float)3.1415926536)
#define MATH_RAD (0.0174532925f)
#define MATH_DEG (57.2957795130f)
#define MATH_DEG_TO_RAD(deg) ((deg) * MATH_RAD)
#define MATH_RAD_TO_DEG(rad) ((rad) * MATH_DEG)

typedef struct Vec2 {
    float x, y;
} Vec2;

bool gmath_circle_vs_aabb(Vec2 center, float radius, Vec2 min, Vec2 max);

static inline Vec2 gmath_direction(float angle)
{
    return (Vec2) { .x = cosf(angle), .y = sinf(angle) };
}

static inline Vec2 gmath_bob(float time, float freq, float velocity, float strength, bool abs_x, bool abs_y)
{
    float x = cosf(time * freq) * velocity * strength;
    float y = sinf(time * freq) * velocity * strength;

    return (Vec2) {
        .x = abs_x ? fabsf(x) : x,
        .y = abs_y ? fabsf(y) : y
    };
}

static inline float gmath_clamp(float x, float min, float max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

static inline Vec2 gmath_vec2(float x, float y)
{
    return (Vec2) { x, y };
}

static inline Vec2 gmath_vec2_add(Vec2 a, Vec2 b)
{
    return (Vec2) { a.x + b.x, a.y + b.y };
}

static inline Vec2 gmath_vec2_sub(Vec2 a, Vec2 b)
{
    return (Vec2) { a.x - b.x, a.y - b.y };
}

static inline Vec2 gmath_vec2_mul(Vec2 v, float s)
{
    return (Vec2) { v.x * s, v.y * s };
}

static inline Vec2 gmath_vec2_div(Vec2 v, float s)
{
    float inv = 1.0f / s;
    return (Vec2) { v.x * inv, v.y * inv };
}

static inline float gmath_vec2_dot(Vec2 a, Vec2 b)
{
    return (a.x * b.x) + (a.y * b.y);
}

static inline float gmath_vec2_len(Vec2 v)
{
    return sqrtf(gmath_vec2_dot(v, v));
}

static inline Vec2 gmath_vec2_norm(Vec2 v)
{
    float vlen = gmath_vec2_len(v);
    return vlen > 1e-8f
        ? gmath_vec2_div(v, vlen)
        : gmath_vec2(0.0f, 0.0f);
}

static inline Vec2 gmath_vec2_dir(Vec2 v)
{
    float vlen = gmath_vec2_len(v);
    if (vlen < 1e-8f) return gmath_vec2(0.0f, 0.0f);
    return gmath_vec2_div(v, vlen);
}

/* Нормализует угол в диапазон [0, 2π) */
static inline float gmath_normalize_angle(float angle)
{
    float two_pi = 2.0f * MATH_PI;
    if (angle < 0.0f)      angle += two_pi;
    if (angle >= two_pi)   angle -= two_pi;
    return angle;
}

#endif // _GAME_MATH_H