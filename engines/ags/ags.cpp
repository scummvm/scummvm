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
#include "ags/game_scanner.h"
#include "ags/music.h"
#include "ags/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/util.h"
#include "engines/util.h"

#include "ags/shared/core/platform.h"
#define AGS_PLATFORM_DEFINES_PSP_VARS (AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_ANDROID)

#include "ags/lib/std/set.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/game.h"
#include "ags/globals.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/room.h"
#include "ags/shared/core/def_version.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/game/savegame.h"
#include "ags/engine/main/config.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/main_header.h"
#include "ags/engine/main/main.h"
#include "ags/engine/main/quit.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/script/script.h"
#include "ags/engine/ac/route_finder.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/script/cc_options.h"

#ifdef ENABLE_AGS_TESTS
#include "ags/tests/test_all.h"
#endif

namespace AGS {

AGSEngine *g_vm;

AGSEngine::AGSEngine(OSystem *syst, const AGSGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("AGS"), _events(nullptr), _music(nullptr),
	_gfxDriver(nullptr), _globals(nullptr), _forceTextAA(false) {
	g_vm = this;

	_events = new EventsManager();
	_music = new Music();
	_globals = new ::AGS3::Globals();

	Common::String forceAA;
	if (ConfMan.getActiveDomain()->tryGetVal("force_text_aa", forceAA))
		Common::parseBool(forceAA, _forceTextAA);
}

AGSEngine::~AGSEngine() {
	if (_globals && _G(proper_exit) == 0) {
		_G(platform)->DisplayAlert("Error: the program has exited without requesting it.\n"
		                           "Program pointer: %+03d  (write this number down), ACI version %s\n"
		                           "If you see a list of numbers above, please write them down and contact\n"
		                           "developers. Otherwise, note down any other information displayed.",
		                           _G(our_eip), _G(EngineVersion).LongString.GetCStr());
	}

	delete _events;
	delete _music;
	delete _globals;
}

uint32 AGSEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

const PluginVersion *AGSEngine::getNeededPlugins() const {
	return _gameDescription->_plugins;
}

Common::String AGSEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::Error AGSEngine::run() {
	if (debugChannelSet(-1, kDebugScan)) {
		// Scan the given folder and subfolders for unknown games
		AGS3::GameScanner scanner;
		scanner.scan(ConfMan.get("path"));
		return Common::kNoError;
	}

	if (isUnsupportedPre25()) {
		GUIError("The selected game is a completely unsupported pre-2.5 version");
		return Common::kNoError;
	}

	if (is64BitGame()) {
		// If the game file was opened and the engine started, but the
		// size is -1, then it must be a game like Strangeland where
		// the data file is > 2Gb
		GUIError("The selected game has a data file greater than 2Gb, " \
			"which isn't supported by your version of ScummVM yet");
		return Common::kNoError;
	}

	if (debugChannelSet(-1, kDebugScript))
		AGS3::ccSetOption(SCOPT_DEBUGRUN, 1);

#ifdef ENABLE_AGS_TESTS
	AGS3::Test_DoAllTests();
	return Common::kNoError;
#endif

	setDebugger(new AGSConsole(this));

	const char *filename = _gameDescription->desc.filesDescriptions[0].fileName;
	const char *ARGV[] = { "scummvm.exe", filename };
	const int ARGC = 2;
	AGS3::main_init(ARGC, ARGV);

	_G(debug_flags) = 0;

	if (ConfMan.hasKey("display_fps"))
		_G(display_fps) = ConfMan.getBool("display_fps") ? AGS3::kFPS_Forced : AGS3::kFPS_Hide;

	AGS3::ConfigTree startup_opts;
	int res = AGS3::main_process_cmdline(startup_opts, ARGC, ARGV);
	if (res != 0)
		return Common::kUnknownError;

	if (_G(justDisplayVersion)) {
		_G(platform)->WriteStdOut(AGS3::get_engine_string().GetCStr());
		return Common::kNoError;
	}

	if (_G(justDisplayHelp)) {
		AGS3::main_print_help();
		return Common::kNoError;
	}

	if (!_G(justTellInfo))
		_G(platform)->SetGUIMode(true);
	AGS3::init_debug(startup_opts, _G(justTellInfo));
	AGS3::AGS::Shared::Debug::Printf("%s", AGS3::get_engine_string().GetCStr());

	AGS3::main_set_gamedir(ARGC, ARGV);

	// Update shell associations and exit
	if (_G(debug_flags) & DBG_REGONLY)
		return Common::kNoError;

	_G(loadSaveGameOnStartup) = ConfMan.getInt("save_slot");

	syncSoundSettings();
	AGS3::initialize_engine(startup_opts);

	// Do shutdown stuff
	::AGS3::quit_free();

	return Common::kNoError;
}

SaveStateList AGSEngine::listSaves() const {
	return getMetaEngine()->listSaves(_targetName.c_str());
}

bool AGSEngine::getPixelFormat(int depth, Graphics::PixelFormat &format) const {
	Common::List<Graphics::PixelFormat> supportedFormatsList = g_system->getSupportedFormats();

	if (depth == 8) {
		format = Graphics::PixelFormat::createFormatCLUT8();
		return true;
	}

	for (Common::List<Graphics::PixelFormat>::iterator it =
			supportedFormatsList.begin(); it != supportedFormatsList.end(); ++it) {
		if (it->bpp() == depth) {
			format = *it;
			return true;
		}
	}

	return false;
}


void AGSEngine::setGraphicsMode(size_t w, size_t h, int colorDepth) {
	Common::List<Graphics::PixelFormat> supportedFormatsList = g_system->getSupportedFormats();
	Graphics::PixelFormat format;
	if (!getPixelFormat(colorDepth, format))
		error("Unsupported color depth %d", colorDepth);

	initGraphics(w, h, &format);
}

bool AGSEngine::isUnsupportedPre25() const {
	return _gameDescription->desc.extra &&
		!strcmp(_gameDescription->desc.extra, "Pre 2.5");
}

bool AGSEngine::is64BitGame() const {
	Common::File f;
	return f.open(_gameDescription->desc.filesDescriptions[0].fileName)
		&& f.size() == -1;
}

bool AGSEngine::canLoadGameStateCurrently() {
	return !_GP(thisroom).Options.SaveLoadDisabled &&
	       !_G(inside_script) && !_GP(play).fast_forward && !_G(no_blocking_functions);
}

bool AGSEngine::canSaveGameStateCurrently() {
	return !_GP(thisroom).Options.SaveLoadDisabled &&
	       !_G(inside_script) && !_GP(play).fast_forward && !_G(no_blocking_functions);
}

Common::Error AGSEngine::loadGameState(int slot) {
	(void)AGS3::try_restore_save(slot);
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
