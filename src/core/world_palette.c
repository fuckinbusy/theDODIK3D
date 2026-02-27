#include "world_palette.h"
#include "assets/assets.h"

u32 world_palette_textocol(u32 texture)
{
    switch (texture) {
        case TEXTURE_TILE_BRICKS:        return WORLD_TILE_COLOR_BLACK;
        case TEXTURE_TILE_BRICKS_SECRET: return WORLD_TILE_COLOR_RED;
        case TEXTURE_TILE_MARKARYAN:     return WORLD_TILE_COLOR_GREEN;
        case TEXTURE_TILE_SAND:          return WORLD_TILE_COLOR_BLUE;
        case TEXTURE_TILE_DOOR:          return WORLD_TILE_COLOR_YELLOW;
        default:                         return WORLD_TILE_COLOR_EMPTY;
    }
}

u32 world_palette_coltotype(u32 color) {
    switch (color) {
        case WORLD_TILE_COLOR_EMPTY:  return WORLD_TILE_TYPE_AIR;
        case WORLD_TILE_COLOR_BLACK:  return WORLD_TILE_TYPE_WALL;
        case WORLD_TILE_COLOR_RED:    return WORLD_TILE_TYPE_WALL;
        case WORLD_TILE_COLOR_GREEN:  return WORLD_TILE_TYPE_WALL;
        case WORLD_TILE_COLOR_BLUE:   return WORLD_TILE_TYPE_WALL;
        case WORLD_TILE_COLOR_YELLOW: return WORLD_TILE_TYPE_DOOR;
        default:                      return WORLD_TILE_TYPE_AIR;
    }
}

u32 world_palette_coltotex(u32 color)
{
    switch (color) {
        case WORLD_TILE_COLOR_BLACK:  return TEXTURE_TILE_BRICKS;
        case WORLD_TILE_COLOR_RED:    return TEXTURE_TILE_BRICKS_SECRET;
        case WORLD_TILE_COLOR_GREEN:  return TEXTURE_TILE_MARKARYAN;
        case WORLD_TILE_COLOR_BLUE:   return TEXTURE_TILE_SAND;
        case WORLD_TILE_COLOR_YELLOW: return TEXTURE_TILE_DOOR;
        default:                      return TEXTURE_TILE_DEFAULT;
    }
}