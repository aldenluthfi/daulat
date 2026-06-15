//! ui.c
//!
//! Immediate-mode UI helpers used by the SDL frontend. `ui_text`
//! wraps SDL3's `SDL_RenderDebugText`; `ui_panel` draws a flat
//! filled rectangle; `ui_button` is a no-op placeholder until the
//! frontend grows mouse handling.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include <SDL3/SDL.h>

#include "app.h"
#include "ui.h"

/*--------------------------------------------------------------------------*\
                              HELPERS
\*--------------------------------------------------------------------------*/

void ui_panel(SDL_Renderer* renderer, SDL_FRect rect, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void ui_text(SDL_Renderer* renderer, float x, float y, const char* text) {
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderDebugText(renderer, x, y, text);
}

bool ui_button(struct App* app, SDL_FRect rect, const char* label) {
    (void)app;
    (void)rect;
    (void)label;
    return false;
}
