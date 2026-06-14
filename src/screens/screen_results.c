//! screen_results.c
//!
//! End-of-run results screen — placeholder. Phase 5 wires the
//! mastery / prestige / profile-save pass. Phase 1 only renders a
//! clear colour and label.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "app.h"
#include "screens.h"
#include "ui.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void results_enter(App* app) {
    (void)app;
}

static void results_event(App* app, const SDL_Event* event) {
    (void)app;
    (void)event;
}

static void results_tick(App* app, float dt) {
    (void)app;
    (void)dt;
}

static void results_render(App* app, SDL_Renderer* renderer) {
    (void)app;
    SDL_SetRenderDrawColor(renderer, 36, 28, 36, 255);
    SDL_RenderClear(renderer);
    ui_text(renderer, 32.0f, 32.0f, "REGNUM - RESULTS");
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
