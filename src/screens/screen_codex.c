//! screen_codex.c
//!
//! Codex browser — placeholder. Phase 5 wires recipe discovery
//! display from `Profile.codex_bits`. Phase 1 only renders a clear
//! colour and label.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "app.h"
#include "screens.h"
#include "ui.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void codex_enter(App* app) {
    (void)app;
}

static void codex_event(App* app, const SDL_Event* event) {
    (void)app;
    (void)event;
}

static void codex_tick(App* app, float dt) {
    (void)app;
    (void)dt;
}

static void codex_render(App* app, SDL_Renderer* renderer) {
    (void)app;
    SDL_SetRenderDrawColor(renderer, 18, 30, 28, 255);
    SDL_RenderClear(renderer);
    ui_text(renderer, 32.0f, 32.0f, "REGNUM - CODEX");
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_CODEX_V = {
    .enter  = codex_enter,
    .leave  = NULL,
    .event  = codex_event,
    .tick   = codex_tick,
    .render = codex_render,
};
