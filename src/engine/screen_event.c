//! screen_event.c
//!
//! Narrative event screen. Presents an EventTemplate and resolves
//! the player's choice through the matching EventOption.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void event_handle(EngineState* engine, const ProtocolVerb* verb) {
    (void)engine;
    (void)verb;
}

static void event_emit(EngineState* engine) {
    protocol_emit_show(engine->out, SCREEN_EVENT, "");
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_EVENT_V = {
    .enter  = NULL,
    .leave  = NULL,
    .handle = event_handle,
    .emit   = event_emit,
};
