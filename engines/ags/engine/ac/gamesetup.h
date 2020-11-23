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

#ifndef AGS_ENGINE_AC_GAMESETUP_H
#define AGS_ENGINE_AC_GAMESETUP_H

#include "ags/engine/main/graphics_mode.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

// Mouse control activation type
enum MouseControlWhen {
	kMouseCtrl_Never,       // never control mouse (track system mouse position)
	kMouseCtrl_Fullscreen,  // control mouse in fullscreen only
	kMouseCtrl_Always,      // always control mouse (fullscreen and windowed)
	kNumMouseCtrlOptions
};

// Mouse speed definition, specifies how the speed setting is applied to the mouse movement
enum MouseSpeedDef {
	kMouseSpeed_Absolute,       // apply speed multiplier directly
	kMouseSpeed_CurrentDisplay, // keep speed/resolution relation based on current system display mode
	kNumMouseSpeedDefs
};

using AGS::Shared::String;

// TODO: reconsider the purpose of this struct.
// Earlier I was trying to remove the uses of this struct from the engine
// and restrict it to only config/init stage, while applying its values to
// respective game/engine subcomponents at init stage.
// However, it did not work well at all times, and consequently I thought
// that engine may use a "config" object or combo of objects to store
// current user config, which may also be changed from script, and saved.
struct GameSetup {
	int digicard;
	int midicard;
	int mod_player;
	int textheight; // text height used on the certain built-in GUI // TODO: move out to game class?
	bool  no_speech_pack;
	bool  enable_antialiasing;
	bool  disable_exception_handling;
	String data_files_dir;
	String main_data_filename;
	String main_data_filepath;
	String install_dir; // optional custom install dir path
	String install_audio_dir; // optional custom install audio dir path
	String install_voice_dir; // optional custom install voice-over dir path
	String user_data_dir; // directory to write savedgames and user files to
	String shared_data_dir; // directory to write shared game files to
	String translation;
	bool  mouse_auto_lock;
	int   override_script_os;
	char  override_multitasking;
	bool  override_upscale;
	float mouse_speed;
	MouseControlWhen mouse_ctrl_when;
	bool  mouse_ctrl_enabled;
	MouseSpeedDef mouse_speed_def;
	bool  RenderAtScreenRes; // render sprites at screen resolution, as opposed to native one
	int   Supersampling;

	ScreenSetup Screen;

	GameSetup();
};

// TODO: setup object is used for two purposes: temporarily storing config
// options before engine is initialized, and storing certain runtime variables.
// Perhaps it makes sense to separate those two group of vars at some point.
extern GameSetup usetup;

} // namespace AGS3

#endif
