//! screen_title.c
//!
//! Title screen. Verbs:
//!   new_run         start a fresh run, generate the first map,
//!                   transition to SCREEN_MAP.
//!   load_run        load the saved run from disk, transition to
//!                   SCREEN_MAP.
//!   open_codex <k>  open the codex popup with kind=<k>.
//!   goto <screen>   delegated through engine_handle_line.
//!
//! The `resume_available` flag is computed on entry by trying to
//! open the run save file.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include <stdlib.h>
#include <string.h>

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HELPERS
\*--------------------------------------------------------------------------*/

static void start_new_run(EngineState* engine) {
    if (engine->run != NULL)
        free(engine->run);
    engine->run = calloc(1, sizeof(RunState));
    if (engine->run == NULL) {
        log_err("title: run allocation failed");
        return;
    }
    uint64_t seed = platform_time_ns();
    run_init(engine->run, seed);
    engine->run->profile          = engine->profile;
    engine->run->current_kingdom  = KINGDOM_HARUSHIMA;
    engine->run->current_map_tier = TIER_TOWN;
    map_generate(
        &engine->run->current_map,
        engine->run->current_kingdom,
        engine->run->current_map_tier,
        seed
    );
    run_save(engine->run);
    screen_goto(engine, SCREEN_MAP);
}

static void continue_run(EngineState* engine) {
    if (engine->run == NULL)
        engine->run = calloc(1, sizeof(RunState));
    if (engine->run == NULL) {
        log_err("title: run allocation failed");
        return;
    }
    if (!run_load(engine->run)) {
        log_warn("title: no saved run to continue");
        free(engine->run);
        engine->run = NULL;
        return;
    }
    engine->run->profile = engine->profile;
    screen_goto(engine, SCREEN_MAP);
}

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void title_enter(EngineState* engine) {
    RunState probe;
    engine->title.resume_available = run_load(&probe);
}

static void title_handle(EngineState* engine, const ProtocolVerb* verb) {
    if (strcmp(verb->verb, "new_run") == 0)
        start_new_run(engine);
    else if (strcmp(verb->verb, "load_run") == 0)
        continue_run(engine);
}

static void title_emit(EngineState* engine) {
    protocol_emit_show(
        engine->out,
        SCREEN_TITLE,
        "resume=%d",
        engine->title.resume_available ? 1 : 0
    );
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_TITLE_V = {
    .enter  = title_enter,
    .leave  = NULL,
    .handle = title_handle,
    .emit   = title_emit,
};
