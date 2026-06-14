//! profile.h
//!
//! Persistent player profile: codex unlocks, per-kingdom mastery
//! levels, prestige tier, Vorath Memory tally, lifetime statistics.
//! Lives at `SDL_GetPrefPath / profile.regsav` and is loaded once
//! at app start. Phase 5 will extend the struct with an embedded
//! `Settings` and use it from the title flow.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef PROFILE_H
#define PROFILE_H

#include "defs.h"

/*--------------------------------------------------------------------------*\
                              PROFILE
\*--------------------------------------------------------------------------*/

/// Profile
///
/// Persistent player data. Independent of any in-flight run; loaded
/// once at startup and saved whenever its fields mutate (end of run,
/// mastery advance, prestige unlock, codex reveal).
///
typedef struct Profile {
    uint32_t version;
    uint8_t  mastery_levels[KINGDOM_COUNT];
    uint64_t codex_bits[2];
    uint8_t  prestige_tier;
    uint32_t vorath_defeat_count;
    uint16_t vorath_memory[PIECE_ID_COUNT];
    uint32_t total_wins;
    uint32_t total_losses;
} Profile;

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

/// profile_new
///
/// Zero-initialize a Profile to its first-launch defaults.
///
/// Params:
/// - Profile* profile -> profile to reset
///
void profile_new(Profile* profile);

/// profile_path
///
/// Resolve the on-disk path for `profile.regsav` under SDL's pref
/// path. Returns a pointer to a process-static string valid for the
/// lifetime of the program.
///
/// Return:
/// const char* -> absolute path, or NULL on SDL error
///
const char* profile_path(void);

/// profile_load
///
/// Read the profile from disk. On error (missing file, bad CRC,
/// bad magic), zero-initializes `*profile` and returns false.
///
/// Params:
/// - Profile* profile -> destination
///
/// Return:
/// bool -> true on success
///
bool profile_load(Profile* profile);

/// profile_save
///
/// Persist the profile to disk atomically via the save codec.
///
/// Params:
/// - const Profile* profile -> source
///
/// Return:
/// bool -> true on success
///
bool profile_save(const Profile* profile);

#endif /* PROFILE_H */
