//! input.h
//!
//! Input edge-detector wrapping SDL3 keyboard and quit events.
//! Tracks per-frame down / pressed / released state for every
//! scancode plus a sticky quit_requested flag.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL.h>
#include <stdbool.h>

/*--------------------------------------------------------------------------*\
                              INPUT STATE
\*--------------------------------------------------------------------------*/

/// Input
///
/// Frame-grained keyboard and quit state. `down` reflects the current
/// held state; `pressed` and `released` mark just-this-frame edges
/// that are cleared by `input_begin_frame`.
///
typedef struct {
    bool down[SDL_SCANCODE_COUNT];
    bool pressed[SDL_SCANCODE_COUNT];
    bool released[SDL_SCANCODE_COUNT];
    bool quit_requested;
} Input;

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

/// input_init
///
/// Zero every tracked field. Must be called once at app start.
///
/// Params:
/// - Input* in -> input state to reset
///
void input_init(Input* in);

/// input_begin_frame
///
/// Clear the per-frame edge bitmaps. Call once at the top of each
/// frame before draining SDL events into `input_consume_event`.
///
/// Params:
/// - Input* in -> input state to roll forward
///
void input_begin_frame(Input* in);

/// input_consume_event
///
/// Fold one SDL_Event into the input state, updating `down`,
/// `pressed`, `released`, and `quit_requested` accordingly.
///
/// Params:
/// - Input* in       -> input state to mutate
/// - const SDL_Event* event -> SDL event to consume
///
void input_consume_event(Input* in, const SDL_Event* event);

/// input_down
///
/// Is the given key currently held?
///
/// Params:
/// - const Input* in -> input state to query
/// - SDL_Scancode key -> scancode to test
///
/// Return:
/// bool -> true if the key is held
///
bool input_down(const Input* in, SDL_Scancode key);

/// input_pressed
///
/// Did the key transition from up to down this frame?
///
/// Params:
/// - const Input* in -> input state to query
/// - SDL_Scancode key -> scancode to test
///
/// Return:
/// bool -> true on a fresh press edge
///
bool input_pressed(const Input* in, SDL_Scancode key);

/// input_released
///
/// Did the key transition from down to up this frame?
///
/// Params:
/// - const Input* in -> input state to query
/// - SDL_Scancode key -> scancode to test
///
/// Return:
/// bool -> true on a fresh release edge
///
bool input_released(const Input* in, SDL_Scancode key);

#endif /* INPUT_H */
