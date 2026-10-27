// SPDX-License-Identifier: GPL-2.0-or-later
// MiSTer hybrid core support, see mister_port.h

#ifdef MISTER_HYBRID

#include "compat.h"
#include "baselayer.h"
#include "build.h"
#include "keyboard.h"
#include "control.h"
#include "joystick.h"
#include "exhumed.h"
#include "config.h"
#include "osdcmds.h"
#include "player.h"
#include "mister_port.h"

int32_t CONFIG_AnalogNameToNum(const char *func); // config.cpp

// The time the game's loop last ran (it calls HandleAsync() more than once
// a pass): playing while it ran in the last quarter of a second
static int PlayingTime = -1000, PlayingSeen;

void MiSTer_Playing(void)
{
    PlayingTime = (int)totalclock;
}

// The resolution, when the player changed it in the OSD
static void UpdateResolution(void)
{
    int32_t x, y;

    misterUpdateMenuMask();
    misterScreenSize(&x, &y);
    if (!in3dmode() || (x == xdim && y == ydim))
        return;
    if (videoSetGameMode(gSetup.fullscreen, x, y, 8, 0) < 0)
        return;
    gSetup.xdim = x;
    gSetup.ydim = y;
    onvideomodechange(0);
    UpdateScreenSize();
}

// The cheats, the game's passwords (DoPassword()): God Mode (LOBODEITY) and
// Slip Mode (LOBOSLIP) stay on while their option is; turned off, they are
// turned off. God Mode takes a tic of the game to change, and is off again in
// a level after a death: looked at again a second after asking for it.
enum { kLoboCop = 2, kLoboDeity = 3, kLoboPick = 5, kLoboSlip = 6, kLoboSphere = 8, kLoboSwag = 9 };

static void UpdateCheats(void)
{
    static int godWait;
    int const god = misterOption(MISTER_OPT_CHEAT1, 1);
    int const slip = misterOption(MISTER_OPT_CHEAT2, 1);

    if (!PlayingSeen || bInDemo || nNetPlayerCount)
        return; // turned on in the title, they apply once the game runs
    int const triggers = MH_OSDTriggers();
    if (godWait > 0)
        godWait--;
    else if (god != (PlayerList[nLocalPlayer].invincibility < 0) && PlayerList[nLocalPlayer].nHealth > 0)
    {
        DoPassword(kLoboDeity);
        godWait = 60;
    }
    if (slip != !!bSlipMode)
        DoPassword(kLoboSlip);

    if (triggers & MISTER_TRIG_56)
        DoPassword(kLoboSwag);
    else if (triggers & MISTER_TRIG_57)
        DoPassword(kLoboCop);
    else if (triggers & MISTER_TRIG_58)
        DoPassword(kLoboPick);
    if (triggers & MISTER_TRIG_59)
        DoPassword(kLoboSphere);
}

// The sticks' sensitivity: the game's own setting of each axis times the OSD's
static void UpdateSticks(void)
{
    for (int axis = 0; axis < 4; axis++)
        CONTROL_SetAnalogAxisScale(axis, JoystickAnalogueScale[axis] * misterStickSensitivity(axis >= 2) / 100,
                                   controldevice_joystick);
}

// The menus read the keyboard alone: there Menu OK is Enter, Menu Back and
// Start Escape, the d-pad (the left stick too) the arrows. While playing,
// Start is Escape, the game's menu. Enter and Escape are typed as well, for
// the screens that wait for any key (KB_KeyWaiting()).
static void UpdateMenuKeys(void)
{
    static const struct { int button; uint8_t key; char ascii; int menu; } keys[] = {
        { CONTROLLER_BUTTON_START, sc_Escape, asc_Escape, 0 },
        { CONTROLLER_BUTTON_A, sc_Return, asc_Enter, 1 },
        { CONTROLLER_BUTTON_B, sc_Escape, asc_Escape, 1 },
        { CONTROLLER_BUTTON_DPAD_UP, sc_UpArrow, 0, 1 },
        { CONTROLLER_BUTTON_DPAD_DOWN, sc_DownArrow, 0, 1 },
        { CONTROLLER_BUTTON_DPAD_LEFT, sc_LeftArrow, 0, 1 },
        { CONTROLLER_BUTTON_DPAD_RIGHT, sc_RightArrow, 0, 1 },
    };
    static uint32_t last;
    uint32_t const buttons = JOYSTICK_GetControllerButtons();

    for (auto const &k : keys)
    {
        uint32_t const bit = 1u << k.button;

        if (!((buttons ^ last) & bit) || (k.menu && PlayingSeen))
            continue;
        keySetState(k.key, !!(buttons & bit));
        // typed while held, dropped when let go if nothing read it: the
        // main menu reads none, and its Enter would choose in the next one
        if (!k.ascii)
            continue;
        if (buttons & bit)
        {
            if (!keyBufferFull())
                keyBufferInsert(k.ascii);
        }
        else
            keyFlushChars();
    }
    last = buttons;
}

void MiSTer_UpdateOptions(void)
{
    if (!MH_IsOpen())
        return;
    PlayingSeen = (int)totalclock - PlayingTime < kTimerTicks / 4;
    UpdateResolution();
    UpdateSticks();
    UpdateCheats();
    UpdateMenuKeys();
    // the d-pad Main_MiSTer presses with the left stick moves the menus, and
    // not the player, whom the stick moves itself
    misterStickDpad = !PlayingSeen;
}

// The core's joystick 1 (mister.h): its J1 buttons, the d-pad walking and
// turning, R2 fire, L3 the run toggle and R3 a look straight ahead; the left
// stick walks and strafes, the right one turns and looks up and down. A and B
// are only Menu OK and Menu Back, Start the menu.
void MiSTer_SetJoystick(void)
{
    static struct { int button; char const *function; } const buttons[] =
    {
        { MISTER_CB_FIRE, "Fire" },
        { MISTER_CB_OPEN, "Open" },
        { MISTER_CB_JUMP, "Jump" },
        { MISTER_CB_CROUCH, "Crouch" },
        { MISTER_CB_NEXTWEAPON, "Next_Weapon" },
        { MISTER_CB_USEITEM, "Inventory" },
        { MISTER_CB_ITEMLEFT, "Inventory_Left" },
        { MISTER_CB_ITEMRIGHT, "Inventory_Right" },
        { MISTER_CB_MAP, "Map" },
        { MISTER_CB_L3, "AutoRun" },
        { MISTER_CB_R3, "Look_Straight" },
        { CONTROLLER_BUTTON_DPAD_UP, "Move_Forward" },
        { CONTROLLER_BUTTON_DPAD_DOWN, "Move_Backward" },
        { CONTROLLER_BUTTON_DPAD_LEFT, "Turn_Left" },
        { CONTROLLER_BUTTON_DPAD_RIGHT, "Turn_Right" },
    };
    static char const *const analog[4] = { "analog_strafing", "analog_moving", "analog_turning", "analog_lookingupanddown" };

    memset(JoystickFunctions, -1, sizeof(JoystickFunctions));
    memset(JoystickDigitalFunctions, -1, sizeof(JoystickDigitalFunctions));
    for (auto const &b : buttons)
        JoystickFunctions[b.button][0] = CONFIG_FunctionNameToNum(b.function);
    JoystickDigitalFunctions[CONTROLLER_AXIS_TRIGGERRIGHT][1] = gamefunc_Fire;

    for (int i = 0; i < MAXJOYBUTTONSANDHATS; i++)
    {
        CONTROL_MapButton(JoystickFunctions[i][0], i, 0, controldevice_joystick);
        CONTROL_MapButton(JoystickFunctions[i][1], i, 1, controldevice_joystick);
    }
    for (int i = 0; i < MAXJOYAXES; i++)
    {
        JoystickAnalogueAxes[i] = i < 4 ? CONFIG_AnalogNameToNum(analog[i]) : -1;
        JoystickAnalogueScale[i] = DEFAULTJOYSTICKANALOGUESCALE;
        JoystickAnalogueDead[i] = DEFAULTJOYSTICKANALOGUEDEAD;
        JoystickAnalogueSaturate[i] = DEFAULTJOYSTICKANALOGUESATURATE;
        JoystickAnalogueInvert[i] = 0;
        CONTROL_MapAnalogAxis(i, JoystickAnalogueAxes[i]);
        CONTROL_MapDigitalAxis(i, JoystickDigitalFunctions[i][0], 0);
        CONTROL_MapDigitalAxis(i, JoystickDigitalFunctions[i][1], 1);
        CONTROL_SetAnalogAxisScale(i, JoystickAnalogueScale[i], controldevice_joystick);
        CONTROL_SetAnalogAxisInvert(i, JoystickAnalogueInvert[i]);
        JOYSTICK_SetDeadZone(i, JoystickAnalogueDead[i], JoystickAnalogueSaturate[i]);
    }
}

#endif
