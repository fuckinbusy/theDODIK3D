#ifndef _CORE_GAME_TIMER_H
#define _CORE_GAME_TIMER_H
#include "SDL3/SDL.h"
#include "utypes.h"

#define GTIMER_TIME_SCALE_DEFAULT 1.0f
#define GTIMER_TIME_SCALE_5X_SLOW 0.2f
#define GTIMER_TIME_SCALE_5X_FAST 5.0f

typedef struct GameTimer {
	float time_scale;
	float delta_raw;
	float delta_scaled;
	u32 delta_ms_raw;
	u32 delta_ms_scaled;
	u16 fps;
	u16 fps_timer;
	u64 counter;
	u64 frequency;
} GameTimer;

GameTimer* gtimer_init();
void gtimer_free(GameTimer* timer);
void gtimer_update(GameTimer* timer);
void gtimer_set_scale(GameTimer* timer, float time_scale);

#endif // _CORE_GAME_TIMER_H