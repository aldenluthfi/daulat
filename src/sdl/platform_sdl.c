//! platform_sdl.c
//!
//! SDL3-backed implementation of platform.h. Used by the SDL
//! frontend so its save-path resolution and time queries match
//! SDL3's conventions on every platform.
//!
//! Created: 2026-06-15
//! Author : Alden Luthfi

#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "platform.h"

/*--------------------------------------------------------------------------*\
                              PREF PATH
\*--------------------------------------------------------------------------*/

static char PREF_PATH[1024];
static bool PREF_PATH_READY = false;

const char* platform_pref_path(const char* org, const char* application) {
    if (PREF_PATH_READY)
        return PREF_PATH;
    char* pref = SDL_GetPrefPath(org, application);
    if (pref == NULL)
        return NULL;
    SDL_snprintf(PREF_PATH, sizeof(PREF_PATH), "%s", pref);
    SDL_free(pref);
    PREF_PATH_READY = true;
    return PREF_PATH;
}

uint64_t platform_time_ns(void) {
    return SDL_GetTicksNS();
}

/*--------------------------------------------------------------------------*\
                              FILE I/O
\*--------------------------------------------------------------------------*/

struct PlatformStream {
    SDL_IOStream* io;
};

static PlatformStream* stream_open(const char* path, const char* mode) {
    SDL_IOStream* io = SDL_IOFromFile(path, mode);
    if (io == NULL)
        return NULL;
    PlatformStream* stream = malloc(sizeof(*stream));
    if (stream == NULL) {
        SDL_CloseIO(io);
        return NULL;
    }
    stream->io = io;
    return stream;
}

PlatformStream* platform_open_write(const char* path) {
    return stream_open(path, "wb");
}

PlatformStream* platform_open_read(const char* path) {
    return stream_open(path, "rb");
}

size_t platform_read(PlatformStream* stream, void* buffer, size_t capacity) {
    if (stream == NULL || stream->io == NULL)
        return 0;
    return SDL_ReadIO(stream->io, buffer, capacity);
}

size_t platform_write(
    PlatformStream* stream,
    const void*     buffer,
    size_t          count
) {
    if (stream == NULL || stream->io == NULL)
        return 0;
    return SDL_WriteIO(stream->io, buffer, count);
}

int64_t platform_size(PlatformStream* stream) {
    if (stream == NULL || stream->io == NULL)
        return -1;
    return (int64_t)SDL_GetIOSize(stream->io);
}

void platform_close(PlatformStream* stream) {
    if (stream == NULL)
        return;
    if (stream->io != NULL)
        SDL_CloseIO(stream->io);
    free(stream);
}

bool platform_remove(const char* path) {
    return SDL_RemovePath(path);
}
