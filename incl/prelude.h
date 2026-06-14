//! prelude.h
//!
//! Umbrella include: pulls every public header for the Regnum engine.
//! Use this in new .c files instead of listing headers individually.
//! Order matters — headers are listed in dependency order (Layer 0→5).
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef PRELUDE_H
#define PRELUDE_H

#include "ai.h"
#include "battle.h"
#include "board.h"
#include "card.h"
#include "core.h"
#include "data.h"
#include "defs.h"
#include "effect.h"
#include "effects.h"
#include "log.h"
#include "meta.h"
#include "movegen.h"
#include "piece.h"
#include "profile.h"
#include "recipe.h"
#include "registry.h"
#include "rng.h"
#include "run.h"
#include "types.h"

#endif /* PRELUDE_H */