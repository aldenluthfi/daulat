//! screen_results.c
//!
//! End-of-run summary screen. On entry runs the mastery, prestige,
//! and win-loss bookkeeping via run_finalize and detaches the run
//! from the engine. The outcome (win/loss) lives in
//! EngineState.results so there are no globals.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include <stdlib.h>
#include <string.h>

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void results_enter(EngineState* engine) {
    if (engine->run == NULL)
        return;
    RunEnd outcome =
        engine->results.outcome ? RUN_END_VORATH_WIN : RUN_END_LOSS;
    run_finalize(engine->run, outcome);
    free(engine->run);
    engine->run = NULL;
}

static void results_handle(EngineState* engine, const ProtocolVerb* verb) {
    if (strcmp(verb->verb, "continue") == 0
        || strcmp(verb->verb, "ack") == 0) {
        screen_goto(engine, SCREEN_TITLE);
    }
}

static void results_emit(EngineState* engine) {
    protocol_emit_show(
        engine->out, SCREEN_RESULTS,
        "outcome=%s", engine->results.outcome ? "win" : "loss"
    );
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_RESULTS_V = {
    .enter  = results_enter,
    .leave  = NULL,
    .handle = results_handle,
    .emit   = results_emit,
};
