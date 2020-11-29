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

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/tokenizer.h"
#include "common/translation.h"

#include "engines/util.h"
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/debugger.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/detection.h"

#include "engines/wintermute/base/sound/base_sound_manager.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script_engine.h"
#include "engines/wintermute/debugger/debugger_controller.h"

#include "gui/message.h"

namespace Wintermute {

// Simple constructor for detection - we need to setup the persistence to avoid special-casing in-engine
// This might not be the prettiest solution
WintermuteEngine::WintermuteEngine() : Engine(g_system) {
	_game = new AdGame("");
	_debugger = nullptr;
	_dbgController = nullptr;
	_gameDescription = nullptr;
}

WintermuteEngine::WintermuteEngine(OSystem *syst, const WMEGameDescription *desc)
	: Engine(syst), _gameDescription(desc) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from init().
	ConfMan.registerDefault("show_fps","false");

	// Do not initialize graphics here

	// However this is the place to specify all default directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	//SearchMan.addSubDirectoryMatching(gameDataDir, "sound");

	// Here is the right place to set up the engine specific debug channels
	DebugMan.addDebugChannel(kWintermuteDebugLog, "enginelog", "Covers the same output as the log-file in WME");
	DebugMan.addDebugChannel(kWintermuteDebugSaveGame, "savegame", "Savegames");
	DebugMan.addDebugChannel(kWintermuteDebugFont, "font", "Text-drawing-related messages");
	DebugMan.addDebugChannel(kWintermuteDebugFileAccess, "file-access", "Non-critical problems like missing files");
	DebugMan.addDebugChannel(kWintermuteDebugAudio, "audio", "audio-playback-related issues");
	DebugMan.addDebugChannel(kWintermuteDebugGeneral, "general", "various issues not covered by any of the above");

	_game = nullptr;
	_debugger = nullptr;
	_dbgController = nullptr;
}

WintermuteEngine::~WintermuteEngine() {
	// Dispose your resources here
	deinit();
	delete _game;
	//_debugger deleted by Engine

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}

bool WintermuteEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsReturnToLauncher:
		return true;
	case kSupportsLoadingDuringRuntime:
		return true;
	case kSupportsSavingDuringRuntime:
		return true;
#ifdef ENABLE_WME3D
	case kSupportsArbitraryResolutions:
		return true;
#endif
	default:
		return false;
	}
	return false;
}

Common::Error WintermuteEngine::run() {
	// Initialize graphics using following:
	Graphics::PixelFormat format(4, 8, 8, 8, 8, 24, 16, 8, 0);
	if (_gameDescription->adDesc.flags & GF_LOWSPEC_ASSETS) {
		initGraphics(320, 240, &format);
#ifdef ENABLE_FOXTAIL
	} else if (BaseEngine::isFoxTailCheck(_gameDescription->targetExecutable)) {
		initGraphics(640, 360, &format);
#endif
	} else {
		initGraphics(800, 600, &format);
	}
	if (g_system->getScreenFormat() != format) {
		return Common::kUnsupportedColorMode;
	}

	// Create debugger console. It requires GFX to be initialized
	_dbgController = new DebuggerController(this);
	_debugger = new Console(this);
	setDebugger(_debugger);

//	DebugMan.enableDebugChannel("enginelog");
	debugC(1, kWintermuteDebugLog, "Engine Debug-LOG enabled");
	debugC(2, kWintermuteDebugSaveGame , "Savegame debugging-enabled");

	int ret = 1;

	// Additional setup.
	debugC(kWintermuteDebugLog, "WintermuteEngine::init");
	ret = init();

	debugC(kWintermuteDebugLog, "WintermuteEngine::messageLoop");
	if (ret == 0) {
		ret = messageLoop();
	}
	deinit();
	return Common::kNoError;
}

int WintermuteEngine::init() {
	BaseEngine::createInstance(_targetName, _gameDescription->adDesc.gameId, _gameDescription->adDesc.language, _gameDescription->targetExecutable, _gameDescription->adDesc.flags);
	BaseEngine &instance = BaseEngine::instance();

	// check if unknown target is a 2.5D game
	if (instance.getFlags() & ADGF_AUTOGENTARGET) {
		Common::ArchiveMemberList actors3d;
		if (instance.getFileManager()->listMatchingPackageMembers(actors3d, "*.act3d")) {
			warning("Unknown 2.5D game detected");
			instance.addFlags(GF_3D);
		}
	}

	#ifdef ENABLE_WME3D
	if (instance.getFlags() & GF_3D) {
		instance.getClassRegistry()->register3DClasses();
	}
	#endif

	// check dependencies for games with high resolution assets
	#if !defined(USE_PNG) || !defined(USE_JPEG) || !defined(USE_VORBIS)
		if (!(instance.getFlags() & GF_LOWSPEC_ASSETS)) {
			GUI::MessageDialog dialog(_("This game requires PNG, JPEG and Vorbis support."));
			dialog.runModal();
			delete _game;
			_game = nullptr;
			return false;
		}
	#endif

	// check dependencies for games with FoxTail subengine
	#if !defined(ENABLE_FOXTAIL)
		if (BaseEngine::isFoxTailCheck(instance.getTargetExecutable())) {
			GUI::MessageDialog dialog(_("This game requires the FoxTail subengine, which is not compiled in."));
			dialog.runModal();
			delete _game;
			_game = nullptr;
			return false;
		}
	#endif

	// check dependencies for games with HeroCraft subengine
	#if !defined(ENABLE_HEROCRAFT)
		if (instance.getTargetExecutable() == WME_HEROCRAFT) {
			GUI::MessageDialog dialog(_("This game requires the HeroCraft subengine, which is not compiled in."));
			dialog.runModal();
			delete _game;
			_game = nullptr;
			return false;
		}
	#endif

	#ifndef ENABLE_WME3D
	// check if game require 3D capabilities
	if (instance.getFlags() & GF_3D) {
		GUI::MessageDialog dialog(_("This game requires 3D capabilities that are out ScummVM scope. As such, it"
			" is likely to be unplayable totally or partially."), _("Start anyway"), _("Cancel"));
		if (dialog.runModal() != GUI::kMessageOK) {
			delete _game;
			_game = nullptr;
			return false;
		}
	}
	#endif

	_game = new AdGame(_targetName);
	if (!_game) {
		return 1;
	}

	#ifdef ENABLE_WME3D
	Common::ArchiveMemberList actors3d;
	_game->_playing3DGame = BaseEngine::instance().getFileManager()->listMatchingPackageMembers(actors3d, "*.act3d");
	#endif
	instance.setGameRef(_game);
	BasePlatform::initialize(this, _game, 0, nullptr);

	_game->initConfManSettings();

	// load general game settings
	_game->initialize1();

	// set gameId, for savegame-naming:
	_game->setGameTargetName(_targetName);

	if (DID_FAIL(_game->loadSettings("startup.settings"))) {
		_game->LOG(0, "Error loading game settings.");
		delete _game;
		_game = nullptr;

		warning("Some of the essential files are missing. Please reinstall.");
		return 2;
	}

	if (!_game->initialize2()) {
		_game->LOG(0, "Error initializing renderer. Exiting.");

		delete _game;
		_game = nullptr;
		return 3;
	}

	bool ret = _game->initRenderer();

	if (DID_FAIL(ret)) {
		_game->LOG(ret, "Error initializing renderer. Exiting.");

		delete _game;
		_game = nullptr;
		return 3;
	}

	_game->initialize3();
	// initialize sound manager (non-fatal if we fail)
	ret = _game->_soundMgr->initialize();
	if (DID_FAIL(ret)) {
		_game->LOG(ret, "Sound is NOT available.");
	}


	// load game
	uint32 dataInitStart = g_system->getMillis();

	if (DID_FAIL(_game->loadGameSettingsFile())) {
		_game->LOG(ret, "Error loading game file. Exiting.");
		delete _game;
		_game = nullptr;
		return false;
	}

	_game->_renderer->_ready = true;
	_game->_miniUpdateEnabled = true;

	_game->LOG(0, "Engine initialized in %d ms", g_system->getMillis() - dataInitStart);
	_game->LOG(0, "");

	if (ConfMan.hasKey("save_slot")) {
		int slot = ConfMan.getInt("save_slot");
		_game->loadGame(slot);
	}

	_game->_scEngine->attachMonitor(_dbgController);

	// all set, ready to go
	return 0;
}

int WintermuteEngine::messageLoop() {
	bool done = false;

	uint32 prevTime = _system->getMillis();
	uint32 time = _system->getMillis();
	uint32 diff = 0;

	const uint32 maxFPS = 60;
	const uint32 frameTime = 2 * (uint32)((1.0 / maxFPS) * 1000);
	while (!done) {
		if (!_game) {
			break;
		}

		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			BasePlatform::handleEvent(&event);
		}

		if (_game && _game->_renderer->_active && _game->_renderer->isReady()) {
			_game->displayContent();
			_game->displayQuickMsg();

			_game->displayDebugInfo();

			time = _system->getMillis();
			diff = time - prevTime;
			if (frameTime > diff) { // Avoid overflows
				_system->delayMillis(frameTime - diff);
			}

			// ***** flip
			if (!_game->getSuspendedRendering()) {
				_game->_renderer->flip();
			}
			if (_game->getIsLoading()) {
				_game->loadGame(_game->_scheduledLoadSlot);
			}
			prevTime = time;
		}
		if (shouldQuit()) {
			break;
		}
		if (_game && _game->_quitting) {
			break;
		}
	}

	if (_game) {
		delete _game;
		_game = nullptr;
	}
	return 0;
}

void WintermuteEngine::deinit() {
	BaseEngine::destroy();
	BasePlatform::deinit();
}

Common::Error WintermuteEngine::loadGameState(int slot) {
	BaseEngine::instance().getGameRef()->loadGame(slot);
	return Common::kNoError;
}

Common::Error WintermuteEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	BaseEngine::instance().getGameRef()->saveGame(slot, desc.c_str(), false);
	return Common::kNoError;
}

bool WintermuteEngine::canSaveGameStateCurrently() {
	return true;
}

bool WintermuteEngine::canLoadGameStateCurrently() {
	return true;
}

bool WintermuteEngine::getGameInfo(const Common::FSList &fslist, Common::String &name, Common::String &caption) {
	bool retVal = false;
	caption = name = "(invalid)";
	Common::SeekableReadStream *stream = nullptr;
	// Quick-fix, instead of possibly breaking the persistence-system, let's just roll with it
	BaseFileManager *fileMan = new BaseFileManager(Common::UNK_LANG, true);
	fileMan->registerPackages(fslist);
	stream = fileMan->openFile("startup.settings", false, false);

	// The process is as follows: Check the "GAME=" tag in startup.settings, to decide where the
	// game-settings are (usually "default.game"), then look into the game-settings to find
	// the NAME = and CAPTION = tags, to use them to generate a gameid and extras-field

	Common::String settingsGameFile = "default.game";
	// If the stream-open failed, lets at least attempt to open the default game file afterwards
	// so, we don't call it a failure yet.
	if (stream) {
		while (!stream->eos() && !stream->err()) {
			Common::String line = stream->readLine();
			line.trim(); // Get rid of indentation
			// Expect "SETTINGS {" or comment, or empty line
			if (line.size() == 0 || line[0] == ';' || (line.contains("{"))) {
				continue;
			} else {
				// We are looking for "GAME ="
				Common::StringTokenizer token(line, "=");
				Common::String key = token.nextToken();
				Common::String value = token.nextToken();
				if (value.size() == 0) {
					continue;
				}
				if (value[0] == '\"') {
					value.deleteChar(0);
				} else {
					continue;
				}
				if (value.lastChar() == '\"') {
					value.deleteLastChar();
				}
				if (key == "GAME") {
					settingsGameFile = value;
					break;
				}
			}
		}
	}

	delete stream;
	stream = fileMan->openFile(settingsGameFile, false, false);
	if (stream) {
		// We do some manual parsing here, as the engine needs gfx to be initalized to do that.
		while (!stream->eos() && !stream->err()) {
			Common::String line = stream->readLine();
			line.trim(); // Get rid of indentation
			// Expect "GAME {" or comment, or empty line
			if (line.size() == 0 || line[0] == ';' || (line.contains("{"))) {
				continue;
			} else {
				Common::StringTokenizer token(line, "=");
				Common::String key = token.nextToken();
				Common::String value = token.nextToken();
				if (value.size() == 0) {
					continue;
				}
				if (value[0] == '\"') {
					value.deleteChar(0);
				} else {
					continue;    // not a string
				}
				if (value.lastChar() == '\"') {
					value.deleteLastChar();
				}
				if (key == "NAME") {
					retVal = true;
					name = value;
				} else if (key == "CAPTION") {
					retVal = true;
					// Remove any translation tags, if they are included in the game description.
					// This can potentially remove parts of a string that has translation tags
					// and contains a "/" in its description (e.g. /tag/Name start / name end will
					// result in "name end"), but it's a very rare case, and this code is just used
					// for fallback anyway.
					if (value.hasPrefix("/")) {
						value.deleteChar(0);
						while (value.contains("/")) {
							value.deleteChar(0);
						}
					}
					caption = value;
				}
			}
		}
		delete stream;
	}
	delete fileMan;
	BaseEngine::destroy();
	return retVal;
}

} // End of namespace Wintermute
