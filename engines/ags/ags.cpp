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
#include "ags/events.h"
#include "ags/music.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "engines/util.h"

#include "ags/shared/core/platform.h"
#define AGS_PLATFORM_DEFINES_PSP_VARS (AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_ANDROID)

#include "ags/lib/std/set.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/globals.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/shared/core/def_version.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/game/savegame.h"
#include "ags/engine/main/config.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/mainheader.h"
#include "ags/engine/main/main.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/engine/ac/route_finder.h"
#include "ags/shared/core/assetmanager.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/path.h"

#ifdef ENABLE_AGS_TESTS
#include "ags/tests/test_all.h"
#endif

namespace AGS3 {

extern char return_to_roomedit[30];
extern char return_to_room[150];

using namespace Shared;
using namespace Engine;

extern HSaveError load_game(const String &path, int slotNumber, bool &data_overwritten);

extern GameSetup usetup;
extern GameState play;
extern int our_eip;
extern AGSPlatformDriver *platform;
extern int convert_16bit_bgr;
extern int editor_debugging_enabled;
extern int editor_debugging_initialized;
extern char editor_debugger_instance_token[100];

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

extern void quit_free();

void main_pre_init() {
	our_eip = -999;
	Shared::AssetManager::SetSearchPriority(Shared::kAssetPriorityDir);
	play.takeover_data = 0;
}

void main_create_platform_driver() {
	platform = AGSPlatformDriver::GetDriver();
}

void main_init(int argc, const char *argv[]) {
	_G(EngineVersion) = Version(ACI_VERSION_STR " " SPECIAL_VERSION);

	_G(SavedgameLowestBackwardCompatVersion) = Version(SVG_VERSION_BWCOMPAT_MAJOR, SVG_VERSION_BWCOMPAT_MINOR, SVG_VERSION_BWCOMPAT_RELEASE, SVG_VERSION_BWCOMPAT_REVISION);
	_G(SavedgameLowestForwardCompatVersion) = Version(SVG_VERSION_FWCOMPAT_MAJOR, SVG_VERSION_FWCOMPAT_MINOR, SVG_VERSION_FWCOMPAT_RELEASE, SVG_VERSION_FWCOMPAT_REVISION);

	Shared::AssetManager::CreateInstance();
	main_pre_init();
	main_create_platform_driver();

	_G(global_argv) = argv;
	_G(global_argc) = argc;
}

String get_engine_string() {
	return String::FromFormat("Adventure Game Studio v%s Interpreter\n"
		"Copyright (c) 1999-2011 Chris Jones and " ACI_COPYRIGHT_YEARS " others\n"
		"ACI version %s\n", _G(EngineVersion).ShortString.GetCStr(), _G(EngineVersion).LongString.GetCStr());
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
			_G(justDisplayHelp) = true;
			return 0;
		}
		if (scumm_stricmp(arg, "-v") == 0 || scumm_stricmp(arg, "--version") == 0) {
			_G(justDisplayVersion) = true;
			return 0;
		} else if (scumm_stricmp(arg, "-updatereg") == 0)
			debug_flags |= DBG_REGONLY;
#if AGS_PLATFORM_DEBUG
		else if ((scumm_stricmp(arg, "--startr") == 0) && (ee < argc - 1)) {
			_G(override_start_room) = atoi(argv[ee + 1]);
			ee++;
		}
#endif
		else if ((scumm_stricmp(arg, "--testre") == 0) && (ee < argc - 2)) {
			strncpy(return_to_roomedit, argv[ee + 1], 30);
			strncpy(return_to_room, argv[ee + 2], 150);
			ee += 2;
		} else if (scumm_stricmp(arg, "-noexceptionhandler") == 0) usetup.disable_exception_handling = true;
		else if (scumm_stricmp(arg, "--setup") == 0) {
			_G(justRunSetup) = true;
		} else if (scumm_stricmp(arg, "-registergame") == 0) {
			_G(justRegisterGame) = true;
		} else if (scumm_stricmp(arg, "-unregistergame") == 0) {
			_G(justUnRegisterGame) = true;
		} else if ((scumm_stricmp(arg, "-loadsavedgame") == 0) && (argc > ee + 1)) {
			_G(loadSaveGameOnStartup) = argv[ee + 1];
			ee++;
		} else if ((scumm_stricmp(arg, "--enabledebugger") == 0) && (argc > ee + 1)) {
			strcpy(editor_debugger_instance_token, argv[ee + 1]);
			editor_debugging_enabled = 1;
			_G(force_window) = 1;
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
				_G(tellInfoKeys).insert(String("all"));
			else if (arg[6] == '-' && arg[7] != 0)
				_G(tellInfoKeys).insert(String(arg + 7));
		}
		//
		// Config overrides
		//
		else if (scumm_stricmp(arg, "-windowed") == 0 || scumm_stricmp(arg, "--windowed") == 0)
			_G(force_window) = 1;
		else if (scumm_stricmp(arg, "-fullscreen") == 0 || scumm_stricmp(arg, "--fullscreen") == 0)
			_G(force_window) = 2;
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
		_G(cmdGameDataPath) = argv[datafile_argv];
	} else {
		// assign standard path for mobile/consoles (defined in their own platform implementation)
		_G(cmdGameDataPath) = _G(psp_game_file_name);
	}

	if (!_G(tellInfoKeys).empty())
		_G(justTellInfo) = true;

	return 0;
}

void main_set_gamedir(int argc, const char *argv[]) {
	_G(appDirectory) = Path::GetDirectoryPath("./");
#ifdef DEPRECATED
	if ((_G(loadSaveGameOnStartup) != nullptr) && (argv[0] != nullptr)) {
		// When launched by double-clicking a save game file, the curdir will
		// be the save game folder unless we correct it
		Directory::SetCurrentDirectory(_G(appDirectory));
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
#endif
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
		_gameDescription(gameDesc), _randomSource("AGS"), _events(nullptr), _music(nullptr),
		_rawScreen(nullptr), _screen(nullptr), _gfxDriver(nullptr),
		_globals(nullptr) {
	g_vm = this;
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugGraphics, "Graphics", "Graphics debug level");

	_events = new EventsManager();
	_music = new Music(_mixer);
	_globals = new ::AGS3::Globals();
}

AGSEngine::~AGSEngine() {
	delete _screen;
	delete _rawScreen;
	delete _events;
	delete _music;
	delete _globals;
}

uint32 AGSEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Error AGSEngine::run() {
	const char *filename = _gameDescription->desc.filesDescriptions[0].fileName;
	const char *ARGV[] = { "scummvm.exe", filename };
	const int ARGC = 2;

#ifdef ENABLE_AGS_TESTS
	AGS3::Test_DoAllTests();
	return Common::kNoError;
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

	if (_G(justDisplayVersion)) {
		AGS3::platform->WriteStdOut(AGS3::get_engine_string());
		return Common::kNoError;
	}

	if (_G(justDisplayHelp)) {
		AGS3::main_print_help();
		return Common::kNoError;
	}

	if (!_G(justTellInfo))
		AGS3::platform->SetGUIMode(true);
	AGS3::init_debug(startup_opts, _G(justTellInfo));
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

		// Do shutdown stuff
		::AGS3::quit_free();

		return result ? Common::kUnknownError : Common::kNoError;
	}
#ifdef USE_CUSTOM_EXCEPTION_HANDLER
	else {
		return initialize_engine_with_exception_handling(initialize_engine, startup_opts);
	}
#endif
}

SaveStateList AGSEngine::listSaves() const {
	return getMetaEngine().listSaves(_targetName.c_str());
}

void AGSEngine::setGraphicsMode(size_t w, size_t h) {
	Graphics::PixelFormat FORMAT(4, 8, 8, 8, 8, 24, 16, 8, 0);
	initGraphics(w, h, &FORMAT);

	_rawScreen = new Graphics::Screen();
	_screen = new ::AGS3::BITMAP(_rawScreen);
}

Common::Error AGSEngine::loadGameState(int slot) {
	bool dataOverwritten;
	(void)AGS3::load_game("", slot, dataOverwritten);
	return Common::kNoError;
}

Common::Error AGSEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	(void)AGS3::save_game(slot, desc.c_str());
	return Common::kNoError;
}

void AGSEngine::GUIError(const Common::String &msg) {
	GUIErrorMessage(msg);
}

} // namespace AGS
