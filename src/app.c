//! app.c
//!
//! Top-level SDL3 lifecycle for Regnum. Owns the window, renderer,
//! input edge-detector, and the active screen dispatch. Implements
//! the four `SDL_App*` callbacks proxied from `src/main.c`.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "app.h"

#include <stdlib.h>

#include "defs.h"
#include "log.h"
#include "profile.h"

/*--------------------------------------------------------------------------*\
                              GLOBAL KEY BINDINGS
\*--------------------------------------------------------------------------*/

/// Handle bindings that apply across every screen: Esc quits, N
/// cycles through the screen registry forward.
///
static void handle_global_bindings(App* app) {
    if (input_pressed(&app->input, SDL_SCANCODE_ESCAPE))
        app->input.quit_requested = true;

    if (input_pressed(&app->input, SDL_SCANCODE_N)) {
        ScreenId next = (ScreenId)((app->current + 1) % SCREEN_COUNT);
        screen_goto(app, next);
    }
}

/*--------------------------------------------------------------------------*\
                              LIFECYCLE
\*--------------------------------------------------------------------------*/

SDL_AppResult app_init(App** out_app) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        log_err("SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    App* app = calloc(1, sizeof(App));
    if (app == NULL) {
        log_err("App allocation failed");
        SDL_Quit();
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(
            "Regnum",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_HIGH_PIXEL_DENSITY,
            &app->window,
            &app->renderer
        )) {
        log_err("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        free(app);
        SDL_Quit();
        return SDL_APP_FAILURE;
    }

    input_init(&app->input);
    app->last_tick_ns = SDL_GetTicksNS();

    app->profile = calloc(1, sizeof(Profile));
    if (app->profile == NULL) {
        log_err("Profile allocation failed");
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        free(app);
        SDL_Quit();
        return SDL_APP_FAILURE;
    }
    if (!profile_load(app->profile))
        log_info("No profile found; starting fresh");

    app->current            = SCREEN_TITLE;
    app->next               = SCREEN_TITLE;
    app->transition_pending = false;
    const Screen* title     = screen_get(SCREEN_TITLE);
    if (title != NULL && title->enter != NULL)
        title->enter(app);

    *out_app = app;
    return SDL_APP_CONTINUE;
}

SDL_AppResult app_iterate(App* app) {
    screen_apply_transition(app);
    handle_global_bindings(app);

    if (app->input.quit_requested)
        return SDL_APP_SUCCESS;

    uint64_t now_ns = SDL_GetTicksNS();
    float    dt_s   = (float)(now_ns - app->last_tick_ns) / 1.0e9f;
    app->last_tick_ns = now_ns;

    const Screen* active = screen_get(app->current);
    if (active != NULL && active->tick != NULL)
        active->tick(app, dt_s);

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);
    if (active != NULL && active->render != NULL)
        active->render(app, app->renderer);
    SDL_RenderPresent(app->renderer);

    input_begin_frame(&app->input);
    return SDL_APP_CONTINUE;
}

SDL_AppResult app_event(App* app, const SDL_Event* event) {
    input_consume_event(&app->input, event);

    const Screen* active = screen_get(app->current);
    if (active != NULL && active->event != NULL)
        active->event(app, event);

    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;
    return SDL_APP_CONTINUE;
}

void app_quit(App* app) {
    if (app == NULL)
        return;
    if (app->profile != NULL) {
        profile_save(app->profile);
        free(app->profile);
    }
    if (app->run != NULL)
        free(app->run);
    if (app->battle != NULL)
        free(app->battle);
    if (app->renderer != NULL)
        SDL_DestroyRenderer(app->renderer);
    if (app->window != NULL)
        SDL_DestroyWindow(app->window);
    free(app);
    SDL_Quit();
}
