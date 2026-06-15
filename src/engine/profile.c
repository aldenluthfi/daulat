//! profile.c
//!
//! Profile persistence backed by the chunked save codec. The save
//! file lands under platform_pref_path() so it ends up in the OS-
//! conventional per-user location regardless of which binary
//! writes it (engine vs. SDL frontend).
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include <stdio.h>
#include <string.h>

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              PATH
\*--------------------------------------------------------------------------*/

static char PROFILE_PATH[1024];
static bool PROFILE_PATH_READY = false;

const char* profile_path(void) {
    if (PROFILE_PATH_READY)
        return PROFILE_PATH;
    const char* pref = platform_pref_path("aldenluthfi", "regnum");
    if (pref == NULL)
        return NULL;
    snprintf(PROFILE_PATH, sizeof(PROFILE_PATH), "%sprofile.regsav", pref);
    PROFILE_PATH_READY = true;
    return PROFILE_PATH;
}

/*--------------------------------------------------------------------------*\
                              SERDE
\*--------------------------------------------------------------------------*/

static bool write_profile_chunk(SaveWriter* writer, const Profile* p) {
    if (!save_write_chunk_begin(writer, CHUNK_PROFILE))
        return false;
    if (!save_write_u32(writer, p->version))
        return false;
    if (!save_write_bytes(writer, p->mastery_levels, sizeof(p->mastery_levels)))
        return false;
    if (!save_write_u64(writer, p->codex_bits[0]))
        return false;
    if (!save_write_u64(writer, p->codex_bits[1]))
        return false;
    if (!save_write_u8(writer, p->prestige_tier))
        return false;
    if (!save_write_u32(writer, p->vorath_defeat_count))
        return false;
    for (size_t i = 0; i < PIECE_ID_COUNT; i++)
        if (!save_write_u16(writer, p->vorath_memory[i]))
            return false;
    if (!save_write_u32(writer, p->total_wins))
        return false;
    if (!save_write_u32(writer, p->total_losses))
        return false;
    return save_write_chunk_end(writer);
}

static bool read_profile_chunk(SaveReader* reader, Profile* p) {
    if (!save_read_u32(reader, &p->version))
        return false;
    if (!save_read_bytes(reader, p->mastery_levels, sizeof(p->mastery_levels)))
        return false;
    if (!save_read_u64(reader, &p->codex_bits[0]))
        return false;
    if (!save_read_u64(reader, &p->codex_bits[1]))
        return false;
    if (!save_read_u8(reader, &p->prestige_tier))
        return false;
    if (!save_read_u32(reader, &p->vorath_defeat_count))
        return false;
    for (size_t i = 0; i < PIECE_ID_COUNT; i++)
        if (!save_read_u16(reader, &p->vorath_memory[i]))
            return false;
    if (!save_read_u32(reader, &p->total_wins))
        return false;
    return save_read_u32(reader, &p->total_losses);
}

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

void profile_new(Profile* profile) {
    memset(profile, 0, sizeof(*profile));
    profile->version = SAVE_VERSION_MAJOR;
}

bool profile_load(Profile* profile) {
    const char* path = profile_path();
    if (path == NULL) {
        profile_new(profile);
        return false;
    }
    SaveReader reader;
    if (!save_reader_open(&reader, path)) {
        profile_new(profile);
        return false;
    }
    for (uint32_t i = 0; i < reader.chunk_count; i++) {
        SaveChunkId id;
        uint32_t    length;
        if (!save_read_chunk_header(&reader, &id, &length)) {
            profile_new(profile);
            return false;
        }
        if (id == CHUNK_PROFILE) {
            if (!read_profile_chunk(&reader, profile)) {
                profile_new(profile);
                return false;
            }
        } else {
            if (!save_skip(&reader, length))
                return false;
        }
    }
    return true;
}

bool profile_save(const Profile* profile) {
    const char* path = profile_path();
    if (path == NULL)
        return false;
    SaveWriter writer;
    save_writer_init(&writer);
    if (!save_write_header(&writer, 1u))
        return false;
    if (!write_profile_chunk(&writer, profile))
        return false;
    return save_writer_flush(&writer, path);
}
