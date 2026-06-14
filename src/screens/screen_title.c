//! screen_title.c
//!
//! Title screen — placeholder. Renders a clear colour and a label.
//! Phase 5 wires New / Continue / Codex / Mastery / Settings / Quit
//! bindings; this Phase 1 body only verifies the dispatch path.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "app.h"
#include "screens.h"
#include "ui.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void title_enter(App* app) {
    (void)app;
}

static void title_event(App* app, const SDL_Event* event) {
    (void)app;
    (void)event;
}

static void title_tick(App* app, float dt) {
    (void)app;
    (void)dt;
}

static void title_render(App* app, SDL_Renderer* renderer) {
    (void)app;
    SDL_SetRenderDrawColor(renderer, 12, 18, 38, 255);
    SDL_RenderClear(renderer);
    ui_text(renderer, 32.0f, 32.0f, "REGNUM - TITLE");
    ui_text(renderer, 32.0f, 56.0f, "press N to cycle screens");
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_TITLE_V = {
    .enter  = title_enter,
    .leave  = NULL,
    .event  = title_event,
    .tick   = title_tick,
    .render = title_render,
};
