#ifndef _ASSETS_FONT_H
#define _ASSETS_FONT_H
#include <stdbool.h>
#include "SDL3/SDL.h"
#include "utypes.h"

/* ------------------------------------------------------------------
 * Шрифтовые битмапы (FontMap)
 *
 * FontMap — единая текстура, содержащая символы 0-9 и A-Z (36 штук)
 * расположенных в сетке слева направо, сверху вниз.
 * Формат файла .DIHT (как TextureMap, но ровно одна текстура):
 *   [textures_count : u32]  (должен быть == 1)
 *   [w : u32][h : u32][pixels : w * h * 4 байта, ARGB8888]
 *
 * Каждый символ занимает квадрат FONT_CHAR_SIZE × FONT_CHAR_SIZE пикселей.
 * Пиксели хранятся построчно (как PNG в raw-формате).
 * ------------------------------------------------------------------ */

#define FONT_CHAR_SIZE   64     /* ширина/высота одного символа в пикселях */
#define FONT_CHARS_COUNT 36     /* 10 цифр + 26 букв                        */
#define FONT_MAPS_MAX    8

typedef struct FontChar {
    u32* pixels;  /* указатель на верхний-левый пиксель символа в FontMap  */
    u32  w, h;    /* размер символа (обычно FONT_CHAR_SIZE × FONT_CHAR_SIZE) */
    u32  stride;  /* шаг строки = ширина всей битмапы шрифта (FontMap->w)  */
} FontChar;

typedef struct FontMap {
    u32 w, h;        /* размер всей битмапы в пикселях  */
    u32 char_size;   /* размер одного символа            */
    u32 columns;     /* символов в строке (= w / char_size) */
    u32 pixels[];    /* ARGB8888, w*h элементов (строка за строкой) */
} FontMap;

typedef enum FontMapId {
    FONT_MAP_DEFAULT    = 0,
    FONT_MAP_MINECRAFT  = 1,
} FontMapId;

extern FontMap* font_maps[FONT_MAPS_MAX];

bool assets_load_fontmap(const char* path, FontMapId id);
void assets_font_free();

/* Возвращает индекс символа в сетке: 0-9 → цифры, 10-35 → A-Z.
 * Возвращает -1 для неподдерживаемых символов. */
static inline int assets_font_index(char c)
{
    int nc = SDL_toupper((unsigned char)c);
    if (nc >= '0' && nc <= '9') return nc - '0';
    if (nc >= 'A' && nc <= 'Z') return (nc - 'A') + 10;
    return -1;
}

/* Возвращает FontChar — «вид» на нужный символ в битмапе шрифта.
 * Если шрифт не загружен или символ не поддерживается — pixels == NULL. */
static inline FontChar assets_get_font_char(FontMapId id, char c)
{
    FontChar fc = {NULL, 0, 0, 0};
    if (id >= FONT_MAPS_MAX) return fc;

    FontMap* fm = font_maps[id];
    if (!fm) return fc;

    int idx = assets_font_index(c);
    if (idx < 0 || idx >= FONT_CHARS_COUNT) return fc;

    u32 col = (u32)idx % fm->columns;
    u32 row = (u32)idx / fm->columns;
    u32 x   = col * fm->char_size;
    u32 y   = row * fm->char_size;

    fc.pixels = &fm->pixels[y * fm->w + x];
    fc.w      = fm->char_size;
    fc.h      = fm->char_size;
    fc.stride = fm->w;
    return fc;
}

#endif /* _ASSETS_FONT_H */
