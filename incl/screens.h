//! screens.h
//!
//! Extern declarations for every concrete screen v-table. Each
//! engine-side screen lives in `src/engine/screen_<name>.c` and
//! exposes a single `const Screen` symbol the registry binds by
//! `ScreenId`.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef SCREENS_H
#define SCREENS_H

#include "screen.h"

/*--------------------------------------------------------------------------*\
                              CONCRETE SCREENS
\*--------------------------------------------------------------------------*/

extern const Screen SCREEN_TITLE_V;
extern const Screen SCREEN_MAP_V;
extern const Screen SCREEN_BATTLE_V;
extern const Screen SCREEN_EVENT_V;
extern const Screen SCREEN_RESULTS_V;
extern const Screen SCREEN_CODEX_V;
extern const Screen SCREEN_MASTERY_V;
extern const Screen SCREEN_SETTINGS_V;

#endif /* SCREENS_H */
