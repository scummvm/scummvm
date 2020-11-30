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

#include "ags/ags.h"
#include "ags/detection.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"

#include "ags/shared/core/platform.h"
#define AGS_PLATFORM_DEFINES_PSP_VARS (AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_ANDROID)

#include "ags/lib/std/set.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/shared/core/def_version.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/main/config.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/mainheader.h"
#include "ags/engine/main/main.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/engine/ac/route_finder.h"
#include "ags/shared/core/assetmanager.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/path.h"


namespace AGS3 {

extern char return_to_roomedit[30];
extern char return_to_room[150];

using namespace Shared;
using namespace Engine;

String appDirectory; // Needed for library loading
String cmdGameDataPath;

const char **global_argv = nullptr;
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
Version EngineVersion;
// Lowest savedgame version, accepted by this engine
Version SavedgameLowestBackwardCompatVersion;
// Lowest engine version, which would accept current savedgames
Version SavedgameLowestForwardCompatVersion;


void main_pre_init() {
	our_eip = -999;
	Shared::AssetManager::SetSearchPriority(Shared::kAssetPriorityDir);
	play.takeover_data = 0;
}

void main_create_platform_driver() {
	platform = AGSPlatformDriver::GetDriver();
}

void main_init(int argc, const char *argv[]) {
	EngineVersion = Version(ACI_VERSION_STR " " SPECIAL_VERSION);

	SavedgameLowestBackwardCompatVersion = Version(SVG_VERSION_BWCOMPAT_MAJOR, SVG_VERSION_BWCOMPAT_MINOR, SVG_VERSION_BWCOMPAT_RELEASE, SVG_VERSION_BWCOMPAT_REVISION);
	SavedgameLowestForwardCompatVersion = Version(SVG_VERSION_FWCOMPAT_MAJOR, SVG_VERSION_FWCOMPAT_MINOR, SVG_VERSION_FWCOMPAT_RELEASE, SVG_VERSION_FWCOMPAT_REVISION);

	Shared::AssetManager::CreateInstance();
	main_pre_init();
	main_create_platform_driver();

	global_argv = argv;
	global_argc = argc;
}

String get_engine_string() {
	return String::FromFormat("Adventure Game Studio v%s Interpreter\n"
		"Copyright (c) 1999-2011 Chris Jones and " ACI_COPYRIGHT_YEARS " others\n"
		"ACI version %s\n", EngineVersion.ShortString.GetCStr(), EngineVersion.LongString.GetCStr());
}

void main_print_help() {
	// No implementation
}

static int main_process_cmdline(ConfigTree &cfg, int argc, const char *argv[]) {
	int datafile_argv = 0;
	for (int ee = 1; ee < argc; ++ee) {
		const char *arg = argv[ee];
		//
		// Startup options
		//
		if (scumm_stricmp(arg, "--help") == 0 || scumm_stricmp(arg, "/?") == 0 || scumm_stricmp(arg, "-?") == 0) {
			justDisplayHelp = true;
			return 0;
		}
		if (scumm_stricmp(arg, "-v") == 0 || scumm_stricmp(arg, "--version") == 0) {
			justDisplayVersion = true;
			return 0;
		} else if (scumm_stricmp(arg, "-updatereg") == 0)
			debug_flags |= DBG_REGONLY;
#if AGS_PLATFORM_DEBUG
		else if ((scumm_stricmp(arg, "--startr") == 0) && (ee < argc - 1)) {
			override_start_room = atoi(argv[ee + 1]);
			ee++;
		}
#endif
		else if ((scumm_stricmp(arg, "--testre") == 0) && (ee < argc - 2)) {
			strcpy(return_to_roomedit, argv[ee + 1]);
			strcpy(return_to_room, argv[ee + 2]);
			ee += 2;
		} else if (scumm_stricmp(arg, "-noexceptionhandler") == 0) usetup.disable_exception_handling = true;
		else if (scumm_stricmp(arg, "--setup") == 0) {
			justRunSetup = true;
		} else if (scumm_stricmp(arg, "-registergame") == 0) {
			justRegisterGame = true;
		} else if (scumm_stricmp(arg, "-unregistergame") == 0) {
			justUnRegisterGame = true;
		} else if ((scumm_stricmp(arg, "-loadsavedgame") == 0) && (argc > ee + 1)) {
			loadSaveGameOnStartup = argv[ee + 1];
			ee++;
		} else if ((scumm_stricmp(arg, "--enabledebugger") == 0) && (argc > ee + 1)) {
			strcpy(editor_debugger_instance_token, argv[ee + 1]);
			editor_debugging_enabled = 1;
			force_window = 1;
			ee++;
		} else if (scumm_stricmp(arg, "--runfromide") == 0 && (argc > ee + 3)) {
			usetup.install_dir = argv[ee + 1];
			usetup.install_audio_dir = argv[ee + 2];
			usetup.install_voice_dir = argv[ee + 3];
			ee += 3;
		} else if (scumm_stricmp(arg, "--takeover") == 0) {
			if (argc < ee + 2)
				break;
			play.takeover_data = atoi(argv[ee + 1]);
			strncpy(play.takeover_from, argv[ee + 2], 49);
			play.takeover_from[49] = 0;
			ee += 2;
		} else if (scumm_strnicmp(arg, "--tell", 6) == 0) {
			if (arg[6] == 0)
				tellInfoKeys.insert(String("all"));
			else if (arg[6] == '-' && arg[7] != 0)
				tellInfoKeys.insert(String(arg + 7));
		}
		//
		// Config overrides
		//
		else if (scumm_stricmp(arg, "-windowed") == 0 || scumm_stricmp(arg, "--windowed") == 0)
			force_window = 1;
		else if (scumm_stricmp(arg, "-fullscreen") == 0 || scumm_stricmp(arg, "--fullscreen") == 0)
			force_window = 2;
		else if ((scumm_stricmp(arg, "-gfxdriver") == 0 || scumm_stricmp(arg, "--gfxdriver") == 0) && (argc > ee + 1)) {
			INIwritestring(cfg, "graphics", "driver", argv[++ee]);
		} else if ((scumm_stricmp(arg, "-gfxfilter") == 0 || scumm_stricmp(arg, "--gfxfilter") == 0) && (argc > ee + 1)) {
			// NOTE: we make an assumption here that if user provides scaling factor,
			// this factor means to be applied to windowed mode only.
			INIwritestring(cfg, "graphics", "filter", argv[++ee]);
			if (argc > ee + 1 && argv[ee + 1][0] != '-')
				INIwritestring(cfg, "graphics", "game_scale_win", argv[++ee]);
			else
				INIwritestring(cfg, "graphics", "game_scale_win", "max_round");
		} else if (scumm_stricmp(arg, "--fps") == 0) display_fps = kFPS_Forced;
		else if (scumm_stricmp(arg, "--test") == 0) debug_flags |= DBG_DEBUGMODE;
		else if (scumm_stricmp(arg, "-noiface") == 0) debug_flags |= DBG_NOIFACE;
		else if (scumm_stricmp(arg, "-nosprdisp") == 0) debug_flags |= DBG_NODRAWSPRITES;
		else if (scumm_stricmp(arg, "-nospr") == 0) debug_flags |= DBG_NOOBJECTS;
		else if (scumm_stricmp(arg, "-noupdate") == 0) debug_flags |= DBG_NOUPDATE;
		else if (scumm_stricmp(arg, "-nosound") == 0) debug_flags |= DBG_NOSFX;
		else if (scumm_stricmp(arg, "-nomusic") == 0) debug_flags |= DBG_NOMUSIC;
		else if (scumm_stricmp(arg, "-noscript") == 0) debug_flags |= DBG_NOSCRIPT;
		else if (scumm_stricmp(arg, "-novideo") == 0) debug_flags |= DBG_NOVIDEO;
		else if (scumm_stricmp(arg, "-dbgscript") == 0) debug_flags |= DBG_DBGSCRIPT;
		else if (scumm_stricmp(arg, "--log") == 0) INIwriteint(cfg, "misc", "log", 1);
		else if (scumm_stricmp(arg, "--no-log") == 0) INIwriteint(cfg, "misc", "log", 0);
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

	if (!tellInfoKeys.empty())
		justTellInfo = true;

	return 0;
}

void main_set_gamedir(int argc, const char *argv[]) {
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
	return "ERROR"; // allegro_error;
}

#define ALLEGRO_ERROR_SIZE 256
char allegro_error[ALLEGRO_ERROR_SIZE];

const char *set_allegro_error(const char *format, ...) {
	va_list argptr;
	va_start(argptr, format);
	Common::String msg = Common::String::format(format, argptr);
	strncpy(allegro_error, msg.c_str(), ALLEGRO_ERROR_SIZE);

	va_end(argptr);
	return allegro_error;
}

} // namespace AGS3

namespace AGS {

AGSEngine *g_vm;

/*------------------------------------------------------------------*/

AGSEngine::AGSEngine(OSystem *syst, const AGSGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("AGS"), _screen(nullptr), _gfxDriver(nullptr) {
	g_vm = this;
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugGraphics, "Graphics", "Graphics debug level");
}

AGSEngine::~AGSEngine() {
}

uint32 AGSEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Error AGSEngine::run() {
	const char *filename = _gameDescription->desc.filesDescriptions[0].fileName;
	const char *ARGV[] = { nullptr, filename };
	const int ARGC = 2;

#ifdef AGS_RUN_TESTS
	Test_DoAllTests();
#endif
	AGS3::main_init(ARGC, ARGV);

#if AGS_PLATFORM_OS_WINDOWS
	setup_malloc_handling();
#endif
	AGS3::debug_flags = 0;

	AGS3::ConfigTree startup_opts;
	int res = AGS3::main_process_cmdline(startup_opts, ARGC, ARGV);
	if (res != 0)
		return Common::kUnknownError;

	if (AGS3::justDisplayVersion) {
		AGS3::platform->WriteStdOut(AGS3::get_engine_string());
		return Common::kNoError;
	}

	if (AGS3::justDisplayHelp) {
		AGS3::main_print_help();
		return Common::kNoError;
	}

	if (!AGS3::justTellInfo)
		AGS3::platform->SetGUIMode(true);
	AGS3::init_debug(startup_opts, AGS3::justTellInfo);
	AGS3::Debug::Printf("%s", AGS3::get_engine_string().GetNullableCStr());

	AGS3::main_set_gamedir(ARGC, ARGV);

	// Update shell associations and exit
	if (AGS3::debug_flags & DBG_REGONLY)
		return Common::kNoError;

#ifdef USE_CUSTOM_EXCEPTION_HANDLER
	if (usetup.disable_exception_handling)
#endif
	{
		int result = AGS3::initialize_engine(startup_opts);
		// TODO: refactor engine shutdown routine (must shutdown and delete everything started and created)
		AGS3::platform->PostAllegroExit();
		return result ? Common::kUnknownError : Common::kNoError;
	}
#ifdef USE_CUSTOM_EXCEPTION_HANDLER
	else {
		return initialize_engine_with_exception_handling(initialize_engine, startup_opts);
	}
#endif

	return Common::kNoError;
}

SaveStateList AGSEngine::listSaves() const {
	return getMetaEngine().listSaves(_targetName.c_str());
}

} // namespace AGS
