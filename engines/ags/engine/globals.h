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

#ifndef AGS_ENGINE_GLOBALS_H
#define AGS_ENGINE_GLOBALS_H

#include "ags/shared/util/string.h"
#include "ags/shared/util/version.h"
#include "ags/lib/std/set.h"

namespace AGS3 {

#define MAXCURSORS 20

using String = AGS::Shared::String;
using Version = AGS::Shared::Version;

namespace AGS {
namespace Shared {
class Bitmap;
} // namespace Shared
} // namespace AGS

struct IAGSEditorDebugger;

class Globals {
public:
	/**
	 * \defgroup Overall flags
	 * @{
	 */

	// Major overall flags
	bool _want_exit = false;
	bool _abort_engine = false;

	/**@}*/

	/**
	 * \defgroup debug globals
	 * @{
	 */

	int _editor_debugging_enabled = 0;
	int _editor_debugging_initialized = 0;
	char _editor_debugger_instance_token[100];
	IAGSEditorDebugger *_editor_debugger = nullptr;
	int _break_on_next_script_step = 0;
	volatile int _game_paused_in_debugger = 0;

	/**@}*/

	/**
	 * \defgroup main globals
	 * @{
	 */

	String _appDirectory; // Needed for library loading
	String _cmdGameDataPath;

	const char **_global_argv = nullptr;
	int _global_argc = 0;

	// Startup flags, set from parameters to engine
	int _force_window = 0;
	int _override_start_room = 0;
	bool _justDisplayHelp = false;
	bool _justDisplayVersion = false;
	bool _justRunSetup = false;
	bool _justRegisterGame = false;
	bool _justUnRegisterGame = false;
	bool _justTellInfo = false;
	std::set<String> _tellInfoKeys;
	const char *_loadSaveGameOnStartup = nullptr;

#if ! AGS_PLATFORM_DEFINES_PSP_VARS
	int _psp_video_framedrop = 1;
	int _psp_audio_enabled = 1;
	int _psp_midi_enabled = 1;
	int _psp_ignore_acsetup_cfg_file = 0;
	int _psp_clear_cache_on_room_change = 0;

	int _psp_midi_preload_patches = 0;
	int _psp_audio_cachesize = 10;
	const char *_psp_game_file_name = "";
	const char *_psp_translation = "default";

	int _psp_gfx_renderer = 0;
	int _psp_gfx_scaling = 1;
	int _psp_gfx_smoothing = 0;
	int _psp_gfx_super_sampling = 1;
	int _psp_gfx_smooth_sprites = 0;
#endif

	// Current engine version
	Version _EngineVersion;
	// Lowest savedgame version, accepted by this engine
	Version _SavedgameLowestBackwardCompatVersion;
	// Lowest engine version, which would accept current savedgames
	Version _SavedgameLowestForwardCompatVersion;

	/**@}*/

	/**
	 * \defgroup mouse globals
	 * @{
	 */

	char _currentcursor = 0;
	// virtual mouse cursor coordinates
	int _mousex = 0, _mousey = 0, _numcurso = -1, _hotx = 0, _hoty = 0;
	// real mouse coordinates and bounds
	int _real_mouse_x = 0, _real_mouse_y = 0;
	int _boundx1 = 0, _boundx2 = 99999, _boundy1 = 0, _boundy2 = 99999;
	int _disable_mgetgraphpos = 0;
	char _ignore_bounds = 0;
	AGS::Shared::Bitmap *_mousecurs[MAXCURSORS];

	/**@}*/

	/**
	 * \defgroup quit globals
	 * @{
	 */

	bool _handledErrorInEditor = false;
	char _return_to_roomedit[30] = "\0";
	char _return_to_room[150] = "\0";
	char _quit_message[256] = "\0";

	 /**@}*/

public:
	Globals();
	~Globals();
};

extern Globals *g_globals;

// Macro for accessing a globals member
#define _G(FIELD) (::AGS3::g_globals->_##FIELD)
// Macro for accessing a globals member that was an object in the original,
// but is a pointer to the object in ScummVM, so that we don't need to
// provide the full class/struct definition here in the header file
#define _GP(FIELD) (*::AGS3::g_globals->_##FIELD)

} // namespace AGS3

#endif
