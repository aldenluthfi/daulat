//! save.c
//!
//! Binary save-file codec: chunked TLV layout with a magic +
//! versioned header and a CRC32 footer. Writers buffer everything
//! into a fixed-size stack array and flush atomically via
//! `SDL_IOStream`; readers slurp the entire file in one go and
//! validate the CRC up-front.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include <SDL3/SDL.h>
#include <string.h>

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              CRC32
\*--------------------------------------------------------------------------*/

uint32_t crc32_ieee(const void* data, size_t count) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t       crc   = 0xFFFFFFFFu;
    for (size_t i = 0; i < count; i++) {
        crc ^= bytes[i];
        for (int j = 0; j < 8; j++) {
            uint32_t mask = -(int32_t)(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

/*--------------------------------------------------------------------------*\
                              WRITER
\*--------------------------------------------------------------------------*/

void save_writer_init(SaveWriter* w) {
    memset(w, 0, sizeof(*w));
}

static bool writer_reserve(SaveWriter* w, size_t count) {
    if (w->overflow)
        return false;
    if ((size_t)w->pos + count > SAVE_BUFFER_BYTES) {
        log_err("save writer overflow at pos=%u need=%zu", w->pos, count);
        w->overflow = true;
        return false;
    }
    return true;
}

bool save_write_u8(SaveWriter* w, uint8_t v) {
    if (!writer_reserve(w, 1))
        return false;
    w->buf[w->pos++] = v;
    return true;
}

bool save_write_u16(SaveWriter* w, uint16_t v) {
    if (!writer_reserve(w, 2))
        return false;
    w->buf[w->pos + 0] = (uint8_t)(v & 0xFFu);
    w->buf[w->pos + 1] = (uint8_t)((v >> 8) & 0xFFu);
    w->pos += 2;
    return true;
}

bool save_write_u32(SaveWriter* w, uint32_t v) {
    if (!writer_reserve(w, 4))
        return false;
    w->buf[w->pos + 0] = (uint8_t)(v & 0xFFu);
    w->buf[w->pos + 1] = (uint8_t)((v >> 8) & 0xFFu);
    w->buf[w->pos + 2] = (uint8_t)((v >> 16) & 0xFFu);
    w->buf[w->pos + 3] = (uint8_t)((v >> 24) & 0xFFu);
    w->pos += 4;
    return true;
}

bool save_write_u64(SaveWriter* w, uint64_t v) {
    if (!save_write_u32(w, (uint32_t)(v & 0xFFFFFFFFu)))
        return false;
    return save_write_u32(w, (uint32_t)((v >> 32) & 0xFFFFFFFFu));
}

bool save_write_bool(SaveWriter* w, bool v) {
    return save_write_u8(w, v ? 1u : 0u);
}

bool save_write_bytes(SaveWriter* w, const void* data, size_t count) {
    if (!writer_reserve(w, count))
        return false;
    memcpy(&w->buf[w->pos], data, count);
    w->pos += (uint32_t)count;
    return true;
}

bool save_write_header(SaveWriter* w, uint32_t chunk_count) {
    if (!save_write_u32(w, SAVE_MAGIC))
        return false;
    if (!save_write_u16(w, SAVE_VERSION_MAJOR))
        return false;
    if (!save_write_u16(w, 0u))
        return false;
    return save_write_u32(w, chunk_count);
}

bool save_write_chunk_begin(SaveWriter* w, SaveChunkId id) {
    if (w->in_chunk) {
        log_err("save_write_chunk_begin: chunk already open");
        return false;
    }
    if (!save_write_u32(w, (uint32_t)id))
        return false;
    w->chunk_origin = w->pos;
    if (!save_write_u32(w, 0u))
        return false;
    w->in_chunk = true;
    return true;
}

bool save_write_chunk_end(SaveWriter* w) {
    if (!w->in_chunk) {
        log_err("save_write_chunk_end: no chunk open");
        return false;
    }
    uint32_t body_len = w->pos - (w->chunk_origin + 4u);
    w->buf[w->chunk_origin + 0] = (uint8_t)(body_len & 0xFFu);
    w->buf[w->chunk_origin + 1] = (uint8_t)((body_len >> 8) & 0xFFu);
    w->buf[w->chunk_origin + 2] = (uint8_t)((body_len >> 16) & 0xFFu);
    w->buf[w->chunk_origin + 3] = (uint8_t)((body_len >> 24) & 0xFFu);
    w->in_chunk = false;
    return true;
}

bool save_writer_flush(const SaveWriter* w, const char* path) {
    if (w->overflow) {
        log_err("save_writer_flush: writer overflowed; refusing to flush");
        return false;
    }
    if (w->in_chunk) {
        log_err("save_writer_flush: chunk still open");
        return false;
    }
    uint32_t crc = crc32_ieee(w->buf, w->pos);

    SDL_IOStream* io = SDL_IOFromFile(path, "wb");
    if (io == NULL) {
        log_err("SDL_IOFromFile failed for %s: %s", path, SDL_GetError());
        return false;
    }
    if (SDL_WriteIO(io, w->buf, w->pos) != w->pos) {
        log_err("SDL_WriteIO body failed: %s", SDL_GetError());
        SDL_CloseIO(io);
        return false;
    }
    uint8_t footer[4] = {
        (uint8_t)(crc & 0xFFu),
        (uint8_t)((crc >> 8) & 0xFFu),
        (uint8_t)((crc >> 16) & 0xFFu),
        (uint8_t)((crc >> 24) & 0xFFu),
    };
    if (SDL_WriteIO(io, footer, sizeof(footer)) != sizeof(footer)) {
        log_err("SDL_WriteIO footer failed: %s", SDL_GetError());
        SDL_CloseIO(io);
        return false;
    }
    SDL_CloseIO(io);
    return true;
}

/*--------------------------------------------------------------------------*\
                              READER
\*--------------------------------------------------------------------------*/

static uint32_t read_u32_le(const uint8_t* position) {
    return ((uint32_t)position[0]) | ((uint32_t)position[1] << 8)
           | ((uint32_t)position[2] << 16) | ((uint32_t)position[3] << 24);
}

static uint16_t read_u16_le(const uint8_t* position) {
    return (uint16_t)(((uint16_t)position[0]) | ((uint16_t)position[1] << 8));
}

bool save_reader_open(SaveReader* r, const char* path) {
    memset(r, 0, sizeof(*r));

    SDL_IOStream* io = SDL_IOFromFile(path, "rb");
    if (io == NULL)
        return false;
    Sint64 size = SDL_GetIOSize(io);
    if (size < 16 || size > (Sint64)SAVE_BUFFER_BYTES) {
        log_err(
            "save_reader_open: %s has invalid size %lld",
            path, (long long)size
        );
        SDL_CloseIO(io);
        return false;
    }
    if (SDL_ReadIO(io, r->buf, (size_t)size) != (size_t)size) {
        log_err("SDL_ReadIO failed: %s", SDL_GetError());
        SDL_CloseIO(io);
        return false;
    }
    SDL_CloseIO(io);

    r->total = (uint32_t)size;

    uint32_t stored_crc  = read_u32_le(&r->buf[r->total - 4u]);
    uint32_t computed_crc = crc32_ieee(r->buf, r->total - 4u);
    if (stored_crc != computed_crc) {
        log_err(
            "save_reader_open: CRC mismatch on %s (file=%08x calc=%08x)",
            path, stored_crc, computed_crc
        );
        return false;
    }

    if (r->total < 12u) {
        log_err("save_reader_open: header too short");
        return false;
    }
    uint32_t magic = read_u32_le(&r->buf[0]);
    if (magic != SAVE_MAGIC) {
        log_err("save_reader_open: bad magic %08x in %s", magic, path);
        return false;
    }
    r->version     = read_u16_le(&r->buf[4]);
    r->chunk_count = read_u32_le(&r->buf[8]);
    r->pos         = 12u;
    return true;
}

static bool reader_have(const SaveReader* r, size_t count) {
    return (size_t)r->pos + count + 4u <= (size_t)r->total;
}

bool save_read_u8(SaveReader* r, uint8_t* out) {
    if (!reader_have(r, 1))
        return false;
    *out = r->buf[r->pos++];
    return true;
}

bool save_read_u16(SaveReader* r, uint16_t* out) {
    if (!reader_have(r, 2))
        return false;
    *out = read_u16_le(&r->buf[r->pos]);
    r->pos += 2;
    return true;
}

bool save_read_u32(SaveReader* r, uint32_t* out) {
    if (!reader_have(r, 4))
        return false;
    *out = read_u32_le(&r->buf[r->pos]);
    r->pos += 4;
    return true;
}

bool save_read_u64(SaveReader* r, uint64_t* out) {
    uint32_t low, high;
    if (!save_read_u32(r, &low) || !save_read_u32(r, &high))
        return false;
    *out = (uint64_t)low | ((uint64_t)high << 32);
    return true;
}

bool save_read_bool(SaveReader* r, bool* out) {
    uint8_t v;
    if (!save_read_u8(r, &v))
        return false;
    *out = (v != 0u);
    return true;
}

bool save_read_bytes(SaveReader* r, void* data, size_t count) {
    if (!reader_have(r, count))
        return false;
    memcpy(data, &r->buf[r->pos], count);
    r->pos += (uint32_t)count;
    return true;
}

bool save_read_chunk_header(
    SaveReader*  r,
    SaveChunkId* id_out,
    uint32_t*    len_out
) {
    uint32_t id;
    uint32_t len;
    if (!save_read_u32(r, &id) || !save_read_u32(r, &len))
        return false;
    *id_out  = (SaveChunkId)id;
    *len_out = len;
    return true;
}

bool save_skip(SaveReader* r, uint32_t count) {
    if (!reader_have(r, count))
        return false;
    r->pos += count;
    return true;
}

/*--------------------------------------------------------------------------*\
                              DUMP
\*--------------------------------------------------------------------------*/

static const char* chunk_name(SaveChunkId id) {
    switch (id) {
        case CHUNK_PROFILE:   return "PROFILE";
        case CHUNK_RUN_META:  return "RUN_META";
        case CHUNK_MAP_STATE: return "MAP_STATE";
        default:              return "UNKNOWN";
    }
}

int save_dump(const char* path) {
    SaveReader r;
    if (!save_reader_open(&r, path)) {
        log_err("save_dump: failed to open %s", path);
        return 1;
    }
    log_info(
        "save: %s | version=%u | chunks=%u | size=%u bytes",
        path, r.version, r.chunk_count, r.total
    );
    for (uint32_t i = 0; i < r.chunk_count; i++) {
        SaveChunkId id;
        uint32_t    len;
        if (!save_read_chunk_header(&r, &id, &len)) {
            log_err("save_dump: truncated at chunk %u", i);
            return 2;
        }
        log_info(
            "  [%u] id=%08x %-10s len=%u offset=%u",
            i, (uint32_t)id, chunk_name(id), len, r.pos
        );
        if (!save_skip(&r, len)) {
            log_err("save_dump: chunk body too long at %u", i);
            return 3;
        }
    }
    log_info("save: CRC verified");
    return 0;
}
