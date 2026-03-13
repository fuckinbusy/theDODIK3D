#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "SDL3/SDL.h"
#include "conio.h"

#define DLL_EXPORT __declspec(dllexport)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define log(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)
#define EXT_TEXTURE ".DIHT"
#define EXT_FONT    ".DIHF"
#define EXT_ANIM    ".DIHA"

typedef uint32_t u32;

typedef struct Texture {
    u32 w, h;
    u32 pixels[];
} Texture;

typedef SDL_Surface* (*SDL_ConvertSurface_t)(SDL_Surface*, SDL_PixelFormat);
typedef SDL_Surface* (*SDL_LoadPNG_t)(const char*);
typedef void (*SDL_DestroySurface_t)(SDL_Surface*);

static HMODULE sdl = NULL;
static SDL_ConvertSurface_t ConvertSurface = NULL;
static SDL_LoadPNG_t LoadPNG = NULL;
static SDL_DestroySurface_t DestroySurface = NULL;

static Texture* create_texture(const char* png_path, size_t *struct_size)
{
    if (!png_path) return NULL;

    SDL_Surface* surface = LoadPNG(png_path);
    if (!surface) { log("Failed to load png `%s`", png_path); return NULL; }

    SDL_Surface* conv_surface = ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);
    if (!conv_surface) { log("Failed to convert `%s`", png_path); DestroySurface(surface); return NULL; }

    DestroySurface(surface);
    surface = conv_surface;

    size_t texture_size  = (size_t)surface->w * (size_t)surface->h;
    size_t texture_struct_size = sizeof(Texture) + texture_size * sizeof(u32);
    *struct_size = texture_struct_size;

    Texture* texture = malloc(texture_struct_size);
    if (!texture) { log("malloc failed"); DestroySurface(surface); return NULL; }
    texture->w = surface->w;
    texture->h = surface->h;
    u32 surface_pitch = surface->pitch / sizeof(u32);

    for (int y = 0; y < surface->h; ++y)
        for (int x = 0; x < surface->w; ++x)
            texture->pixels[y * texture->w + x] = ((u32*)surface->pixels)[y * surface_pitch + x];

    DestroySurface(surface);

    return texture;
}

DLL_EXPORT
void convert(const char* input, const char* output)
{
    if (!input || !output) { log("Invalid path"); return; }

    size_t struct_size = 0;
    Texture* texture = create_texture(input, &struct_size);

    FILE *f = fopen(output, "wb");
    if (!f) { log("Failed to create `%s`", output); free(texture); return; }

    size_t write_result = fwrite(texture, 1, struct_size, f);
    if (write_result < struct_size) {
        log("Write error `%s`: expected %zu got %zu", output, struct_size, write_result);
        fclose(f); 
        free(texture);
        return;
    }

    log("`%s` -> `%s`", input, output);
    fclose(f);
    free(texture);
}

DLL_EXPORT
void initialize(void)
{
    sdl = LoadLibraryW(L".\\lib\\SDL3.dll");
    if (!sdl) {
        log("Failed to load SDL3.dll");
        return;
    }

    ConvertSurface   = (SDL_ConvertSurface_t)GetProcAddress(sdl, "SDL_ConvertSurface");
    LoadPNG          = (SDL_LoadPNG_t)GetProcAddress(sdl, "SDL_LoadPNG");
    DestroySurface   = (SDL_DestroySurface_t)GetProcAddress(sdl, "SDL_DestroySurface");
}

DLL_EXPORT
void deinitialize()
{
    if (sdl) FreeLibrary(sdl);
}