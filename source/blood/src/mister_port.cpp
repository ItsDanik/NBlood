// SPDX-License-Identifier: GPL-2.0-or-later
// MiSTer hybrid core support, see mister_port.h

#ifdef MISTER_HYBRID

#include "build.h"
#include "compat.h"
#include "control.h"
#include "common_game.h"
#include "blood.h"
#include "config.h"
#include "demo.h"
#include "gamemenu.h"
#include "globals.h"
#include "levels.h"
#include "messages.h"
#include "player.h"
#include "screen.h"
#include "view.h"
#include "mister_port.h"

// The resolution, when the player changed it in the OSD
static void UpdateResolution(void)
{
    int32_t x, y;

    misterUpdateMenuMask();
    misterScreenSize(&x, &y);
    if (!in3dmode() || (x == xdim && y == ydim))
        return;
    if (videoSetGameMode(gSetup.fullscreen, x, y, 8, gUpscaleFactor) < 0)
        return;
    gSetup.xdim = x;
    gSetup.ydim = y;
    scrSetDac();
    viewResizeView(gViewSize);
}

// An option of the Cheats page: on, it keeps the cheat on (the game turns
// them off with every level); turned off, it turns it off. A cheat typed in
// stays while the option is off.
struct CheatOption
{
    int bit;
    CCheatMgr::CHEATCODE cheat;
    int last;
};
static CheatOption CheatOptions[] = {
    { MISTER_OPT_CHEAT1, CCheatMgr::kCheatMpkfa, 0 },    // God Mode, MPKFA
    { MISTER_OPT_CHEAT2, CCheatMgr::kCheatEvaGalli, 0 }, // No Clipping, EVA GALLI
    { MISTER_OPT_CHEAT3, CCheatMgr::kCheatGoonies, 0 },  // Show Full Map, GOONIES
};

static int CheatOn(CCheatMgr::CHEATCODE cheat)
{
    switch (cheat)
    {
        case CCheatMgr::kCheatMpkfa: return gMe->godMode;
        case CCheatMgr::kCheatEvaGalli: return gNoClip;
        default: return gFullMap;
    }
}

static void UpdateCheats(void)
{
    int const triggers = MH_OSDTriggers();
    int const playing = gGameStarted && !gDemo.at1 && gGameOptions.nGameType == kGameTypeSinglePlayer
                        && gInputMode != INPUT_MODE_3 && gMe && gMe->pXSprite && gMe->pXSprite->health > 0;

    for (auto &option : CheatOptions)
    {
        int const on = misterOption(option.bit, 1);

        if (!playing)
            continue; // turned on in the title, it applies once the game runs
        option.last = on;
        if (on != CheatOn(option.cheat))
            gCheatMgr.Process(option.cheat, nullptr);
    }

    if (!playing)
        return;
    if (triggers & MISTER_TRIG_56)
        gCheatMgr.Process(CCheatMgr::kCheatMontana, nullptr); // weapons, ammo and items
    else if (triggers & MISTER_TRIG_57)
        gCheatMgr.Process(CCheatMgr::kCheatIdaho, nullptr);
    else if (triggers & MISTER_TRIG_58)
        gCheatMgr.Process(CCheatMgr::kCheatKeyMaster, nullptr);
    if (triggers & MISTER_TRIG_59)
        levelEndLevel(kLevelExitNormal); // CALGON, also in the original's mode
}

// The sticks' sensitivity: the game's own setting of each axis times the OSD's
static void UpdateSticks(void)
{
    for (int axis = 0; axis < 4; axis++)
        CONTROL_SetAnalogAxisScale(axis, JoystickAnalogueScale[axis] * misterStickSensitivity(axis >= 2) / 100,
                                   controldevice_joystick);
}

void MiSTer_UpdateOptions(void)
{
    if (!MH_IsOpen())
        return;
    UpdateResolution();
    UpdateSticks();
    UpdateCheats();
    // the d-pad Main_MiSTer presses with the left stick moves the menus, and
    // not the player, whom the stick moves itself
    misterStickDpad = gGameMenuMgr.m_bActive || !gGameStarted || gInputMode != INPUT_MODE_0 || gDemo.at1;
}

#endif
