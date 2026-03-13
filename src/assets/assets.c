#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "assets.h"
#include "assets/font.h"

TextureMap* texture_maps[ASSETS_TEXTUREMAPS_MAX] = { 0 };

static void assets_free_texmap(TextureMap* tm)
{
    if (!tm) return;
    for (u32 i = 0; i < tm->textures_count; ++i)
        if (tm->textures[i]) free(tm->textures[i]);
    free(tm);
}

typedef struct { u32 w, h; } TextureMapMeta;

static const TextureMapMeta s_tex_meta[ASSETS_TEXTUREMAPS_MAX] = {
    [TEXTURE_MAP_TILES]  = { ASSETS_TEXTURE_WIDTH_TILE,   ASSETS_TEXTURE_HEIGHT_TILE   },
    [TEXTURE_MAP_DODIK]  = { ASSETS_TEXTURE_WIDTH_DODIK,  ASSETS_TEXTURE_HEIGHT_DODIK  },
    [TEXTURE_MAP_WEAPON] = { ASSETS_TEXTURE_WIDTH_WEAPON, ASSETS_TEXTURE_HEIGHT_WEAPON },
    [TEXTURE_MAP_UI_BG]  = { ASSETS_TEXTURE_WIDTH_BG,     ASSETS_TEXTURE_HEIGHT_BG     },
    [TEXTURE_MAP_ENEMY]  = { ASSETS_TEXTURE_WIDTH_ENEMY,  ASSETS_TEXTURE_HEIGHT_ENEMY  },
};

static bool assets_read_atlas(FILE* file, TextureMap* tex_map, u32 tex_w, u32 tex_h)
{
    size_t pixels_total = tex_map->h * tex_map->w;
    u32* pixels_atlas = malloc(pixels_total * sizeof(u32));
    if (!pixels_atlas) return false;

    if (fread(pixels_atlas, sizeof(u32), pixels_total, file) != pixels_total) {
        free(pixels_atlas);
        return false;
    }

    u32 cols = tex_map->w / tex_w;
    for (u32 tex_i = 0; tex_i < tex_map->textures_count; ++tex_i) {
        u32 tile_x = tex_i % cols;
        u32 tile_y = tex_i / cols;

        Texture* texture = malloc(sizeof(Texture) + tex_w * tex_h * sizeof(u32));
        if (!texture) { free(pixels_atlas); return false; }

        texture->w = tex_w;
        texture->h = tex_h;

        for (u32 row = 0; row < tex_h; ++row) {
            u32 atlas_y = tile_y * tex_h + row;
            u32 atlas_x = tile_x * tex_w;

            memcpy(
                &texture->pixels[row * tex_w],
                &pixels_atlas[atlas_y * tex_map->w + atlas_x],
                tex_w * sizeof(u32)
            );
        }

        tex_map->textures[tex_i] = texture;
    }

    return true;
}

bool assets_load_texmap(const char* path, TextureMapId map_id)
{
    if (!path) return false;
    if (map_id < 0 || map_id >= ASSETS_TEXTUREMAPS_MAX) return false;

    FILE* f = fopen(path, "rb");
    if (!f) {
        SDL_Log("assets: failed to open `%s`", path);
        return false;
    }

    u32 map_w = 0;
    u32 map_h = 0;
    if (fread(&map_w, sizeof(u32), 1, f) != 1 ||
        fread(&map_h, sizeof(u32), 1, f) != 1) {
        SDL_Log("assets: failed to read texture map sizes in `%s`", path);
        fclose(f);
        return false;
    }

    u32 tex_w = s_tex_meta[map_id].w;
    u32 tex_h = s_tex_meta[map_id].h;
    u32 cols = map_w / tex_w;
    u32 rows = map_h / tex_h;
    u32 tex_count = cols * rows;

    TextureMap* tex_map = malloc(sizeof(TextureMap) + sizeof(Texture*) * tex_count);
    if (!tex_map) {
        SDL_Log("assets: not enough memory for TextureMap");
        fclose(f);
        return false;
    }
    
    tex_map->h = map_h;
    tex_map->w = map_w;
    tex_map->textures_count = tex_count;

    memset(tex_map->textures, 0, sizeof(Texture*) * tex_count);

    if (!assets_read_atlas(f, tex_map, tex_w, tex_h)) {
        SDL_Log("assets: failed to read texture map textures `%s`", path);
        fclose(f);
        assets_free_texmap(tex_map);
        return false;
    }

    if (texture_maps[map_id])
        assets_free_texmap(texture_maps[map_id]);
    texture_maps[map_id] = tex_map;

    SDL_Log("assets: loaded TextureMap `%s` (%u textures)", path, tex_count);
    return true;
}

void assets_free()
{
    for (int i = 0; i < ASSETS_TEXTUREMAPS_MAX; ++i) {
        if (texture_maps[i]) {
            assets_free_texmap(texture_maps[i]);
            texture_maps[i] = NULL;
        }
    }
    assets_font_free();
}