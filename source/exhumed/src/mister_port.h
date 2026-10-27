// SPDX-License-Identifier: GPL-2.0-or-later
// MiSTer hybrid core (github.com/ItsDanik/NBlood_MiSTer): what is specific
// to PowerSlave. What the three programs share is in source/build (mister.h,
// mister.cpp, MISTER_HYBRID in sdlayer.cpp, engine.cpp, timer.cpp).
// Everything here does nothing without the core, so the PC build of the same
// code still runs.

#pragma once

#ifdef MISTER_HYBRID

#include "mister.h"

// In every loop of the game (HandleAsync()): the OSD options that apply
// while it runs (the resolution, the sticks, the cheats), and the
// controller in the menus, which read the keyboard alone
void MiSTer_UpdateOptions(void);
// The game's own loop calls this before HandleAsync(): while it runs, the
// player plays; any other loop is a menu, a movie or a screen of text
void MiSTer_Playing(void);

// The core's buttons and sticks (config.cpp)
void MiSTer_SetJoystick(void);

#else

#define MISTER_HUD(lines, stmt) do { stmt; } while (0)
static inline int misterHudUp(void) { return 0; }
static inline int misterHudDown(void) { return 0; }
static inline int misterHudBottom(void) { return 0; }

#endif
