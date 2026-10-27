// SPDX-License-Identifier: GPL-2.0-or-later
// MiSTer hybrid core: what the three programs share of the core's OSD and
// of the way out, see mister.h

#ifdef MISTER_HYBRID

#include "build.h"
#include "mister.h"

#include <unistd.h>

#define EXIT_TO_MENU 0
#define EXIT_TO_LIST 42  // danik_hybrid_launch.sh starts NBlood again: the list
#define EXIT_RELOADED 43 // the same, after the core has settled

static int misterProgram;

int misterOption(int lsb, int width)
{
    return MH_OSD_GAME_BITS(MH_OSDStatus(), lsb, width);
}

// Resolution: the video mode of the core. One the core does not have (an
// older core) is the one of 200 lines, which every core has; 320x200 as well
// when there is no core to ask.
static int misterScreenMode(void)
{
    static const int option[4] = { MH_MODE_320x200, MH_MODE_640x200, MH_MODE_320x240, MH_MODE_640x240 };
    int mode;

    if (!MH_IsOpen())
        return MH_MODE_320x200;
    mode = option[misterOption(MISTER_OPT_RESOLUTION, 2)];
    if (!MH_ModeAvailable(mode))
        mode = MH_ModeWidth(mode) == 640 ? MH_MODE_640x200 : MH_MODE_320x200;
    return mode;
}

void misterScreenSize(int32_t *x, int32_t *y)
{
    *x = MH_ModeWidth(misterScreenMode());
    *y = MH_ModeHeight(misterScreenMode());
}

void misterUpdateMenuMask(void)
{
    int32_t x, y;

    misterScreenSize(&x, &y);
    // the HUD lines of the OSD only do something at 240 lines (D0)
    MH_SetMenuMask(misterProgram | (y > 200 ? 0 : 1));
}

// The HUD at 240 lines: the 200 lines of 2D art are in the middle, 20 lines
// from each edge; the HUD goes to the player's margins from there
static int misterHudMove(int edge)
{
    int top, bottom;

    if (ydim <= 200)
        return 0;
    MH_HudMargins(&top, &bottom);
    return (ydim - 200) / 2 - (edge ? bottom : top) * ydim / 240;
}

int misterHudUp(void)
{
    return misterHudMove(0);
}

int misterHudDown(void)
{
    return misterHudMove(1);
}

int misterHudBottom(void)
{
    int top, bottom;

    if (ydim <= 200)
        return 0;
    MH_HudMargins(&top, &bottom);
    return bottom * ydim / 240;
}

int misterRumble(void)
{
    static const int rumble[4] = { 1, 2, 0, 0 };

    return MH_IsOpen() ? rumble[misterOption(MISTER_OPT_RUMBLE, 2)] : 1;
}

// The OSD's 100%, in percent of the game's own sensitivity (Danik,
// 2026-10-27): the left stick 250 in every game, the right one 125 in
// Redneck Rampage and PowerSlave and 425 in Blood. At the same setting Blood
// turns 3.4 times slower than Redneck Rampage (on the PC, the right stick
// held right at 250%: 706 against 2405 units of 2048 a second).
int misterStickSensitivity(int stick)
{
    static const int percent[8] = { 100, 125, 150, 200, 300, 25, 50, 75 };
    int const base = !stick ? 250 : misterProgram == MISTER_BLOOD ? 425 : 125;

    return MH_IsOpen() ? percent[misterOption(stick ? MISTER_OPT_RSTICK : MISTER_OPT_LSTICK, 3)] * base / 100 : 100;
}

int misterLimitLook(void)
{
    // On (the default) is the bit clear
    return MH_IsOpen() && !misterOption(MISTER_OPT_LIMITLOOK, 1);
}

int misterExitCode(void)
{
    // The player loaded the core again while the game ran
    if (MH_CoreReloaded())
        return EXIT_RELOADED;
    // Not if we are quitting because another core was loaded
    if (getenv("MISTER_HYBRID_FROM_LIST") && MH_Open())
        return EXIT_TO_LIST;
    return EXIT_TO_MENU;
}

// Runs after everything else of exit(), whichever of the game's ways out was
// taken
static void misterExitWithCode(void)
{
    _exit(misterExitCode());
}

void misterInit(int program)
{
    misterProgram = program;
    atexit(misterExitWithCode);
}

#endif
