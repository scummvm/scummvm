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

#include "ags/ags.h"
#include "ags/detection.h"
#include "ags/events.h"
#include "ags/game_scanner.h"
#include "ags/music.h"
#include "ags/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/compression/clickteam.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/tokenizer.h"
#include "common/util.h"
#include "engines/util.h"

#include "ags/shared/core/platform.h"

#include "common/std/set.h"
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
#include "ags/engine/game/savegame_components.h"
#include "ags/engine/main/config.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/main.h"
#include "ags/engine/main/quit.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/script/cc_instance.h"
#include "ags/engine/script/script.h"
#include "ags/engine/ac/route_finder.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/script/cc_common.h"

#ifdef ENABLE_AGS_TESTS
#include "ags/tests/test_all.h"
#endif

// Include translation.h last as some AGS classes have member such as _sc, which clash with
// macro defined in translation.h.
#include "common/translation.h"

namespace AGS {

AGSEngine *g_vm;

AGSEngine::AGSEngine(OSystem *syst, const AGSGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("AGS"), _events(nullptr), _music(nullptr),
	_gfxDriver(nullptr), _globals(nullptr), _forceTextAA(false) {
	g_vm = this;

	AGS3::script_commands_init();
	AGS3::Engine::SavegameComponents::component_handlers_init();
	_events = new EventsManager();
	_globals = new ::AGS3::Globals();

	Common::String forceAA;
	if (ConfMan.getActiveDomain()->tryGetVal("force_text_aa", forceAA))
		Common::parseBool(forceAA, _forceTextAA);

	// WORKAROUND: Certain games need to force AA to render the text correctly
	if (_gameDescription->features & GAMEFLAG_FORCE_AA)
		_forceTextAA = true;
}

AGSEngine::~AGSEngine() {
	if (_globals && _G(proper_exit) == 0) {
		_G(platform)->DisplayAlert("Error: the program has exited without requesting it.\n"
		                           "Program pointer: %+03d  (write this number down), engine version %s\n"
		                           "If you see a list of numbers above, please write them down and contact\n"
		                           "developers. Otherwise, note down any other information displayed.",
		                           _G(our_eip), _G(EngineVersion).LongString.GetCStr());
	}

	delete _events;
	delete _music;
	delete _globals;
	AGS3::Engine::SavegameComponents::component_handlers_free();
	AGS3::script_commands_free();
}

uint32 AGSEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

static const PluginVersion AGSTEAM_WADJETEYE[] = { { "agsteam", kWadjetEye }, { nullptr, 0 } };
static const PluginVersion AGS_FLASHLIGHT[] = { { "agsflashlight", 0 }, { nullptr, 0 } };
static const PluginVersion AGSSPRITEFONT_CLIFFTOP[] = { { "agsspritefont", kClifftopGames }, { "agsplugin.spritefont", kClifftopGames }, { nullptr, 0 } };

static const PluginVersion *const PLUGIN_VERSIONS[] = {
	nullptr,
	AGSTEAM_WADJETEYE,
	AGS_FLASHLIGHT,
	AGSSPRITEFONT_CLIFFTOP
};

const PluginVersion *AGSEngine::getNeededPlugins() const {
	uint index = (_gameDescription->features & GAMEFLAG_PLUGINS_MASK);

	if (index >= ARRAYSIZE(PLUGIN_VERSIONS))
		return nullptr;
	else
		return PLUGIN_VERSIONS[index];
}

Common::String AGSEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::Error AGSEngine::run() {
#ifdef DETECTION_STATIC
	// The game scanner is not available when detection is dynamic
	if (debugChannelSet(-1, kDebugScan)) {
		// Scan the given folder and subfolders for unknown games
		AGS3::GameScanner scanner;
		scanner.scan(ConfMan.getPath("path"));
		return Common::kNoError;
	}
#endif

	if (isUnsupportedPre25()) {
		GUIErrorMessage(_("The selected game uses a pre-2.5 version of the AGS engine, which is not supported."));
		return Common::kNoError;
	}

	if (isUnsupportedAGS4()) {
		GUIErrorMessage(_("The selected game uses version 4 of the AGS engine, which is not supported."));
		return Common::kNoError;
	}

	if (is64BitGame()) {
		// If the game file was opened and the engine started, but the
		// size is -1, then it must be a game like Strangeland where
		// the data file is > 2Gb
		GUIErrorMessage(_("The selected game has a data file greater than 2Gb, "
			"which isn't supported by your version of ScummVM yet."));
		return Common::kNoError;
	}

	if (debugChannelSet(-1, kDebugScript))
		AGS3::ccSetOption(SCOPT_DEBUGRUN, 1);

#ifdef ENABLE_AGS_TESTS
	AGS3::Test_DoAllTests();
	return Common::kNoError;
#endif

	setDebugger(new AGSConsole(this));

	Common::String filename(_gameDescription->desc.filesDescriptions[0].fileName);
	Common::StringTokenizer tok(filename, ":");

	Common::String type = tok.nextToken();
	if (type.equals("clk")) {
		Common::File *f = new Common::File();
		f->open(tok.nextToken().c_str());
		SearchMan.add("installer", Common::ClickteamInstaller::open(f, DisposeAfterUse::YES));
		filename = tok.nextToken();
	}

	const char *ARGV[] = { "scummvm.exe", filename.c_str() };
	const int ARGC = 2;
	AGS3::main_init(ARGC, ARGV);

	_G(debug_flags) = 0;

	if (ConfMan.hasKey("display_fps"))
		_G(display_fps) = ConfMan.getBool("display_fps") ? AGS3::kFPS_Forced : AGS3::kFPS_Hide;

	Common::String saveOverrideOption;
	bool saveOverride = false;
	ConfMan.getActiveDomain()->tryGetVal("save_override", saveOverrideOption);
	if (!saveOverrideOption.empty())
		parseBool(saveOverrideOption, saveOverride);
	_G(noScummAutosave) = (Common::checkGameGUIOption(GAMEOPTION_NO_AUTOSAVE, ConfMan.get("guioptions"))) && !saveOverride;
	_G(noScummSaveLoad) = (Common::checkGameGUIOption(GAMEOPTION_NO_SAVELOAD, ConfMan.get("guioptions"))) && !saveOverride;
	if (_G(noScummSaveLoad))
		_G(noScummAutosave) = true;

	_G(saveThumbnail) = !(Common::checkGameGUIOption(GAMEOPTION_NO_SAVE_THUMBNAIL, ConfMan.get("guioptions")));

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

	_music = new Music();

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
	if (depth == 8) {
		format = Graphics::PixelFormat::createFormatCLUT8();
		return true;
	}

	Common::List<Graphics::PixelFormat> supportedFormatsList = g_system->getSupportedFormats();

	// Internally the engine uses the following format (see create_bitmap_ex())
	//  16 bit: RGB565   / Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)
	//  32 bit: ARGB8888 / Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24)
	// If supported by the backend, use it so that we can do direct rendering.
	Graphics::PixelFormat engine_format = depth == 16 ? Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0) : Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
	if (Common::find(supportedFormatsList.begin(), supportedFormatsList.end(), engine_format) != supportedFormatsList.end()) {
		format = engine_format;
		return true;
	}

	// For 32 bit color depth, the next best formats for which there is some optimization in the
	// engine are RGBA8888 or ABGR8888 (see ScummVMRendererGraphicsDriver::Present()).
	if (depth == 32) {
		Graphics::PixelFormat preferred_format1 = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
		Graphics::PixelFormat preferred_format2 = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
		for (Common::List<Graphics::PixelFormat>::iterator it =
				supportedFormatsList.begin(); it != supportedFormatsList.end(); ++it) {
			if (*it == preferred_format1 || *it == preferred_format2) {
				format = *it;
				return true;
			}
		}
	}

	// Otherwise prefer another format with the same color depth
	for (Common::List<Graphics::PixelFormat>::iterator it =
			supportedFormatsList.begin(); it != supportedFormatsList.end(); ++it) {
		if (it->bpp() == depth) {
			format = *it;
			return true;
		}
	}

	// Allow using 16 bit <-> 32 bit conversions by using the preferred graphics mode
	if (!supportedFormatsList.empty()) {
		format = supportedFormatsList.front();
		return true;
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
		   Common::String(_gameDescription->desc.extra).contains("Pre 2.5");
}

bool AGSEngine::isUnsupportedAGS4() const {
	return _gameDescription->desc.extra &&
		   Common::String(_gameDescription->desc.extra).contains("AGS 4");
}

bool AGSEngine::is64BitGame() const {
	Common::File f;

	// TODO: There are no more entries in the tables with -1 filesize, so this check doesn't really do anything.
	// Maybe find a more reliable way to detect if the system can't handle these files?

	if (_gameDescription->desc.filesDescriptions[0].fileName[0] == '\0')
		return false;
	else
		return f.open(_gameDescription->desc.filesDescriptions[0].fileName) && f.size() == -1;
}

Common::FSNode AGSEngine::getGameFolder() {
	return Common::FSNode(ConfMan.getPath("path"));
}

bool AGSEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	if (msg) {
		if (ConfMan.get("gameid") == "strangeland") {
			*msg = _("This game does not support loading from the menu. Use in-game interface");
			return false;
		}
		if (_G(noScummSaveLoad))
			*msg = _("To preserve the original experience, this game should be loaded using the in-game interface.\nYou can, however, override this setting in Game Options.");
	}

	return !_GP(thisroom).Options.SaveLoadDisabled &&
		   !_G(inside_script) && !_GP(play).fast_forward && !_G(no_blocking_functions) &&
		   !_G(noScummSaveLoad);
}

bool AGSEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	if (msg) {
		if (ConfMan.get("gameid") == "strangeland") {
			*msg = _("This game does not support saving from the menu. Use in-game interface");
			return false;
		}
		if (_G(noScummSaveLoad))
			*msg = _("To preserve the original experience, this game should be saved using the in-game interface.\nYou can, however, override this setting in Game Options.");
	}

	return !_GP(thisroom).Options.SaveLoadDisabled &&
		   !_G(inside_script) && !_GP(play).fast_forward && !_G(no_blocking_functions) &&
		   !_G(noScummSaveLoad);
}

Common::Error AGSEngine::loadGameState(int slot) {
	(void)AGS3::try_restore_save(slot);
	return Common::kNoError;
}

Common::Error AGSEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	(void)AGS3::save_game(slot, desc.c_str());
	return Common::kNoError;
}

int AGSEngine::getAutosaveSlot() const {
	if (!g_engine || !_G(noScummAutosave))
		return 0;
	else
		return -1;
}

void AGSEngine::GUIError(const Common::String &msg) {
	GUIErrorMessage(msg);
}

void AGSEngine::syncSoundSettings() {
	// Digital audio
	Engine::syncSoundSettings();
	// MIDI
	_music->syncVolume();
}

} // namespace AGS
