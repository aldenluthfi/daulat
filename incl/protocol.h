//! protocol.h
//!
//! Line-oriented stdio grammar between the headless engine and any
//! frontend. The engine writes lines prefixed with `< ` to stdout
//! and reads frontend verbs prefixed with `> ` (the prefix is
//! optional on input) from stdin. Lines end with `\n` and are
//! capped at PROTOCOL_LINE_BYTES.
//!
//! Engine output verbs:
//! ```
//! < SHOW   <screen> [k=v ...]
//! < STATE  <topic>  [k=v ...]
//! < POPUP  <name>   [k=v ...]   (name == "close" closes any popup)
//! < LOG    <level>  <text>
//! < QUIT
//! ```
//!
//! Frontend input verbs:
//! ```
//! > new_run
//! > load_run
//! > goto <screen>
//! > open_codex <kind>
//! > close_popup
//! > select_node <id>
//! > play_card <hand_index> [target=piece:<id>]
//! > sell_card <hand_index>
//! > move piece=<id> to=(<x>,<y>)
//! > buy template=<name> at=(<x>,<y>)
//! > combine a=<id> b=<id>
//! > end_turn
//! > quit
//! ```
//!
//! Created: 2026-06-15
//! Author : Alden Luthfi

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "screen.h"

#define PROTOCOL_LINE_BYTES  1024
#define PROTOCOL_VERB_BYTES  64
#define PROTOCOL_VALUE_BYTES 64

/*--------------------------------------------------------------------------*\
                              TYPES
\*--------------------------------------------------------------------------*/

/// Severity tag for `< LOG` lines.
typedef enum {
    PROTOCOL_LOG_INFO,
    PROTOCOL_LOG_WARN,
    PROTOCOL_LOG_ERR
} ProtocolSeverity;

/// Parsed frontend verb. `verb` is the first whitespace-delimited
/// token, `tail` points into the caller's line buffer at the first
/// character after the verb (NUL-terminated). When the line was
/// blank or only whitespace, verb[0] == '\0'.
typedef struct ProtocolVerb {
    char        verb[PROTOCOL_VERB_BYTES];
    const char* tail;
} ProtocolVerb;

/*--------------------------------------------------------------------------*\
                              READ / PARSE
\*--------------------------------------------------------------------------*/

/// Read one '\n'-terminated line from `in` into `buffer`, stripping
/// trailing CR/LF. Returns false at EOF.
bool protocol_read_line(FILE* in, char* buffer, size_t capacity);

/// Parse a frontend line into a ProtocolVerb. Strips an optional
/// leading "> " prefix so transcripts copy-pasted into stdin still
/// parse.
void protocol_parse(const char* line, ProtocolVerb* out);

/// Find a key=value pair anywhere in `tail`. On success copies the
/// value (up to capacity-1 chars, NUL-terminated) into `out` and
/// returns true.
bool protocol_arg_str(
    const char* tail,
    const char* key,
    char*       out,
    size_t      capacity
);

/// Same as protocol_arg_str but parses the value as a signed int.
bool protocol_arg_int(const char* tail, const char* key, int* out);

/// Same as protocol_arg_str but parses the value as "(x,y)".
bool protocol_arg_xy(const char* tail, const char* key, int* x, int* y);

/*--------------------------------------------------------------------------*\
                              EMIT
\*--------------------------------------------------------------------------*/

/// Emit `< SHOW <screen> [tail]\n`.
void protocol_emit_show(FILE* out, ScreenId screen, const char* fmt, ...);

/// Emit `< STATE <topic> [tail]\n`.
void protocol_emit_state(FILE* out, const char* topic, const char* fmt, ...);

/// Emit `< POPUP <name> [tail]\n`. Pass name == "close" to close.
void protocol_emit_popup(FILE* out, const char* name, const char* fmt, ...);

/// Emit `< LOG <severity> <text>\n`.
void protocol_emit_log(
    FILE*            out,
    ProtocolSeverity severity,
    const char*      fmt,
    ...
);

/// Emit `< QUIT\n`.
void protocol_emit_quit(FILE* out);

#endif /* PROTOCOL_H */
