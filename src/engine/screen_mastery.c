//! screen_mastery.c
//!
//! Mastery levels browser. Renders per-kingdom mastery progression
//! from Profile.mastery_levels and the rewards earned at each
//! level.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void mastery_handle(EngineState* engine, const ProtocolVerb* verb) {
    (void)engine;
    (void)verb;
}

static void mastery_emit(EngineState* engine) {
    Profile* profile = engine->profile;
    protocol_emit_show(engine->out, SCREEN_MASTERY, "");
    if (profile == NULL)
        return;
    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        protocol_emit_state(
            engine->out,
            "mastery.kingdom",
            "id=%zu level=%u",
            kingdom,
            profile->mastery_levels[kingdom]
        );
    }
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_MASTERY_V = {
    .enter  = NULL,
    .leave  = NULL,
    .handle = mastery_handle,
    .emit   = mastery_emit,
};
