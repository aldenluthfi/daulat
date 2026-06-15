//! ui.c
//!
//! Immediate-mode UI helpers. `ui_text` writes through SDL's debug
//! text helper; `ui_button` reports a click when the cursor is over
//! its rect and the primary button was released this frame.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

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
