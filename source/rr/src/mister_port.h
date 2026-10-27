// SPDX-License-Identifier: GPL-2.0-or-later
// MiSTer hybrid core (github.com/ItsDanik/NBlood_MiSTer): what is specific
// to Redneck Rampage. What the three programs share is in source/build
// (mister.h, mister.cpp, MISTER_HYBRID in sdlayer.cpp, engine.cpp,
// timer.cpp). Everything here does nothing without the core, so the PC build
// of the same code still runs.

#pragma once

#ifdef MISTER_HYBRID

#include "mister.h"

// At the start, before anything else
static inline void MiSTer_Init(void) { misterInit(MISTER_REDNECK); }

// The video mode the OSD's Resolution says: the size of the game's screen
static inline void MiSTer_ScreenSize(int32_t *x, int32_t *y) { misterScreenSize(x, y); }
// Once per frame, in the game's loops: the OSD options that apply while the
// game runs (the resolution, the sticks, the cheats)
void MiSTer_UpdateOptions(void);

// The HUD at 240 lines (mister.h)
static inline int MiSTer_HudUp(void) { return misterHudUp(); }
static inline int MiSTer_HudDown(void) { return misterHudDown(); }
static inline int MiSTer_HudBottom(void) { return misterHudBottom(); }
// 1 if the status bar is drawn over the view: at 240 lines, which the view
// fills, whatever the game's setting
int MiSTer_StatusBarOverlay(void);

static inline int MiSTer_Rumble(void) { return misterRumble(); }
static inline int MiSTer_LimitLook(void) { return misterLimitLook(); }

#else

#define MISTER_HUD(lines, stmt) do { stmt; } while (0)
static inline int MiSTer_HudUp(void) { return 0; }
static inline int MiSTer_HudDown(void) { return 0; }
static inline int MiSTer_HudBottom(void) { return 0; }

#endif
