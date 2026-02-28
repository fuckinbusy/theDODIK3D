#include <stdio.h>
#include <malloc.h>
#include <ctype.h>
#include "assets.h"

TextureMap* map_buffer[ASSETS_TEXTUREMAPS_MAX] = {0};
static u32 map_counter = 0;

static bool assets_add(TextureMap* texture_map, TextureMapId mid)
{
    map_buffer[mid] = texture_map;
    map_counter++;
    return true;
}

bool assets_load_texmap(const char* path, TextureMapId map_id, u32 texture_size)
{
    if (!path) return false;
    if (map_id < 0 || map_id >= ASSETS_TEXTURES_MAX)
        return false;

    FILE* f = fopen(path, "rb");
    if (!f) {
        SDL_Log("Failed to open file `%s`", path);
        return false;
    }

    u32 w = 0;
    u32 h = 0;

    if (fread(&w, sizeof(u32), 1, f) != 1||
        fread(&h, sizeof(u32), 1, f) != 1) {
        SDL_Log("Failed to read file `%s` (Wrong format or file corrupted)", path);
        goto exit_failure;
    }

    if (w == 0 || h == 0) goto exit_failure;

    size_t pixels_size = w * h * sizeof(u32);
    size_t struct_size = sizeof(TextureMap) + pixels_size;

    TextureMap* map = (TextureMap*)malloc(struct_size);
    if (!map) goto exit_failure;

    map->w = w;
    map->h = h;
    map->texture_size = texture_size;

    size_t read = fread(map->pixels, sizeof(u8), pixels_size, f);

    if (read != pixels_size) {
        free(map);
        SDL_Log("Failed to read that DIH `%s`", path);
        goto exit_failure;
    }

    if (!assets_add(map, map_id)) {
        free(map);
        SDL_Log("Failed to load texture `%s` (Textures buffer full)", path);
        goto exit_failure;
    }

    fclose(f);
    SDL_Log("Texture loaded %-*s %ux%u", 55, path, w, h);
    return true;
 
exit_failure:
    fclose(f);
    return false;
}

void assets_free()
{
    for (int i = 0; i < ASSETS_TEXTUREMAPS_MAX; ++i)
        if (map_buffer[i]) { free(map_buffer[i]); map_buffer[i] = NULL; }
}

FontChar assets_font_char(Texture* font, char c)
{
    FontChar fc = {NULL, 0, 0, 0};
    if (!font) return fc;

    int idx = assets_font_index(c);

    if (idx < 0 || idx >= ASSETS_FONT_CHARS_COUNT) return fc;

    u32 columns = 10; // font->w / ASSETS_FONT_CHAR_SIZE;
    u32 col = idx % columns;
    u32 row = idx / columns;
    u32 x = col * ASSETS_FONT_CHAR_SIZE;
    u32 y = row * ASSETS_FONT_CHAR_SIZE;

    fc.pixels = &font->pixels[y * font->size + x];
    fc.w = ASSETS_FONT_CHAR_SIZE;
    fc.h = ASSETS_FONT_CHAR_SIZE;
    fc.stride = font->size;

    return fc;
}
