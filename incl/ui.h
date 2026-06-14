//! ui.h
//!
//! Immediate-mode UI helpers (button, panel, text) used by every
//! screen. Bodies are stubs in Phase 1: signatures land so screen
//! code compiles cleanly. Pixel rendering arrives in a later round.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include <stdbool.h>

struct App;

/*--------------------------------------------------------------------------*\
                              UI HELPERS
\*--------------------------------------------------------------------------*/

/// ui_panel
///
/// Draw a flat rectangle at the given screen rect with a solid fill.
/// In Phase 1, this is a no-op.
///
/// Params:
/// - SDL_Renderer* renderer -> active renderer
/// - SDL_FRect     rect     -> screen-space rectangle
/// - SDL_Color     color    -> fill colour
///
void ui_panel(SDL_Renderer* renderer, SDL_FRect rect, SDL_Color color);

/// ui_text
///
/// Draw a debug-text label at the given position. Wraps SDL3's
/// `SDL_RenderDebugText`. In Phase 1, calls the SDL helper directly.
///
/// Params:
/// - SDL_Renderer* renderer -> active renderer
/// - float         x        -> left edge in pixels
/// - float         y        -> top edge in pixels
/// - const char*   text     -> NUL-terminated label
///
void ui_text(SDL_Renderer* renderer, float x, float y, const char* text);

/// ui_button
///
/// Draw a rectangular button with a label and return whether the
/// player just activated it via keyboard / mouse this frame. In
/// Phase 1, always returns false.
///
/// Params:
/// - struct App*   app      -> app holding input + renderer
/// - SDL_FRect     rect     -> screen-space rectangle
/// - const char*   label    -> button text
///
/// Return:
/// bool -> true on click edge
///
bool ui_button(struct App* app, SDL_FRect rect, const char* label);

#endif /* UI_H */
