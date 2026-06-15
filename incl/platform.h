//! platform.h
//!
//! Headless platform abstraction. The engine never touches SDL; it
//! reads time via platform_time_ns, opens files via platform_open_*,
//! and locates the per-user save directory via platform_pref_path.
//! Two implementations live under src/: src/engine/platform_posix.c
//! (libc only) is linked into the headless engine; src/sdl/
//! platform_sdl.c (uses SDL3 IOStream + GetPrefPath + GetTicksNS) is
//! linked into the SDL frontend.
//!
//! Created: 2026-06-15
//! Author : Alden Luthfi

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*--------------------------------------------------------------------------*\
                              STREAM
\*--------------------------------------------------------------------------*/

/// Opaque platform stream handle. Callers pass it to platform_read,
/// platform_write, platform_size, platform_close.
typedef struct PlatformStream PlatformStream;

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

/// Return an absolute path to the per-user data directory, ending
/// with '/'. The returned pointer is owned by the platform layer
/// and remains valid for the lifetime of the process. Returns NULL
/// on failure.
const char* platform_pref_path(const char* org, const char* application);

/// Monotonic clock in nanoseconds since an unspecified origin. Used
/// for run seeds and timing diffs; not a wall-clock.
uint64_t platform_time_ns(void);

/// Open a file for writing in binary mode. Truncates if it exists.
PlatformStream* platform_open_write(const char* path);

/// Open a file for reading in binary mode.
PlatformStream* platform_open_read(const char* path);

/// Read up to capacity bytes from the stream into buffer. Returns
/// the number of bytes read (zero on EOF or error).
size_t platform_read(PlatformStream* stream, void* buffer, size_t capacity);

/// Write count bytes from buffer to the stream. Returns the number
/// of bytes successfully written.
size_t platform_write(PlatformStream* stream, const void* buffer, size_t count);

/// Total byte size of the underlying file at the time the stream
/// was opened. Returns -1 on error.
int64_t platform_size(PlatformStream* stream);

/// Close the stream. Safe to call on NULL.
void platform_close(PlatformStream* stream);

/// Delete a file by path. Returns true on success or when the file
/// did not exist.
bool platform_remove(const char* path);

#endif /* PLATFORM_H */
