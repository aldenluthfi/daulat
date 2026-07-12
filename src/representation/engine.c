//! engine.c
//!
//! Engine lifecycle and persistence. Initializes and frees the master
//! game state, encodes and decodes the text save format, and finalizes
//! runs by advancing masteries and the cleared difficulty.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/// engine_init
///
/// Initializes a fresh engine state with no masteries, no cleared
/// difficulty, no run, no battle, and the title screen active.
///
/// Params:
/// - engine -> engine state to initialize
///
void engine_init(EngineState* engine) {
    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        engine->masteries[kingdom] = MASTERY_NONE;
    }

    engine->cleared = DIFFICULTY_NONE;
    engine->screen  = &SCREEN_REGISTRY[SCREEN_TITLE];
    engine->run     = nullptr;
    engine->battle  = nullptr;
}

/// engine_free
///
/// Frees everything the engine owns: the active battle if any, then the
/// active run if any.
///
/// Params:
/// - engine -> engine state to deallocate
///
void engine_free(EngineState* engine) {
    if (engine->battle) {
        battle_free(engine->battle);
        free(engine->battle);
        engine->battle = nullptr;
    }

    if (engine->run) {
        run_free(engine->run);
        engine->run = nullptr;
    }
}

/// engine_save
///
/// Writes the engine state to the given path in the text save format.
/// Battles are never saved; saving is rejected while one is active.
///
/// Params:
/// - engine -> engine state to save
/// - path   -> file path to write to
///
/// Return: true when the save was written
///
bool engine_save(EngineState* engine, const char* path) {
    (void) engine;
    (void) path;

    return false;
}

/// engine_load
///
/// Reads the engine state from the given path, rebuilding the campaign
/// maps from the saved seed and reapplying saved bits and events. A
/// missing file yields a fresh profile.
///
/// Params:
/// - engine -> engine state to load into
/// - path   -> file path to read from
///
/// Return: true when an existing save was loaded
///
bool engine_load(EngineState* engine, const char* path) {
    (void) engine;
    (void) path;

    return false;
}

/// engine_finalize_run
///
/// Concludes the active run: advances masteries for kingdoms that were
/// never chained when Vorath was defeated, bumps the cleared difficulty,
/// and frees the run.
///
/// Params:
/// - engine     -> engine owning the run
/// - vorath_won -> whether the run ended by defeating Vorath
///
void engine_finalize_run(EngineState* engine, bool vorath_won) {
    (void) engine;
    (void) vorath_won;
}
