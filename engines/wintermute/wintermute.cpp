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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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
#include "common/EventRecorder.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/tokenizer.h"

#include "engines/util.h"
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/base_registry.h"

#include "engines/wintermute/base/sound/base_sound_manager.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/scriptables/script_engine.h"

namespace WinterMute {

WinterMuteEngine *g_wintermute;

// Simple constructor for detection - we need to setup the persistence to avoid special-casing in-engine
// This might not be the prettiest solution
WinterMuteEngine::WinterMuteEngine() : Engine(g_system) {
	g_wintermute = this;
	_classReg = new SystemClassRegistry();
	_classReg->registerClasses();

	_game = new AdGame();
	_rnd = NULL;
}

WinterMuteEngine::WinterMuteEngine(OSystem *syst, const ADGameDescription *desc)
	: Engine(syst), _gameDescription(desc) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from init().

	// Do not initialize graphics here

	// However this is the place to specify all default directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	//SearchMan.addSubDirectoryMatching(gameDataDir, "sound");

	// Here is the right place to set up the engine specific debug channels
	DebugMan.addDebugChannel(kWinterMuteDebugLog, "enginelog", "Covers the same output as the log-file in WME");
	DebugMan.addDebugChannel(kWinterMuteDebugSaveGame, "savegame", "Savegames");
	DebugMan.addDebugChannel(kWinterMuteDebugFont, "font", "Text-drawing-related messages");
	DebugMan.addDebugChannel(kWinterMuteDebugFileAccess, "file-access", "Non-critical problems like missing files");
	DebugMan.addDebugChannel(kWinterMuteDebugAudio, "audio", "audio-playback-related issues");
	// Don't forget to register your random source
	_rnd = new Common::RandomSource("WinterMute");

	debug("WinterMuteEngine::WinterMuteEngine");
	_game = NULL;

	g_wintermute = this;
	_classReg = NULL;
}

WinterMuteEngine::~WinterMuteEngine() {
	debug("WinterMuteEngine::~WinterMuteEngine");

	// Dispose your resources here
	delete _classReg;
	delete _rnd;
	delete _game;
	g_wintermute = NULL;

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}

bool WinterMuteEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsRTL:
		return true;
	default:
		return false;
	}
	return false;
}

Common::Error WinterMuteEngine::run() {
	// Initialize graphics using following:
	Graphics::PixelFormat format(4, 8, 8, 8, 8, 24, 16, 8, 0);
	initGraphics(800, 600, true, &format);
	if (g_system->getScreenFormat() != format) {
		error("Wintermute currently REQUIRES 32bpp");
	}
	// You could use backend transactions directly as an alternative,
	// but it isn't recommended, until you want to handle the error values
	// from OSystem::endGFXTransaction yourself.
	// This is just an example template:
	//_system->beginGFXTransaction();
	//  // This setup the graphics mode according to users seetings
	//  initCommonGFX(false);
	//
	//  // Specify dimensions of game graphics window.
	//  // In this example: 320x200
	//  _system->initSize(320, 200);
	//FIXME: You really want to handle
	//OSystem::kTransactionSizeChangeFailed here
	//_system->endGFXTransaction();

	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);

	// Additional setup.
	debug("WinterMuteEngine::init");

	// Your main even loop should be (invoked from) here.
	debug("WinterMuteEngine::go: Hello, World!");

	DebugMan.enableDebugChannel("enginelog");
	// This test will show up if -d1 and --debugflags=example are specified on the commandline
	debugC(1, kWinterMuteDebugLog, "Engine Debug-LOG enabled");
	debugC(2, kWinterMuteDebugSaveGame , "Savegame debugging-enabled");

	int ret = 1;

	ret = init();

	if (ret == 0) {
		ret = messageLoop();
	}
	deinit();
	return Common::kNoError;
}

int WinterMuteEngine::init() {
	_classReg = new SystemClassRegistry();
	_classReg->registerClasses();

	_game = new AdGame;
	if (!_game) return 1;
	BasePlatform::initialize(_game, 0, NULL);

	bool windowedMode = !ConfMan.getBool("fullscreen");

	// parse command line
	char *saveGame = NULL;
	/*  for (int i = 0; i < argc; i++) {
	        strcpy(param, argv[i]);

	        if (scumm_stricmp(param, "-project") == 0) {
	            if (argc > i) strcpy(param, argv[i + 1]);
	            else param[0] = '\0';

	            if (strcmp(param, "") != 0) {
	                char *IniDir = BaseUtils::GetPath(param);
	                char *IniName = BaseUtils::GetFilename(param);

	                // switch to ini's dir
	                warning("TODO: Place ini-files somewhere");
	                //              chdir(IniDir);

	                // set ini name
	                sprintf(param, "./%s", IniName);
	                _game->_registry->SetIniName(param);

	                delete[] IniDir;
	                delete[] IniName;
	            }
	        } else if (scumm_stricmp(param, "-windowed") == 0) windowedMode = true;
	    }*/


	if (_game->_registry->readBool("Debug", "DebugMode")) _game->DEBUG_DebugEnable("./wme.log");

	_game->_debugShowFPS = _game->_registry->readBool("Debug", "ShowFPS");

	if (_game->_registry->readBool("Debug", "DisableSmartCache")) {
		_game->LOG(0, "Smart cache is DISABLED");
		_game->_smartCache = false;
	}

	/*  bool AllowDirectDraw = _game->_registry->readBool("Debug", "AllowDirectDraw", false);*/

	// load general game settings
	_game->initialize1();

	// set gameId, for savegame-naming:
	_game->setGameId(_targetName);

	if (DID_FAIL(_game->loadSettings("startup.settings"))) {
		_game->LOG(0, "Error loading game settings.");
		delete _game;
		_game = NULL;

		warning("Some of the essential files are missing. Please reinstall.");
		return 2;
	}

	_game->initialize2();

	_game->getDebugMgr()->onGameInit();
	_game->_scEngine->loadBreakpoints();

	bool ret;

	// initialize the renderer
	ret = _game->_renderer->initRenderer(_game->_settingsResWidth, _game->_settingsResHeight, windowedMode);
	if (DID_FAIL(ret)) {
		_game->LOG(ret, "Error initializing renderer. Exiting.");

		delete _game;
		_game = NULL;
		return 3;
	}

	_game->initialize3();

	// initialize sound manager (non-fatal if we fail)
	ret = _game->_soundMgr->initialize();
	if (DID_FAIL(ret)) {
		_game->LOG(ret, "Sound is NOT available.");
	}


	// load game
	uint32 DataInitStart = g_system->getMillis();

	if (DID_FAIL(_game->loadFile(_game->_settingsGameFile ? _game->_settingsGameFile : "default.game"))) {
		_game->LOG(ret, "Error loading game file. Exiting.");
		delete _game;
		_game = NULL;
		return false;
	}
	//_game->setWindowTitle();
	_game->_renderer->_ready = true;
	_game->_miniUpdateEnabled = true;

	_game->LOG(0, "Engine initialized in %d ms", g_system->getMillis() - DataInitStart);
	_game->LOG(0, "");

	if (ConfMan.hasKey("save_slot")) {
		int slot = ConfMan.getInt("save_slot");
		Common::String str = Common::String::format("save00%d.DirtySplitSav", slot);
		_game->loadGame(str.c_str());
	}

	if (saveGame) {
		_game->loadGame(saveGame);
		delete[] saveGame;
	}

	// all set, ready to go
	return 0;
}

int WinterMuteEngine::messageLoop() {
	bool done = false;

	uint32 prevTime = _system->getMillis();
	uint32 time = _system->getMillis();
	uint32 diff = 0;

	const uint32 maxFPS = 60;
	const uint32 frameTime = (uint32)((1.0 / maxFPS) * 1000);
	while (!done) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			BasePlatform::handleEvent(&event);
		}

		if (_game && _game->_renderer->_active && _game->_renderer->_ready) {
			_game->displayContent();
			_game->displayQuickMsg();

			_game->displayDebugInfo();

			time = _system->getMillis();
			diff = time - prevTime;
			if (frameTime > diff) // Avoid overflows
				_system->delayMillis(frameTime - diff);

			// ***** flip
			if (!_game->_suspendedRendering) _game->_renderer->flip();
			if (_game->_loading) _game->loadGame(_game->_scheduledLoadSlot);
			prevTime = time;
		}
		if (_game->_quitting) break;
	}

	if (_game) {
		// remember previous window position
		/*
		 if (_game->_renderer && _game->_renderer->_windowed)
		 {
		 if (!::IsIconic(_game->_renderer->_window))
		 {
		 int PosX = _game->_renderer->_windowRect.left;
		 int PosY = _game->_renderer->_windowRect.top;
		 PosX -= _game->_renderer->_monitorRect.left;
		 PosY -= _game->_renderer->_monitorRect.top;

		 _game->_registry->writeInt("Video", "WindowPosX", PosX);
		 _game->_registry->writeInt("Video", "WindowPosY", PosY);
		 }
		 }
		 */

		delete _game;
		_game = NULL;
	}
	return 0;
}

void WinterMuteEngine::deinit() {
	delete _classReg;
	_classReg = NULL;
}

bool WinterMuteEngine::getGameInfo(const Common::FSList &fslist, Common::String &name, Common::String &caption) {
	bool retVal = false;
	caption = name = "(invalid)";
	Common::SeekableReadStream *stream = NULL;
	// Quick-fix, instead of possibly breaking the persistence-system, let's just roll with it
	BaseFileManager *fileMan = new BaseFileManager();
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
			if (line.size() == 0 || line[0] == ';' || (line.contains("{")))
				continue;
			else {
				// We are looking for "GAME ="
				Common::StringTokenizer token(line, "=");
				Common::String key = token.nextToken();
				Common::String value = token.nextToken();
				if (value.size() == 0)
					continue;
				if (value[0] == '\"')
					value.deleteChar(0);
				else
					continue;
				if (value.lastChar() == '\"')
					value.deleteLastChar();
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
			if (line.size() == 0 || line[0] == ';' || (line.contains("{")))
				continue;
			else {
				Common::StringTokenizer token(line, "=");
				Common::String key = token.nextToken();
				Common::String value = token.nextToken();
				if (value.size() == 0)
					continue;
				if (value[0] == '\"')
					value.deleteChar(0);
				else
					continue; // not a string
				if (value.lastChar() == '\"')
					value.deleteLastChar();
				if (key == "NAME") {
					retVal = true;
					name = value;
				} else if (key == "CAPTION") {
					retVal = true;
					caption = value;
				}
			}
		}
		delete stream;
	}
	delete fileMan;
	return retVal;
}

uint32 WinterMuteEngine::randInt(int from, int to) {
	return _rnd->getRandomNumberRng(from, to);
}

} // End of namespace WinterMute
