#ifndef _PLAYER_H
#define _PLAYER_H
#include "utypes.h"
#include "math/gmath.h"

#define PLAYER_RADIUS 0.1f
#define PLAYER_VELOCITY 3.5f
#define PLAYER_ROTATION_VELOCITY 2.5f

typedef struct GameState GameState;

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

Player player_create(float x, float y, float angle);
void player_destroy(Player *player);
void player_update(GameState *gs, float dt); 

#endif // _PLAYER_H