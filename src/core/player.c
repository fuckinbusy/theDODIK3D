#include "player.h"
#include "input.h"
#include "math/gmath.h"
#include "game/game_state.h"

Player player_create(float x, float y, float angle)
{
    return (Player){
        .pos.x  = x,
        .pos.y  = y,
        .angle        = angle,
        .velocity     = PLAYER_VELOCITY,
        .rot_velocity = PLAYER_ROTATION_VELOCITY,
        .radius       = PLAYER_RADIUS,
        .bob_timer    = 0.0f,

        .health       = 100,
        .armor        = 0,
    };
}

void player_destroy(Player *player)
{
    if (player) {
        player->pos = (Vec2){ 0 };
        player->angle  = 0;
        player->velocity = 0;
        player->rot_velocity = 0;
        player->radius = 0;

        player->armor  = 0;
        player->health = 0;
    }
}

void player_update(GameState* gs, float dt)
{
    Player* player = &gs->player;
    Input* input = &gs->input;

    bool is_moving = false;

    Vec2 move = { 0.0f, 0.0f };
    
    if (input->turn_right) {
        player->angle += player->rot_velocity * dt;
    }   
    if (input->turn_left) {
        player->angle -= player->rot_velocity * dt;
    }
    
    if (player->angle < 0) player->angle += 2 * MATH_PI;
    if (player->angle >= 2 * MATH_PI) player->angle -= 2 * MATH_PI;
    
    Vec2 d = gmath_direction(player->angle);

    if (input->move_forward) {
        move.x += d.x;
        move.y += d.y;
        is_moving = true;
    }
    if (input->move_backward) {
        move.x -= d.x;
        move.y -= d.y;
        is_moving = true;
    }

    if (input->strafe_left) {
        move.x += d.y;
        move.y -= d.x;
        is_moving = true;
    }
    if (input->strafe_right) {
        move.x -= d.y;
        move.y += d.x;
        is_moving = true;
    }

    if (is_moving) {
        player->bob_timer += dt;
    }
    else {
        player->bob_timer = 0.0f;
    }

    move = gmath_vec2_norm(move);
    move.x *= player->velocity * dt;
    move.y *= player->velocity * dt;

    float next_pos_x = player->pos.x + move.x;
    if (!world_is_colliding(gs->current_world, (Vec2) { next_pos_x, player->pos.y }, player->radius)) {
        player->pos.x = next_pos_x;
    }

    float next_pos_y = player->pos.y + move.y;
    if (!world_is_colliding(gs->current_world, (Vec2) { player->pos.x, next_pos_y }, player->radius)) {
        player->pos.y = next_pos_y;
    }
}