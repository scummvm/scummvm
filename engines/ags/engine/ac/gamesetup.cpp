//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include "util/wgt2allg.h" // DIGI_AUTODETECT & MIDI_AUTODETECT
#include "ac/gamesetup.h"

GameSetup::GameSetup()
{
    digicard=DIGI_AUTODETECT;
    midicard=MIDI_AUTODETECT;
    mod_player=1;
    no_speech_pack = false;
    textheight = 0;
    enable_antialiasing = false;
    disable_exception_handling = false;
    mouse_auto_lock = false;
    override_script_os = -1;
    override_multitasking = -1;
    override_upscale = false;
    mouse_speed = 1.f;
    mouse_ctrl_when = kMouseCtrl_Fullscreen;
    mouse_ctrl_enabled = true;
    mouse_speed_def = kMouseSpeed_CurrentDisplay;
    RenderAtScreenRes = false;
    Supersampling = 1;

    Screen.DisplayMode.ScreenSize.MatchDeviceRatio = true;
    Screen.DisplayMode.ScreenSize.SizeDef = kScreenDef_MaxDisplay;
    Screen.DisplayMode.RefreshRate = 0;
    Screen.DisplayMode.VSync = false;
    Screen.DisplayMode.Windowed = false;
    Screen.FsGameFrame = GameFrameSetup(kFrame_MaxProportional);
    Screen.WinGameFrame = GameFrameSetup(kFrame_MaxRound);
}
