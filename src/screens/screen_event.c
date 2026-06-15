//! screen_event.c
//!
//! Narrative event screen. Presents an `EventTemplate` and
//! resolves the player's choice through the matching `EventOption`.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void event_enter(App* app) {
    (void)app;
}

static void event_event(App* app, const SDL_Event* event) {
    (void)app;
    (void)event;
}

static void event_tick(App* app, float dt) {
    (void)app;
    (void)dt;
}

static void event_render(App* app, SDL_Renderer* renderer) {
    (void)app;
    SDL_SetRenderDrawColor(renderer, 30, 22, 12, 255);
    SDL_RenderClear(renderer);
    ui_text(renderer, 32.0f, 32.0f, "REGNUM - EVENT");
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_EVENT_V = {
    .enter  = event_enter,
    .leave  = NULL,
    .event  = event_event,
    .tick   = event_tick,
    .render = event_render,
};
