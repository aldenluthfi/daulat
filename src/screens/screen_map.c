//! screen_map.c
//!
//! Overworld map screen — placeholder. Phase 4 fills this in: node
//! graph navigation, event launch, battle launch, autosave on
//! transition. Phase 1 only renders a clear colour and label.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "app.h"
#include "screens.h"
#include "ui.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void map_enter(App* app) {
    (void)app;
}

static void map_event(App* app, const SDL_Event* event) {
    (void)app;
    (void)event;
}

static void map_tick(App* app, float dt) {
    (void)app;
    (void)dt;
}

static void map_render(App* app, SDL_Renderer* renderer) {
    (void)app;
    SDL_SetRenderDrawColor(renderer, 24, 36, 18, 255);
    SDL_RenderClear(renderer);
    ui_text(renderer, 32.0f, 32.0f, "REGNUM - MAP");
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_MAP_V = {
    .enter  = map_enter,
    .leave  = NULL,
    .event  = map_event,
    .tick   = map_tick,
    .render = map_render,
};
