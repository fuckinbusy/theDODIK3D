#ifndef _ASSETS_H
#define _ASSETS_H
#include <stdbool.h>
#include "SDL3/SDL.h"
#include "utypes.h"

#define ASSETS_TEXTURES_MAX    256
#define ASSETS_TEXTUREMAPS_MAX 32
#define ASSETS_ANIMS_MAX       256
#define ASSETS_PIXELFORMAT     SDL_PIXELFORMAT_ARGB8888
#define ASSETS_EXT_TEXTURE     ".DIHT"
#define ASSETS_EXT_FONT        ".DIHF"
#define ASSETS_EXT_ANIM        ".DIHA"

#define ASSETS_TEXTURE_HEIGHT_TILE   128
#define ASSETS_TEXTURE_WIDTH_TILE    128
#define ASSETS_TEXTURE_WIDTH_DODIK   64
#define ASSETS_TEXTURE_HEIGHT_DODIK  64
#define ASSETS_TEXTURE_WIDTH_WEAPON  128
#define ASSETS_TEXTURE_HEIGHT_WEAPON 128
#define ASSETS_TEXTURE_WIDTH_BG      1280
#define ASSETS_TEXTURE_HEIGHT_BG     128
#define ASSETS_TEXTURE_WIDTH_ENEMY   128
#define ASSETS_TEXTURE_HEIGHT_ENEMY  128

typedef struct Texture {
    u32 w, h;
    u32 pixels[];   /* ARGB8888, w*h */
} Texture;

typedef struct TextureMap {
    u32      textures_count;
    u32      w, h;
    Texture* textures[];
} TextureMap;

typedef struct AnimatedTexture {
    u32* pixels;
    u32   w, h;
    u32   frame_count;
    u32   current_frame;
    float frame_duration;
    float frame_timer;
} AnimatedTexture;

typedef enum TextureMapId {
    TEXTURE_MAP_TILES = 0,
    TEXTURE_MAP_DODIK,
    TEXTURE_MAP_UI_BG,
    TEXTURE_MAP_WEAPON,
    TEXTURE_MAP_ENEMY,
} TextureMapId;

typedef enum TextureId {
    /* --- TEXTURE_MAP_TILES --- */
    TEXTURE_TILE_DEFAULT = 0,
    TEXTURE_TILE_BRICKS,
    TEXTURE_TILE_BRICKS_SECRET,
    TEXTURE_TILE_MARKARYAN,
    TEXTURE_TILE_SAND,
    TEXTURE_TILE_DOOR,

    /* --- TEXTURE_MAP_UI --- */
    TEXTURE_UI_BG = 0,
    TEXTURE_UI_BG2,

    /* --- TEXTURE_MAP_DODIK --- */
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

    /* --- TEXTURE_MAP_WEAPON --- */
    TEXTURE_WEAPON_HAND = 0,
    TEXTURE_WEAPON_SHOTGUN,

    /* --- TEXTURE_MAP_ENEMY --- */
    TEXTURE_ENTITY_FRONT = 0,
    TEXTURE_ENTITY_RIGHT,
    TEXTURE_ENTITY_RIGHT_BACK,
    TEXTURE_ENTITY_BACK,
    TEXTURE_ENTITY_LEFT,
    TEXTURE_ENTITY_LEFT_BACK,
} TextureId;

typedef enum AnimationId {
    TEXTURE_ANIM_TEST,
} AnimationId;

extern TextureMap* texture_maps[ASSETS_TEXTUREMAPS_MAX];

bool assets_load_texmap(const char* path, TextureMapId map_id);
bool assets_load_anim(const char* path, AnimationId id);
void assets_free();

static inline Texture* assets_get_texture(TextureMapId mid, TextureId tid)
{
    if (mid < 0 || mid >= ASSETS_TEXTUREMAPS_MAX) return NULL;

    TextureMap* tm = texture_maps[mid];
    if (!tm || (u32)tid >= tm->textures_count) return NULL;

    return tm->textures[(u32)tid];
}

#endif /* _ASSETS_H */