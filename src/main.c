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
/// or --log-level for debugging.
///
int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;
}
