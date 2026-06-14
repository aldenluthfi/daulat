//! save.h
//!
//! Binary, chunked, versioned save-file codec backed by `SDL_IOStream`.
//! Every save file starts with a magic + version header, contains a
//! sequence of TLV chunks, and ends with a CRC32 footer over the
//! preceding bytes. The codec is the foundation for both profile and
//! run persistence; the format admits forward-compatible additions
//! by appending new chunk ids.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "defs.h"

/*--------------------------------------------------------------------------*\
                              CHUNK IDS
\*--------------------------------------------------------------------------*/

/// SaveChunkId
///
/// Tag for each TLV chunk written into a save file. Values are
/// 4-byte ASCII for readability under hex dump.
///
typedef enum {
    CHUNK_PROFILE   = 0x46524F50u, /* 'PROF' little-endian */
    CHUNK_RUN_META  = 0x4D4E5552u, /* 'RUNM' little-endian */
    CHUNK_MAP_STATE = 0x5350414Du, /* 'MAPS' little-endian */
} SaveChunkId;

/*--------------------------------------------------------------------------*\
                              WRITER
\*--------------------------------------------------------------------------*/

/// SaveWriter
///
/// Fixed-capacity write buffer with current position. Save files are
/// small enough (profile + run < 1 KB in Phase 2) that an 8 KB
/// stack buffer comfortably covers every plausible save.
///
typedef struct {
    uint8_t  buf[SAVE_BUFFER_BYTES];
    uint32_t pos;
    uint32_t chunk_origin;
    bool     in_chunk;
    bool     overflow;
} SaveWriter;

void save_writer_init(SaveWriter* w);

bool save_write_u8(SaveWriter* w, uint8_t v);
bool save_write_u16(SaveWriter* w, uint16_t v);
bool save_write_u32(SaveWriter* w, uint32_t v);
bool save_write_u64(SaveWriter* w, uint64_t v);
bool save_write_bool(SaveWriter* w, bool v);
bool save_write_bytes(SaveWriter* w, const void* data, size_t n);

bool save_write_header(SaveWriter* w, uint32_t chunk_count);

/// Begin a chunk. Writes id + placeholder length; `save_write_chunk_end`
/// patches the length once the body is complete.
bool save_write_chunk_begin(SaveWriter* w, SaveChunkId id);
bool save_write_chunk_end(SaveWriter* w);

/// Finalize the buffer with a CRC32 footer and persist to disk.
bool save_writer_flush(const SaveWriter* w, const char* path);

/*--------------------------------------------------------------------------*\
                              READER
\*--------------------------------------------------------------------------*/

/// SaveReader
///
/// Read-only view over a fully-loaded save file. The CRC footer is
/// verified once during `save_reader_open`; subsequent reads simply
/// advance the cursor.
///
typedef struct {
    uint8_t  buf[SAVE_BUFFER_BYTES];
    uint32_t total;
    uint32_t pos;
    uint16_t version;
    uint32_t chunk_count;
} SaveReader;

bool save_reader_open(SaveReader* r, const char* path);

bool save_read_u8(SaveReader* r, uint8_t* out);
bool save_read_u16(SaveReader* r, uint16_t* out);
bool save_read_u32(SaveReader* r, uint32_t* out);
bool save_read_u64(SaveReader* r, uint64_t* out);
bool save_read_bool(SaveReader* r, bool* out);
bool save_read_bytes(SaveReader* r, void* data, size_t n);

bool save_read_chunk_header(
    SaveReader*  r,
    SaveChunkId* id_out,
    uint32_t*    len_out
);
bool save_skip(SaveReader* r, uint32_t n);

/*--------------------------------------------------------------------------*\
                              UTILITIES
\*--------------------------------------------------------------------------*/

/// crc32_ieee
///
/// Standard IEEE 802.3 polynomial 0xEDB88320, init 0xFFFFFFFF,
/// final XOR 0xFFFFFFFF. Used for save-file integrity.
///
/// Params:
/// - const void* data -> input bytes
/// - size_t       n   -> length in bytes
///
/// Return:
/// uint32_t -> 32-bit checksum
///
uint32_t crc32_ieee(const void* data, size_t n);

/// save_dump
///
/// Pretty-print a save file: header, chunk index (id, length), CRC
/// status. Used by the `--dump-save` CLI subcommand.
///
/// Params:
/// - const char* path -> path to a `.regsav` file
///
/// Return:
/// int -> 0 on success, nonzero on error
///
int save_dump(const char* path);

#endif /* SAVE_H */
