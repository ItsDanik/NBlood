// SPDX-License-Identifier: GPL-2.0-or-later
// MiSTer hybrid core (github.com/ItsDanik/NBlood_MiSTer): the game runs on
// the MiSTer's ARM, the FPGA core shows the picture at 15kHz and gives sound
// and input, through SDL's "mister" drivers and the library mister_hybrid.h
// (Hybrid_MiSTer). What the engine and the game share about it.

#pragma once

#ifdef MISTER_HYBRID

#include <mister_hybrid.h>

#include "compat.h"
#include "sdl_inc.h"

// The core's joystick 1 (its "J1" list, core/NBlood.sv) is read by the
// engine and shown to the game as a game controller (sdlayer.cpp,
// misterJoyUpdate()), so that the menus take Menu OK and Menu Back from A and
// B and nothing else: the eight actions are on buttons the menus do not read.
enum
{
    MISTER_J1_FIRE,
    MISTER_J1_OPEN,
    MISTER_J1_JUMP,
    MISTER_J1_CROUCH,
    MISTER_J1_NEXTWEAPON,
    MISTER_J1_USEITEM,
    MISTER_J1_MAP,
    MISTER_J1_MENU,
    MISTER_J1_MENUOK,   // the core's own, for a spare button
    MISTER_J1_MENUBACK,
    MISTER_J1_NUMBUTTONS,

    MISTER_JOY_MENUOK   = 28,  // the MiSTer menu's (MH_MenuButtons())
    MISTER_JOY_MENUBACK = 29,
    MISTER_JOY_L3       = 30,
    MISTER_JOY_R3       = 31,
};

// ... and the game controller buttons they become (SDL_GameControllerButton)
#define MISTER_CB_FIRE       SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
#define MISTER_CB_OPEN       SDL_CONTROLLER_BUTTON_X
#define MISTER_CB_JUMP       SDL_CONTROLLER_BUTTON_Y
#define MISTER_CB_CROUCH     SDL_CONTROLLER_BUTTON_LEFTSHOULDER
#define MISTER_CB_NEXTWEAPON SDL_CONTROLLER_BUTTON_MISC1
#define MISTER_CB_USEITEM    SDL_CONTROLLER_BUTTON_PADDLE1
#define MISTER_CB_ITEMLEFT   SDL_CONTROLLER_BUTTON_PADDLE2  // Use Item held, d-pad left
#define MISTER_CB_ITEMRIGHT  SDL_CONTROLLER_BUTTON_PADDLE3  // and right
#define MISTER_CB_MAP        SDL_CONTROLLER_BUTTON_BACK
#define MISTER_CB_MENU       SDL_CONTROLLER_BUTTON_START
#define MISTER_CB_L3         SDL_CONTROLLER_BUTTON_LEFTSTICK
#define MISTER_CB_R3         SDL_CONTROLLER_BUTTON_RIGHTSTICK

// The video modes: those of the core that are 15kHz with one frame per field
// (320x200, 640x200, 320x240, 640x240), all of them shown 4:3. 2D art is
// drawn from 320x200 with each of its rows one line of the screen and each
// column xdim/320 pixels (engine.cpp, dorotspr_handle_bit2()); with 240 lines
// its 200 are in the middle.
static FORCE_INLINE int misterActive(void) { return MH_IsOpen(); }

// Lines the 2D art drawn while it is not 0 (the HUD) is moved down the
// screen: the game sets it around its HUD at 240 lines, from the player's
// HUD margins (hybrid/README.md, "HUD margins").
extern int32_t rotatesprite_mister_yofs;

// Called when the core is gone (another core was loaded): the game quits at
// once, without its screens, wherever it is
extern void (*misterCoreGone)(void);

// 1 while the d-pad Main_MiSTer presses with the left stick counts (menus and
// other screens), 0 while playing: the stick walks then, by its axes alone
// (misterJoyUpdate() in sdlayer.cpp; the game sets it every frame)
extern int misterStickDpad;

// The field the last frame was shown at: a frame drawn before the next one
// waits for it (SDL_MISTER_VSYNC)
extern uint32_t misterShownField;

// Frame pacing (hybrid/CLAUDE.md, "Frame pacing, drawing ahead"): with
// MISTER_DRAW_AHEAD 1 (sdlayer.cpp) every frame is drawn for the field it is
// to be shown at, from two fields before it on, and handed to the core with
// MH_PresentAtField(); 0 shows each at the next field. The field the frame
// being drawn is for, which the game's clock is the time of (timer.cpp).
uint32_t misterField(void);

// Profiling (make MISTER_PROFILE=1, never in a release): the time of each
// part of a frame, and where the game thread is, sampled 1000 times a second
// (sdlayer.cpp). MISTER_PROF_SCOPE(part) times the rest of the block; its
// lines are in #ifdef MISTER_PROFILE.
#ifdef MISTER_PROFILE
enum { MPROF_TICS, MPROF_DRAWROOMS, MPROF_ROOMS, MPROF_MASKS, MPROF_SPRITEANIM, MPROF_HUD, MPROF_COPY, MPROF_PRESENT, MPROF_NUM };
void misterProfAdd(int part, uint64_t ns);
uint64_t misterProfNow(void);
struct MisterProfScope
{
    int part; uint64_t t0;
    MisterProfScope(int p) : part(p), t0(misterProfNow()) {}
    ~MisterProfScope() { misterProfAdd(part, misterProfNow() - t0); }
};
#define MISTER_PROF_SCOPE(part) MisterProfScope misterProfScope_##part(part)
#endif

// What the three programs (blood, rednukem, pcexhumed) share of the core's
// OSD (core/NBlood.sv; mister.cpp). Each does nothing without the core, so
// the PC build of the same code still runs.

// The OSD's game options, status bits 24..63, each with 0 its first entry
enum
{
    MISTER_OPT_RESOLUTION = 24, // 2 bits
    MISTER_OPT_LSTICK     = 26, // 3 bits
    MISTER_OPT_RSTICK     = 29, // 3 bits
    MISTER_OPT_LIMITLOOK  = 32, // On is 0
    MISTER_OPT_RUMBLE     = 33, // 2 bits: On, Strong, Off
    MISTER_OPT_CHEAT1     = 35, // the Cheats page: God Mode, the same in each
    MISTER_OPT_CHEAT2     = 36, // No Clipping (Slip Mode in PowerSlave)
    MISTER_OPT_CHEAT3     = 37, // Show Full Map (Blood, Redneck Rampage)
    MISTER_OPT_CHEAT4     = 38, // No Monsters (Redneck Rampage)
};
// Triggers T[56..59], bits of MH_OSDTriggers(): the Cheats page's
enum
{
    MISTER_TRIG_56 = 1, // Give Everything (Give Items in PowerSlave)
    MISTER_TRIG_57 = 2, // Give Weapons
    MISTER_TRIG_58 = 4, // Give Keys
    MISTER_TRIG_59 = 8, // Next Level (Show Full Map in PowerSlave)
};
// The program, by the menu mask bit that shows its lines of the OSD
enum
{
    MISTER_BLOOD   = 1 << 1,
    MISTER_REDNECK = 1 << 2,
    MISTER_EXHUMED = 1 << 3,
};

// At the start of the program, before anything else: the core, and the exit
// code of every way out of the game (package/games/NBlood/
// danik_hybrid_launch.sh): back to the core's list of games when the game
// was picked from it (MISTER_HYBRID_FROM_LIST, src/nblood.c), started again
// after the core was loaded again, else back to the MiSTer's menu
void misterInit(int program);
int misterExitCode(void);

// An OSD option: `width` bits from `lsb`
int misterOption(int lsb, int width);
// The video mode the OSD's Resolution says: the size of the game's screen
void misterScreenSize(int32_t *x, int32_t *y);
// Once per frame: the program's lines of the OSD shown, and the HUD lines
// greyed at 200 lines (menu mask bit 0)
void misterUpdateMenuMask(void);

// The HUD at 240 lines (hybrid/README.md, "HUD margins"): how many lines the
// HUD at the top is moved up from where it is in the 200 lines in the middle
// of the screen, and the one at the bottom down. 0 at 200 lines.
int misterHudUp(void);
int misterHudDown(void);
// The bottom margin in lines: what the weapon, at the bottom of the view, is
// moved up by. 0 at 200 lines.
int misterHudBottom(void);

// Rumble from the OSD: 0 off, 1 as the game has it, 2 stronger
int misterRumble(void);
// The sensitivity of the left (0) and right (1) stick, in percent
int misterStickSensitivity(int stick);
// Limit Vertical Look: 1 to look up and down half as far from the middle
int misterLimitLook(void);

// The 2D art drawn in `stmt` (the HUD) moved down the screen by `lines`
#define MISTER_HUD(lines, stmt) do { rotatesprite_mister_yofs = (lines); stmt; rotatesprite_mister_yofs = 0; } while (0)

// The game's clock is the core's field counter (timer.cpp): 6.25MHz /
// (400 * 262) = 59.64 fields per second.
#define MISTER_FIELD_NUM 262
#define MISTER_FIELD_DEN 15625

#endif
