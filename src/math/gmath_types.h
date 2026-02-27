#ifndef _GAME_MATH_TYPES_H
#define _GAME_MATH_TYPES_H

#define MATH_PI ((float)3.1415926536)
#define MATH_RAD (0.0174532925f)
#define MATH_DEG (57.2957795130f)
#define MATH_DEG_TO_RAD(deg) ((deg) * MATH_RAD)
#define MATH_RAD_TO_DEG(rad) ((rad) * MATH_DEG)

typedef struct Vec2 {
    float x, y;
} Vec2;

#endif // _GAME_MATH_TYPES_H