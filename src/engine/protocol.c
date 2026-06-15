//! protocol.c
//!
//! Line-oriented stdio grammar implementation. The engine prints
//! `< ` lines via protocol_emit_*; the frontend writes verbs that
//! the engine reads via protocol_read_line + protocol_parse.
//!
//! Created: 2026-06-15
//! Author : Alden Luthfi

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protocol.h"

/*--------------------------------------------------------------------------*\
                              SCREEN NAMES
\*--------------------------------------------------------------------------*/

static const char* const NAMES[SCREEN_COUNT] = {
    [SCREEN_TITLE]    = "title",
    [SCREEN_MAP]      = "map",
    [SCREEN_BATTLE]   = "battle",
    [SCREEN_EVENT]    = "event",
    [SCREEN_RESULTS]  = "results",
    [SCREEN_CODEX]    = "codex",
    [SCREEN_MASTERY]  = "mastery",
    [SCREEN_SETTINGS] = "settings",
};

const char* screen_name(ScreenId id) {
    if (id < 0 || id >= SCREEN_COUNT)
        return NULL;
    return NAMES[id];
}

/*--------------------------------------------------------------------------*\
                              READ / PARSE
\*--------------------------------------------------------------------------*/

bool protocol_read_line(FILE* in, char* buffer, size_t capacity) {
    if (in == NULL || buffer == NULL || capacity == 0)
        return false;
    if (fgets(buffer, (int)capacity, in) == NULL)
        return false;
    size_t length = strlen(buffer);
    while (length > 0
           && (buffer[length - 1] == '\n' || buffer[length - 1] == '\r')) {
        buffer[--length] = '\0';
    }
    return true;
}

void protocol_parse(const char* line, ProtocolVerb* out) {
    out->verb[0] = '\0';
    out->tail    = "";
    if (line == NULL)
        return;
    while (*line != '\0' && isspace((unsigned char)*line))
        line++;
    if ((line[0] == '>' || line[0] == '<')
        && (line[1] == ' ' || line[1] == '\t' || line[1] == '\0')) {
        line += (line[1] == '\0') ? 1 : 2;
    }
    while (*line != '\0' && isspace((unsigned char)*line))
        line++;
    if (*line == '\0' || *line == '#')
        return;
    size_t index = 0;
    while (*line != '\0' && !isspace((unsigned char)*line)
           && index < PROTOCOL_VERB_BYTES - 1) {
        out->verb[index++] = *line++;
    }
    out->verb[index] = '\0';
    while (*line != '\0' && isspace((unsigned char)*line))
        line++;
    out->tail = line;
}

/*--------------------------------------------------------------------------*\
                              ARG LOOKUP
\*--------------------------------------------------------------------------*/

/// Locate `key=` inside `tail`. Returns a pointer to the character
/// after the `=` (still within tail) or NULL.
static const char* find_key(const char* tail, const char* key) {
    if (tail == NULL || key == NULL)
        return NULL;
    size_t key_length = strlen(key);
    const char* cursor = tail;
    while (*cursor != '\0') {
        while (*cursor != '\0' && isspace((unsigned char)*cursor))
            cursor++;
        if (*cursor == '\0')
            return NULL;
        const char* start = cursor;
        while (*cursor != '\0' && !isspace((unsigned char)*cursor)
               && *cursor != '=') {
            cursor++;
        }
        if (*cursor == '=' && (size_t)(cursor - start) == key_length
            && memcmp(start, key, key_length) == 0) {
            return cursor + 1;
        }
        while (*cursor != '\0' && !isspace((unsigned char)*cursor))
            cursor++;
    }
    return NULL;
}

bool protocol_arg_str(
    const char* tail,
    const char* key,
    char*       out,
    size_t      capacity
) {
    const char* value = find_key(tail, key);
    if (value == NULL || capacity == 0)
        return false;
    size_t index = 0;
    while (*value != '\0' && !isspace((unsigned char)*value)
           && index < capacity - 1) {
        out[index++] = *value++;
    }
    out[index] = '\0';
    return true;
}

bool protocol_arg_int(const char* tail, const char* key, int* out) {
    char buffer[PROTOCOL_VALUE_BYTES];
    if (!protocol_arg_str(tail, key, buffer, sizeof(buffer)))
        return false;
    char* end = NULL;
    long parsed = strtol(buffer, &end, 10);
    if (end == buffer)
        return false;
    *out = (int)parsed;
    return true;
}

bool protocol_arg_xy(const char* tail, const char* key, int* x, int* y) {
    char buffer[PROTOCOL_VALUE_BYTES];
    if (!protocol_arg_str(tail, key, buffer, sizeof(buffer)))
        return false;
    if (buffer[0] != '(')
        return false;
    char* end = NULL;
    long parsed_x = strtol(buffer + 1, &end, 10);
    if (end == NULL || *end != ',')
        return false;
    long parsed_y = strtol(end + 1, &end, 10);
    if (end == NULL || *end != ')')
        return false;
    *x = (int)parsed_x;
    *y = (int)parsed_y;
    return true;
}

/*--------------------------------------------------------------------------*\
                              EMIT
\*--------------------------------------------------------------------------*/

static void emit_prefixed(
    FILE*       out,
    const char* prefix,
    const char* fmt,
    va_list     args
) {
    fputs(prefix, out);
    if (fmt != NULL && fmt[0] != '\0') {
        fputc(' ', out);
        vfprintf(out, fmt, args);
    }
    fputc('\n', out);
}

void protocol_emit_show(
    FILE*       out,
    ScreenId    screen,
    const char* fmt,
    ...
) {
    const char* name = screen_name(screen);
    if (name == NULL)
        name = "unknown";
    char prefix[64];
    snprintf(prefix, sizeof(prefix), "< SHOW %s", name);
    va_list args;
    va_start(args, fmt);
    emit_prefixed(out, prefix, fmt, args);
    va_end(args);
}

void protocol_emit_state(FILE* out, const char* topic, const char* fmt, ...) {
    char prefix[96];
    snprintf(prefix, sizeof(prefix), "< STATE %s", topic);
    va_list args;
    va_start(args, fmt);
    emit_prefixed(out, prefix, fmt, args);
    va_end(args);
}

void protocol_emit_popup(FILE* out, const char* name, const char* fmt, ...) {
    char prefix[96];
    snprintf(prefix, sizeof(prefix), "< POPUP %s", name);
    va_list args;
    va_start(args, fmt);
    emit_prefixed(out, prefix, fmt, args);
    va_end(args);
}

void protocol_emit_log(
    FILE*            out,
    ProtocolSeverity severity,
    const char*      fmt,
    ...
) {
    const char* tag = "info";
    if (severity == PROTOCOL_LOG_WARN)
        tag = "warn";
    else if (severity == PROTOCOL_LOG_ERR)
        tag = "err";
    char prefix[32];
    snprintf(prefix, sizeof(prefix), "< LOG %s", tag);
    va_list args;
    va_start(args, fmt);
    emit_prefixed(out, prefix, fmt, args);
    va_end(args);
}

void protocol_emit_quit(FILE* out) {
    fputs("< QUIT\n", out);
}
