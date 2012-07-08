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

#include "engines/util.h"
#include "engines/wintermute/Ad/AdGame.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/BRegistry.h"

#include "engines/wintermute/Base/BSoundMgr.h"
#include "engines/wintermute/Base/scriptables/ScEngine.h"

namespace WinterMute {

WinterMuteEngine *g_wintermute;

WinterMuteEngine::WinterMuteEngine(OSystem *syst)
	: Engine(syst) {
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
	delete _rnd;

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
	initGraphics(800, 600, false, &format);
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
	_classReg = new CSysClassRegistry();
	_classReg->registerClasses();

	_game = new CAdGame;
	if (!_game) return 1;
	CBPlatform::Initialize(_game, 0, NULL);

	bool windowedMode = !ConfMan.getBool("fullscreen");
	
	// parse command line
	char *SaveGame = NULL;
/*	for (int i = 0; i < argc; i++) {
		strcpy(param, argv[i]);
		
		if (scumm_stricmp(param, "-project") == 0) {
			if (argc > i) strcpy(param, argv[i + 1]);
			else param[0] = '\0';
			
			if (strcmp(param, "") != 0) {
				char *IniDir = CBUtils::GetPath(param);
				char *IniName = CBUtils::GetFilename(param);
				
				// switch to ini's dir
				warning("TODO: Place ini-files somewhere");
				//				chdir(IniDir);
				
				// set ini name
				sprintf(param, "./%s", IniName);
				_game->_registry->SetIniName(param);
				
				delete [] IniDir;
				delete [] IniName;
			}
		} else if (scumm_stricmp(param, "-windowed") == 0) windowedMode = true;
	}*/
	
	
	if (_game->_registry->readBool("Debug", "DebugMode")) _game->DEBUG_DebugEnable("./wme.log");
	
	_game->_dEBUG_ShowFPS = _game->_registry->readBool("Debug", "ShowFPS");
	
	if (_game->_registry->readBool("Debug", "DisableSmartCache")) {
		_game->LOG(0, "Smart cache is DISABLED");
		_game->_smartCache = false;
	}
	
	/*	bool AllowDirectDraw = _game->_registry->readBool("Debug", "AllowDirectDraw", false);*/
	
	// load general game settings
	_game->initialize1();
	
	
	if (FAILED(_game->loadSettings("startup.settings"))) {
		_game->LOG(0, "Error loading game settings.");
		delete _game;
		_game = NULL;
		
		warning("Some of the essential files are missing. Please reinstall.");
		return 2;
	}
	
	_game->initialize2();
	
	_game->getDebugMgr()->onGameInit();
	_game->_scEngine->loadBreakpoints();
	
	
	
	HRESULT ret;
	
	// initialize the renderer
	ret = _game->_renderer->initRenderer(_game->_settingsResWidth, _game->_settingsResHeight, windowedMode);
	if (FAILED(ret)) {
		_game->LOG(ret, "Error initializing renderer. Exiting.");
		
		delete _game;
		_game = NULL;
		return 3;
	}
	
	_game->initialize3();
	
#ifdef __IPHONEOS__
	SDL_AddEventWatch(CBPlatform::SDLEventWatcher, NULL);
#endif
	
	// initialize sound manager (non-fatal if we fail)
	ret = _game->_soundMgr->initialize();
	if (FAILED(ret)) {
		_game->LOG(ret, "Sound is NOT available.");
	}
	
	
	// load game
	uint32 DataInitStart = CBPlatform::GetTime();
	
	if (FAILED(_game->loadFile(_game->_settingsGameFile ? _game->_settingsGameFile : "default.game"))) {
		_game->LOG(ret, "Error loading game file. Exiting.");
		delete _game;
		_game = NULL;
		return false;
	}
	_game->setWindowTitle();
	_game->_renderer->_ready = true;
	_game->_miniUpdateEnabled = true;
	
	_game->LOG(0, "Engine initialized in %d ms", CBPlatform::GetTime() - DataInitStart);
	_game->LOG(0, "");
	
	if (ConfMan.hasKey("save_slot")) {
		int slot = ConfMan.getInt("save_slot");
		Common::String str = Common::String::format("save00%d.DirtySplitSav", slot);
		_game->loadGame(str.c_str());
	}
	
	if (SaveGame) {
		_game->loadGame(SaveGame);
		delete [] SaveGame;
	}
	
	// all set, ready to go
	return 0;
}

int WinterMuteEngine::messageLoop() {
	bool done = false;

	uint32 prevTime = _system->getMillis();
	uint32 time = _system->getMillis();
	uint32 diff = 0;

	const uint32 maxFPS = 25;
	const uint32 frameTime = (1.0/maxFPS) * 1000;
	while (!done) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			CBPlatform::HandleEvent(&event);
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
		 if(_game->_renderer && _game->_renderer->_windowed)
		 {
		 if(!::IsIconic(_game->_renderer->_window))
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
}

uint32 WinterMuteEngine::randInt(int from, int to) {
	return _rnd->getRandomNumberRng(from, to);
}

} // End of namespace WinterMute
