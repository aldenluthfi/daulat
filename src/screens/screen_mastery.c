//! screen_mastery.c
//!
//! Mastery levels browser. Renders per-kingdom mastery
//! progression from `Profile.mastery_levels` and the rewards earned
//! at each level.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void mastery_enter(App* app) {
    (void)app;
}

static void mastery_event(App* app, const SDL_Event* event) {
    (void)app;
    (void)event;
}

static void mastery_tick(App* app, float dt) {
    (void)app;
    (void)dt;
}

static void mastery_render(App* app, SDL_Renderer* renderer) {
    (void)app;
    SDL_SetRenderDrawColor(renderer, 30, 18, 28, 255);
    SDL_RenderClear(renderer);
    ui_text(renderer, 32.0f, 32.0f, "REGNUM - MASTERY");
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_MASTERY_V = {
    .enter  = mastery_enter,
    .leave  = NULL,
    .event  = mastery_event,
    .tick   = mastery_tick,
    .render = mastery_render,
};
