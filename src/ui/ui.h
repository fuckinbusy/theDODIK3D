#ifndef _UI_H
#define _UI_H
#include "utypes.h"

typedef struct GameState GameState;

void ui_update(GameState* gs, u32 x_start, u32 y_start, float delta_ms);

#endif // _UI_H