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
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"
#include "common/random.h"
#include "common/events.h"
#include "common/EventRecorder.h"
#include "common/debug-channels.h"

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/mouse.h"
#include "hugo/inventory.h"
#include "hugo/parser.h"
#include "hugo/route.h"
#include "hugo/util.h"
#include "hugo/sound.h"
#include "hugo/intro.h"
#include "hugo/object.h"
#include "hugo/text.h"

#include "engines/util.h"

namespace Hugo {

HugoEngine *HugoEngine::s_Engine = 0;

overlay_t HugoEngine::_boundary;
overlay_t HugoEngine::_overlay;
overlay_t HugoEngine::_ovlBase;
overlay_t HugoEngine::_objBound;

maze_t      _maze;                              // Default to not in maze 
hugo_boot_t _boot;                              // Boot info structure file

HugoEngine::HugoEngine(OSystem *syst, const HugoGameDescription *gd) : Engine(syst), _gameDescription(gd), _mouseX(0), _mouseY(0),
	_arrayReqs(0), _hotspots(0), _invent(0), _uses(0), _catchallList(0), _backgroundObjects(0),	_points(0), _cmdList(0), 
	_screenActs(0), _hero(0), _heroImage(0), _defltTunes(0), _introX(0), _introY(0), _maxInvent(0), _numBonuses(0),
	_numScreens(0), _tunesNbr(0), _soundSilence(0), _soundTest(0), _screenStates(0), _score(0), _maxscore(0),
	_backgroundObjectsSize(0), _screenActsSize(0), _usesSize(0)

{
	_system = syst;
	DebugMan.addDebugChannel(kDebugSchedule, "Schedule", "Script Schedule debug level");
	DebugMan.addDebugChannel(kDebugEngine, "Engine", "Engine debug level");
	DebugMan.addDebugChannel(kDebugDisplay, "Display", "Display debug level");
	DebugMan.addDebugChannel(kDebugMouse, "Mouse", "Mouse debug level");
	DebugMan.addDebugChannel(kDebugParser, "Parser", "Parser debug level");
	DebugMan.addDebugChannel(kDebugFile, "File", "File IO debug level");
	DebugMan.addDebugChannel(kDebugRoute, "Route", "Route debug level");
	DebugMan.addDebugChannel(kDebugInventory, "Inventory", "Inventory debug level");
	DebugMan.addDebugChannel(kDebugObject, "Object", "Object debug level");
	DebugMan.addDebugChannel(kDebugMusic, "Music", "Music debug level");

	_console = new HugoConsole(this);
	_rnd = 0;
}

HugoEngine::~HugoEngine() {
	shutdown();

	_screen->freePalette();
	_text->freeAllTexts();

	free(_introX);
	free(_introY);

	if (_arrayReqs) {
		for (int i = 0; _arrayReqs[i] != 0; i++)
			free(_arrayReqs[i]);
		free(_arrayReqs);
	}

	free(_hotspots);
	free(_invent);

	if (_uses) {
		for (int i = 0; i < _usesSize; i++)
			free(_uses[i].targets);
		free(_uses);
	}

	free(_catchallList);

	if (_backgroundObjects) {
		for (int i = 0; i < _backgroundObjectsSize; i++)
			free(_backgroundObjects[i]);
		free(_backgroundObjects);
	}

	free(_points);

	if (_cmdList) {
		for (int i = 0; i < _cmdListSize; i++)
			free(_cmdList[i]);
		free(_cmdList);
	}

	if (_cmdList) {
		for (int i = 0; i < _screenActsSize; i++)
			free(_screenActs[i]);
		free(_screenActs);
	}

	_object->freeObjectArr();
	_scheduler->freeActListArr();

	free(_defltTunes);
	free(_screenStates);

	_screen->freeFonts();

	delete _topMenu;

	delete _object;
	delete _sound;
	delete _route;
	delete _parser;
	delete _inventory;
	delete _mouse;
	delete _screen;
	delete _intro;
	delete _scheduler;
	delete _file;

	DebugMan.clearAllDebugChannels();
	delete _console;
	delete _rnd;
}

GameType HugoEngine::getGameType() const {
	return _gameType;
}

Common::Platform HugoEngine::getPlatform() const {
	return _platform;
}

bool HugoEngine::isPacked() const {
	return _packedFl;
}

Common::Error HugoEngine::run() {
	s_Engine = this;
	initGraphics(320, 200, false);

	_mouse = new MouseHandler(this);
	_inventory = new InventoryHandler(this);
	_route = new Route(this);
	_sound = new SoundHandler(this);
	_text = new TextHandler(this);

	_topMenu = new TopMenu(this);

	switch (_gameVariant) {
	case kGameVariantH1Win: // H1 Win
		_file = new FileManager_v1w(this);
		_scheduler = new Scheduler_v1w(this);
		_intro = new intro_v1w(this);
		_screen = new Screen_v1w(this);
		_parser = new Parser_v1w(this);
		_object = new ObjectHandler_v1w(this);
		_normalTPS = 9;
		break;
	case kGameVariantH2Win:
		_file = new FileManager_v2w(this);
		_scheduler = new Scheduler_v1w(this);
		_intro = new intro_v2w(this);
		_screen = new Screen_v1w(this);
		_parser = new Parser_v1w(this);
		_object = new ObjectHandler_v1w(this);
		_normalTPS = 9;
		break;
	case kGameVariantH3Win:
		_file = new FileManager_v2w(this);
		_scheduler = new Scheduler_v1w(this);
		_intro = new intro_v3w(this);
		_screen = new Screen_v1w(this);
		_parser = new Parser_v1w(this);
		_object = new ObjectHandler_v1w(this);
		_normalTPS = 9;
		break;
	case kGameVariantH1Dos: // H1 DOS
		_file = new FileManager_v1d(this);
		_scheduler = new Scheduler_v1d(this);
		_intro = new intro_v1d(this);
		_screen = new Screen_v1d(this);
		_parser = new Parser_v1d(this);
		_object = new ObjectHandler_v1d(this);
		_normalTPS = 8;
		break;
	case kGameVariantH2Dos:
		_file = new FileManager_v2d(this);
		_scheduler = new Scheduler_v2d(this);
		_intro = new intro_v2d(this);
		_screen = new Screen_v1d(this);
		_parser = new Parser_v2d(this);
		_object = new ObjectHandler_v2d(this);
		_normalTPS = 8;
		break;
	case kGameVariantH3Dos:
		_file = new FileManager_v3d(this);
		_scheduler = new Scheduler_v3d(this);
		_intro = new intro_v3d(this);
		_screen = new Screen_v1d(this);
		_parser = new Parser_v3d(this);
		_object = new ObjectHandler_v3d(this);
		_normalTPS = 9;
		break;
	}

	if (!loadHugoDat())
		return Common::kUnknownError;

	// Use Windows-looking mouse cursor
	_screen->setCursorPal();
	_screen->resetInventoryObjId();

	initStatus();                                   // Initialize game status
	initConfig();                                   // Initialize user's config
	initialize();
	resetConfig();                                  // Reset user's config

	initMachine();

	// Start the state machine
	_status.viewState = kViewIntroInit;

	_status.doQuitFl = false;

	while (!_status.doQuitFl) {
		g_system->updateScreen();
		_sound->pcspkr_player();
		runMachine();
		// Handle input
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_d && event.kbd.hasFlags(Common::KBD_CTRL)) {
					this->getDebugger()->attach();
					this->getDebugger()->onFrame();
				}
				_parser->keyHandler(event);
				break;
			case Common::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				break;
			case Common::EVENT_LBUTTONUP:
				_status.leftButtonFl = true;
				break;
			case Common::EVENT_RBUTTONUP:
				_status.rightButtonFl = true;
				break;
			case Common::EVENT_QUIT:
				_status.doQuitFl = true;
				break;
			default:
				break;
			}
		}
		_mouse->mouseHandler();                     // Mouse activity - adds to display list
		_screen->displayList(kDisplayDisplay);      // Blit the display list to screen

		_status.doQuitFl |= shouldQuit();           // update game quit flag
	}
	return Common::kNoError;
}

void HugoEngine::initMachine() {
	if (_gameVariant == kGameVariantH1Dos)
		readScreenFiles(0);
	else
		_file->readBackground(_numScreens - 1);     // Splash screen
	_object->readObjectImages();                    // Read all object images
	if (_platform == Common::kPlatformWindows)
		_file->readUIFImages();                     // Read all uif images (only in Win versions)
}

/**
* Hugo game state machine - called during onIdle
*/
void HugoEngine::runMachine() {
	static uint32 lastTime;
	uint32 curTime;

	status_t &gameStatus = getGameStatus();
	// Don't process if we're in a textbox
	if (gameStatus.textBoxFl)
		return;

	// Don't process if gameover
	if (gameStatus.gameOverFl)
		return;

	curTime = g_system->getMillis();
	// Process machine once every tick
	while (curTime - lastTime < (uint32)(1000 / getTPS())) {
		g_system->delayMillis(5);
		curTime = g_system->getMillis();
	}
	lastTime = curTime;

	switch (gameStatus.viewState) {
	case kViewIdle:                                 // Not processing state machine
		_screen->hideCursor();
		_intro->preNewGame();                       // Any processing before New Game selected
		break;
	case kViewIntroInit:                            // Initialization before intro begins
		_intro->introInit();
		gameStatus.viewState = kViewIntro;
		break;
	case kViewIntro:                                // Do any game-dependant preamble
		if (_intro->introPlay()) {                  // Process intro screen
			_scheduler->newScreen(0);               // Initialize first screen
			gameStatus.viewState = kViewPlay;
		}
		break;
	case kViewPlay:                                 // Playing game
		_screen->showCursor();
		_parser->charHandler();                     // Process user cmd input
		_object->moveObjects();                     // Process object movement
		_scheduler->runScheduler();                 // Process any actions
		_screen->displayList(kDisplayRestore);      // Restore previous background
		_object->updateImages();                    // Draw into _frontBuffer, compile display list
		_screen->drawStatusText();
		_screen->displayList(kDisplayDisplay);      // Blit the display list to screen
		_sound->checkMusic();
		break;
	case kViewInvent:                               // Accessing inventory
		_inventory->runInventory();                 // Process Inventory state machine
		break;
	case kViewExit:                                 // Game over or user exited
		gameStatus.viewState = kViewIdle;
		_status.doQuitFl = true;
		break;
	}
}

/**
* Loads Hugo.dat file, which contains all the hardcoded data in the original executables
*/
bool HugoEngine::loadHugoDat() {
	Common::File in;
	in.open("hugo.dat");

	if (!in.isOpen()) {
		Common::String errorMessage = "You're missing the 'hugo.dat' file. Get it from the ScummVM website";
		GUIErrorMessage(errorMessage);
		warning("%s", errorMessage.c_str());
		return false;
	}

	// Read header
	char buf[256];
	in.read(buf, 4);
	buf[4] = '\0';

	if (strcmp(buf, "HUGO")) {
		Common::String errorMessage = "File 'hugo.dat' is corrupt. Get it from the ScummVM website";
		GUIErrorMessage(errorMessage);
		warning("%s", errorMessage.c_str());
		return false;
	}

	int majVer = in.readByte();
	int minVer = in.readByte();

	if ((majVer != HUGO_DAT_VER_MAJ) || (minVer != HUGO_DAT_VER_MIN)) {
		snprintf(buf, 256, "File 'hugo.dat' is wrong version. Expected %d.%d but got %d.%d. Get it from the ScummVM website", HUGO_DAT_VER_MAJ, HUGO_DAT_VER_MIN, majVer, minVer);
		GUIErrorMessage(buf);
		warning("%s", buf);

		return false;
	}

	_numVariant = in.readUint16BE();

	_screen->loadPalette(in);

	_text->loadAllTexts(in);

	// Read x_intro and y_intro
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		int numRows = in.readUint16BE();
		if (varnt == _gameVariant) {
			_introXSize = numRows;
			_introX = (byte *)malloc(sizeof(byte) * _introXSize);
			_introY = (byte *)malloc(sizeof(byte) * _introXSize);
			for (int i = 0; i < _introXSize; i++) {
				_introX[i] = in.readByte();
				_introY[i] = in.readByte();
			}
		} else {
			for (int i = 0; i < numRows; i++) {
				in.readByte();
				in.readByte();
			}
		}
	}

	// Read _arrayReqs
	_arrayReqs = loadLongArray(in);

	// Read _hotspots
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		int numRows = in.readUint16BE();
		if (varnt == _gameVariant) {
			_hotspots = (hotspot_t *)malloc(sizeof(hotspot_t) * numRows);
			for (int i = 0; i < numRows; i++) {
				_hotspots[i].screenIndex = in.readSint16BE();
				_hotspots[i].x1 = in.readSint16BE();
				_hotspots[i].y1 = in.readSint16BE();
				_hotspots[i].x2 = in.readSint16BE();
				_hotspots[i].y2 = in.readSint16BE();
				_hotspots[i].actIndex = in.readUint16BE();
				_hotspots[i].viewx = in.readSint16BE();
				_hotspots[i].viewy = in.readSint16BE();
				_hotspots[i].direction = in.readSint16BE();
			}
		} else {
			for (int i = 0; i < numRows; i++) {
				in.readSint16BE();
				in.readSint16BE();
				in.readSint16BE();
				in.readSint16BE();
				in.readSint16BE();
				in.readUint16BE();
				in.readSint16BE();
				in.readSint16BE();
				in.readSint16BE();
			}
		}
	}

	int numElem, numSubElem;
	//Read _invent
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_maxInvent = numElem;
			_invent = (int16 *)malloc(sizeof(int16) * numElem);
			for (int i = 0; i < numElem; i++)
				_invent[i] = in.readSint16BE();
		} else {
			for (int i = 0; i < numElem; i++)
				in.readSint16BE();
		}
	}

	//Read _uses
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_usesSize = numElem;
			_uses = (uses_t *)malloc(sizeof(uses_t) * numElem);
			for (int i = 0; i < numElem; i++) {
				_uses[i].objId = in.readSint16BE();
				_uses[i].dataIndex = in.readUint16BE();
				numSubElem = in.readUint16BE();
				_uses[i].targets = (target_t *)malloc(sizeof(target_t) * numSubElem);
				for (int j = 0; j < numSubElem; j++) {
					_uses[i].targets[j].nounIndex = in.readUint16BE();
					_uses[i].targets[j].verbIndex = in.readUint16BE();
				}
			}
		} else {
			for (int i = 0; i < numElem; i++) {
				in.readSint16BE();
				in.readUint16BE();
				numSubElem = in.readUint16BE();
				for (int j = 0; j < numSubElem; j++) {
					in.readUint16BE();
					in.readUint16BE();
				}
			}
		}
	}

	//Read _catchallList
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_catchallList = (background_t *)malloc(sizeof(background_t) * numElem);
			for (int i = 0; i < numElem; i++) {
				_catchallList[i].verbIndex = in.readUint16BE();
				_catchallList[i].nounIndex = in.readUint16BE();
				_catchallList[i].commentIndex = in.readSint16BE();
				_catchallList[i].matchFl = (in.readByte() != 0);
				_catchallList[i].roomState = in.readByte();
				_catchallList[i].bonusIndex = in.readByte();
			}
		} else {
			for (int i = 0; i < numElem; i++) {
				in.readUint16BE();
				in.readUint16BE();
				in.readSint16BE();
				in.readByte();
				in.readByte();
				in.readByte();
			}
		}
	}

// Read _background_objects
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_backgroundObjectsSize = numElem;
			_backgroundObjects = (background_t **)malloc(sizeof(background_t *) * _backgroundObjectsSize);
			for (int i = 0; i < _backgroundObjectsSize; i++) {
				numSubElem = in.readUint16BE();
				_backgroundObjects[i] = (background_t *)malloc(sizeof(background_t) * numSubElem);
				for (int j = 0; j < numSubElem; j++) {
					_backgroundObjects[i][j].verbIndex = in.readUint16BE();
					_backgroundObjects[i][j].nounIndex = in.readUint16BE();
					_backgroundObjects[i][j].commentIndex = in.readSint16BE();
					_backgroundObjects[i][j].matchFl = (in.readByte() != 0);
					_backgroundObjects[i][j].roomState = in.readByte();
					_backgroundObjects[i][j].bonusIndex = in.readByte();
				}
			}
		} else {
			for (int i = 0; i < numElem; i++) {
				numSubElem = in.readUint16BE();
				for (int j = 0; j < numSubElem; j++) {
					in.readUint16BE();
					in.readUint16BE();
					in.readSint16BE();
					in.readByte();
					in.readByte();
					in.readByte();
				}
			}
		}
	}

	// Read _points
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_numBonuses = numElem;
			_points = (point_t *)malloc(sizeof(point_t) * _numBonuses);
			for (int i = 0; i < _numBonuses; i++) {
				_points[i].score = in.readByte();
				_points[i].scoredFl = false;
			}
		} else {
			for (int i = 0; i < numElem; i++)
				in.readByte();
		}
	}

	// Read _cmdList
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_cmdListSize = numElem;
			_cmdList = (cmd **)malloc(sizeof(cmd *) * _cmdListSize);
			for (int i = 0; i < _cmdListSize; i++) {
				numSubElem = in.readUint16BE();
				_cmdList[i] = (cmd *)malloc(sizeof(cmd) * numSubElem);
				for (int j = 0; j < numSubElem; j++) {
					_cmdList[i][j].verbIndex = in.readUint16BE();
					_cmdList[i][j].reqIndex = in.readUint16BE();
					_cmdList[i][j].textDataNoCarryIndex = in.readUint16BE();
					_cmdList[i][j].reqState = in.readByte();
					_cmdList[i][j].newState = in.readByte();
					_cmdList[i][j].textDataWrongIndex = in.readUint16BE();
					_cmdList[i][j].textDataDoneIndex = in.readUint16BE();
					_cmdList[i][j].actIndex = in.readUint16BE();
				}
			}
		} else {
			for (int i = 0; i < numElem; i++) {
				numSubElem = in.readUint16BE();
				for (int j = 0; j < numSubElem; j++) {
					in.readUint16BE();
					in.readUint16BE();
					in.readUint16BE();
					in.readByte();
					in.readByte();
					in.readUint16BE();
					in.readUint16BE();
					in.readUint16BE();
				}
			}
		}
	}

	// Read _screenActs
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_screenActsSize = numElem;
			_screenActs = (uint16 **)malloc(sizeof(uint16 *) * _screenActsSize);
			for (int i = 0; i < _screenActsSize; i++) {
				numSubElem = in.readUint16BE();
				if (numSubElem == 0) {
					_screenActs[i] = 0;
				} else {
					_screenActs[i] = (uint16 *)malloc(sizeof(uint16) * numSubElem);
					for (int j = 0; j < numSubElem; j++)
						_screenActs[i][j] = in.readUint16BE();
				}
			}
		} else {
			for (int i = 0; i < numElem; i++) {
				numSubElem = in.readUint16BE();
				for (int j = 0; j < numSubElem; j++)
					in.readUint16BE();
			}
		}
	}

	_object->loadObjectArr(in);

	_hero = &_object->_objects[kHeroIndex];         // This always points to hero
	_screen_p = &(_object->_objects[kHeroIndex].screenIndex); // Current screen is hero's
	_heroImage = kHeroIndex;                        // Current in use hero image

	_scheduler->loadActListArr(in);

	for (int varnt = 0; varnt < _numVariant; varnt++) {
		if (varnt == _gameVariant) {
			_tunesNbr     = in.readSByte();
			_soundSilence = in.readSByte();
			_soundTest    = in.readSByte();
		} else {
			in.readSByte();
			in.readSByte();
			in.readSByte();
		}
	}

	//Read _defltTunes
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_defltTunes = (int16 *)malloc(sizeof(int16) * numElem);
			for (int i = 0; i < numElem; i++)
				_defltTunes[i] = in.readSint16BE();
		} else {
			for (int i = 0; i < numElem; i++)
				in.readSint16BE();
		}
	}

	//Read _screenStates size
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_screenStates = (byte *)malloc(sizeof(byte) * numElem);
			for (int i = 0; i < numElem; i++)
				_screenStates[i] = 0;
		}
	}

	//Read look, take and drop special verbs indexes
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		if (varnt == _gameVariant) {
			_look = in.readUint16BE();
			_take = in.readUint16BE();
			_drop = in.readUint16BE();
		} else {
			in.readUint16BE();
			in.readUint16BE();
			in.readUint16BE();
		}
	}

	_object->loadNumObj(in);
	_scheduler->loadAlNewscrIndex(in);
	_sound->loadIntroSong(in);
	_screen->loadFontArr(in);
	_topMenu->loadBmpArr(in);

	return true;
}

uint16 **HugoEngine::loadLongArray(Common::File &in) {
	uint16 **resArray = 0;

	for (int varnt = 0; varnt < _numVariant; varnt++) {
		uint16 numRows = in.readUint16BE();
		if (varnt == _gameVariant) {
			resArray = (uint16 **)malloc(sizeof(uint16 *) * (numRows + 1));
			resArray[numRows] = 0;
		}
		for (int i = 0; i < numRows; i++) {
			uint16 numElems = in.readUint16BE();
			if (varnt == _gameVariant) {
				uint16 *resRow = (uint16 *)malloc(sizeof(uint16) * numElems);
				for (int j = 0; j < numElems; j++)
					resRow[j] = in.readUint16BE();
				resArray[i] = resRow;
			} else {
				for (int j = 0; j < numElems; j++)
					in.readUint16BE();
			}
		}
	}
	return resArray;
}

/**
* Sets the playlist to be the default tune selection
*/
void HugoEngine::initPlaylist(bool playlist[kMaxTunes]) {
	debugC(1, kDebugEngine, "initPlaylist");

	for (int16 i = 0; i < kMaxTunes; i++)
		playlist[i] = false;
	for (int16 i = 0; _defltTunes[i] != -1; i++)
		playlist[_defltTunes[i]] = true;
}

/**
* Initialize the dynamic game status
*/
void HugoEngine::initStatus() {
	debugC(1, kDebugEngine, "initStatus");
	_status.storyModeFl   = false;                  // Not in story mode
	_status.gameOverFl    = false;                  // Hero not knobbled yet
	_status.demoFl        = false;                  // Not demo mode
	_status.textBoxFl     = false;                  // Not processing a text box
	_status.lookFl        = false;                  // Toolbar "look" button
	_status.recallFl      = false;                  // Toolbar "recall" button
	_status.leftButtonFl  = false;                  // Left mouse button pressed
	_status.rightButtonFl = false;                  // Right mouse button pressed
	_status.newScreenFl   = false;                  // Screen not just loaded
	_status.jumpExitFl    = false;                  // Can't jump to a screen exit
	_status.godModeFl     = false;                  // No special cheats allowed
	_status.helpFl        = false;                  // Not calling WinHelp()
	_status.doQuitFl      = false;
	_status.skipIntroFl   = false;
	_status.path[0]       = 0;                      // Path to write files

	// Initialize every start of new game
	_status.tick            = 0;                    // Tick count
	_status.viewState       = kViewIdle;            // View state
	_status.inventoryState  = kInventoryOff;        // Inventory icon bar state
	_status.inventoryHeight = 0;                    // Inventory icon bar pos
	_status.inventoryObjId  = -1;                   // Inventory object selected (none)
	_status.routeIndex      = -1;                   // Hero not following a route
	_status.go_for          = kRouteSpace;          // Hero walking to space
	_status.go_id           = -1;                   // Hero not walking to anything

// Strangerke - Suppress as related to playback
//	_status.recordFl      = false;                  // Not record mode
//	_status.playbackFl    = false;                  // Not playback mode
// Strangerke - Not used ?
//	_status.mmtime        = false;                  // Multimedia timer support
//	_status.screenWidth   = 0;                      // Desktop screen width
//	_status.saveTick      = 0;                      // Time of last save
//	_status.saveSlot      = 0;                      // Slot to save/restore game
}

/**
* Initialize default config values.  Must be done before Initialize().
*/
void HugoEngine::initConfig() {
	debugC(1, kDebugEngine, "initConfig()");

	_config.musicFl = true;                            // Music state initially on
	_config.soundFl = true;                            // Sound state initially on
	_config.turboFl = false;                           // Turbo state initially off
	initPlaylist(_config.playlist);                    // Initialize default tune playlist
	_file->readBootFile();                             // Read startup structure
}

/**
* Reset config parts. Currently only reset music played based on playlist
*/
void HugoEngine::resetConfig() {
	debugC(1, kDebugEngine, "resetConfig()");

	// Find first tune and play it
	for (int16 i = 0; i < kMaxTunes; i++) {
		if (_config.playlist[i]) {
			_sound->playMusic(i);
			break;
		}
	}
}

void HugoEngine::initialize() {
	debugC(1, kDebugEngine, "initialize");

	_maze.enabledFl = false;
	_line[0] = '\0';

	_sound->initSound();
	_scheduler->initEventQueue();                   // Init scheduler stuff
	_screen->initDisplay();                         // Create Dibs and palette
	_file->openDatabaseFiles();                     // Open database files
	calcMaxScore();                                 // Initialise maxscore

	_rnd = new Common::RandomSource();
	g_eventRec.registerRandomSource(*_rnd, "hugo");
	_rnd->setSeed(42);                              // Kick random number generator

	switch (_gameVariant) {
	case kGameVariantH1Dos:
		_episode = "\"Hugo's House of Horrors\"";
		_picDir = "";
		break;
	case kGameVariantH2Dos:
		_episode = "\"Hugo II: Whodunit?\"";
		_picDir = "";
		break;
	case kGameVariantH3Dos:
		_episode = "\"Hugo III: Jungle of Doom\"";
		_picDir = "pictures/";
		break;
	case kGameVariantH1Win:
		_episode = "\"Hugo's Horrific Adventure\"";
		_picDir = "hugo1/";
		break;
	case kGameVariantH2Win:
		_episode = "\"Hugo's Mystery Adventure\"";
		_picDir = "hugo2/";
		break;
	case kGameVariantH3Win:
		_episode = "\"Hugo's Amazon Adventure\"";
		_picDir = "hugo3/";
		break;
	default:
		error("Unknown game");
	}
}

/**
* Restore all resources before termination
*/
void HugoEngine::shutdown() {
	debugC(1, kDebugEngine, "shutdown");

	_file->closeDatabaseFiles();
	_object->freeObjects();
}

/**
* Read scenery, overlay files for given screen number
*/
void HugoEngine::readScreenFiles(int screenNum) {
	debugC(1, kDebugEngine, "readScreenFiles(%d)", screenNum);

	_file->readBackground(screenNum);               // Scenery file
	memcpy(_screen->getBackBuffer(), _screen->getFrontBuffer(), sizeof(_screen->getFrontBuffer())); // Make a copy
	_file->readOverlay(screenNum, _boundary, kOvlBoundary); // Boundary file
	_file->readOverlay(screenNum, _overlay, kOvlOverlay);   // Overlay file
	_file->readOverlay(screenNum, _ovlBase, kOvlBase);      // Overlay base file

	// Suppress a boundary used in H3 DOS in 'Crash' screen, which blocks
	// pathfinding and is useless.
	if ((screenNum == 0) && (_gameVariant == kGameVariantH3Dos))
		clearScreenBoundary(50, 311, 152);
}

/**
* Return maximum allowed movement (from zero to vx) such that object does
* not cross a boundary (either background or another object)
*/
int HugoEngine::deltaX(int x1, int x2, int vx, int y) {
// Explanation of algorithm:  The boundaries are drawn as contiguous
// lines 1 pixel wide.  Since DX,DY are not necessarily 1, we must
// detect boundary crossing.  If vx positive, examine each pixel from
// x1 old to x2 new, else x2 old to x1 new, both at the y2 line.
// If vx zero, no need to check.  If vy non-zero then examine each
// pixel on the line segment x1 to x2 from y old to y new.
// Fix from Hugo I v1.5:
// Note the diff is munged in the return statement to cater for a special
// cases arising from differences in image widths from one sequence to
// another.  The problem occurs reversing direction at a wall where the
// new image intersects before the object can move away.  This is cured
// by comparing the intersection with half the object width pos. If the
// intersection is in the other half wrt the intended direction, use the
// desired vx, else use the computed delta.  i.e. believe the desired vx

	debugC(3, kDebugEngine, "deltaX(%d, %d, %d, %d)", x1, x2, vx, y);

	if (vx == 0)
		return 0 ;                                  // Object stationary

	y *= kCompLineSize;                             // Offset into boundary file
	if (vx > 0) {
		// Moving to right
		for (int i = x1 >> 3; i <= (x2 + vx) >> 3; i++) {// Search by byte
			int b = Utils::firstBit((byte)(_boundary[y + i] | _objBound[y + i]));
			if (b < 8) {   // b is index or 8
				// Compute x of boundary and test if intersection
				b += i << 3;
				if ((b >= x1) && (b <= x2 + vx))
					return (b < x1 + ((x2 - x1) >> 1)) ? vx : b - x2 - 1; // return dx
			}
		}
	} else {
		// Moving to left
		for (int i = x2 >> 3; i >= (x1 + vx) >> 3; i--) {// Search by byte
			int b = Utils::lastBit((byte)(_boundary[y + i] | _objBound[y + i]));
			if (b < 8) {    // b is index or 8
				// Compute x of boundary and test if intersection
				b += i << 3;
				if ((b >= x1 + vx) && (b <= x2))
					return (b > x1 + ((x2 - x1) >> 1)) ? vx : b - x1 + 1; // return dx
			}
		}
	}
	return vx;
}

/**
* Similar to Delta_x, but for movement in y direction.  Special case of
* bytes at end of line segment; must only count boundary bits falling on
* line segment.
*/
int HugoEngine::deltaY(int x1, int x2, int vy, int y) {
	debugC(3, kDebugEngine, "deltaY(%d, %d, %d, %d)", x1, x2, vy, y);

	if (vy == 0)
		return 0;                                   // Object stationary

	int inc = (vy > 0) ? 1 : -1;
	for (int j = y + inc; j != (y + vy + inc); j += inc) { //Search by byte
		for (int i = x1 >> 3; i <= x2 >> 3; i++) {
			int b = _boundary[j * kCompLineSize + i] | _objBound[j * kCompLineSize + i];
			if (b != 0) {                           // Any bit set
				// Make sure boundary bits fall on line segment
				if (i == (x2 >> 3))                 // Adjust right end
					b &= 0xff << ((i << 3) + 7 - x2);
				else if (i == (x1 >> 3))            // Adjust left end
					b &= 0xff >> (x1 - (i << 3));
				if (b)
					return j - y - inc;
			}
		}
	}
	return vy;
}

/**
* Store a horizontal line segment in the object boundary file
*/
void HugoEngine::storeBoundary(int x1, int x2, int y) {
	debugC(5, kDebugEngine, "storeBoundary(%d, %d, %d)", x1, x2, y);

	for (int i = x1 >> 3; i <= x2 >> 3; i++) {      // For each byte in line
		byte *b = &_objBound[y * kCompLineSize + i];// get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b |= 0xff << ((i << 3) + 7 - x2);
		else if (i == x1 >> 3)                      // Adjust left end
			*b |= 0xff >> (x1 - (i << 3));
		else
			*b = 0xff;
	}
}

/**
* Clear a horizontal line segment in the object boundary file
*/
void HugoEngine::clearBoundary(int x1, int x2, int y) {
	debugC(5, kDebugEngine, "clearBoundary(%d, %d, %d)", x1, x2, y);

	for (int i = x1 >> 3; i <= x2 >> 3; i++) {      // For each byte in line
		byte *b = &_objBound[y * kCompLineSize + i];// get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b &= ~(0xff << ((i << 3) + 7 - x2));
		else if (i == x1 >> 3)                      // Adjust left end
			*b &= ~(0xff >> (x1 - (i << 3)));
		else
			*b = 0;
	}
}

/**
* Clear a horizontal line segment in the screen boundary file
* Used to fix some data issues
*/
void HugoEngine::clearScreenBoundary(int x1, int x2, int y) {
	debugC(5, kDebugEngine, "clearScreenBoundary(%d, %d, %d)", x1, x2, y);

	for (int i = x1 >> 3; i <= x2 >> 3; i++) {      // For each byte in line
		byte *b = &_boundary[y * kCompLineSize + i];// get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b &= ~(0xff << ((i << 3) + 7 - x2));
		else if (i == x1 >> 3)                      // Adjust left end
			*b &= ~(0xff >> (x1 - (i << 3)));
		else
			*b = 0;
	}
}

/**
* Search background command list for this screen for supplied object.
* Return first associated verb (not "look") or 0 if none found.
*/
char *HugoEngine::useBG(char *name) {
	debugC(1, kDebugEngine, "useBG(%s)", name);

	objectList_t p = _backgroundObjects[*_screen_p];
	for (int i = 0; p[i].verbIndex != 0; i++) {
		if ((name == _text->getNoun(p[i].nounIndex, 0) &&
		     p[i].verbIndex != _look) &&
		    ((p[i].roomState == kStateDontCare) || (p[i].roomState == _screenStates[*_screen_p])))
			return _text->getVerb(p[i].verbIndex, 0);
	}

	return 0;
}

/**
* Add action lists for this screen to event queue
*/
void HugoEngine::screenActions(int screenNum) {
	debugC(1, kDebugEngine, "screenActions(%d)", screenNum);

	uint16 *screenAct = _screenActs[screenNum];
	if (screenAct) {
		for (int i = 0; screenAct[i]; i++)
			_scheduler->insertActionList(screenAct[i]);
	}
}

/**
* Set the new screen number into the hero object and any carried objects
*/
void HugoEngine::setNewScreen(int screenNum) {
	debugC(1, kDebugEngine, "setNewScreen(%d)", screenNum);

	*_screen_p = screenNum;                         // HERO object
	_object->setCarriedScreen(screenNum);           // Carried objects
}

/**
* An object has collided with a boundary. See if any actions are required
*/
void HugoEngine::boundaryCollision(object_t *obj) {
	debugC(1, kDebugEngine, "boundaryCollision");

	if (obj == _hero) {
		// Hotspots only relevant to HERO
		int x;
		if (obj->vx > 0)
			x = obj->x + obj->currImagePtr->x2;
		else
			x = obj->x + obj->currImagePtr->x1;
		int y = obj->y + obj->currImagePtr->y2;

		for (int i = 0; _hotspots[i].screenIndex >= 0; i++) {
			hotspot_t *hotspot = &_hotspots[i];
			if (hotspot->screenIndex == obj->screenIndex)
				if ((x >= hotspot->x1) && (x <= hotspot->x2) && (y >= hotspot->y1) && (y <= hotspot->y2)) {
					_scheduler->insertActionList(hotspot->actIndex);
					break;
				}
		}
	} else {
		// Check whether an object collided with HERO
		int dx = _hero->x + _hero->currImagePtr->x1 - obj->x - obj->currImagePtr->x1;
		int dy = _hero->y + _hero->currImagePtr->y2 - obj->y - obj->currImagePtr->y2;
		// If object's radius is infinity, use a closer value
		int8 radius = obj->radius;
		if (radius < 0)
			radius = kStepDx * 2;
		if ((abs(dx) <= radius) && (abs(dy) <= radius))
			_scheduler->insertActionList(obj->actIndex);
	}
}

/**
* Add up all the object values and all the bonus points
*/
void HugoEngine::calcMaxScore() {
	debugC(1, kDebugEngine, "calcMaxScore");

	_maxscore = _object->calcMaxScore();

	for (int i = 0; i < _numBonuses; i++)
		_maxscore += _points[i].score;
}

/**
* Exit game, advertise trilogy, show copyright
*/
void HugoEngine::endGame() {
	debugC(1, kDebugEngine, "endGame");

	if (!_boot.registered)
		Utils::Box(kBoxAny, "%s", _text->getTextEngine(kEsAdvertise));
	Utils::Box(kBoxAny, "%s\n%s", _episode, getCopyrightString());
	_status.viewState = kViewExit;
}

bool HugoEngine::canLoadGameStateCurrently() {
	return true;
}

bool HugoEngine::canSaveGameStateCurrently() {
	return (_status.viewState == kViewPlay);
}

int8 HugoEngine::getTPS() {
	return ((_config.turboFl) ? kTurboTps : _normalTPS);
}

void HugoEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_sound->syncVolume();
}


} // End of namespace Hugo
