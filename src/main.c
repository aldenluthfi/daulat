//! main.c
//!
//! Entry point for the game. Handles command-line argument parsing and
//! delegates to the game engine for initialization and the main loop.
//! This file should remain minimal - all game logic belongs in other modules.
//!
//! Created: 23/06/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/// main
///
/// Entry point for the application. Accepts standard argc/argv arguments
/// for future command-line options such as --seed for deterministic runs
/// or --save for a custom save path.
///
/// Params:
/// - argc -> number of command-line arguments
/// - argv -> command-line argument array
///
/// Return: process exit status
///
int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  EngineState engine;
  engine_init(&engine);

  Protocol protocol = {&engine, stdin, stdout};
  protocol_run(&protocol);

  engine_free(&engine);

  return EXIT_SUCCESS;
}
