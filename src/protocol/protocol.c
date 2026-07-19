//! protocol.c
//!
//! Line-based stdio protocol core. Reads one command per line, tokenizes
//! it, and dispatches to the active screen's handler. Owns the bound
//! protocol sink that protocol_emit writes output lines through.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <string.h>

#include <forward.h>
#include <prelude.h>

static Protocol *BOUND_PROTOCOL;

/// protocol_emit
///
/// Writes one formatted output line through the bound protocol,
/// appending the newline and flushing so piped consumers see it
/// immediately.
///
/// Params:
/// - format -> printf-style format string
/// - ...    -> format arguments
///
void protocol_emit(const char *format, ...) {
  if (!BOUND_PROTOCOL) {
    return;
  }

  va_list args;

  va_start(args, format);
  vfprintf(BOUND_PROTOCOL->out, format, args);
  va_end(args);

  fputc('\n', BOUND_PROTOCOL->out);
  fflush(BOUND_PROTOCOL->out);
}

/// arg_value
///
/// Finds the value of a key=value token among the command arguments.
///
/// Params:
/// - argc -> number of tokens
/// - argv -> token array
/// - key  -> key to look for
///
/// Return: value string, nullptr when the key is absent
///
const char *arg_value(int argc, char **argv, const char *key) {
  size_t length = strlen(key);

  for (int i = 1; i < argc; i++) {
    if (strncmp(argv[i], key, length) == 0 && argv[i][length] == '=') {
      return argv[i] + length + 1;
    }
  }

  return nullptr;
}

/// arg_long
///
/// Parses the value of a key=value token among the command arguments as
/// a long integer.
///
/// Params:
/// - argc     -> number of tokens
/// - argv     -> token array
/// - key      -> key to look for
/// - fallback -> value returned when the key is absent or malformed
///
/// Return: parsed value or the fallback
///
int32_t arg_long(int argc, char **argv, const char *key, long fallback) {
  const char *value = arg_value(argc, argv, key);

  if (!value) {
    return fallback;
  }

  char *end;
  int32_t parsed = strtol(value, &end, 10);

  return end == value ? fallback : parsed;
}

/// protocol_run
///
/// Runs the stdio loop: announces the active screen, then reads lines,
/// tokenizes them on whitespace, skips blanks and comment lines, handles
/// the global quit verb, and dispatches everything else to the active
/// screen's handler.
///
/// Params:
/// - protocol -> protocol whose streams and engine drive the loop
///
void protocol_run(Protocol *protocol) {
  BOUND_PROTOCOL = protocol;
  protocol_emit("screen %s", protocol->engine->screen->name);

  char line[PROTOCOL_LINE_MAX];

  while (fgets(line, PROTOCOL_LINE_MAX, protocol->in)) {
    char *argv[PROTOCOL_MAX_ARGS];
    int argc = 0;

    char *token = strtok_r(line, " \t\r\n");

    while (token && argc < PROTOCOL_MAX_ARGS) {
      argv[argc] = token;
      argc++;

      token = strtok_r(nullptr, " \t\r\n");
    }

    if (argc == 0) {
      continue;
    }

    if (strcmp(argv[0], "quit") == 0) {
      break;
    }

    protocol->engine->screen->handle(protocol->engine, argc, argv);
  }

  protocol_emit("bye");
}
