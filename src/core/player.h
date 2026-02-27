#ifndef _PLAYER_H
#define _PLAYER_H
#include "math/gmath_types.h"
#include "utypes.h"

#define PLAYER_RADIUS 0.1f
#define PLAYER_VELOCITY 3.5f
#define PLAYER_ROTATION_VELOCITY 2.5f

typedef struct Player {
    Vec2  pos;
    float angle;
    float velocity;
    float rot_velocity;
    float radius;
    float bob_timer;
    
    u32 health;
    u32 armor;
} Player;

typedef struct GameState GameState;

Player player_create(float x, float y, float angle);
void player_destroy(Player *player);
void player_update(GameState *gs, float dt); 

#endif // _PLAYER_H