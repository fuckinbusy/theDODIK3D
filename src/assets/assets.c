#include <stdio.h>
#include <malloc.h>
#include <ctype.h>
#include "string.h"
#include "assets.h"

TextureMap* texture_maps[ASSETS_TEXTUREMAPS_MAX] = {0};
static u32 map_counter = 0;

static bool assets_add(TextureMap* texture_map, TextureMapId mid)
{
    texture_maps[mid] = texture_map;
    map_counter++;
    return true;
}

bool assets_load_texmap(const char* path, TextureMapId map_id)
{
    if (!path) return false;
    if (map_id < 0 || map_id >= ASSETS_TEXTURES_MAX)
        return false;

    FILE* f = fopen(path, "rb");
    if (!f) {
        SDL_Log("Failed to open file `%s`", path);
        return false;
    }

    u32 tex_count = 0;
    if (fread(&tex_count, sizeof(u32), 1, f) != 1) {
        SDL_Log("Failed to read file `%s` (Wrong format or file corrupted)", path);
        goto exit_failure;
    }

    TextureMap* tex_map = malloc(sizeof(u32) + (sizeof(Texture*) * tex_count));
    if (!tex_map) {
        SDL_Log("Failed to initialize memory for texture map");
        goto exit_failure;
    }

    memset(tex_map->textures, 0, sizeof(Texture*) * tex_count);

    for (u32 i = 0; i < tex_count; ++i) {
        u32 w = 0;
        u32 h = 0;

        if (fread(&w, sizeof(u32), 1, f) != 1 ||
            fread(&h, sizeof(u32), 1, f) != 1) {
            SDL_Log("Failed to read file `%s` (Wrong format or file corrupted)", path);
            goto exit_failure;
        }

        if (w == 0 || h == 0) goto exit_failure;

        size_t pixels_size = w * h * sizeof(u32);
        size_t struct_size = sizeof(Texture) + pixels_size;

        Texture* texture = (Texture*)malloc(struct_size);
        if (!texture) goto exit_failure;

        size_t read = fread(texture->pixels, sizeof(u8), pixels_size, f);

        if (read != pixels_size) {
            free(texture);
            SDL_Log("Failed to read that DIH `%s`", path);
            goto exit_failure;
        }

        if (!assets_add(tex_map, map_id)) {
            free(texture);
            SDL_Log("Failed to load texture `%s` (Textures buffer full)", path);
            goto exit_failure;
        }
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
        if (texture_maps[i]) { free(texture_maps[i]); texture_maps[i] = NULL; }
}

FontChar assets_font_char(TextureMapId mid, char c)
{
    FontChar fc = {NULL, 0, 0, 0};
    TextureMap* tm = texture_maps[mid];
    if (!tm) return fc;

    int idx = assets_font_index(c);

    if (idx < 0 || idx >= ASSETS_FONT_CHARS_COUNT) return fc;

    u32 columns = 10; // font->w / ASSETS_FONT_CHAR_SIZE;
    u32 x = (idx & (columns - 1)) * ASSETS_FONT_CHAR_SIZE;
    u32 y = (idx / columns) * ASSETS_FONT_CHAR_SIZE;

    fc.pixels = &(tm->textures[0])->pixels[y * tm->w + x];
    fc.w = ASSETS_FONT_CHAR_SIZE;
    fc.h = ASSETS_FONT_CHAR_SIZE;
    fc.stride = tm->w;

    return fc;
}
