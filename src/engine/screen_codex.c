//! screen_codex.c
//!
//! Standalone codex screen. The codex is more often surfaced as a
//! popup over the map; this screen is reached via `goto codex` and
//! lets the player page through pieces, cards, relics, and
//! combinations from the Profile codex bits.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void codex_handle(EngineState* engine, const ProtocolVerb* verb) {
    (void)engine;
    (void)verb;
}

static void codex_emit(EngineState* engine) {
    protocol_emit_show(
        engine->out, SCREEN_CODEX,
        "kind=%s", engine->codex.kind[0] ? engine->codex.kind : "piece"
    );
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_CODEX_V = {
    .enter  = NULL,
    .leave  = NULL,
    .handle = codex_handle,
    .emit   = codex_emit,
};
