#ifndef _ASSETS_FONT_H
#define _ASSETS_FONT_H
#include <stdbool.h>
#include "SDL3/SDL.h"
#include "utypes.h"

#define FONT_CHAR_SIZE   64
#define FONT_CHARS_COUNT 36
#define FONT_MAPS_MAX    8

typedef struct FontChar {
    u32* pixels;
    u32  w, h;
    u32  stride;
} FontChar;

typedef struct FontMap {
    u32 w, h;     
    u32 char_size;
    u32 columns;  
    u32 pixels[]; 
} FontMap;

typedef enum FontMapId {
    FONT_MAP_DEFAULT = 0,
    FONT_MAP_MINECRAFT = 1,
} FontMapId;

extern FontMap* font_maps[FONT_MAPS_MAX];

bool assets_load_fontmap(const char* path, FontMapId id);
void assets_font_free();

static inline int assets_font_index(char c)
{
    int nc = SDL_toupper((unsigned char)c);
    if (nc >= '0' && nc <= '9') return nc - '0';
    if (nc >= 'A' && nc <= 'Z') return (nc - 'A') + 10;
    return -1;
}

static inline FontChar assets_get_font_char(FontMapId id, char c)
{
    FontChar fc = { NULL, 0, 0, 0 };
    if (id >= FONT_MAPS_MAX) return fc;

    FontMap* fm = font_maps[id];
    if (!fm) return fc;

    int idx = assets_font_index(c);
    if (idx < 0 || idx >= FONT_CHARS_COUNT) return fc;

    u32 col = (u32)idx % fm->columns;
    u32 row = (u32)idx / fm->columns;
    u32 x = col * fm->char_size;
    u32 y = row * fm->char_size;

    fc.pixels = &fm->pixels[y * fm->w + x];
    fc.w = fm->char_size;
    fc.h = fm->char_size;
    fc.stride = fm->w;
    return fc;
}

#endif /* _ASSETS_FONT_H */