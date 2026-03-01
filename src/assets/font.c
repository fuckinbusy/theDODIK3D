#include <stdio.h>
#include <stdlib.h>
#include "assets/font.h"

FontMap* font_maps[FONT_MAPS_MAX] = {0};

/*
 * Загружает FontMap из файла .DIHT.
 * Ожидается ровно одна текстура — вся битмапа шрифта целиком.
 *
 * Формат (совпадает с TextureMap, но с одной текстурой):
 *   [textures_count : u32]   (должен быть >= 1; берём только первую)
 *   [w : u32][h : u32][pixels : w * h * 4 байта, ARGB8888]
 */
bool assets_load_fontmap(const char* path, FontMapId id)
{
    if (!path || id >= FONT_MAPS_MAX) return false;

    FILE* f = fopen(path, "rb");
    if (!f) {
        SDL_Log("font: не удалось открыть `%s`", path);
        return false;
    }

    u32 tex_count = 0;
    if (fread(&tex_count, sizeof(u32), 1, f) != 1 || tex_count == 0) {
        SDL_Log("font: неверный формат файла `%s`", path);
        fclose(f);
        return false;
    }

    /* читаем только первую текстуру */
    u32 w = 0, h = 0;
    if (fread(&w, sizeof(u32), 1, f) != 1 ||
        fread(&h, sizeof(u32), 1, f) != 1 ||
        w == 0 || h == 0)
    {
        SDL_Log("font: ошибка чтения размеров в `%s`", path);
        fclose(f);
        return false;
    }

    size_t pixels_size = (size_t)w * h * sizeof(u32);
    FontMap* fm = malloc(sizeof(FontMap) + pixels_size);
    if (!fm) {
        SDL_Log("font: нет памяти для FontMap");
        fclose(f);
        return false;
    }

    if (fread(fm->pixels, 1, pixels_size, f) != pixels_size) {
        SDL_Log("font: ошибка чтения пикселей в `%s`", path);
        free(fm);
        fclose(f);
        return false;
    }

    fclose(f);

    fm->w         = w;
    fm->h         = h;
    fm->char_size = FONT_CHAR_SIZE;
    fm->columns   = (w / FONT_CHAR_SIZE > 0) ? w / FONT_CHAR_SIZE : 1;

    if (font_maps[id]) free(font_maps[id]);
    font_maps[id] = fm;

    SDL_Log("font: загружен FontMap `%s` %ux%u cols=%u", path, w, h, fm->columns);
    return true;
}

void assets_font_free()
{
    for (int i = 0; i < FONT_MAPS_MAX; ++i) {
        if (font_maps[i]) { free(font_maps[i]); font_maps[i] = NULL; }
    }
}
