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

static void settings_enter(App* app) {
    (void)app;
}

static void settings_event(App* app, const SDL_Event* event) {
    (void)app;
    (void)event;
}

static void settings_tick(App* app, float dt) {
    (void)app;
    (void)dt;
}

static void settings_render(App* app, SDL_Renderer* renderer) {
    (void)app;
    SDL_SetRenderDrawColor(renderer, 22, 22, 32, 255);
    SDL_RenderClear(renderer);
    ui_text(renderer, 32.0f, 32.0f, "REGNUM - SETTINGS");
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_SETTINGS_V = {
    .enter  = settings_enter,
    .leave  = NULL,
    .event  = settings_event,
    .tick   = settings_tick,
    .render = settings_render,
};
