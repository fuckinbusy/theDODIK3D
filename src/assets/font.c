#include <stdio.h>
#include <stdlib.h>
#include "assets/font.h"

FontMap* font_maps[FONT_MAPS_MAX] = { 0 };

bool assets_load_fontmap(const char* path, FontMapId id)
{
    if (!path || id >= FONT_MAPS_MAX) return false;

    FILE* f = fopen(path, "rb");
    if (!f) {
        SDL_Log("font: failed to open `%s`", path);
        return false;
    }

    u32 tex_count = 0;
    if (fread(&tex_count, sizeof(u32), 1, f) != 1 || tex_count == 0) {
        SDL_Log("font: wrong file format `%s`", path);
        fclose(f);
        return false;
    }

    u32 w = 0, h = 0;
    if (fread(&w, sizeof(u32), 1, f) != 1 ||
        fread(&h, sizeof(u32), 1, f) != 1 ||
        w == 0 || h == 0)
    {
        SDL_Log("font: failed to read sizes `%s`", path);
        fclose(f);
        return false;
    }

    size_t pixels_size = (size_t)w * h * sizeof(u32);
    FontMap* fm = malloc(sizeof(FontMap) + pixels_size);
    if (!fm) {
        SDL_Log("font: not enough memory for FontMap");
        fclose(f);
        return false;
    }

    if (fread(fm->pixels, 1, pixels_size, f) != pixels_size) {
        SDL_Log("font: failed to read `%s`", path);
        free(fm);
        fclose(f);
        return false;
    }

    fclose(f);

    fm->w = w;
    fm->h = h;
    fm->char_size = FONT_CHAR_SIZE;
    fm->columns = (w / FONT_CHAR_SIZE > 0) ? w / FONT_CHAR_SIZE : 1;

    if (font_maps[id]) free(font_maps[id]);
    font_maps[id] = fm;

    SDL_Log("font: loaded FontMap `%s` %ux%u cols=%u", path, w, h, fm->columns);
    return true;
}

void assets_font_free()
{
    for (int i = 0; i < FONT_MAPS_MAX; ++i) {
        if (font_maps[i]) { free(font_maps[i]); font_maps[i] = NULL; }
    }
}