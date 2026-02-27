#include "game_systems.h"
#include "game_state.h"
#include "core/input.h"
#include "core/player.h"

static SDL_AppResult game_handle_keyboard_event(const SDL_KeyboardEvent *key_event, GameState *gs)
{
    if (key_event->key == SDLK_ESCAPE && (!key_event->down))
        return SDL_APP_SUCCESS;

    Input* input = &gs->input;

    if (key_event->down) { // key down
        SDL_Keycode key = key_event->key;
        if (key == SDLK_W) input->move_forward = true;
        if (key == SDLK_S) input->move_backward = true;
        if (key == SDLK_LEFT) input->turn_left = true;
        if (key == SDLK_RIGHT) input->turn_right = true;
        if (key == SDLK_A) input->strafe_left = true;
        if (key == SDLK_D) input->strafe_right = true;
        if (key == SDLK_E) {
            game_handle_interactions(gs);
        }
        if (key == SDLK_LALT) {
            if (gs->timer->time_scale == 1.0f) {
                gtimer_set_scale(gs->timer, GTIMER_TIME_SCALE_5X_SLOW);
            }
            else {
                gtimer_set_scale(gs->timer, GTIMER_TIME_SCALE_DEFAULT);
            }
        }
        if (key == SDLK_EQUALS) {
            float new_zoom = powf(gs->zoom_speed, 1.0f);
            if (gs->zoom < 1.5f)
                gs->zoom *= new_zoom;
        }
        if (key == SDLK_MINUS) {
            float new_zoom = powf(gs->zoom_speed, -1.0f);
            if (gs->zoom > 0.5f)
                gs->zoom *= new_zoom;
        }
    }
    else { // key up
        SDL_Keycode key = key_event->key;
        if (key == SDLK_W) input->move_forward = false;
        if (key == SDLK_S) input->move_backward = false;
        if (key == SDLK_LEFT) input->turn_left = false;
        if (key == SDLK_RIGHT) input->turn_right = false;
        if (key == SDLK_A) input->strafe_left = false;
        if (key == SDLK_D) input->strafe_right = false;
    }

    return SDL_APP_CONTINUE;
}

static void game_handle_focus_lost(GameState *gs)
{
    gs->active = false;
}

static void game_handle_focus_gained(GameState* gs)
{
    gs->active = true;
    gs->timer->counter = SDL_GetPerformanceCounter();
}

static void game_handle_mouse_motion(const SDL_MouseMotionEvent* mm_event, Player *player)
{
    player->angle += mm_event->xrel * 0.0015f;
}

SDL_AppResult game_handle_event(SDL_Event *event, GameState *gs)
{
    switch (event->type) {
        case SDL_EVENT_QUIT: 
            return SDL_APP_SUCCESS;

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: 
            return game_handle_keyboard_event(&event->key, gs);

        case SDL_EVENT_WINDOW_FOCUS_LOST: 
            game_handle_focus_lost(gs);
            break;

        case SDL_EVENT_WINDOW_FOCUS_GAINED: 
            game_handle_focus_gained(gs);
            break;

        case SDL_EVENT_MOUSE_MOTION: 
            game_handle_mouse_motion(&event->motion, &gs->player);
            break;
    }

    return SDL_APP_CONTINUE;
}