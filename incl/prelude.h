//! prelude.h
//!
//! Engine-side umbrella include. Every .c file under src/ except
//! src/sdl/ includes this and only this; ad-hoc internal includes
//! are forbidden. SDL-coupled headers (app.h, input.h, ui.h) live
//! outside the prelude and are pulled in directly by src/sdl/
//! files. The order follows the dependency layering so a clean
//! rebuild from scratch always succeeds.
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
#include "engine.h"
#include "log.h"
#include "map.h"
#include "meta.h"
#include "movegen.h"
#include "piece.h"
#include "platform.h"
#include "profile.h"
#include "protocol.h"
#include "recipe.h"
#include "registry.h"
#include "rng.h"
#include "run.h"
#include "save.h"
#include "screen.h"
#include "screens.h"
#include "types.h"

#endif /* PRELUDE_H */
