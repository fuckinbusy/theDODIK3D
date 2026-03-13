#include "ui_dodik.h"
#include "assets/assets.h"
#include "core/player.h"

static Dodik dodik = {
    .state = DODIK_STATE_IDLE,
    .state_time = 0,
    .frame_time = 0,
    .mirrored = false,
    .hit = false
};

static void dodik_set_state(DodikState state)
{
    dodik.state = state;
    dodik.state_time = 0;
    dodik.frame_time = 0;
}

TextureId dodik_get_texture()
{
    switch (dodik.state) {
        case DODIK_STATE_IDLE:    return TEXTURE_UI_DODIK_IDLE;

        case DODIK_STATE_LOOK_L:
        case DODIK_STATE_LOOK_R:  return TEXTURE_UI_DODIK_SIDE_LOOK;

        case DODIK_STATE_EXCITED: return TEXTURE_UI_DODIK_EXCITED;
        case DODIK_STATE_ANGRY:   return TEXTURE_UI_DODIK_ANGRY;

        default:                  return TEXTURE_UI_DODIK_IDLE;
    }
}

Dodik* ui_dodik_get()
{
    return &dodik;
}

void ui_dodik_update(const Player* player, u32 delta_ms)
{
    dodik.frame_time += delta_ms;
    dodik.state_time += delta_ms;

    switch (dodik.state) {
        case DODIK_STATE_IDLE: {
            if (dodik.state_time >= 5000) {
                dodik_set_state(DODIK_STATE_LOOK_L);
            }
        } break;

        case DODIK_STATE_LOOK_L: {
            if (dodik.state_time >= 700) {
                dodik_set_state(DODIK_STATE_LOOK_R);
                dodik.mirrored = true;
            }
        } break;

        case DODIK_STATE_LOOK_R: {
            if (dodik.state_time >= 700) {
                dodik_set_state(DODIK_STATE_IDLE);
                dodik.mirrored = false;
            }
        } break;

        case DODIK_STATE_ANGRY: {
            if (dodik.state_time >= 1000) {
                dodik_set_state(DODIK_STATE_EXCITED);
            }
        } break;

        case DODIK_STATE_EXCITED: {
            if (dodik.state_time >= 1000) {
                dodik_set_state(DODIK_STATE_IDLE);
            }
        } break;

        default: break;
    }
}