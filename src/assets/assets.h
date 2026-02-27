#ifndef _ASSETS_H
#define _ASSETS_H
#include <stdbool.h>
#include "SDL3/SDL.h"
#include "utypes.h"

#define ASSETS_TEXTURES_COUNT_MAX 1024
#define ASSETS_ANIMS_COUNT_MAX    64
#define ASSETS_PIXELFORMAT        SDL_PIXELFORMAT_ARGB8888
#define ASSETS_EXT_TEX            ".DIHT"
#define ASSETS_EXT_ANIM           ".DIHA"
#define ASSETS_FONT_CHARS_COUNT   36
#define ASSETS_FONT_CHAR_SIZE     64

typedef struct FontChar {
    u32* pixels;
    u32 w, h;
    u32 stride;
} FontChar;

typedef struct Texture {
    u32 w, h;
    u32 pixels[];
} Texture;

// Animated texture: all frames stored sequentially in pixels[].
// pixels layout: frame_count * w * h pixels, frame N starts at N*w*h.
typedef struct AnimatedTexture {
    u32   w, h;
    u32   frame_count;
    u32   current_frame;
    float frame_duration; // seconds per frame
    float frame_timer;    // accumulated time
    u32   pixels[];
} AnimatedTexture;

typedef enum AnimId {
    // Add animated texture IDs here as .DIHA assets are created
    ANIM_COUNT
} AnimId;

typedef enum TextureId {
    TEXTURE_TILE_DEFAULT = 0,
    TEXTURE_TILE_SAND,
    TEXTURE_TILE_MARKARYAN,
    TEXTURE_TILE_BRICKS,
    TEXTURE_TILE_BRICKS_SECRET,
    TEXTURE_TILE_DOOR,

    TEXTURE_UI_BACKGROUND,

    TEXTURE_UI_DODIK_ANGRY,
    TEXTURE_UI_DODIK_ANGRY_HIT,
    TEXTURE_UI_DODIK_DEAD,
    TEXTURE_UI_DODIK_FUCKU,
    TEXTURE_UI_DODIK_GRAY,
    TEXTURE_UI_DODIK_HOLY,
    TEXTURE_UI_DODIK_IDLE,
    TEXTURE_UI_DODIK_IDLE_HIT,
    TEXTURE_UI_DODIK_SAD,
    TEXTURE_UI_DODIK_SAD_HIT,
    TEXTURE_UI_DODIK_SIDE_LOOK,
    TEXTURE_UI_DODIK_SIDE_LOOK_HIT,
    TEXTURE_UI_DODIK_SIDE_LOOK_ANGRY,
    TEXTURE_UI_DODIK_SIDE_LOOK_ANGRY_HIT,
    TEXTURE_UI_DODIK_WOW,
    TEXTURE_UI_DODIK_WOW_HIT,
    TEXTURE_UI_DODIK_FRAME,
    TEXTURE_UI_DODIK_GUN_TEST,
    TEXTURE_UI_DODIK_HAND_TEST,

    TEXTURE_FONT_DEFAULT,
    TEXTURE_FONT_MINECRAFT,
} TextureId;

bool            assets_load_texture(const char* path, TextureId tex_id);
Texture*        assets_get(TextureId id);
u32             assets_get_size(TextureId id);
u32             assets_get_w(TextureId id);
u32             assets_get_h(TextureId id);

bool            assets_load_anim(const char* path, AnimId id, float frame_duration);
AnimatedTexture* assets_anim_get(AnimId id);
void            assets_update(float dt); // advance all animation timers

void            assets_free();

FontChar assets_font_char(Texture* texture, char ch);

static inline Texture* assets_font_get(TextureId id)
{
    return assets_get(id);
}

static inline int assets_font_index(char c)
{
    int nc = SDL_toupper((unsigned)c);
    if (nc >= '0' && nc <= '9')
        return nc - '0';
    if (nc >= 'A' && nc <= 'Z')
        return (nc - 'A') + 10;

    return -1;
}

static inline u32 assets_font_columns(Texture* texture)
{
    return texture ? texture->w / ASSETS_FONT_CHAR_SIZE : 0;
}

#endif // _ASSETS_H