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

#include "ags/shared/core/platform.h"
#include "common/std/set.h"
#include "ags/lib/allegro.h" // allegro_exit
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/engine/ac/game_state.h"
#include "ags/shared/core/def_version.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/main/config.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/main.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/platform/base/sys_main.h"
#include "ags/engine/ac/route_finder.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/string_compat.h"
#include "ags/shared/util/string_utils.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

void main_init(int argc, const char *argv[]) {
	set_our_eip(-999);

	// Init libraries: set text encoding
	set_uformat(U_UTF8);
	set_filename_encoding(U_UNICODE);

	_G(EngineVersion) = Version(ACI_VERSION_STR " " SPECIAL_VERSION);
#if defined (BUILD_STR)
	_G(EngineVersion).BuildInfo = BUILD_STR;
#endif

	_G(platform) = AGSPlatformDriver::GetDriver();
	_G(platform)->SetCommandArgs(argv, argc);
	_G(platform)->MainInit();

	_GP(AssetMgr).reset(new AssetManager());
	_GP(AssetMgr)->SetSearchPriority(Shared::kAssetPriorityDir);
}

String get_engine_string() {
	return String::FromFormat("Adventure Game Studio v%s Interpreter\n"
							  "Copyright (c) 1999-2011 Chris Jones and " ACI_COPYRIGHT_YEARS " others\n"
							  "Engine version %s\n",
							  _G(EngineVersion).ShortString.GetCStr(),
							  get_engine_version_and_build().GetCStr());
}

void main_print_help() {
	_G(platform)->WriteStdOut("%s",
	                          "Usage: ags [OPTIONS] [GAMEFILE or DIRECTORY]\n\n"
	                          //--------------------------------------------------------------------------------|
	                          "Options:\n"
                              "  --background                 Keeps game running in background\n"
                              "                               (this does not work in exclusive fullscreen)\n"
                              "  --clear-cache-on-room-change Clears sprite cache on every room change\n"
	                          "  --conf FILEPATH              Specify explicit config file to read on startup\n"
#if AGS_PLATFORM_OS_WINDOWS
	                          "  --console-attach             Write output to the parent process's console\n"
#endif
	                          "  --fps                        Display fps counter\n"
	                          "  --fullscreen                 Force display mode to fullscreen\n"
	                          "  --gfxdriver <id>             Request graphics driver. Available options:\n"
#if AGS_PLATFORM_OS_WINDOWS
	                          "                                 d3d9, ogl, software\n"
#else
	                          "                                 ogl, software\n"
#endif
	                          "  --gfxfilter FILTER [SCALING]\n"
	                          "                               Request graphics filter. Available options:\n"
							  "                                 stdscale, linear\n"
							  "                               (support may differ between graphic drivers);\n"
							  "                               Scaling is specified as:\n"
							  "                                 proportional, round, stretch,\n"
							  "                                 or an explicit integer multiplier.\n"
	                          "  --help                       Print this help message and stop\n"
	                          "  --loadsavedgame FILEPATH     Load savegame on startup\n"
	                          "  --localuserconf              Read and write user config in the game's \n"
	                          "                               directory rather than using standard system path.\n"
	                          "                               Game directory must be writeable.\n"
	                          "  --log-OUTPUT=GROUP[:LEVEL][,GROUP[:LEVEL]][,...]\n"
	                          "  --log-OUTPUT=+GROUPLIST[:LEVEL]\n"
	                          "                               Setup logging to the chosen OUTPUT with given\n"
	                          "                               log groups and verbosity levels. Groups may\n"
	                          "                               be also defined by a LIST of one-letter IDs,\n"
	                          "                               preceded by '+', e.g. +ABCD:LEVEL. Verbosity may\n"
	                          "                               be also defined by a numberic ID.\n"
	                          "                               OUTPUTs are\n"
	                          "                                 stdout, file, console\n"
	                          "                               (where \"console\" is internal engine's console)\n"
	                          "                               GROUPs are:\n"
	                          "                                 all, main (m), game (g), manobj (o),\n"
	                          "                                 sprcache (c)\n"
	                          "                               LEVELs are:\n"
	                          "                                 all, alert (1), fatal (2), error (3), warn (4),\n"
	                          "                                 info (5), debug (6)\n"
	                          "                               Examples:\n"
	                          "                                 --log-stdout=+mg:debug\n"
	                          "                                 --log-file=all:warn\n"
	                          "  --log-file-path=PATH         Define custom path for the log file\n"
	                          //--------------------------------------------------------------------------------|
	                          "  --no-message-box             Disable alerts as modal message boxes\n"
		                      "  --no-translation             Use default game language on start\n"
	                          "  --noiface                    Don't draw game GUI\n"
	                          "  --noscript                   Don't run room scripts; *WARNING:* unreliable\n"
	                          "  --nospr                      Don't draw room objects and characters\n"
	                          "  --noupdate                   Don't run game update\n"
	                          "  --novideo                    Don't play game videos\n"
                              "  --rotation <MODE>            Screen rotation preferences. MODEs are:\n"
                              "                                 unlocked (0), portrait (1), landscape (2)\n"
#if AGS_PLATFORM_OS_WINDOWS
	                          "  --setup                      Run setup application\n"
#endif
	                          "  --shared-data-dir DIR        Set the shared game data directory\n"
	                          "  --startr <room_number>       Start game by loading certain room.\n"
	                          "  --tell                       Print various information concerning engine\n"
	                          "                                 and the game; for selected output use:\n"
	                          "  --tell-config                Print contents of merged game config\n"
	                          "  --tell-configpath            Print paths to available config files\n"
	                          "  --tell-data                  Print information on game data and its location\n"
	                          "  --tell-gameproperties        Print information on game general settings\n"
	                          "  --tell-engine                Print engine name and version\n"
	                          "  --tell-filepath              Print all filepaths engine uses for the game\n"
	                          "  --tell-graphicdriver         Print list of supported graphic drivers\n"
	                          "\n"
	                          "  --test                       Run game in the test mode\n"
                              "  --translation <name>         Select the given translation on start\n"
		                      "  --version                    Print engine's version and stop\n"
	                          "  --user-data-dir DIR          Set the save game directory\n"
	                          "  --windowed                   Force display mode to windowed\n"
	                          "\n"
	                          "Gamefile options:\n"
	                          "  /dir/path/game/              Launch the game in specified directory\n"
	                          "  /dir/path/game/penguin.exe   Launch penguin.exe\n"
	                          "  [nothing]                    Launch the game in the current directory\n"
	                          //--------------------------------------------------------------------------------|
	                         );
}

int main_process_cmdline(ConfigTree &cfg, int argc, const char *argv[]) {
	int datafile_argv = 0;
	for (int ee = 1; ee < argc; ++ee) {
		const char *arg = argv[ee];
		//
		// Startup options
		//
		if (ags_stricmp(arg, "--help") == 0 || ags_stricmp(arg, "/?") == 0 || ags_stricmp(arg, "-?") == 0) {
			_G(justDisplayHelp) = true;
		}
		if (ags_stricmp(arg, "-v") == 0 || ags_stricmp(arg, "--version") == 0) {
			_G(justDisplayVersion) = true;
		} else if (ags_stricmp(arg, "--updatereg") == 0)
			_G(debug_flags) |= DBG_REGONLY;
		else if ((ags_stricmp(arg, "--startr") == 0) && (ee < argc - 1)) {
			_G(override_start_room) = atoi(argv[ee + 1]);
			ee++;
		} else if (ags_stricmp(arg, "--noexceptionhandler") == 0) _GP(usetup).disable_exception_handling = true;
		else if (ags_stricmp(arg, "--setup") == 0) {
			_G(justRunSetup) = true;
		} else if ((ags_stricmp(arg, "--loadsavedgame") == 0) && (argc > ee + 1)) {
			_G(loadSaveGameOnStartup) = atoi(argv[ee + 1]);
			ee++;
		} else if ((ags_stricmp(arg, "--enabledebugger") == 0) && (argc > ee + 1)) {
			snprintf(_G(editor_debugger_instance_token), sizeof(_G(editor_debugger_instance_token)), "%s", argv[ee + 1]);
			_G(editor_debugging_enabled) = 1;
			ee++;
		} else if (ags_stricmp(arg, "--conf") == 0 && (argc > ee + 1)) {
			_GP(usetup).conf_path = argv[++ee];
		} else if (ags_stricmp(arg, "--localuserconf") == 0) {
			_GP(usetup).local_user_conf = true;
		} else if (ags_stricmp(arg, "--localuserconf") == 0) {
			_GP(usetup).user_conf_dir = ".";
		} else if ((ags_stricmp(arg, "--user-conf-dir") == 0) && (argc > ee + 1)) {
			_GP(usetup).user_conf_dir = argv[++ee];
		} else if (ags_stricmp(arg, "--runfromide") == 0 && (argc > ee + 4)) {
			_GP(usetup).install_dir = argv[ee + 1];
			_GP(usetup).opt_data_dir = argv[ee + 2];
			_GP(usetup).opt_audio_dir = argv[ee + 3];
			_GP(usetup).opt_voice_dir = argv[ee + 4];
			ee += 4;
		} else if (ags_stricmp(arg, "--takeover") == 0) {
			if (argc < ee + 2)
				break;
			_GP(play).takeover_data = atoi(argv[ee + 1]);
			snprintf(_GP(play).takeover_from, sizeof(_GP(play).takeover_from), "%s", argv[ee + 2]);
			ee += 2;
		} else if (ags_stricmp(arg, "--clear-cache-on-room-change") == 0) {
			cfg["misc"]["clear_cache_on_room_change"] = "1";
		} else if (ags_strnicmp(arg, "--tell", 6) == 0) {
			if (arg[6] == 0)
				_G(tellInfoKeys).insert(String("all"));
			else if (arg[6] == '-' && arg[7] != 0)
				_G(tellInfoKeys).insert(String(arg + 7));
		}
		//
		// Config overrides
		//
		else if ((ags_stricmp(arg, "--user-data-dir") == 0) && (argc > ee + 1))
			cfg["misc"]["user_data_dir"] = argv[++ee];
		else if ((ags_stricmp(arg, "--shared-data-dir") == 0) && (argc > ee + 1))
			cfg["misc"]["shared_data_dir"] = argv[++ee];
		else if (ags_stricmp(arg, "--windowed") == 0)
			cfg["graphics"]["windowed"] = "1";
		else if (ags_stricmp(arg, "--fullscreen") == 0)
			cfg["graphics"]["windowed"] = "0";
		else if ((ags_stricmp(arg, "--gfxdriver") == 0) && (argc > ee + 1)) {
			cfg["graphics"]["driver"] = argv[++ee];
		} else if ((ags_stricmp(arg, "--gfxfilter") == 0) && (argc > ee + 1)) {
			cfg["graphics"]["filter"] = argv[++ee];
			if (argc > ee + 1 && argv[ee + 1][0] != '-') {
				// NOTE: we make an assumption here that if user provides scaling
				// multiplier, then it's meant to be applied to windowed mode only;
				// Otherwise the scaling style is applied to both.
				String scale_value = argv[++ee];
				int scale_mul = StrUtil::StringToInt(scale_value);
				if (scale_mul > 0) {
					cfg["graphics"]["window"] = String::FromFormat("x%d", scale_mul);
					cfg["graphics"]["game_scale_win"] = "round";
				} else {
					cfg["graphics"]["game_scale_fs"] = scale_value;
					cfg["graphics"]["game_scale_win"] = scale_value;
				}
			}
		} else if ((ags_stricmp(arg, "--translation") == 0) && (argc > ee + 1)) {
			cfg["language"]["translation"] = argv[++ee];
		} else if (ags_stricmp(arg, "--no-translation") == 0) {
			cfg["language"]["translation"] = "";
		} else if (ags_stricmp(arg, "--background") == 0) {
			cfg["override"]["multitasking"] = "1";
		} else if (ags_stricmp(arg, "--fps") == 0)
			cfg["misc"]["show_fps"] = "1";
		else if (ags_stricmp(arg, "--test") == 0) _G(debug_flags) |= DBG_DEBUGMODE;
		else if (ags_stricmp(arg, "--noiface") == 0) _G(debug_flags) |= DBG_NOIFACE;
		else if (ags_stricmp(arg, "--nosprdisp") == 0) _G(debug_flags) |= DBG_NODRAWSPRITES;
		else if (ags_stricmp(arg, "--nospr") == 0) _G(debug_flags) |= DBG_NOOBJECTS;
		else if (ags_stricmp(arg, "--noupdate") == 0) _G(debug_flags) |= DBG_NOUPDATE;
		else if (ags_stricmp(arg, "--nosound") == 0) _G(debug_flags) |= DBG_NOSFX;
		else if (ags_stricmp(arg, "--nomusic") == 0) _G(debug_flags) |= DBG_NOMUSIC;
		else if (ags_stricmp(arg, "--noscript") == 0) _G(debug_flags) |= DBG_NOSCRIPT;
		else if (ags_stricmp(arg, "--novideo") == 0) _G(debug_flags) |= DBG_NOVIDEO;
		else if (ags_stricmp(arg, "--rotation") == 0 && (argc > ee + 1)) {
			cfg["graphics"]["rotation"] = argv[++ee];
		} else if (ags_strnicmp(arg, "--log-", 6) == 0 && arg[6] != 0) {
			String logarg = arg + 6;
			size_t split_at = logarg.FindChar('=');
			if (split_at != String::NoIndex)
				cfg["log"][logarg.Left(split_at)] = logarg.Mid(split_at + 1);
			else
				cfg["log"][logarg] = "";
		} else if (arg[0] != '-') datafile_argv = ee;
	}

	if (datafile_argv > 0) {
		_G(cmdGameDataPath) = _G(platform)->GetCommandArg(datafile_argv);
	}

	if (_G(tellInfoKeys).size() > 0)
		_G(justTellInfo) = true;

	return 0;
}

void main_set_gamedir(int argc, const char *argv[]) {
	_G(appPath) = Path::MakeAbsolutePath(_G(platform)->GetCommandArg(0));
	_G(appDirectory) = Path::GetDirectoryPath(_G(appPath));

	// TODO: remove following when supporting unicode paths
	{
		// It looks like Allegro library does not like ANSI (ACP) paths.
		// When *not* working in U_UNICODE filepath mode, whenever it gets
		// current directory for its own operations, it "fixes" it by
		// substituting non-ASCII symbols with '^'.
		// Here we explicitly set current directory to ASCII path.
		String cur_dir = Directory::GetCurrentDirectory();
		String path = Path::GetPathInASCII(cur_dir);
		if (!path.IsEmpty())
			Directory::SetCurrentDirectory(Path::MakeAbsolutePath(path));
		else
			Debug::Printf(kDbgMsg_Error, "Unable to determine current directory: GetPathInASCII failed.\nArg: %s", cur_dir.GetCStr());
	}
}

} // namespace AGS3
