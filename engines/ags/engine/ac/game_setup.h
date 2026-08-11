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

#ifndef AGS_ENGINE_AC_GAME_SETUP_H
#define AGS_ENGINE_AC_GAME_SETUP_H

#include "ags/engine/main/graphics_mode.h"
#include "ags/shared/ac/game_version.h"
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

// Screen rotation mode on supported platforms and devices
enum ScreenRotation {
	kScreenRotation_Unlocked = 0,     // player can freely rotate the screen if possible
	kScreenRotation_Portrait,         // screen can only be in portrait orientation
	kScreenRotation_Landscape,        // screen can only be in landscape orientation
	kNumScreenRotationOptions
};

using AGS::Shared::String;

// TODO: reconsider the purpose of this struct in the future.
 // Currently it's been used as both initial storage for config options
 // before they are used to initialize engine, and as persistent storage
 // for options that may be changed at runtime (and later written back
 // to the config file).
struct GameSetup {
	static const size_t DefSpriteCacheSize = (128 * 1024); // 128 MB
	static const size_t DefTexCacheSize = (128 * 1024);    // 128 MB

	bool  audio_enabled;
	String audio_driver;
	int   textheight; // text height used on the certain built-in GUI // TODO: move out to game class?
	bool  no_speech_pack;
	bool  enable_antialiasing;
	bool  disable_exception_handling;
	String startup_dir; // directory where the default game config is located (usually same as main_data_dir)
	String main_data_dir; // main data directory
	String main_data_file; // full path to main data file
	// Following 4 optional dirs are currently for compatibility with Editor only (debug runs)
	// This is bit ugly, but remain so until more flexible configuration is designed
	String install_dir; // optional custom install dir path (also used as extra data dir)
	String opt_data_dir; // optional data dir number 2
	String opt_audio_dir; // optional custom install audio dir path
	String opt_voice_dir; // optional custom install voice-over dir path
	//
	String conf_path; // a read-only config path (if set the regular config is ignored)
	bool   local_user_conf; // search for user config in the game directory
	String user_conf_dir; // directory to read and write user config in
	String user_data_dir; // directory to write savedgames and user files to
	String shared_data_dir; // directory to write shared game files to
	String translation;
	bool  mouse_auto_lock;
	float mouse_speed;
	MouseControlWhen mouse_ctrl_when;
	bool  mouse_ctrl_enabled;
	MouseSpeedDef mouse_speed_def;
	bool  RenderAtScreenRes; // render sprites at screen resolution, as opposed to native one
	size_t SpriteCacheSize = DefSpriteCacheSize;  // in KB
	size_t TextureCacheSize = DefTexCacheSize;  // in KB
	bool  clear_cache_on_room_change; // for low-end devices: clear resource caches on room change
	bool  load_latest_save; // load latest saved game on launch
	ScreenRotation rotation;
	bool  show_fps;
	bool  multitasking = false; // whether run on background, when game is switched out

	DisplayModeSetup Screen;
	String software_render_driver;

	// User's overrides and hacks
	int   override_script_os; // pretend engine is running on this eScriptSystemOSID
	signed char  override_multitasking; // -1 for none, 0 or 1 to lock in the on/off mode
	bool  override_upscale; // whether upscale old games that supported that
	// assume game data version when restoring legacy save format
	GameDataVersion legacysave_assume_dataver = kGameVersion_Undefined;
	// allow to read mismatching number of guis from legacy save file
	bool legacysave_let_gui_diff = false;
	// Optional keys for calling built-in save/restore dialogs;
	// primarily meant for the test runs of the games where save functionality
	// is not implemented (or does not work correctly).
	int   key_save_game = 0;
	int   key_restore_game = 0;

	GameSetup();
};

} // namespace AGS3

#endif
