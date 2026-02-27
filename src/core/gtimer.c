#include <malloc.h>
#include <string.h>
#include "gtimer.h"
#include "SDL3/SDL_log.h"

GameTimer* gtimer_init()
{
    GameTimer* timer = malloc(sizeof(GameTimer));
    if (!timer) {
        SDL_Log("Game timer allocation failed");
        return NULL;
    }

    memset(timer, 0, sizeof(GameTimer));

    timer->frequency = SDL_GetPerformanceFrequency();
    timer->time_scale = GTIMER_TIME_SCALE_DEFAULT;

    SDL_Log("Game timer initialized (fucking cool yo)");
    return timer;
}

void gtimer_free(GameTimer* timer)
{
    if (timer) free(timer);
}

void gtimer_update(GameTimer* timer)
{
    u64 now = SDL_GetPerformanceCounter();
    u64 diff = now - timer->counter;
    timer->counter = now;

    double raw = (double)diff / (double)timer->frequency;
    double scaled = raw;

    timer->delta_raw = (float)raw;
    timer->delta_ms_raw = (u32)(raw * 1000.0);

    scaled *= timer->time_scale;

    if (scaled > 0.2) scaled = 0.2;

    timer->delta_scaled = (float)scaled;
    timer->delta_ms_scaled = (u32)(scaled * 1000.0);

    timer->frames_passed_sec++;
    timer->fps_timer += timer->delta_ms_raw;
    if (timer->fps_timer >= 1000) {
        timer->fps = timer->frames_passed_sec;
        timer->frames_passed_sec = 0;
        timer->fps_timer = 0;
    }
}

void gtimer_set_scale(GameTimer* timer, float time_scale)
{
    if (!timer || 
        time_scale > GTIMER_TIME_SCALE_5X_FAST || 
        time_scale < GTIMER_TIME_SCALE_5X_SLOW) 
        return;

    timer->time_scale = time_scale;
}
