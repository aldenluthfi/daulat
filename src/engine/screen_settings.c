//! screen_settings.c
//!
//! Settings screen. Exposes volume, fullscreen, and reset-profile
//! controls.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void settings_handle(EngineState* engine, const ProtocolVerb* verb) {
    (void)engine;
    (void)verb;
}

static void settings_emit(EngineState* engine) {
    protocol_emit_show(engine->out, SCREEN_SETTINGS, "");
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_SETTINGS_V = {
    .enter  = NULL,
    .leave  = NULL,
    .handle = settings_handle,
    .emit   = settings_emit,
};
