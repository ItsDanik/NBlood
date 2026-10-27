// SPDX-License-Identifier: GPL-2.0-or-later
// MiSTer hybrid core support, see mister_port.h

#ifdef MISTER_HYBRID

#include "duke3d.h"
#include "mister.h"
#include "mister_port.h"
#include "cheats.h"
#include "osdcmds.h"

int MiSTer_StatusBarOverlay(void)
{
    return ud.statusbarmode || (MH_IsOpen() && ydim > 200);
}

// The resolution, when the player changed it in the OSD
static void UpdateResolution(void)
{
    int32_t x, y;

    misterUpdateMenuMask();
    misterScreenSize(&x, &y);
    if (!in3dmode() || (x == xdim && y == ydim))
        return;
    if (videoSetGameMode(ud.setup.fullscreen, x, y, 8, 1) < 0)
        return;
    onvideomodechange(0);
    g_restorePalette = -1;
    G_UpdateScreenArea();
    ud.setup.xdim = x;
    ud.setup.ydim = y;
}

// The game refuses cheats at Psychobilly (its hardest skill, and Rides
// Again's levels that say so), as G_DoCheats() does
static int CheatsRefused(DukePlayer_t const *pPlayer)
{
    return ud.player_skill == 4 || (RR && ud.player_skill > 3) || (RRRA && pPlayer->nocheat);
}

// The game's own cheat, as from its console: its message, and refused where
// the game refuses it. One a frame.
static int Cheat(int cheat)
{
    if (osdcmd_cheatsinfo_stat.cheatnum != -1)
        return 0;
    osdcmd_cheatsinfo_stat.cheatnum = cheat;
    return 1;
}

// An option of the Cheats page: on, it keeps the cheat on (the game turns
// God Mode off when a demo plays, Show Full Map with every level); turned
// off, it turns it off. A cheat typed in stays while the option is off.
struct CheatOption
{
    int bit, cheat, last;
};
static CheatOption CheatOptions[] = {
    { MISTER_OPT_CHEAT1, CHEAT_CORNHOLIO, 0 }, // God Mode, RDELVIS
    { MISTER_OPT_CHEAT2, CHEAT_CLIP, 0 },      // No Clipping, RDCLIP
    { MISTER_OPT_CHEAT3, CHEAT_SHOWMAP, 0 },   // Show Full Map, RDSHOWMAP
    { MISTER_OPT_CHEAT4, CHEAT_MONSTERS, 0 },  // No Monsters, RDCRITTERS
};

static int CheatOn(int cheat)
{
    switch (cheat)
    {
        case CHEAT_CORNHOLIO: return ud.god;
        case CHEAT_CLIP: return ud.noclip;
        case CHEAT_SHOWMAP: return ud.showallmap;
        default: return g_noEnemies == 1;
    }
}

// Next Level: as the level's exit; after the last level of an episode, its end
static void NextLevel(DukePlayer_t *pPlayer)
{
    int const next = ud.volume_number * MAXLEVELS + ud.level_number + 1;

    pPlayer->player_par = 0; // no best time
    if (ud.from_bonus || (ud.level_number + 1 < MAXLEVELS && g_mapInfo[next].filename != NULL))
        P_EndLevel();
    else
    {
        ud.eog = 1;
        pPlayer->gm |= MODE_EOL;
    }
}

static void UpdateCheats(void)
{
    auto const pPlayer = g_player[myconnectindex].ps;
    int const triggers = MH_OSDTriggers();
    int const playing = (pPlayer->gm & MODE_GAME) && !(pPlayer->gm & (MODE_DEMO | MODE_EOL | MODE_RESTART))
                        && numplayers < 2 && ud.recstat == 0 && !G_HaveUserMap();

    for (auto &option : CheatOptions)
    {
        int const on = misterOption(option.bit, 1);
        int const changed = on != option.last;

        if (!playing)
            continue; // turned on in the title, it applies once the game runs
        option.last = on;
        if (on == CheatOn(option.cheat) || (!on && !changed))
            continue;
        if (CheatsRefused(pPlayer))
        {
            if (changed)
                P_DoQuote(QUOTE_CHEATS_DISABLED, pPlayer);
            continue;
        }
        if (option.cheat == CHEAT_MONSTERS)
        {
            // No Monsters is the game's "Monsters: Off", not its cycle
            g_noEnemies = on;
            Bsprintf(apStrings[QUOTE_RESERVED4], "Monsters: %s", on ? "Off" : "On");
            P_DoQuote(QUOTE_RESERVED4, pPlayer);
        }
        else if (!changed && option.cheat == CHEAT_SHOWMAP)
        {
            // again with every level: no message
            ud.showallmap = 1;
            for (char &i : show2dsector)
                i = 255;
        }
        else if (!Cheat(option.cheat))
            option.last = !on; // the next frame
    }

    if (!playing)
        return;
    if (triggers & MISTER_TRIG_56)
        Cheat(CHEAT_STUFF); // RDALL
    else if (triggers & MISTER_TRIG_57)
        Cheat(CHEAT_WEAPONS); // RDGUNS
    else if (triggers & MISTER_TRIG_58)
        Cheat(CHEAT_KEYS); // RDKEYS
    if (triggers & MISTER_TRIG_59)
    {
        if (CheatsRefused(pPlayer))
            P_DoQuote(QUOTE_CHEATS_DISABLED, pPlayer);
        else
            NextLevel(pPlayer);
    }
}

// The sticks' sensitivity: the game's own setting of each axis times the OSD's
static void UpdateSticks(void)
{
    for (int axis = 0; axis < 4; axis++)
        CONTROL_SetAnalogAxisScale(axis, ud.config.JoystickAnalogueScale[axis] * misterStickSensitivity(axis >= 2) / 100,
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
    int const gm = g_player[myconnectindex].ps->gm;
    misterStickDpad = (gm & MODE_MENU) || !(gm & MODE_GAME) || (gm & (MODE_DEMO | MODE_EOL));
}

#endif
