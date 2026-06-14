//! screen_title.c
//!
//! Title screen — wires the top-level navigation menu. Logic only;
//! visible rendering lands in a later round. Key 1 starts a new
//! run; 2 resumes the saved run; 3-5 navigate to the
//! Codex / Mastery / Settings screens.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include <stdlib.h>

#include "app.h"
#include "log.h"
#include "map.h"
#include "prelude.h"
#include "screens.h"
#include "ui.h"

/*--------------------------------------------------------------------------*\
                              RUN HELPERS
\*--------------------------------------------------------------------------*/

static void start_new_run(App* app) {
    if (app->run != NULL)
        free(app->run);
    app->run = calloc(1, sizeof(RunState));
    if (app->run == NULL) {
        log_err("Run allocation failed");
        return;
    }
    uint64_t seed = SDL_GetTicksNS();
    run_init(app->run, seed);
    app->run->profile         = app->profile;
    app->run->current_kingdom = KINGDOM_HARUSHIMA;
    app->run->current_map_tier = TIER_TOWN;
    map_generate(
        &app->run->current_map,
        app->run->current_kingdom,
        app->run->current_map_tier,
        seed
    );
    run_save(app->run);
    screen_goto(app, SCREEN_MAP);
}

static void continue_run(App* app) {
    if (app->run == NULL)
        app->run = calloc(1, sizeof(RunState));
    if (app->run == NULL) {
        log_err("Run allocation failed");
        return;
    }
    if (!run_load(app->run)) {
        log_warn("No saved run to continue");
        free(app->run);
        app->run = NULL;
        return;
    }
    app->run->profile = app->profile;
    screen_goto(app, SCREEN_MAP);
}

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void title_enter(App* app) {
    (void)app;
}

static void title_event(App* app, const SDL_Event* event) {
    if (event->type != SDL_EVENT_KEY_DOWN || event->key.repeat)
        return;
    switch (event->key.scancode) {
        case SDL_SCANCODE_1:
            start_new_run(app);
            break;
        case SDL_SCANCODE_2:
            continue_run(app);
            break;
        case SDL_SCANCODE_3:
            screen_goto(app, SCREEN_CODEX);
            break;
        case SDL_SCANCODE_4:
            screen_goto(app, SCREEN_MASTERY);
            break;
        case SDL_SCANCODE_5:
            screen_goto(app, SCREEN_SETTINGS);
            break;
        default:
            break;
    }
}

static void title_tick(App* app, float dt) {
    (void)app;
    (void)dt;
}

static void title_render(App* app, SDL_Renderer* renderer) {
    (void)app;
    SDL_SetRenderDrawColor(renderer, 12, 18, 38, 255);
    SDL_RenderClear(renderer);
    ui_text(renderer, 32.0f, 32.0f, "REGNUM - TITLE");
    ui_text(renderer, 32.0f, 56.0f, "1=NEW  2=CONTINUE  3=CODEX");
    ui_text(renderer, 32.0f, 72.0f, "4=MASTERY  5=SETTINGS  ESC=QUIT");
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_TITLE_V = {
    .enter  = title_enter,
    .leave  = NULL,
    .event  = title_event,
    .tick   = title_tick,
    .render = title_render,
};
