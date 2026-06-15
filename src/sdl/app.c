//! app.c
//!
//! SDL frontend lifecycle. Forks the headless engine binary on
//! init, sets up two pipes for verbs in / display lines out, and
//! pumps both directions every frame. Rendering reads from
//! `App.model`; input writes verbs through `app_send`.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <SDL3/SDL.h>

#include "app.h"
#include "defs.h"
#include "log.h"
#include "protocol.h"
#include "ui.h"

/*--------------------------------------------------------------------------*\
                              CHILD ENGINE
\*--------------------------------------------------------------------------*/

/// Set O_NONBLOCK on a file descriptor.
static bool set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

/// Fork the engine. Parent gets read/write FILE* through pipes.
/// Returns false on failure.
static bool spawn_engine(App* app, const char* engine_path) {
    int verb_pipe[2]; /* parent -> child stdin                  */
    int reply_pipe[2]; /* child  -> parent stdout                */
    if (pipe(verb_pipe) != 0 || pipe(reply_pipe) != 0) {
        log_err("spawn_engine: pipe failed (%s)", strerror(errno));
        return false;
    }
    pid_t child = fork();
    if (child < 0) {
        log_err("spawn_engine: fork failed (%s)", strerror(errno));
        return false;
    }
    if (child == 0) {
        dup2(verb_pipe[0], STDIN_FILENO);
        dup2(reply_pipe[1], STDOUT_FILENO);
        close(verb_pipe[0]);
        close(verb_pipe[1]);
        close(reply_pipe[0]);
        close(reply_pipe[1]);
        char* const argv[2] = {(char*)engine_path, NULL};
        execv(engine_path, argv);
        fprintf(stderr, "execv %s failed: %s\n", engine_path, strerror(errno));
        _exit(127);
    }
    close(verb_pipe[0]);
    close(reply_pipe[1]);
    if (!set_non_blocking(reply_pipe[0])) {
        log_err("spawn_engine: nonblock failed (%s)", strerror(errno));
        return false;
    }
    app->engine_pid = child;
    app->engine_in  = fdopen(verb_pipe[1], "w");
    app->engine_out = fdopen(reply_pipe[0], "r");
    if (app->engine_in == NULL || app->engine_out == NULL) {
        log_err("spawn_engine: fdopen failed");
        return false;
    }
    setvbuf(app->engine_in, NULL, _IOLBF, 0);
    return true;
}

/*--------------------------------------------------------------------------*\
                              MODEL UPDATES
\*--------------------------------------------------------------------------*/

/// Copy the first whitespace-delimited token from `in` into `out`.
static void copy_token(char* out, size_t capacity, const char* in) {
    size_t index = 0;
    while (*in != '\0' && !isspace((unsigned char)*in) &&
           index < capacity - 1) {
        out[index++] = *in++;
    }
    out[index] = '\0';
}

void app_consume_line(App* app, const char* line) {
    ProtocolVerb parsed;
    protocol_parse(line, &parsed);
    if (parsed.verb[0] == '\0')
        return;
    if (strcmp(parsed.verb, "SHOW") == 0) {
        copy_token(app->model.screen, sizeof(app->model.screen), parsed.tail);
        const char* tail = parsed.tail;
        while (*tail != '\0' && !isspace((unsigned char)*tail))
            tail++;
        while (*tail != '\0' && isspace((unsigned char)*tail))
            tail++;
        snprintf(app->model.detail, sizeof(app->model.detail), "%s", tail);
        app->model.popup[0]        = '\0';
        app->model.popup_detail[0] = '\0';
    } else if (strcmp(parsed.verb, "POPUP") == 0) {
        char name[32];
        copy_token(name, sizeof(name), parsed.tail);
        if (strcmp(name, "close") == 0) {
            app->model.popup[0]        = '\0';
            app->model.popup_detail[0] = '\0';
            return;
        }
        snprintf(app->model.popup, sizeof(app->model.popup), "%s", name);
        const char* tail = parsed.tail;
        while (*tail != '\0' && !isspace((unsigned char)*tail))
            tail++;
        while (*tail != '\0' && isspace((unsigned char)*tail))
            tail++;
        snprintf(
            app->model.popup_detail,
            sizeof(app->model.popup_detail),
            "%s",
            tail
        );
    } else if (strcmp(parsed.verb, "QUIT") == 0) {
        app->model.engine_quit = true;
    }
}

/// Drain bytes from the engine pipe into App.read_buffer and flush
/// complete lines through app_consume_line.
static void drain_engine(App* app) {
    if (app->engine_out == NULL)
        return;
    char chunk[256];
    while (true) {
        size_t count = fread(chunk, 1, sizeof(chunk), app->engine_out);
        if (count == 0)
            break;
        for (size_t i = 0; i < count; i++) {
            if (chunk[i] == '\n') {
                app->read_buffer[app->read_length] = '\0';
                app_consume_line(app, app->read_buffer);
                app->read_length = 0;
                continue;
            }
            if (app->read_length < sizeof(app->read_buffer) - 1)
                app->read_buffer[app->read_length++] = chunk[i];
        }
        if (count < sizeof(chunk))
            break;
    }
    clearerr(app->engine_out);
}

/*--------------------------------------------------------------------------*\
                              VIEW
\*--------------------------------------------------------------------------*/

typedef struct ScreenStyle {
    const char* upper;
    SDL_Color   fill;
} ScreenStyle;

static ScreenStyle style_for(const char* screen) {
    if (strcmp(screen, "title") == 0)
        return (ScreenStyle){"TITLE", {12, 18, 38, 255}};
    if (strcmp(screen, "map") == 0)
        return (ScreenStyle){"MAP", {24, 36, 18, 255}};
    if (strcmp(screen, "battle") == 0)
        return (ScreenStyle){"BATTLE", {38, 14, 14, 255}};
    if (strcmp(screen, "event") == 0)
        return (ScreenStyle){"EVENT", {30, 22, 12, 255}};
    if (strcmp(screen, "results") == 0)
        return (ScreenStyle){"RESULTS", {36, 28, 36, 255}};
    if (strcmp(screen, "codex") == 0)
        return (ScreenStyle){"CODEX", {18, 30, 28, 255}};
    if (strcmp(screen, "mastery") == 0)
        return (ScreenStyle){"MASTERY", {30, 18, 28, 255}};
    if (strcmp(screen, "settings") == 0)
        return (ScreenStyle){"SETTINGS", {22, 22, 32, 255}};
    return (ScreenStyle){"REGNUM", {0, 0, 0, 255}};
}

static void render_model(App* app) {
    ScreenStyle style = style_for(app->model.screen);
    SDL_SetRenderDrawColor(
        app->renderer,
        style.fill.r,
        style.fill.g,
        style.fill.b,
        255
    );
    SDL_RenderClear(app->renderer);
    char banner[64];
    snprintf(banner, sizeof(banner), "REGNUM - %s", style.upper);
    ui_text(app->renderer, 32.0f, 32.0f, banner);
    if (app->model.detail[0] != '\0')
        ui_text(app->renderer, 32.0f, 56.0f, app->model.detail);
    if (strcmp(app->model.screen, "title") == 0) {
        ui_text(
            app->renderer,
            32.0f,
            96.0f,
            "1=NEW 2=CONTINUE 3=CODEX 4=MASTERY 5=SETTINGS ESC=QUIT"
        );
    } else {
        ui_text(app->renderer, 32.0f, 96.0f, "ESC=BACK");
    }
    if (app->model.popup[0] != '\0') {
        char popup[128];
        snprintf(
            popup,
            sizeof(popup),
            "POPUP %s %s",
            app->model.popup,
            app->model.popup_detail
        );
        ui_text(app->renderer, 32.0f, 160.0f, popup);
        ui_text(app->renderer, 32.0f, 184.0f, "SPACE=CLOSE");
    }
    SDL_RenderPresent(app->renderer);
}

/*--------------------------------------------------------------------------*\
                              LIFECYCLE
\*--------------------------------------------------------------------------*/

SDL_AppResult app_init(App** out_app, const char* engine_binary_path) {
    App* app = calloc(1, sizeof(App));
    if (app == NULL) {
        log_err("app_init: allocation failed");
        return SDL_APP_FAILURE;
    }
    if (!spawn_engine(app, engine_binary_path)) {
        free(app);
        return SDL_APP_FAILURE;
    }
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        log_err("SDL_Init failed: %s", SDL_GetError());
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
        return SDL_APP_FAILURE;
    }
    input_init(&app->input);
    app->last_tick_ns = SDL_GetTicksNS();
    snprintf(app->model.screen, sizeof(app->model.screen), "title");
    *out_app = app;
    return SDL_APP_CONTINUE;
}

SDL_AppResult app_iterate(App* app) {
    drain_engine(app);
    if (app->model.engine_quit || app->input.quit_requested)
        return SDL_APP_SUCCESS;
    render_model(app);
    input_begin_frame(&app->input);
    return SDL_APP_CONTINUE;
}

SDL_AppResult app_event(App* app, const SDL_Event* event) {
    input_consume_event(&app->input, event);
    if (event->type == SDL_EVENT_QUIT) {
        app_send(app, "quit");
        return SDL_APP_SUCCESS;
    }
    if (event->type != SDL_EVENT_KEY_DOWN || event->key.repeat)
        return SDL_APP_CONTINUE;
    SDL_Scancode key = event->key.scancode;
    if (key == SDL_SCANCODE_ESCAPE) {
        if (strcmp(app->model.screen, "title") == 0) {
            app_send(app, "quit");
            return SDL_APP_SUCCESS;
        }
        app_send(app, "goto title");
        return SDL_APP_CONTINUE;
    }
    if (app->model.popup[0] != '\0') {
        if (key == SDL_SCANCODE_SPACE || key == SDL_SCANCODE_RETURN)
            app_send(app, "close_popup");
        return SDL_APP_CONTINUE;
    }
    if (strcmp(app->model.screen, "title") == 0) {
        switch (key) {
        case SDL_SCANCODE_1:
            app_send(app, "new_run");
            break;
        case SDL_SCANCODE_2:
            app_send(app, "load_run");
            break;
        case SDL_SCANCODE_3:
            app_send(app, "open_codex piece");
            break;
        case SDL_SCANCODE_4:
            app_send(app, "goto mastery");
            break;
        case SDL_SCANCODE_5:
            app_send(app, "goto settings");
            break;
        default:
            break;
        }
    }
    return SDL_APP_CONTINUE;
}

void app_quit(App* app) {
    if (app == NULL)
        return;
    if (app->engine_in != NULL) {
        app_send(app, "quit");
        fclose(app->engine_in);
        app->engine_in = NULL;
    }
    if (app->engine_pid > 0) {
        int status = 0;
        waitpid(app->engine_pid, &status, 0);
    }
    if (app->engine_out != NULL) {
        fclose(app->engine_out);
        app->engine_out = NULL;
    }
    if (app->renderer != NULL)
        SDL_DestroyRenderer(app->renderer);
    if (app->window != NULL)
        SDL_DestroyWindow(app->window);
    free(app);
    SDL_Quit();
}

/*--------------------------------------------------------------------------*\
                              PIPE I/O
\*--------------------------------------------------------------------------*/

void app_send(App* app, const char* fmt, ...) {
    if (app == NULL || app->engine_in == NULL)
        return;
    fputs("> ", app->engine_in);
    va_list args;
    va_start(args, fmt);
    vfprintf(app->engine_in, fmt, args);
    va_end(args);
    fputc('\n', app->engine_in);
    fflush(app->engine_in);
}
