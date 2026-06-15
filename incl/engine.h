//! engine.h
//!
//! Headless engine state. Owns the profile, run, battle, the active
//! screen id, per-screen scratch, and the input/output streams. The
//! engine never touches SDL; the SDL frontend forks the engine
//! binary and drives it over pipes per `protocol.h`.
//!
//! Created: 2026-06-15
//! Author : Alden Luthfi

#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "screen.h"

struct Profile;
struct RunState;
struct BattleState;
struct ProtocolVerb;

/*--------------------------------------------------------------------------*\
                              PER-SCREEN STATE
\*--------------------------------------------------------------------------*/

/// Title screen scratch.
typedef struct TitleState {
    bool resume_available;
} TitleState;

/// Map screen scratch: the highlighted node index.
typedef struct MapStateUi {
    uint8_t node_cursor;
} MapStateUi;

/// Codex popup scratch.
typedef struct CodexState {
    char kind[16];
    bool open;
} CodexState;

/// Results screen scratch: which side won this run.
typedef struct ResultsState {
    bool outcome;
} ResultsState;

/*--------------------------------------------------------------------------*\
                              ENGINE STATE
\*--------------------------------------------------------------------------*/

/// Top-level engine state. There are no globals.
typedef struct EngineState {
    FILE* in;
    FILE* out;

    ScreenId current;
    ScreenId next;
    bool     transition_pending;
    bool     quitting;

    struct Profile*     profile;
    struct RunState*    run;
    struct BattleState* battle;

    TitleState   title;
    MapStateUi   map;
    CodexState   codex;
    ResultsState results;
} EngineState;

/*--------------------------------------------------------------------------*\
                              LIFECYCLE
\*--------------------------------------------------------------------------*/

/// Zero state, attach streams, load profile from disk, mark the
/// title screen active and emit its initial SHOW line. Returns
/// false on profile-allocation failure.
bool engine_init(EngineState* engine, FILE* in, FILE* out);

/// Consume one already-read line from the frontend: parse the
/// verb, dispatch to the active screen's handler, apply any
/// pending screen transition, and flush stdout.
void engine_handle_line(EngineState* engine, const char* line);

/// Persist profile, free run/battle, leave streams to the caller.
void engine_destroy(EngineState* engine);

#endif /* ENGINE_H */
