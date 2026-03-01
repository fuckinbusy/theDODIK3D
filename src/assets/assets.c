#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assets.h"
#include "assets/font.h"

TextureMap* texture_maps[ASSETS_TEXTUREMAPS_MAX] = {0};

/* Освобождает одну TextureMap вместе со всеми текстурами внутри */
static void assets_free_texmap(TextureMap* tm)
{
    if (!tm) return;
    for (u32 i = 0; i < tm->textures_count; ++i)
        if (tm->textures[i]) free(tm->textures[i]);
    free(tm);
}

/*
 * Загружает TextureMap из файла .DIHT.
 *
 * Формат файла:
 *   [textures_count : u32]
 *   Для каждой текстуры:
 *     [w : u32][h : u32][pixels : w * h * 4 байта, ARGB8888]
 */
bool assets_load_texmap(const char* path, TextureMapId map_id)
{
    if (!path) return false;
    if (map_id >= ASSETS_TEXTUREMAPS_MAX) return false;

    FILE* f = fopen(path, "rb");
    if (!f) {
        SDL_Log("assets: не удалось открыть `%s`", path);
        return false;
    }

    /* --- читаем количество текстур --- */
    u32 tex_count = 0;
    if (fread(&tex_count, sizeof(u32), 1, f) != 1 || tex_count == 0) {
        SDL_Log("assets: не удалось прочитать кол-во текстур в `%s`", path);
        fclose(f);
        return false;
    }

    /* --- выделяем TextureMap (гибкий массив указателей) --- */
    TextureMap* tex_map = malloc(sizeof(TextureMap) + sizeof(Texture*) * tex_count);
    if (!tex_map) {
        SDL_Log("assets: нет памяти для TextureMap");
        fclose(f);
        return false;
    }
    tex_map->textures_count = tex_count;
    memset(tex_map->textures, 0, sizeof(Texture*) * tex_count);

    /* --- читаем каждую текстуру --- */
    for (u32 i = 0; i < tex_count; ++i) {
        u32 w = 0, h = 0;
        if (fread(&w, sizeof(u32), 1, f) != 1 ||
            fread(&h, sizeof(u32), 1, f) != 1 ||
            w == 0 || h == 0)
        {
            SDL_Log("assets: ошибка чтения размеров текстуры %u в `%s`", i, path);
            goto exit_failure;
        }

        size_t pixels_size = (size_t)w * h * sizeof(u32);
        Texture* texture = malloc(sizeof(Texture) + pixels_size);
        if (!texture) {
            SDL_Log("assets: нет памяти для текстуры %u", i);
            goto exit_failure;
        }

        texture->w = w;
        texture->h = h;

        if (fread(texture->pixels, 1, pixels_size, f) != pixels_size) {
            free(texture);
            SDL_Log("assets: ошибка чтения пикселей текстуры %u в `%s`", i, path);
            goto exit_failure;
        }

        tex_map->textures[i] = texture;
    }

    fclose(f);

    /* заменяем старую карту, если была */
    if (texture_maps[map_id])
        assets_free_texmap(texture_maps[map_id]);
    texture_maps[map_id] = tex_map;

    SDL_Log("assets: загружена TextureMap `%s` (%u текстур)", path, tex_count);
    return true;

exit_failure:
    fclose(f);
    assets_free_texmap(tex_map);
    return false;
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
