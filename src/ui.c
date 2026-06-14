//! ui.c
//!
//! Immediate-mode UI stubs for Phase 1. Bodies are deliberately
//! minimal — `ui_text` forwards to SDL's debug text helper so each
//! screen has at least one visible label; `ui_panel` and `ui_button`
//! are no-ops until the rendering pass lands in a later round.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "ui.h"

/*--------------------------------------------------------------------------*\
                              STUBS
\*--------------------------------------------------------------------------*/

void ui_panel(SDL_Renderer* renderer, SDL_FRect rect, SDL_Color color) {
    (void)renderer;
    (void)rect;
    (void)color;
}

void ui_text(SDL_Renderer* renderer, float x, float y, const char* text) {
    SDL_RenderDebugText(renderer, x, y, text);
}

bool ui_button(struct App* app, SDL_FRect rect, const char* label) {
    (void)app;
    (void)rect;
    (void)label;
    return false;
}
