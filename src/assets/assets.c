#include <stdio.h>
#include <malloc.h>
#include <ctype.h>
#include "assets.h"

static Texture* textures[ASSETS_TEXTURES_COUNT_MAX];
static u32 textures_counter = 0;

static bool assets_add(Texture *texture, TextureId id)
{
    textures[id] = texture;
    textures_counter++;
    return true;
}

bool assets_load_texture(const char* path, TextureId tex_id)
{
    if (!path) return false;
    if (tex_id < 0 || tex_id >= ASSETS_TEXTURES_COUNT_MAX) 
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
        fclose(f);
        SDL_Log("Failed to read file `%s` (Wrong format or file corrupted)", path);
        return false;
    }

    if (w == 0 || h == 0)
        return false;

    size_t pixels_size = w * h * sizeof(u32);
    size_t struct_size = sizeof(Texture) + pixels_size;

    Texture* texture = (Texture*)malloc(struct_size);

    if (!texture) {
        fclose(f);
        return false;
    }

    texture->w = w;
    texture->h = h;

    size_t read = fread(texture->pixels, sizeof(u8), pixels_size, f);

    if (read != pixels_size) {
        fclose(f);
        free(texture);
        SDL_Log("Failed to read that DIH `%s`", path);
        return false;
    }

    if (!assets_add(texture, tex_id)) {
        fclose(f);
        free(texture);
        SDL_Log("Failed to load texture `%s` (Textures buffer full)", path);
        return false;
    }

    fclose(f);
    SDL_Log("Texture loaded %-*s %ux%u", 55, path, w, h);
    return true;
 
}

Texture* assets_get(TextureId id)
{
    if (id < 0 || id >= ASSETS_TEXTURES_COUNT_MAX) 
        return NULL;

    return textures[id];
}

u32 assets_get_size(TextureId id)
{
    Texture* tex = assets_get(id);
    return tex ? tex->w * tex->h : 0;
}

u32 assets_get_w(TextureId id)
{
    Texture* tex = assets_get(id);
    return tex ? tex->w : 0;
}

u32 assets_get_h(TextureId id)
{
    Texture* tex = assets_get(id);
    return tex ? tex->h : 0;
}

void assets_free()
{
    for (int i = 0; i < textures_counter; ++i) {
        if (textures[i]) {
            free(textures[i]);
            textures[i] = NULL;
        }
    }

    textures_counter = 0;
}

Texture* assets_font_get(TextureId id)
{
    Texture* font = assets_get(id);
    return font ? font : NULL;
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

    fc.pixels = &font->pixels[y * font->w + x];
    fc.w = ASSETS_FONT_CHAR_SIZE;
    fc.h = ASSETS_FONT_CHAR_SIZE;
    fc.stride = font->w;

    return fc;
}
