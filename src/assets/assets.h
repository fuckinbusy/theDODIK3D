#ifndef _ASSETS_H
#define _ASSETS_H
#include <stdbool.h>
#include "SDL3/SDL.h"
#include "utypes.h"

#define ASSETS_TEXTURES_MAX     256
#define ASSETS_TEXTUREMAPS_MAX  32
#define ASSETS_ANIMS_MAX        256
#define ASSETS_PIXELFORMAT      SDL_PIXELFORMAT_ARGB8888
#define ASSETS_EXT_TEX          ".DIHT"
#define ASSETS_EXT_ANIM         ".DIHA"
#define ASSETS_FONT_CHARS_COUNT 36
#define ASSETS_FONT_CHAR_SIZE   64

typedef struct FontChar {
    u32* pixels;
    u32  w, h;
    u32  stride;
} FontChar;

typedef struct TextureMap {
    u32 textures_count;
    Texture* textures[];
} TextureMap;

typedef struct Texture {
    u32  w, h;
    u32 pixels[];
} Texture;

typedef struct AnimatedTexture {
    u32*  pixels;
    u32   w, h;
    u32   frame_count;
    u32   current_frame;
    float frame_duration;
    float frame_timer;
} AnimatedTexture;

typedef enum TextureMapId {
    TEXTURE_MAP_TILES = 0,
    TEXTURE_MAP_DODIK,
    TEXTURE_MAP_UI,
    TEXTURE_MAP_WEAPON,
    TEXTURE_MAP_FONT_DEFAULT,
    TEXTURE_MAP_FONT_MINECRAFT,
    TEXTURE_MAP_ENEMY,
} TextureMapId;

typedef enum TextureId {
    TEXTURE_TILE_DEFAULT = 0,
    TEXTURE_TILE_BRICKS,
    TEXTURE_TILE_BRICKS_SECRET,
    TEXTURE_TILE_DOOR,

    TEXTURE_UI_BACKGROUND = 0,
    TEXTURE_UI_DODIK_IDLE = 0,
    TEXTURE_UI_DODIK_SIDE_LOOK,
    TEXTURE_UI_DODIK_ANGRY,
    TEXTURE_UI_DODIK_SIDE_LOOK_ANGRY,
    TEXTURE_UI_DODIK_EXCITED,
    TEXTURE_UI_DODIK_SURPRISED,
    TEXTURE_UI_DODIK_IDLE_HIT,
    TEXTURE_UI_DODIK_SIDE_LOOK_HIT,
    TEXTURE_UI_DODIK_ANGRY_HIT,
    TEXTURE_UI_DODIK_SIDE_LOOK_ANGRY_HIT,
    TEXTURE_UI_DODIK_EXCITED_HIT,
    TEXTURE_UI_DODIK_SURPRISED_HIT,
    TEXTURE_UI_DODIK_DEAD,
    TEXTURE_UI_DODIK_GRAY,
    TEXTURE_UI_DODIK_HOLY,
    TEXTURE_UI_DODIK_FUCKU,
    TEXTURE_UI_DODIK_FRAME,

    TEXTURE_UI_WEAPON_HAND_TEST,
    TEXTURE_UI_WEAPON_SHOTGUN_TEST,

    TEXTURE_ENTITY_ENEMY = 0,
} TextureId;

#define TEXTURE_SIZE_64  (u32)64
#define TEXTURE_SIZE_128 (u32)128

typedef enum AnimationId {
    TEXTURE_ANIM_TEST,
} AnimationId;

extern TextureMap* texture_maps[ASSETS_TEXTUREMAPS_MAX];

bool assets_load_texmap(const char* path, TextureMapId map_id, u32 texture_w, u32 texture_h);
bool assets_load_anim(const char* path, AnimationId id);
void assets_free();
FontChar assets_font_char(TextureMapId mid, char ch);

static inline Texture* assets_get_texture(TextureMapId mid, TextureId tid)
{
    if (mid < 0 || mid >= ASSETS_TEXTUREMAPS_MAX)
        return NULL;

    TextureMap* tm = texture_maps[mid];
    if (!tm) return NULL;

    return tm->textures[tid];
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

static inline u32 assets_font_columns(TextureMap* texmap)
{
    return texmap ? texmap->w / ASSETS_FONT_CHAR_SIZE : 0;
}

#endif // _ASSETS_H