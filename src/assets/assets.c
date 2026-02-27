#include <stdio.h>
#include <malloc.h>
#include <ctype.h>
#include "assets.h"

static Texture*         textures[ASSETS_TEXTURES_COUNT_MAX];
static AnimatedTexture* anims[ASSETS_ANIMS_COUNT_MAX];
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

    if (w == 0 || h == 0) {
        fclose(f);
        return false;
    }

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
    // Iterate the full range — textures are indexed by TextureId, not by insertion order.
    for (int i = 0; i < ASSETS_TEXTURES_COUNT_MAX; ++i) {
        if (textures[i]) {
            free(textures[i]);
            textures[i] = NULL;
        }
    }
    for (int i = 0; i < ASSETS_ANIMS_COUNT_MAX; ++i) {
        if (anims[i]) {
            free(anims[i]);
            anims[i] = NULL;
        }
    }

    textures_counter = 0;
}

bool assets_load_anim(const char* path, AnimId id, float frame_duration)
{
    if (!path) return false;
    if (id < 0 || id >= ASSETS_ANIMS_COUNT_MAX) return false;

    FILE* f = fopen(path, "rb");
    if (!f) {
        SDL_Log("Failed to open animation file `%s`", path);
        return false;
    }

    u32 w = 0, h = 0, frame_count = 0;

    if (fread(&w,           sizeof(u32), 1, f) != 1 ||
        fread(&h,           sizeof(u32), 1, f) != 1 ||
        fread(&frame_count, sizeof(u32), 1, f) != 1) {
        fclose(f);
        SDL_Log("Failed to read animation header `%s` (Wrong format or file corrupted)", path);
        return false;
    }

    if (w == 0 || h == 0 || frame_count == 0) {
        fclose(f);
        SDL_Log("Invalid animation dimensions in `%s`", path);
        return false;
    }

    size_t frame_pixels = (size_t)w * h;
    size_t pixels_size  = frame_pixels * frame_count * sizeof(u32);
    AnimatedTexture* anim = (AnimatedTexture*)malloc(sizeof(AnimatedTexture) + pixels_size);
    if (!anim) {
        fclose(f);
        return false;
    }

    anim->w              = w;
    anim->h              = h;
    anim->frame_count    = frame_count;
    anim->current_frame  = 0;
    anim->frame_duration = frame_duration;
    anim->frame_timer    = 0.0f;

    if (fread(anim->pixels, sizeof(u32), frame_pixels * frame_count, f) != frame_pixels * frame_count) {
        fclose(f);
        free(anim);
        SDL_Log("Failed to read animation pixels `%s`", path);
        return false;
    }

    anims[id] = anim;
    fclose(f);
    SDL_Log("Animation loaded %-*s %ux%u x%u frames", 55, path, w, h, frame_count);
    return true;
}

AnimatedTexture* assets_anim_get(AnimId id)
{
    if (id < 0 || id >= ASSETS_ANIMS_COUNT_MAX)
        return NULL;

    return anims[id];
}

void assets_update(float dt)
{
    for (int i = 0; i < ASSETS_ANIMS_COUNT_MAX; ++i) {
        AnimatedTexture* anim = anims[i];
        if (!anim) continue;

        anim->frame_timer += dt;
        if (anim->frame_timer >= anim->frame_duration) {
            anim->frame_timer  -= anim->frame_duration;
            anim->current_frame = (anim->current_frame + 1) % anim->frame_count;
        }
    }
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
