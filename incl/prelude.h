//! prelude.h
//!
//! The master include file for the entire codebase. It pulls in all standard
//! C library headers and defines global constants, then includes the main
//! module headers in dependency order.
//!
//! Created: 12/06/2026
//! Author : Alden Luthfi

/*----------------------------------------------------------------------------*\
                                    C STD
\*----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

#include <stdio.h>

/*----------------------------------------------------------------------------*\
                                  GLOBAL DEFS
\*----------------------------------------------------------------------------*/

/// RNG_SEED
///
/// Fixed seed value for the deterministic random number generator used
/// throughout the game for map generation and any seeded randomness.
///
#define RNG_SEED 0xDEADBEEF

/*----------------------------------------------------------------------------*\
                                 LOCAL HEADERS
\*----------------------------------------------------------------------------*/

#include <data_structure.h>
#include <representation.h>
#include <protocol.h>
