//! platform_posix.c
//!
//! POSIX implementation of platform.h backed by libc + a couple of
//! POSIX syscalls. Linked only into the headless engine binary;
//! the SDL frontend links src/sdl/platform_sdl.c instead.
//!
//! Pref-path layout matches SDL3's defaults so save files written
//! by the SDL frontend round-trip through the engine on the same
//! machine:
//!
//!   macOS  : $HOME/Library/Application Support/<org>/<app>/
//!   Linux  : $XDG_DATA_HOME/<org>/<app>/ (or $HOME/.local/share/...)
//!
//! Created: 2026-06-15
//! Author : Alden Luthfi

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              PREF PATH
\*--------------------------------------------------------------------------*/

static char PREF_PATH[1024];
static bool PREF_PATH_READY = false;

/// Create `path` and every missing parent directory along the way.
/// Returns true if the leaf exists at the end.
static bool ensure_directory(const char* path) {
    char   buffer[1024];
    size_t length = strlen(path);
    if (length >= sizeof(buffer))
        return false;
    memcpy(buffer, path, length + 1);
    for (size_t i = 1; i < length; i++) {
        if (buffer[i] != '/')
            continue;
        buffer[i] = '\0';
        if (mkdir(buffer, 0700) != 0 && errno != EEXIST)
            return false;
        buffer[i] = '/';
    }
    if (mkdir(buffer, 0700) != 0 && errno != EEXIST)
        return false;
    return true;
}

const char* platform_pref_path(const char* org, const char* application) {
    if (PREF_PATH_READY)
        return PREF_PATH;
    const char* home = getenv("HOME");
    if (home == NULL || home[0] == '\0') {
        log_err("platform_pref_path: HOME not set");
        return NULL;
    }
#ifdef __APPLE__
    snprintf(
        PREF_PATH,
        sizeof(PREF_PATH),
        "%s/Library/Application Support/%s/%s/",
        home,
        org,
        application
    );
#else
    const char* xdg = getenv("XDG_DATA_HOME");
    if (xdg != NULL && xdg[0] != '\0')
        snprintf(
            PREF_PATH,
            sizeof(PREF_PATH),
            "%s/%s/%s/",
            xdg,
            org,
            application
        );
    else
        snprintf(
            PREF_PATH,
            sizeof(PREF_PATH),
            "%s/.local/share/%s/%s/",
            home,
            org,
            application
        );
#endif
    if (!ensure_directory(PREF_PATH)) {
        log_err(
            "platform_pref_path: mkdir %s failed (%s)",
            PREF_PATH,
            strerror(errno)
        );
        return NULL;
    }
    PREF_PATH_READY = true;
    return PREF_PATH;
}

/*--------------------------------------------------------------------------*\
                              TIME
\*--------------------------------------------------------------------------*/

uint64_t platform_time_ns(void) {
    struct timespec timespec_now;
#ifdef CLOCK_MONOTONIC
    clock_gettime(CLOCK_MONOTONIC, &timespec_now);
#else
    clock_gettime(CLOCK_REALTIME, &timespec_now);
#endif
    return (uint64_t)timespec_now.tv_sec * 1000000000ULL +
           (uint64_t)timespec_now.tv_nsec;
}

/*--------------------------------------------------------------------------*\
                              FILE I/O
\*--------------------------------------------------------------------------*/

struct PlatformStream {
    FILE* fp;
};

static PlatformStream* stream_open(const char* path, const char* mode) {
    FILE* fp = fopen(path, mode);
    if (fp == NULL)
        return NULL;
    PlatformStream* stream = malloc(sizeof(*stream));
    if (stream == NULL) {
        fclose(fp);
        return NULL;
    }
    stream->fp = fp;
    return stream;
}

PlatformStream* platform_open_write(const char* path) {
    return stream_open(path, "wb");
}

PlatformStream* platform_open_read(const char* path) {
    return stream_open(path, "rb");
}

size_t platform_read(PlatformStream* stream, void* buffer, size_t capacity) {
    if (stream == NULL || stream->fp == NULL)
        return 0;
    return fread(buffer, 1, capacity, stream->fp);
}

size_t
platform_write(PlatformStream* stream, const void* buffer, size_t count) {
    if (stream == NULL || stream->fp == NULL)
        return 0;
    return fwrite(buffer, 1, count, stream->fp);
}

int64_t platform_size(PlatformStream* stream) {
    if (stream == NULL || stream->fp == NULL)
        return -1;
    long origin = ftell(stream->fp);
    if (origin < 0)
        return -1;
    if (fseek(stream->fp, 0, SEEK_END) != 0)
        return -1;
    long end = ftell(stream->fp);
    if (end < 0)
        return -1;
    if (fseek(stream->fp, origin, SEEK_SET) != 0)
        return -1;
    return (int64_t)end;
}

void platform_close(PlatformStream* stream) {
    if (stream == NULL)
        return;
    if (stream->fp != NULL)
        fclose(stream->fp);
    free(stream);
}

bool platform_remove(const char* path) {
    if (unlink(path) == 0)
        return true;
    return errno == ENOENT;
}
