#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "include/SDL3/SDL.h"
#include "conio.h"

#define DLL_EXPORT __declspec(dllexport)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define log(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)
#define EXT ".DIH"

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

DLL_EXPORT
void convert(const char* input, const char* output)
{
    if (!input || !output) { log("Invalid path"); return; }

    SDL_Surface* surface = LoadPNG(input);
    if (!surface) { log("Failed to load png `%s`", input); return; }

    SDL_Surface* conv_surface = ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);
    if (!conv_surface) { log("Failed to convert `%s`", input); DestroySurface(surface); return; }

    DestroySurface(surface);
    surface = conv_surface;

    size_t texture_size  = (size_t)surface->w * (size_t)surface->h;
    size_t texture_struct_size = sizeof(Texture) + texture_size * sizeof(u32);

    Texture* texture = malloc(texture_struct_size);
    if (!texture) { log("malloc failed"); DestroySurface(surface); return; }
    texture->w = surface->w;
    texture->h = surface->h;
    u32 surface_pitch = surface->pitch / sizeof(u32);

    for (int y = 0; y < surface->h; ++y)
        for (int x = 0; x < surface->w; ++x)
            texture->pixels[y * texture->w + x] = ((u32*)surface->pixels)[y * surface_pitch + x];

    DestroySurface(surface);

    FILE *f = fopen(output, "wb");
    if (!f) { log("Failed to create `%s`", output); free(texture); return; }

    size_t write_result = fwrite(texture, 1, texture_struct_size, f);
    if (write_result < texture_struct_size) {
        log("Write error `%s`: expected %zu got %zu", output, texture_struct_size, write_result);
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