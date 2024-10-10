/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ags/engine/ac/game_setup.h"

namespace AGS3 {

GameSetup::GameSetup() {
	local_user_conf = false;
	audio_enabled = true;
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
	clear_cache_on_room_change = false;
	load_latest_save = false;
	rotation = kScreenRotation_Unlocked;
	show_fps = false;

	Screen.Params.RefreshRate = 0;
	Screen.Params.VSync = false;
	Screen.Windowed = false;
	Screen.FsGameFrame = kFrame_Proportional;
	Screen.WinGameFrame = kFrame_Round;
}

} // namespace AGS3
