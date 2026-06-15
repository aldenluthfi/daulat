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

static bool write_profile_chunk(SaveWriter* w, const Profile* p) {
    if (!save_write_chunk_begin(w, CHUNK_PROFILE))
        return false;
    if (!save_write_u32(w, p->version))
        return false;
    if (!save_write_bytes(w, p->mastery_levels, sizeof(p->mastery_levels)))
        return false;
    if (!save_write_u64(w, p->codex_bits[0]))
        return false;
    if (!save_write_u64(w, p->codex_bits[1]))
        return false;
    if (!save_write_u8(w, p->prestige_tier))
        return false;
    if (!save_write_u32(w, p->vorath_defeat_count))
        return false;
    for (size_t i = 0; i < PIECE_ID_COUNT; i++)
        if (!save_write_u16(w, p->vorath_memory[i]))
            return false;
    if (!save_write_u32(w, p->total_wins))
        return false;
    if (!save_write_u32(w, p->total_losses))
        return false;
    return save_write_chunk_end(w);
}

static bool read_profile_chunk(SaveReader* r, Profile* p) {
    if (!save_read_u32(r, &p->version))
        return false;
    if (!save_read_bytes(r, p->mastery_levels, sizeof(p->mastery_levels)))
        return false;
    if (!save_read_u64(r, &p->codex_bits[0]))
        return false;
    if (!save_read_u64(r, &p->codex_bits[1]))
        return false;
    if (!save_read_u8(r, &p->prestige_tier))
        return false;
    if (!save_read_u32(r, &p->vorath_defeat_count))
        return false;
    for (size_t i = 0; i < PIECE_ID_COUNT; i++)
        if (!save_read_u16(r, &p->vorath_memory[i]))
            return false;
    if (!save_read_u32(r, &p->total_wins))
        return false;
    return save_read_u32(r, &p->total_losses);
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
    SaveReader r;
    if (!save_reader_open(&r, path)) {
        profile_new(profile);
        return false;
    }
    for (uint32_t i = 0; i < r.chunk_count; i++) {
        SaveChunkId id;
        uint32_t    len;
        if (!save_read_chunk_header(&r, &id, &len)) {
            profile_new(profile);
            return false;
        }
        if (id == CHUNK_PROFILE) {
            if (!read_profile_chunk(&r, profile)) {
                profile_new(profile);
                return false;
            }
        } else {
            if (!save_skip(&r, len))
                return false;
        }
    }
    return true;
}

bool profile_save(const Profile* profile) {
    const char* path = profile_path();
    if (path == NULL)
        return false;
    SaveWriter w;
    save_writer_init(&w);
    if (!save_write_header(&w, 1u))
        return false;
    if (!write_profile_chunk(&w, profile))
        return false;
    return save_writer_flush(&w, path);
}
