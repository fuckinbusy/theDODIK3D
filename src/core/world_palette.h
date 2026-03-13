#ifndef _WORLD_PALETTE_H
#define _WORLD_PALETTE_H
#include "utypes.h"

typedef enum WorldTileType {
    WORLD_TILE_TYPE_AIR,
    WORLD_TILE_TYPE_WALL,
    WORLD_TILE_TYPE_DOOR,
} WorldTileType;

typedef enum WorldTileColor {
    WORLD_TILE_COLOR_EMPTY = 0x00000000,
    WORLD_TILE_COLOR_BLACK = 0xFF000000,
    WORLD_TILE_COLOR_RED = 0xFFFF0000,
    WORLD_TILE_COLOR_GREEN = 0xFF00FF00,
    WORLD_TILE_COLOR_BLUE = 0xFF0000FF,
    WORLD_TILE_COLOR_YELLOW = 0xFFFFFF00,
} WorldTileColor;

u32 world_palette_coltotex(u32 color);
u32 world_palette_coltotype(u32 color);
u32 world_palette_textocol(u32 texture);

#endif // _WORLD_PALETTE_H
