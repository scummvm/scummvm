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

//
// Entry point of the application here.
//
//
// For Windows main() function is really called _mangled_main and is called
// not by system, but from insides of allegro library.
// (See allegro\platform\alwin.h)
// What about other platforms?
//

#include "ags/shared/core/platform.h"
#define AGS_PLATFORM_DEFINES_PSP_VARS (AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_ANDROID)

//include <set>
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/gamesetup.h"
#include "ags/shared/ac/gamestate.h"
#include "ags/shared/core/def_version.h"
#include "ags/shared/debug/debugger.h"
#include "ags/shared/debug/debug_log.h"
#include "ags/shared/debug/out.h"
#include "ags/shared/main/config.h"
#include "ags/shared/main/engine.h"
#include "ags/shared/main/mainheader.h"
#include "ags/shared/main/main.h"
#include "ags/shared/platform/base/agsplatformdriver.h"
#include "ags/shared/ac/route_finder.h"
#include "ags/shared/core/assetmanager.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/string_compat.h"

#if AGS_PLATFORM_OS_WINDOWS
#include "ags/shared/platform/windows/win_ex_handling.h"
#endif
#if AGS_PLATFORM_DEBUG
#include "ags/shared/test/test_all.h"
#endif

#if AGS_PLATFORM_OS_WINDOWS && !AGS_PLATFORM_DEBUG
#define USE_CUSTOM_EXCEPTION_HANDLER
#endif

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

String appDirectory; // Needed for library loading
String cmdGameDataPath;

char **global_argv = nullptr;
int    global_argc = 0;


extern GameSetup usetup;
extern GameState play;
extern int our_eip;
extern AGSPlatformDriver *platform;
extern int convert_16bit_bgr;
extern int editor_debugging_enabled;
extern int editor_debugging_initialized;
extern char editor_debugger_instance_token[100];


// Startup flags, set from parameters to engine
int force_window = 0;
int override_start_room = 0;
bool justDisplayHelp = false;
bool justDisplayVersion = false;
bool justRunSetup = false;
bool justRegisterGame = false;
bool justUnRegisterGame = false;
bool justTellInfo = false;
bool attachToParentConsole = false;
bool hideMessageBoxes = false;
std::set<String> tellInfoKeys;
const char *loadSaveGameOnStartup = nullptr;

#if ! AGS_PLATFORM_DEFINES_PSP_VARS
int psp_video_framedrop = 1;
int psp_audio_enabled = 1;
int psp_midi_enabled = 1;
int psp_ignore_acsetup_cfg_file = 0;
int psp_clear_cache_on_room_change = 0;

int psp_midi_preload_patches = 0;
int psp_audio_cachesize = 10;
char psp_game_file_name[] = "";
char psp_translation[] = "default";

int psp_gfx_renderer = 0;
int psp_gfx_scaling = 1;
int psp_gfx_smoothing = 0;
int psp_gfx_super_sampling = 1;
int psp_gfx_smooth_sprites = 0;
#endif


void main_pre_init() {
	our_eip = -999;
	Common::AssetManager::SetSearchPriority(Common::kAssetPriorityDir);
	play.takeover_data = 0;
}

void main_create_platform_driver() {
	platform = AGSPlatformDriver::GetDriver();
}

// this needs to be updated if the "play" struct changes
#define SVG_VERSION_BWCOMPAT_MAJOR      3
#define SVG_VERSION_BWCOMPAT_MINOR      2
#define SVG_VERSION_BWCOMPAT_RELEASE    0
#define SVG_VERSION_BWCOMPAT_REVISION   1103
// CHECKME: we may lower this down, if we find that earlier versions may still
// load new savedgames
#define SVG_VERSION_FWCOMPAT_MAJOR      3
#define SVG_VERSION_FWCOMPAT_MINOR      2
#define SVG_VERSION_FWCOMPAT_RELEASE    1
#define SVG_VERSION_FWCOMPAT_REVISION   1111

// Current engine version
AGS::Shared::Version EngineVersion;
// Lowest savedgame version, accepted by this engine
AGS::Shared::Version SavedgameLowestBackwardCompatVersion;
// Lowest engine version, which would accept current savedgames
AGS::Shared::Version SavedgameLowestForwardCompatVersion;

void main_init(int argc, char *argv[]) {
	EngineVersion = Version(ACI_VERSION_STR " " SPECIAL_VERSION);
#if defined (BUILD_STR)
	EngineVersion.BuildInfo = BUILD_STR;
#endif
	SavedgameLowestBackwardCompatVersion = Version(SVG_VERSION_BWCOMPAT_MAJOR, SVG_VERSION_BWCOMPAT_MINOR, SVG_VERSION_BWCOMPAT_RELEASE, SVG_VERSION_BWCOMPAT_REVISION);
	SavedgameLowestForwardCompatVersion = Version(SVG_VERSION_FWCOMPAT_MAJOR, SVG_VERSION_FWCOMPAT_MINOR, SVG_VERSION_FWCOMPAT_RELEASE, SVG_VERSION_FWCOMPAT_REVISION);

	Common::AssetManager::CreateInstance();
	main_pre_init();
	main_create_platform_driver();

	global_argv = argv;
	global_argc = argc;
}

String get_engine_string() {
	return String::FromFormat("Adventure Game Studio v%s Interpreter\n"
		"Copyright (c) 1999-2011 Chris Jones and " ACI_COPYRIGHT_YEARS " others\n"
#ifdef BUILD_STR
		"ACI version %s (Build: %s)\n",
		EngineVersion.ShortString.GetCStr(), EngineVersion.LongString.GetCStr(), EngineVersion.BuildInfo.GetCStr());
#else
		"ACI version %s\n", EngineVersion.ShortString.GetCStr(), EngineVersion.LongString.GetCStr());
#endif
}

extern char return_to_roomedit[30];
extern char return_to_room[150];

void main_print_help() {
	platform->WriteStdOut(
		"Usage: ags [OPTIONS] [GAMEFILE or DIRECTORY]\n\n"
		//--------------------------------------------------------------------------------|
		"Options:\n"
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
		"                                 hqx, linear, none, stdscale\n"
		"                                 (support differs between graphic drivers);\n"
		"                                 scaling is specified by integer number\n"
		"  --help                       Print this help message and stop\n"
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
		"                                 script (s), sprcache (c)\n"
		"                               LEVELs are:\n"
		"                                 all, alert (1), fatal (2), error (3), warn (4),\n"
		"                                 info (5), debug (6)\n"
		"                               Examples:\n"
		"                                 --log-stdout=+mgs:debug\n"
		"                                 --log-file=all:warn\n"
		"  --log-file-path=PATH         Define custom path for the log file\n"
		//--------------------------------------------------------------------------------|
#if AGS_PLATFORM_OS_WINDOWS
		"  --no-message-box             Disable reporting of alerts to message boxes\n"
		"  --setup                      Run setup application\n"
#endif
		"  --tell                       Print various information concerning engine\n"
		"                                 and the game; for selected output use:\n"
		"  --tell-config                Print contents of merged game config\n"
		"  --tell-configpath            Print paths to available config files\n"
		"  --tell-data                  Print information on game data and its location\n"
		"  --tell-engine                Print engine name and version\n"
		"  --tell-graphicdriver         Print list of supported graphic drivers\n"
		"\n"
		"  --version                    Print engine's version and stop\n"
		"  --windowed                   Force display mode to windowed\n"
		"\n"
		"Gamefile options:\n"
		"  /dir/path/game/              Launch the game in specified directory\n"
		"  /dir/path/game/penguin.exe   Launch penguin.exe\n"
		"  [nothing]                    Launch the game in the current directory\n"
		//--------------------------------------------------------------------------------|
	);
}

static int main_process_cmdline(ConfigTree &cfg, int argc, char *argv[]) {
	int datafile_argv = 0;
	for (int ee = 1; ee < argc; ++ee) {
		const char *arg = argv[ee];
		//
		// Startup options
		//
		if (ags_stricmp(arg, "--help") == 0 || ags_stricmp(arg, "/?") == 0 || ags_stricmp(arg, "-?") == 0) {
			justDisplayHelp = true;
			return 0;
		}
		if (ags_stricmp(arg, "-v") == 0 || ags_stricmp(arg, "--version") == 0) {
			justDisplayVersion = true;
			return 0;
		} else if (ags_stricmp(arg, "-updatereg") == 0)
			debug_flags |= DBG_REGONLY;
#if AGS_PLATFORM_DEBUG
		else if ((ags_stricmp(arg, "--startr") == 0) && (ee < argc - 1)) {
			override_start_room = atoi(argv[ee + 1]);
			ee++;
		}
#endif
		else if ((ags_stricmp(arg, "--testre") == 0) && (ee < argc - 2)) {
			strcpy(return_to_roomedit, argv[ee + 1]);
			strcpy(return_to_room, argv[ee + 2]);
			ee += 2;
		} else if (ags_stricmp(arg, "-noexceptionhandler") == 0) usetup.disable_exception_handling = true;
		else if (ags_stricmp(arg, "--setup") == 0) {
			justRunSetup = true;
		} else if (ags_stricmp(arg, "-registergame") == 0) {
			justRegisterGame = true;
		} else if (ags_stricmp(arg, "-unregistergame") == 0) {
			justUnRegisterGame = true;
		} else if ((ags_stricmp(arg, "-loadsavedgame") == 0) && (argc > ee + 1)) {
			loadSaveGameOnStartup = argv[ee + 1];
			ee++;
		} else if ((ags_stricmp(arg, "--enabledebugger") == 0) && (argc > ee + 1)) {
			strcpy(editor_debugger_instance_token, argv[ee + 1]);
			editor_debugging_enabled = 1;
			force_window = 1;
			ee++;
		} else if (ags_stricmp(arg, "--runfromide") == 0 && (argc > ee + 3)) {
			usetup.install_dir = argv[ee + 1];
			usetup.install_audio_dir = argv[ee + 2];
			usetup.install_voice_dir = argv[ee + 3];
			ee += 3;
		} else if (ags_stricmp(arg, "--takeover") == 0) {
			if (argc < ee + 2)
				break;
			play.takeover_data = atoi(argv[ee + 1]);
			strncpy(play.takeover_from, argv[ee + 2], 49);
			play.takeover_from[49] = 0;
			ee += 2;
		} else if (ags_strnicmp(arg, "--tell", 6) == 0) {
			if (arg[6] == 0)
				tellInfoKeys.insert(String("all"));
			else if (arg[6] == '-' && arg[7] != 0)
				tellInfoKeys.insert(String(arg + 7));
		}
		//
		// Config overrides
		//
		else if (ags_stricmp(arg, "-windowed") == 0 || ags_stricmp(arg, "--windowed") == 0)
			force_window = 1;
		else if (ags_stricmp(arg, "-fullscreen") == 0 || ags_stricmp(arg, "--fullscreen") == 0)
			force_window = 2;
		else if ((ags_stricmp(arg, "-gfxdriver") == 0 || ags_stricmp(arg, "--gfxdriver") == 0) && (argc > ee + 1)) {
			INIwritestring(cfg, "graphics", "driver", argv[++ee]);
		} else if ((ags_stricmp(arg, "-gfxfilter") == 0 || ags_stricmp(arg, "--gfxfilter") == 0) && (argc > ee + 1)) {
			// NOTE: we make an assumption here that if user provides scaling factor,
			// this factor means to be applied to windowed mode only.
			INIwritestring(cfg, "graphics", "filter", argv[++ee]);
			if (argc > ee + 1 && argv[ee + 1][0] != '-')
				INIwritestring(cfg, "graphics", "game_scale_win", argv[++ee]);
			else
				INIwritestring(cfg, "graphics", "game_scale_win", "max_round");
		} else if (ags_stricmp(arg, "--fps") == 0) display_fps = kFPS_Forced;
		else if (ags_stricmp(arg, "--test") == 0) debug_flags |= DBG_DEBUGMODE;
		else if (ags_stricmp(arg, "-noiface") == 0) debug_flags |= DBG_NOIFACE;
		else if (ags_stricmp(arg, "-nosprdisp") == 0) debug_flags |= DBG_NODRAWSPRITES;
		else if (ags_stricmp(arg, "-nospr") == 0) debug_flags |= DBG_NOOBJECTS;
		else if (ags_stricmp(arg, "-noupdate") == 0) debug_flags |= DBG_NOUPDATE;
		else if (ags_stricmp(arg, "-nosound") == 0) debug_flags |= DBG_NOSFX;
		else if (ags_stricmp(arg, "-nomusic") == 0) debug_flags |= DBG_NOMUSIC;
		else if (ags_stricmp(arg, "-noscript") == 0) debug_flags |= DBG_NOSCRIPT;
		else if (ags_stricmp(arg, "-novideo") == 0) debug_flags |= DBG_NOVIDEO;
		else if (ags_stricmp(arg, "-dbgscript") == 0) debug_flags |= DBG_DBGSCRIPT;
		else if (ags_strnicmp(arg, "--log-", 6) == 0 && arg[6] != 0) {
			String logarg = arg + 6;
			size_t split_at = logarg.FindChar('=');
			if (split_at >= 0)
				cfg["log"][logarg.Left(split_at)] = logarg.Mid(split_at + 1);
			else
				cfg["log"][logarg] = "";
		} else if (ags_stricmp(arg, "--console-attach") == 0) attachToParentConsole = true;
		else if (ags_stricmp(arg, "--no-message-box") == 0) hideMessageBoxes = true;
		//
		// Special case: data file location
		//
		else if (arg[0] != '-') datafile_argv = ee;
	}

	if (datafile_argv > 0) {
		cmdGameDataPath = GetPathFromCmdArg(datafile_argv);
	} else {
		// assign standard path for mobile/consoles (defined in their own platform implementation)
		cmdGameDataPath = psp_game_file_name;
	}

	if (tellInfoKeys.size() > 0)
		justTellInfo = true;

	return 0;
}

void main_set_gamedir(int argc, char *argv[]) {
	appDirectory = Path::GetDirectoryPath(GetPathFromCmdArg(0));

	if ((loadSaveGameOnStartup != nullptr) && (argv[0] != nullptr)) {
		// When launched by double-clicking a save game file, the curdir will
		// be the save game folder unless we correct it
		Directory::SetCurrentDirectory(appDirectory);
	} else {
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

String GetPathFromCmdArg(int arg_index) {
	if (arg_index < 0 || arg_index >= global_argc)
		return "";
	String path = Path::GetCmdLinePathInASCII(global_argv[arg_index], arg_index);
	if (!path.IsEmpty())
		return Path::MakeAbsolutePath(path);
	Debug::Printf(kDbgMsg_Error, "Unable to determine path: GetCmdLinePathInASCII failed.\nCommand line argument %i: %s", arg_index, global_argv[arg_index]);
	return global_argv[arg_index];
}

const char *get_allegro_error() {
	return allegro_error;
}

const char *set_allegro_error(const char *format, ...) {
	va_list argptr;
	va_start(argptr, format);
	uvszprintf(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text(format), argptr);
	va_end(argptr);
	return allegro_error;
}

int ags_entry_point(int argc, char *argv[]) {

#ifdef AGS_RUN_TESTS
	Test_DoAllTests();
#endif
	main_init(argc, argv);

#if AGS_PLATFORM_OS_WINDOWS
	setup_malloc_handling();
#endif
	debug_flags = 0;

	ConfigTree startup_opts;
	int res = main_process_cmdline(startup_opts, argc, argv);
	if (res != 0)
		return res;

	if (attachToParentConsole)
		platform->AttachToParentConsole();

	if (justDisplayVersion) {
		platform->WriteStdOut(get_engine_string());
		return EXIT_NORMAL;
	}

	if (justDisplayHelp) {
		main_print_help();
		return EXIT_NORMAL;
	}

	if (!justTellInfo && !hideMessageBoxes)
		platform->SetGUIMode(true);

	init_debug(startup_opts, justTellInfo);
	Debug::Printf(kDbgMsg_Alert, get_engine_string());

	main_set_gamedir(argc, argv);

	// Update shell associations and exit
	if (debug_flags & DBG_REGONLY)
		exit(EXIT_NORMAL);

#ifdef USE_CUSTOM_EXCEPTION_HANDLER
	if (usetup.disable_exception_handling)
#endif
	{
		int result = initialize_engine(startup_opts);
		// TODO: refactor engine shutdown routine (must shutdown and delete everything started and created)
		allegro_exit();
		platform->PostAllegroExit();
		return result;
	}
#ifdef USE_CUSTOM_EXCEPTION_HANDLER
	else {
		return initialize_engine_with_exception_handling(initialize_engine, startup_opts);
	}
#endif
}

} // namespace AGS3
