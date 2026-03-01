#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "SDL3/SDL.h"

#define DLL_EXPORT __declspec(dllexport)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define log(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)

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

static Texture* texture_from_png(const char* input)
{
    if (!input) {
        log("Invalid input path");
        return NULL;
    }

    SDL_Surface* surface = LoadPNG(input);
    if (!surface) {
        log("Failed to load png `%s`", input);
        return NULL;
    }

    SDL_Surface* conv_surface = ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);
    if (!conv_surface) {
        log("Failed to convert `%s`", input);
        DestroySurface(surface);
        return NULL;
    }

    DestroySurface(surface);
    surface = conv_surface;

    size_t texture_size = (size_t)surface->w * (size_t)surface->h;
    size_t texture_struct_size = sizeof(Texture) + texture_size * sizeof(u32);

    Texture* texture = (Texture*)malloc(texture_struct_size);
    if (!texture) {
        log("malloc failed");
        DestroySurface(surface);
        return NULL;
    }

    texture->w = (u32)surface->w;
    texture->h = (u32)surface->h;

    u32 surface_pitch = (u32)surface->pitch / sizeof(u32);
    for (u32 y = 0; y < texture->h; ++y) {
        for (u32 x = 0; x < texture->w; ++x) {
            texture->pixels[y * texture->w + x] = ((u32*)surface->pixels)[y * surface_pitch + x];
        }
    }

    DestroySurface(surface);
    return texture;
}

static int write_texture(FILE* f, const Texture* texture)
{
    size_t pixel_count = (size_t)texture->w * (size_t)texture->h;

    if (fwrite(&texture->w, sizeof(u32), 1, f) != 1 ||
        fwrite(&texture->h, sizeof(u32), 1, f) != 1) {
        return 0;
    }

    if (fwrite(texture->pixels, sizeof(u32), pixel_count, f) != pixel_count) {
        return 0;
    }

    return 1;
}

DLL_EXPORT
int convert(const char* input, const char* output)
{
    if (!input || !output) {
        log("Invalid path");
        return 0;
    }

    Texture* texture = texture_from_png(input);
    if (!texture) {
        return 0;
    }

    FILE* f = fopen(output, "wb");
    if (!f) {
        log("Failed to create `%s`", output);
        free(texture);
        return 0;
    }

    int ok = write_texture(f, texture);
    fclose(f);
    free(texture);

    if (!ok) {
        log("Write error `%s`", output);
        return 0;
    }

    log("`%s` -> `%s`", input, output);
    return 1;
}

DLL_EXPORT
int convert_many(const char** inputs, u32 count, const char* output)
{
    if (!inputs || !output || count == 0) {
        log("Invalid arguments");
        return 0;
    }

    FILE* f = fopen(output, "wb");
    if (!f) {
        log("Failed to create `%s`", output);
        return 0;
    }

    if (fwrite(&count, sizeof(u32), 1, f) != 1) {
        log("Write error `%s`", output);
        fclose(f);
        return 0;
    }

    for (u32 i = 0; i < count; ++i) {
        Texture* texture = texture_from_png(inputs[i]);
        if (!texture) {
            fclose(f);
            return 0;
        }

        int ok = write_texture(f, texture);
        free(texture);

        if (!ok) {
            log("Write error `%s`", output);
            fclose(f);
            return 0;
        }
    }

    fclose(f);
    log("%u png -> `%s`", count, output);
    return 1;
}

DLL_EXPORT
void initialize(void)
{
    sdl = LoadLibraryW(L".\\lib\\SDL3.dll");
    if (!sdl) {
        log("Failed to load SDL3.dll");
        return;
    }

    ConvertSurface = (SDL_ConvertSurface_t)GetProcAddress(sdl, "SDL_ConvertSurface");
    LoadPNG = (SDL_LoadPNG_t)GetProcAddress(sdl, "SDL_LoadPNG");
    DestroySurface = (SDL_DestroySurface_t)GetProcAddress(sdl, "SDL_DestroySurface");

    if (!ConvertSurface || !LoadPNG || !DestroySurface) {
        log("Failed to resolve SDL symbols");
    }
}

DLL_EXPORT
void deinitialize()
{
    if (sdl) {
        FreeLibrary(sdl);
        sdl = NULL;
    }

    ConvertSurface = NULL;
    LoadPNG = NULL;
    DestroySurface = NULL;
}