/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/shared/util/wgt2allg.h" // DIGI_AUTODETECT & MIDI_AUTODETECT
#include "ags/shared/ac/gamesetup.h"

namespace AGS3 {

GameSetup::GameSetup() {
	digicard = DIGI_AUTODETECT;
	midicard = MIDI_AUTODETECT;
	mod_player = 1;
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

} // namespace AGS3
