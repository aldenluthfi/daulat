//! screen_battle.c
//!
//! Battle screen. Drives the battle engine via the map-node
//! payload and forwards player input to `battle_action_*`.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void battle_enter(App* app) {
    (void)app;
}

static void battle_event(App* app, const SDL_Event* event) {
    (void)app;
    (void)event;
}

static void battle_tick(App* app, float dt) {
    (void)app;
    (void)dt;
}

static void battle_render(App* app, SDL_Renderer* renderer) {
    (void)app;
    SDL_SetRenderDrawColor(renderer, 38, 14, 14, 255);
    SDL_RenderClear(renderer);
    ui_text(renderer, 32.0f, 32.0f, "REGNUM - BATTLE");
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_BATTLE_V = {
    .enter  = battle_enter,
    .leave  = NULL,
    .event  = battle_event,
    .tick   = battle_tick,
    .render = battle_render,
};
