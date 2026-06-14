//! screen_results.c
//!
//! End-of-run summary screen. On entry, runs the mastery /
//! prestige / win-loss bookkeeping via run_finalize and detaches
//! the run from App. Logic only — visible summary rendering lands
//! in a later round.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include <stdlib.h>

#include "app.h"
#include "prelude.h"
#include "screens.h"
#include "ui.h"

/*--------------------------------------------------------------------------*\
                              END-OF-RUN PASS
\*--------------------------------------------------------------------------*/

static bool last_run_won = false;

void screen_results_set_outcome(bool won) {
    last_run_won = won;
}

static void results_enter(App* app) {
    if (app->run == NULL)
        return;
    RunEnd outcome = last_run_won ? RUN_END_VORATH_WIN : RUN_END_LOSS;
    run_finalize(app->run, outcome);
    free(app->run);
    app->run = NULL;
}

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void results_event(App* app, const SDL_Event* event) {
    if (event->type != SDL_EVENT_KEY_DOWN || event->key.repeat)
        return;
    if (event->key.scancode == SDL_SCANCODE_RETURN
        || event->key.scancode == SDL_SCANCODE_SPACE)
        screen_goto(app, SCREEN_TITLE);
}

static void results_tick(App* app, float dt) {
    (void)app;
    (void)dt;
}

static void results_render(App* app, SDL_Renderer* renderer) {
    (void)app;
    SDL_SetRenderDrawColor(renderer, 36, 28, 36, 255);
    SDL_RenderClear(renderer);
    ui_text(renderer, 32.0f, 32.0f,
            last_run_won ? "REGNUM - VICTORY" : "REGNUM - DEFEAT");
    ui_text(renderer, 32.0f, 64.0f, "press ENTER to return to title");
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_RESULTS_V = {
    .enter  = results_enter,
    .leave  = NULL,
    .event  = results_event,
    .tick   = results_tick,
    .render = results_render,
};
