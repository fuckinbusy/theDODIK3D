#include "world_palette.h"
#include "assets/assets.h"

/* Единая таблица соответствий: цвет ↔ тип тайла ↔ текстура.
   Добавить новый тип тайла = одна строка здесь вместо трёх switch-ей. */
typedef struct {
    u32 color;
    u32 tile_type;
    u32 texture_id;
} WorldPaletteEntry;

static const WorldPaletteEntry s_palette[] = {
    { WORLD_TILE_COLOR_BLACK,  WORLD_TILE_TYPE_WALL, TEXTURE_TILE_BRICKS        },
    { WORLD_TILE_COLOR_RED,    WORLD_TILE_TYPE_WALL, TEXTURE_TILE_BRICKS_SECRET },
    { WORLD_TILE_COLOR_GREEN,  WORLD_TILE_TYPE_WALL, TEXTURE_TILE_MARKARYAN     },
    { WORLD_TILE_COLOR_BLUE,   WORLD_TILE_TYPE_WALL, TEXTURE_TILE_SAND          },
    { WORLD_TILE_COLOR_YELLOW, WORLD_TILE_TYPE_DOOR, TEXTURE_TILE_DOOR          },
};

#define PALETTE_SIZE ((u32)(sizeof(s_palette) / sizeof(s_palette[0])))

u32 world_palette_coltotype(u32 color)
{
    for (u32 i = 0; i < PALETTE_SIZE; ++i)
        if (s_palette[i].color == color) return s_palette[i].tile_type;
    return WORLD_TILE_TYPE_AIR;
}

u32 world_palette_coltotex(u32 color)
{
    for (u32 i = 0; i < PALETTE_SIZE; ++i)
        if (s_palette[i].color == color) return s_palette[i].texture_id;
    return TEXTURE_TILE_DEFAULT;
}

u32 world_palette_textocol(u32 texture)
{
    for (u32 i = 0; i < PALETTE_SIZE; ++i)
        if (s_palette[i].texture_id == texture) return s_palette[i].color;
    return WORLD_TILE_COLOR_EMPTY;
}
