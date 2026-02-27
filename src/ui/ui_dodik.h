#ifndef _UI_DODIK_H
#define _UI_DODIK_H
#include <stdbool.h>
#include "utypes.h"

typedef enum DodikState {
    DODIK_STATE_IDLE = 0,
    DODIK_STATE_ANGRY,
    DODIK_STATE_SAD,
    DODIK_STATE_LOOK_L,
    DODIK_STATE_LOOK_R,
} DodikState;

typedef struct Dodik {
    u32  state;
    u32  state_time; // how long in this state
    u32  frame_time; // anim timer
    bool mirrored;
    bool hit;
} Dodik;

typedef struct Player Player;
typedef enum TextureId TextureId;

void ui_dodik_update(Player* player, u32 delta_ms);
TextureId dodik_get_texture();
Dodik* ui_dodik_get();

#endif // _UI_DODIK_H