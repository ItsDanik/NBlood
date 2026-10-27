// SPDX-License-Identifier: GPL-2.0-or-later
// MiSTer hybrid core (github.com/ItsDanik/NBlood_MiSTer): what is specific
// to Blood. What the three programs share is in source/build (mister.h,
// mister.cpp, MISTER_HYBRID in sdlayer.cpp, engine.cpp, timer.cpp).
// Everything here does nothing without the core, so the PC build of the same
// code still runs.

#pragma once

#ifdef MISTER_HYBRID

#include "mister.h"

// Once per frame, in the game's loop: the OSD options that apply while the
// game runs (the resolution, the sticks, the cheats)
void MiSTer_UpdateOptions(void);

// 1 if the status bar is drawn over the view: at 240 lines, which the view
// fills, whatever the game's screen size
static inline int MiSTer_StatusBarOverlay(void) { return MH_IsOpen() && ydim > 200; }

// Limit Vertical Look: half as far up and down
#define MISTER_LOOK(angle) (misterLimitLook() ? (angle) / 2 : (angle))

#else

#define MISTER_HUD(lines, stmt) do { stmt; } while (0)
#define MISTER_LOOK(angle) (angle)
static inline int misterHudUp(void) { return 0; }
static inline int misterHudDown(void) { return 0; }
static inline int misterHudBottom(void) { return 0; }

#endif
