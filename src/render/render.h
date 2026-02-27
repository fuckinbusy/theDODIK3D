#ifndef _RENDER_H
#define _RENDER_H
#include <string.h>
#include <stdbool.h>
#include "SDL3/SDL.h"
#include "utypes.h"
#include "math/gmath_types.h"

#define RENDER_COLOR_BLACK (u32)0xFF000000
#define RENDER_COLOR_WHITE (u32)0xFFFFFFFF
#define RENDER_COLOR_RED (u32)0xFFFF0000
#define RENDER_COLOR_GREEN (u32)0xFF00FF00
#define RENDER_COLOR_BLUE (u32)0xFF0000FF

typedef struct Player Player;
typedef struct World World;
typedef struct GameState GameState;
typedef enum TextureId TextureId;
typedef enum AnimId AnimId;

typedef struct FrameBuffer {
    u32 w, h, size;
    u32 buffer[];
} FrameBuffer;

extern FrameBuffer *frame_buffer;

bool render_init(GameState* gs);
void render_free();
void render_player(Player *player);
void render_update(GameState* gs, int fov);
void render_present();
u32  render_apply_shade(u32 color, float shade);
void render_draw_texture(int x_start, int y_start, int w, int h, TextureId tex_id, bool mirrored);
void render_draw_anim(int x_start, int y_start, int w, int h, AnimId anim_id, bool mirrored);
void render_draw_font(Vec2 start, int screen_w, int screen_h, int size, u32 color, TextureId tex_id, const char* str);

static inline void render_clear()
{
    if (!frame_buffer) return;
    memset(frame_buffer->buffer, 0, frame_buffer->size * sizeof(u32));
}
static inline u32 render_buffer_get_pixel(u32 x, u32 y)
{
    if (x >= frame_buffer->w || y >= frame_buffer->h) return 0;
    return frame_buffer->buffer[y * frame_buffer->w + x];
}
static inline void render_buffer_put_pixel(u32 x, u32 y, u32 color)
{
    if (x >= frame_buffer->w || y >= frame_buffer->h) return;
    frame_buffer->buffer[y * frame_buffer->w + x] = color;
}
static inline u32 render_color(u8 r, u8 g, u8 b, u8 a)
{
    // Cast to u32 before shifting: u8 promotes to signed int,
    // and shifting into the sign bit is UB in C.
    return ((u32)a << 24) | ((u32)r << 16) | ((u32)g << 8) | (u32)b;
}

#endif // _RENDER_H