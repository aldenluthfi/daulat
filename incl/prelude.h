//! prelude.h
//!
//! Umbrella include. Every .c file in the project includes this and
//! only this; ad-hoc internal includes are forbidden. Order follows
//! the dependency layering so a clean rebuild from scratch always
//! succeeds.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef PRELUDE_H
#define PRELUDE_H

#include "ai.h"
#include "app.h"
#include "battle.h"
#include "board.h"
#include "card.h"
#include "core.h"
#include "data.h"
#include "defs.h"
#include "effect.h"
#include "effects.h"
#include "input.h"
#include "log.h"
#include "map.h"
#include "meta.h"
#include "movegen.h"
#include "piece.h"
#include "profile.h"
#include "recipe.h"
#include "registry.h"
#include "rng.h"
#include "run.h"
#include "save.h"
#include "screen.h"
#include "screens.h"
#include "types.h"
#include "ui.h"

#endif /* PRELUDE_H */
