#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "core/player.h"
#include "core/world.h"
#include "engine/raycast.h"
#include "assets/assets.h"
#include "assets/font.h"
#include "game/game_state.h"
#include "math/gmath.h"
#include "core/entity.h"

FrameBuffer* frame_buffer = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* frame_texture = NULL;
static float* zbuffer = NULL;
static float* shade_table = NULL;

static void render_draw_plane(int x, int y_start, int y_finish, u32 color)
{
    for (int y = y_start; y < y_finish; ++y)
        render_buffer_put_pixel(x, y, render_apply_shade(color, shade_table[y]));
}

static void render_draw_wall(int x, int y1, int y2, int wall_len,
    int screen_h, RayResult* ray, TextureId tex_id)
{
    Texture* texture = assets_get_texture(TEXTURE_MAP_TILES, tex_id);
    if (!texture) {
        texture = assets_get_texture(TEXTURE_MAP_TILES, TEXTURE_TILE_DEFAULT);
    }

    int texture_x = (int)gmath_clamp(ray->u * texture->w, 0.0f, (float)(texture->w - 1));

    if ((ray->side == 1 && ray->dy > 0) || (ray->side == 0 && ray->dx < 0))
        texture_x = (int)texture->w - texture_x - 1;

    float texture_shade = 1.0f / (RENDER_SHADE_BASE + ray->side + ray->dist);
    float texture_step = (float)texture->h / wall_len;
    float texture_pos = (y1 - (float)screen_h * 0.5f + (float)wall_len * 0.5f) * texture_step;

    for (int y = y1; y <= y2; ++y) {
        int texture_y = (int)texture_pos;
        if (texture_y < 0)                 texture_y += (int)texture->h;
        if (texture_y >= (int)texture->h)  texture_y = (int)texture->h - 1;

        u32 wall_color = texture->pixels[texture_y * texture->w + texture_x];
        render_buffer_put_pixel(x, y, render_apply_shade(wall_color, texture_shade));
        texture_pos += texture_step;
    }
}

static void render_draw_entities(GameState* gs, Vec2 camera_plane, float proj_dist)
{
    if (!gs) return;

    EntityPool* pool = gs->entities;
    if (!pool || pool->size == 0) return;

    Player* player = &gs->player;

    for (int i = 0; i < (int)pool->size; ++i) {
        Entity* ent = &pool->entities[i];
        if (!ent) continue;

        Texture* texture = assets_get_texture(TEXTURE_MAP_ENEMY, ent->texture_id);
        if (!texture) continue;

        Vec2 d = gmath_vec2_sub(ent->pos, player->pos);
        Vec2 dir = gmath_direction(player->angle);

        float inv_det = 1.0f / (camera_plane.x * dir.y - dir.x * camera_plane.y);
        float transform_x = inv_det * (d.x * dir.y - d.y * dir.x);
        float transform_y = inv_det * (-camera_plane.y * d.x + d.y * camera_plane.x);
        if (transform_y < RENDER_SPRITE_CULL_DIST * gs->zoom) continue;

        float shade = 1.0f / (transform_y + RENDER_SHADE_BASE);
        float inv_ty = 1.0f / transform_y;

        int screen_x = (int)((gs->game_w * 0.5f) * (1.0f + transform_x * inv_ty));
        int sprite_h = (int)fabsf(proj_dist * inv_ty * RENDER_SPRITE_HEIGHT_SCALE);
        int sprite_w = sprite_h;
        int sprite_center = (int)(gs->game_h * 0.5f + proj_dist * inv_ty * RENDER_SPRITE_OFFSET_SCALE);

        float tex_step_x = (float)texture->w / sprite_w;
        float tex_step_y = (float)texture->h / sprite_h;

        int start_x = (int)(screen_x - sprite_w * 0.5f);
        int stop_x = (int)(screen_x + sprite_w * 0.5f);
        int start_y = (int)(sprite_center - sprite_h * 0.5f);
        int stop_y = (int)(sprite_center + sprite_h * 0.5f);

        int draw_start_x = start_x;
        int draw_start_y = start_y;

        if (start_x < 0) start_x = 0;
        if (start_y < 0) start_y = 0;

        float tex_pos_y = (start_y - draw_start_y) * tex_step_y;
        for (int y = start_y; y < stop_y; ++y) {
            if (y >= (int)gs->game_h) break;
            int tex_y = (int)gmath_clamp(tex_pos_y, 0.0f, (float)(texture->h - 1));

            float tex_pos_x = (start_x - draw_start_x) * tex_step_x;
            for (int x = start_x; x < stop_x; ++x) {
                if (x >= (int)gs->game_w) break;
                int tex_x = (int)gmath_clamp(tex_pos_x, 0.0f, (float)(texture->w - 1));
                u32 color = texture->pixels[tex_y * texture->w + tex_x];

                if (transform_y < zbuffer[x]) {
                    if (render_has_alpha(color))
                        render_buffer_put_pixel(x, y, render_apply_shade(color, shade));
                }
                tex_pos_x += tex_step_x;
            }
            tex_pos_y += tex_step_y;
        }
    }
}

void render_update(GameState* gs, int fov)
{
    World* world = world_get(gs->current_world);
    if (!world) return;
    Player* player = &gs->player;
    u32     width = gs->game_w;
    u32     height = gs->game_h;

    float half_h = (float)height * 0.5f;
    float half_w = (float)width * 0.5f;
    float fov_rad = MATH_DEG_TO_RAD((float)fov / gs->zoom);
    float half_fov_tan = tanf(fov_rad * 0.5f);
    float proj_dist = half_w / half_fov_tan;

    Vec2 dir = gmath_direction(player->angle);

    float planeX = -dir.y * half_fov_tan;
    float planeY = dir.x * half_fov_tan;

    float inv_w = 1.0f / width;
    for (int x = 0; x < (int)width; ++x) {
        float u = (float)x * inv_w;
        float cameraX = u * 2.0f - 1.0f;

        Vec2 ray_dir = {
            .x = dir.x + planeX * cameraX,
            .y = dir.y + planeY * cameraX
        };

        RayResult ray = raycast_ray(world, player->pos, ray_dir);
        zbuffer[x] = ray.dist;

        int line_h = (int)(proj_dist / ray.dist);
        int y1 = (int)(-line_h * 0.5f + half_h);
        int y2 = (int)(line_h * 0.5f + half_h);

        if (y1 < 0)             y1 = 0;
        if (y2 >= (int)height)  y2 = (int)height - 1;

        render_draw_plane(x, 0, y1, RENDER_CEILING_COLOR);
        render_draw_wall(x, y1, y2, line_h, (int)height, &ray,
            world_tile_texture(world, ray.map_x, ray.map_y));
        render_draw_plane(x, y2, (int)height, RENDER_FLOOR_COLOR);
    }

    render_draw_entities(gs, gmath_vec2(planeX, planeY), proj_dist);
}

bool render_init(GameState* gs)
{
    u32 size = gs->window_w * gs->window_h;
    frame_buffer = malloc(sizeof(FrameBuffer) + size * sizeof(u32));
    if (!frame_buffer) return false;

    frame_buffer->w = gs->window_w;
    frame_buffer->h = gs->window_h;
    frame_buffer->size = size;
    renderer = gs->renderer;

    frame_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        frame_buffer->w,
        frame_buffer->h);
    if (!frame_texture) return false;

    shade_table = malloc(gs->game_h * sizeof(float));
    if (shade_table) {
        float half_screen = (float)gs->game_h * 0.5f;
        for (int y = 0; y < (int)gs->game_h; ++y) {
            float dist = half_screen / (y - half_screen);
            shade_table[y] = 1.0f / (RENDER_SHADE_BASE + dist);
        }
    }
    else {
        SDL_Log("render: not enough memory for shade table");
    }

    zbuffer = malloc(gs->game_w * sizeof(float));
    if (!zbuffer) SDL_Log("render: not enough memory for zbuffer");

    SDL_Log("render: initialized");
    return true;
}

void render_free()
{
    if (frame_buffer) { free(frame_buffer);  frame_buffer = NULL; }
    if (zbuffer) { free(zbuffer);       zbuffer = NULL; }
    if (shade_table) { free(shade_table);   shade_table = NULL; }
    if (frame_texture) { SDL_DestroyTexture(frame_texture); frame_texture = NULL; }
}

void render_player(Player* player)
{
    if (!player || !renderer) return;

    SDL_FRect pr = { player->pos.x - 8.0f, player->pos.y - 8.0f, 16.0f, 16.0f };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &pr);

    Vec2 d = gmath_direction(player->angle);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderLine(renderer,
        player->pos.x, player->pos.y,
        player->pos.x + d.x * 25.0f, player->pos.y + d.y * 25.0f);
}

void render_present()
{
    if (!renderer || !frame_buffer || !frame_buffer->buffer) return;

    SDL_UpdateTexture(frame_texture, NULL,
        frame_buffer->buffer,
        (int)(frame_buffer->w * sizeof(u32)));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, frame_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

u32 render_apply_shade(u32 color, float shade)
{
    shade = gmath_clamp(shade, 0.0f, 1.0f);

    u8 a = (color >> 24) & 0xFF;
    u8 r = (color >> 16) & 0xFF;
    u8 g = (color >> 8) & 0xFF;
    u8 b = color & 0xFF;

    r = (u8)(r * shade);
    g = (u8)(g * shade);
    b = (u8)(b * shade);

    return ((u32)a << 24) | ((u32)r << 16) | ((u32)g << 8) | (u32)b;
}

void render_draw_texture(int x_start, int y_start, int w, int h,
    TextureMapId mid, TextureId tid, bool mirrored)
{
    Texture* texture = assets_get_texture(mid, tid);
    if (!texture) return;

    float tex_step_x = (float)texture->w / w;
    float tex_step_y = (float)texture->h / h;
    if (mirrored) tex_step_x = -tex_step_x;

    float tex_pos_y = 0.0f;
    for (int y = y_start; y < y_start + h; ++y) {
        int tex_y = (int)gmath_clamp(tex_pos_y, 0.0f, (float)(texture->h - 1));

        float tex_pos_x = mirrored ? (float)(texture->w - 1) : 0.0f;
        for (int x = x_start; x < x_start + w; ++x) {
            int tex_x = (int)gmath_clamp(tex_pos_x, 0.0f, (float)(texture->w - 1));
            u32 color = texture->pixels[tex_y * texture->w + tex_x];
            if (render_has_alpha(color))
                render_buffer_put_pixel(x, y, color);
            tex_pos_x += tex_step_x;
        }
        tex_pos_y += tex_step_y;
    }
}

static void render_draw_font_glyph(FontChar fc, Vec2 pos, int size,
    u32 color, int screen_w, int screen_h)
{
    float font_step_x = (float)fc.w / size;
    float font_step_y = (float)fc.h / size;
    float font_pos_y = 0.0f;

    for (int y = (int)pos.y; y < (int)pos.y + size; ++y) {
        if (y < 0) { font_pos_y += font_step_y; continue; }
        if (y >= screen_h) break;

        int   font_y = (int)font_pos_y;
        float font_pos_x = 0.0f;

        for (int x = (int)pos.x; x < (int)pos.x + size; ++x) {
            if (x < 0) { font_pos_x += font_step_x; continue; }
            if (x >= screen_w) break;

            int font_x = (int)font_pos_x;
            u32 pixel = fc.pixels[font_y * fc.stride + font_x];
            if (render_has_alpha(pixel))
                render_buffer_put_pixel(x, y, color);
            font_pos_x += font_step_x;
        }
        font_pos_y += font_step_y;
    }
}

void render_draw_font(Vec2 start, int screen_w, int screen_h,
    int size, u32 color, FontMapId mid, const char* str)
{
    if (!str) return;
    if (mid < 0 || mid >= FONT_MAPS_MAX || !font_maps[mid]) return;

    start.y -= size;
    int x_origin = (int)start.x;

    while (*str) {
        if (*str == '\n') {
            str++;
            start.x = (float)x_origin;
            start.y += (float)size * 1.1f;
            continue;
        }

        FontChar fc = assets_get_font_char(mid, *str);
        if (!fc.pixels) {
            str++;
            start.x += size;
            continue;
        }

        render_draw_font_glyph(fc, start, size, color, screen_w, screen_h);

        str++;
        start.x += size;
    }
}