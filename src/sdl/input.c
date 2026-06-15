//! input.c
//!
//! SDL frontend input edge-detector. Mirrors what src/sdl/app.c
//! needs to read each frame: down / pressed / released state per
//! scancode and a quit_requested flag.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include <string.h>

#include <SDL3/SDL.h>

#include "input.h"

/*--------------------------------------------------------------------------*\
                              LIFECYCLE
\*--------------------------------------------------------------------------*/

void input_init(Input* in) {
    memset(in, 0, sizeof(*in));
}

void input_begin_frame(Input* in) {
    memset(in->pressed, 0, sizeof(in->pressed));
    memset(in->released, 0, sizeof(in->released));
}

/*--------------------------------------------------------------------------*\
                              EVENT FOLD
\*--------------------------------------------------------------------------*/

void input_consume_event(Input* in, const SDL_Event* event) {
    switch (event->type) {
    case SDL_EVENT_QUIT:
        in->quit_requested = true;
        break;
    case SDL_EVENT_KEY_DOWN: {
        SDL_Scancode key = event->key.scancode;
        if (key < 0 || key >= SDL_SCANCODE_COUNT)
            break;
        if (event->key.repeat)
            break;
        if (!in->down[key])
            in->pressed[key] = true;
        in->down[key] = true;
        break;
    }
    case SDL_EVENT_KEY_UP: {
        SDL_Scancode key = event->key.scancode;
        if (key < 0 || key >= SDL_SCANCODE_COUNT)
            break;
        if (in->down[key])
            in->released[key] = true;
        in->down[key] = false;
        break;
    }
    default:
        break;
    }
}

/*--------------------------------------------------------------------------*\
                              QUERIES
\*--------------------------------------------------------------------------*/

bool input_down(const Input* in, SDL_Scancode key) {
    if (key < 0 || key >= SDL_SCANCODE_COUNT)
        return false;
    return in->down[key];
}

bool input_pressed(const Input* in, SDL_Scancode key) {
    if (key < 0 || key >= SDL_SCANCODE_COUNT)
        return false;
    return in->pressed[key];
}

bool input_released(const Input* in, SDL_Scancode key) {
    if (key < 0 || key >= SDL_SCANCODE_COUNT)
        return false;
    return in->released[key];
}
