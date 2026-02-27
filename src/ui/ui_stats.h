#ifndef _UI_STATS_H
#define _UI_STATS_H
#include <stdbool.h>

typedef struct GameState GameState;

bool ui_draw_debug(GameState* gs);
bool ui_draw_stats(GameState* gs);

#endif // _UI_STATS_H