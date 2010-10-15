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
#include "hugo/global.h"
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

#include "engines/util.h"

namespace Hugo {

HugoEngine *HugoEngine::s_Engine = 0;

overlay_t HugoEngine::_boundary;
overlay_t HugoEngine::_overlay;
overlay_t HugoEngine::_ovlBase;
overlay_t HugoEngine::_objBound;

config_t    _config;                            // User's config
maze_t      _maze;                              // Default to not in maze
hugo_boot_t _boot;                              // Boot info structure file
char        _textBoxBuffer[MAX_BOX];            // Buffer for text box
command_t   _line;                              // Line of user text input

HugoEngine::HugoEngine(OSystem *syst, const HugoGameDescription *gd) : Engine(syst), _gameDescription(gd), _mouseX(0), _mouseY(0),
	_textData(0), _stringtData(0), _screenNames(0), _textEngine(0), _textIntro(0), _textMouse(0), _textParser(0), _textSchedule(0), _textUtil(0),
	_arrayNouns(0), _arrayVerbs(0), _arrayReqs(0), _hotspots(0), _invent(0), _uses(0), _catchallList(0), _backgroundObjects(0),
	_points(0), _cmdList(0), _screenActs(0), _objects(0), _actListArr(0), _heroImage(0), _defltTunes(0), _palette(0), _introX(0),
	_introY(0), _maxInvent(0), _numBonuses(0), _numScreens(0), _tunesNbr(0), _soundSilence(0), _soundTest(0), _screenStates(0), _numObj(0),
	_score(0), _maxscore(0)

{
	DebugMan.addDebugChannel(kDebugSchedule, "Schedule", "Script Schedule debug level");
	DebugMan.addDebugChannel(kDebugEngine, "Engine", "Engine debug level");
	DebugMan.addDebugChannel(kDebugDisplay, "Display", "Display debug level");
	DebugMan.addDebugChannel(kDebugMouse, "Mouse", "Mouse debug level");
	DebugMan.addDebugChannel(kDebugParser, "Parser", "Parser debug level");
	DebugMan.addDebugChannel(kDebugFile, "File", "File IO debug level");
	DebugMan.addDebugChannel(kDebugRoute, "Route", "Route debug level");
	DebugMan.addDebugChannel(kDebugInventory, "Inventory", "Inventory debug level");

	for (int j = 0; j < NUM_FONTS; j++)
		_arrayFont[j] = 0;
}

HugoEngine::~HugoEngine() {
	delete _soundHandler;
	delete _route;
	delete _parser;
	delete _inventoryHandler;
	delete _mouseHandler;
	delete _screen;
	delete _scheduler;
	delete _fileManager;

	free(_palette);
	free(_introX);
	free(_introY);

#if 0
	freeTexts(_textData);
	freeTexts(_stringtData);
	freeTexts(_textEngine);
	freeTexts(_textIntro);
	freeTexts(_textMouse);
	freeTexts(_textParser);
	freeTexts(_textSchedule);
	freeTexts(_textUtil);
#endif
	free(_textData);
	free(_stringtData);
	free(_screenNames);
	free(_textEngine);
	free(_textIntro);
	free(_textMouse);
	free(_textParser);
	free(_textSchedule);
	free(_textUtil);

	warning("Missing: free _arrayNouns");
	warning("Missing: free _arrayVerbs");

	free(_arrayReqs);
	free(_hotspots);
	free(_invent);
	free(_uses);
	free(_catchallList);

	warning("Missing: free _background_objects");

	free(_points);

	warning("Missing: free _cmdList");
	warning("Missing: free _screenActs");
	warning("Missing: free _objects");

	free(_defltTunes);
	free(_screenStates);

	if (_arrayFont[0])
		free(_arrayFont[0]);

	if (_arrayFont[1])
		free(_arrayFont[1]);

	if (_arrayFont[2])
		free(_arrayFont[2]);
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

	_mouseHandler = new MouseHandler(*this);
	_inventoryHandler = new InventoryHandler(*this);
	_route = new Route(*this);
	_soundHandler = new SoundHandler(*this);

	switch (_gameVariant) {
	case 0: // H1 Win
		_fileManager = new FileManager_v1w(*this);
		_scheduler = new Scheduler_v3d(*this);
		_introHandler = new intro_v1w(*this);
		_screen = new Screen_v1w(*this);
		_parser = new Parser_v1w(*this);
		break;
	case 1:
		_fileManager = new FileManager_v2d(*this);
		_scheduler = new Scheduler_v3d(*this);
		_introHandler = new intro_v2w(*this);
		_screen = new Screen_v1w(*this);
		_parser = new Parser_v1w(*this);
		break;
	case 2:
		_fileManager = new FileManager_v2d(*this);
		_scheduler = new Scheduler_v3d(*this);
		_introHandler = new intro_v3w(*this);
		_screen = new Screen_v1w(*this);
		_parser = new Parser_v1w(*this);
		break;
	case 3: // H1 DOS
		_fileManager = new FileManager_v1d(*this);
		_scheduler = new Scheduler_v1d(*this);
		_introHandler = new intro_v1d(*this);
		_screen = new Screen_v1d(*this);
		_parser = new Parser_v1d(*this);
		break;
	case 4:
		_fileManager = new FileManager_v2d(*this);
		_scheduler = new Scheduler_v1d(*this);
		_introHandler = new intro_v2d(*this);
		_screen = new Screen_v1d(*this);
		_parser = new Parser_v2d(*this);
		break;
	case 5:
		_fileManager = new FileManager_v3d(*this);
		_scheduler = new Scheduler_v3d(*this);
		_introHandler = new intro_v3d(*this);
		_screen = new Screen_v1d(*this);
		_parser = new Parser_v3d(*this);
		break;
	}

	if (!loadHugoDat())
		return Common::kUnknownError;

	// Interesting situation: We have no cursor to show, since
	// the DOS version had none, and the Windows version just used
	// the windows default one. Meaning this call will just use whatever
	// was used last, i.e. the launcher GUI cursor. What to do?
	g_system->showMouse(true);

	initStatus();                                   // Initialize game status
	initConfig(INSTALL);                            // Initialize user's config
	initialize();
	initConfig(RESET);                              // Reset user's config

	file().restoreGame(-1);

	initMachine();

	// Start the state machine
	_status.viewState = V_INTROINIT;

	_status.doQuitFl = false;

	while (!_status.doQuitFl) {
		g_system->updateScreen();

		runMachine();
		// Handle input
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				parser().keyHandler(event.kbd.keycode, 0);
				break;
			case Common::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				break;
			case Common::EVENT_LBUTTONDOWN:
				_status.leftButtonFl = true;
				break;
			case Common::EVENT_LBUTTONUP:
				_status.leftButtonFl = false;
				break;
			case Common::EVENT_RBUTTONDOWN:
				_status.rightButtonFl = true;
				break;
			case Common::EVENT_RBUTTONUP:
				_status.rightButtonFl = false;
				break;
			case Common::EVENT_QUIT:
				_status.doQuitFl = true;
				break;
			default:
				break;
			}
		}
	}
	return Common::kNoError;
}

void HugoEngine::initMachine() {
	if (_gameVariant == kGameVariantH1Dos)
		readScreenFiles(0);
	else
		file().readBackground(_numScreens - 1);         // Splash screen
	readObjectImages();                             // Read all object images
	if (_platform == Common::kPlatformWindows)
		readUIFImages();                            // Read all uif images (only in Win versions)
}

void HugoEngine::runMachine() {
// Hugo game state machine - called during onIdle
	static uint32 lastTime;

	status_t &gameStatus = getGameStatus();
	// Don't process if we're in a textbox
	if (gameStatus.textBoxFl)
		return;

	// Don't process if gameover
	if (gameStatus.gameOverFl)
		return;

	// Process machine once every tick
	if (g_system->getMillis() - lastTime < (uint32)(1000 / TPS))
		return;
	lastTime = g_system->getMillis();

	switch (gameStatus.viewState) {
	case V_IDLE:                                    // Not processing state machine
		intro().preNewGame();                           // Any processing before New Game selected
		break;
	case V_INTROINIT:                               // Initialization before intro begins
		intro().introInit();
		g_system->showMouse(false);
		gameStatus.viewState = V_INTRO;
		break;
	case V_INTRO:                                   // Do any game-dependant preamble
		if (intro().introPlay())    {               // Process intro screen
			scheduler().newScreen(0);               // Initialize first screen
			gameStatus.viewState = V_PLAY;
		}
		break;
	case V_PLAY:                                    // Playing game
		g_system->showMouse(true);
		parser().charHandler();                     // Process user cmd input
		moveObjects();                              // Process object movement
		scheduler().runScheduler();                 // Process any actions
		screen().displayList(D_RESTORE);            // Restore previous background
		updateImages();                             // Draw into _frontBuffer, compile display list
		mouse().mouseHandler();                     // Mouse activity - adds to display list
		screen().drawStatusText();
		screen().displayList(D_DISPLAY);            // Blit the display list to screen
		break;
	case V_INVENT:                                  // Accessing inventory
		inventory().runInventory();                 // Process Inventory state machine
		break;
	case V_EXIT:                                    // Game over or user exited
		gameStatus.viewState = V_IDLE;
		_status.doQuitFl = true;
		break;
	}
}

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

	// Read textData
	_textData = loadTextsVariante(in, 0);

	// Read stringtData
	// Only Hugo 1 DOS should use this array
	_stringtData = loadTextsVariante(in, 0);

	// Read arrayNouns
	_arrayNouns = loadTextsArray(in);

	// Read arrayVerbs
	_arrayVerbs = loadTextsArray(in);

	// Read screenNames
	_screenNames = loadTextsVariante(in, &_numScreens);

	// Read palette
	_paletteSize = in.readUint16BE();
	_palette = (byte *)malloc(sizeof(byte) * _paletteSize);
	for (int i = 0; i < _paletteSize; i++)
		_palette[i] = in.readByte();

	// Read textEngine
	_textEngine = loadTexts(in);

	// Read textIntro
	_textIntro = loadTextsVariante(in, 0);

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

	// Read textMouse
	_textMouse = loadTexts(in);

	// Read textParser
	_textParser = loadTexts(in);

	// Read textSchedule
	_textSchedule = loadTexts(in);

	// Read textUtil
	_textUtil = loadTexts(in);

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

	int numElem, numSubElem, numSubAct;
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
			_backgroundObjects = (background_t **)malloc(sizeof(background_t *) * numElem);
			for (int i = 0; i < numElem; i++) {
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
			_cmdList = (cmd **)malloc(sizeof(cmd *) * numElem);
			for (int i = 0; i < numElem; i++) {
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

// TODO: For Hugo2 and Hugo3, if not in story mode, increment _screenActs[0][0] (ex: kALcrashStory + 1 == kALcrashNoStory)
	// Read _screenActs
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_screenActs = (uint16 **)malloc(sizeof(uint16 *) * numElem);
			for (int i = 0; i < numElem; i++) {
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

// TODO: For Hugo3, if not in story mode, set _objects[2].state to 3
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_objects = (object_t *)malloc(sizeof(object_t) * numElem);
			for (int i = 0; i < numElem; i++) {
				_objects[i].nounIndex = in.readUint16BE();
				_objects[i].dataIndex = in.readUint16BE();
				numSubElem = in.readUint16BE();
				if (numSubElem == 0)
					_objects[i].stateDataIndex = 0;
				else
					_objects[i].stateDataIndex = (uint16 *)malloc(sizeof(uint16) * numSubElem);
				for (int j = 0; j < numSubElem; j++)
					_objects[i].stateDataIndex[j] = in.readUint16BE();
				_objects[i].pathType = (path_t) in.readSint16BE();
				_objects[i].vxPath = in.readSint16BE();
				_objects[i].vyPath = in.readSint16BE();
				_objects[i].actIndex = in.readUint16BE();
				_objects[i].seqNumb = in.readByte();
				_objects[i].currImagePtr = 0;
				if (_objects[i].seqNumb == 0) {
					_objects[i].seqList[0].imageNbr = 0;
					_objects[i].seqList[0].seqPtr = 0;
				}
				for (int j = 0; j < _objects[i].seqNumb; j++) {
					_objects[i].seqList[j].imageNbr = in.readUint16BE();
					_objects[i].seqList[j].seqPtr = 0;
				}
				_objects[i].cycling = (cycle_t)in.readByte();
				_objects[i].cycleNumb = in.readByte();
				_objects[i].frameInterval = in.readByte();
				_objects[i].frameTimer = in.readByte();
				_objects[i].radius = in.readByte();
				_objects[i].screenIndex = in.readByte();
				_objects[i].x = in.readSint16BE();
				_objects[i].y = in.readSint16BE();
				_objects[i].oldx = in.readSint16BE();
				_objects[i].oldy = in.readSint16BE();
				_objects[i].vx = in.readByte();
				_objects[i].vy = in.readByte();
				_objects[i].objValue = in.readByte();
				_objects[i].genericCmd = in.readSint16BE();
				_objects[i].cmdIndex = in.readUint16BE();
				_objects[i].carriedFl = (in.readByte() != 0);
				_objects[i].state = in.readByte();
				_objects[i].verbOnlyFl = (in.readByte() != 0);
				_objects[i].priority = in.readByte();
				_objects[i].viewx = in.readSint16BE();
				_objects[i].viewy = in.readSint16BE();
				_objects[i].direction = in.readSint16BE();
				_objects[i].curSeqNum = in.readByte();
				_objects[i].curImageNum = in.readByte();
				_objects[i].oldvx = in.readByte();
				_objects[i].oldvy = in.readByte();
			}
		} else {
			for (int i = 0; i < numElem; i++) {
				in.readUint16BE();
				in.readUint16BE();
				numSubElem = in.readUint16BE();
				for (int j = 0; j < numSubElem; j++)
					in.readUint16BE();
				in.readSint16BE();
				in.readSint16BE();
				in.readSint16BE();
				in.readUint16BE();
				numSubElem = in.readByte();
				for (int j = 0; j < numSubElem; j++)
					in.readUint16BE();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readSint16BE();
				in.readSint16BE();
				in.readSint16BE();
				in.readSint16BE();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readSint16BE();
				in.readUint16BE();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readSint16BE();
				in.readSint16BE();
				in.readUint16BE();
				in.readByte();
				in.readByte();
				in.readByte();
				in.readByte();
			}
		}
	}
//#define HERO 0
	_hero = &_objects[HERO];                        // This always points to hero
	_screen_p = &(_objects[HERO].screenIndex);      // Current screen is hero's
	_heroImage = HERO;                              // Current in use hero image

//read _actListArr
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_actListArr = (act **)malloc(sizeof(act *) * numElem);
			for (int i = 0; i < numElem; i++) {
				numSubElem = in.readUint16BE();
				_actListArr[i] = (act *) malloc(sizeof(act) * (numSubElem + 1));
				for (int j = 0; j < numSubElem; j++) {
					_actListArr[i][j].a0.actType = (action_t) in.readByte();
					switch (_actListArr[i][j].a0.actType) {
					case ANULL:              // -1
						break;
					case ASCHEDULE:          // 0
						_actListArr[i][j].a0.timer = in.readSint16BE();
						_actListArr[i][j].a0.actIndex = in.readUint16BE();
						break;
					case START_OBJ:          // 1
						_actListArr[i][j].a1.timer = in.readSint16BE();
						_actListArr[i][j].a1.objNumb = in.readSint16BE();
						_actListArr[i][j].a1.cycleNumb = in.readSint16BE();
						_actListArr[i][j].a1.cycle = (cycle_t) in.readByte();
						break;
					case INIT_OBJXY:         // 2
						_actListArr[i][j].a2.timer = in.readSint16BE();
						_actListArr[i][j].a2.objNumb = in.readSint16BE();
						_actListArr[i][j].a2.x = in.readSint16BE();
						_actListArr[i][j].a2.y = in.readSint16BE();
						break;
					case PROMPT:             // 3
						_actListArr[i][j].a3.timer = in.readSint16BE();
						_actListArr[i][j].a3.promptIndex = in.readSint16BE();
						numSubAct = in.readUint16BE();
						_actListArr[i][j].a3.responsePtr = (int *) malloc(sizeof(int) * numSubAct);
						for (int k = 0; k < numSubAct; k++)
							_actListArr[i][j].a3.responsePtr[k] = in.readSint16BE();
						_actListArr[i][j].a3.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a3.actFailIndex = in.readUint16BE();
						_actListArr[i][j].a3.encodedFl = (in.readByte() == 1) ? true : false;
						break;
					case BKGD_COLOR:         // 4
						_actListArr[i][j].a4.timer = in.readSint16BE();
						_actListArr[i][j].a4.newBackgroundColor = in.readUint32BE();
						break;
					case INIT_OBJVXY:        // 5
						_actListArr[i][j].a5.timer = in.readSint16BE();
						_actListArr[i][j].a5.objNumb = in.readSint16BE();
						_actListArr[i][j].a5.vx = in.readSint16BE();
						_actListArr[i][j].a5.vy = in.readSint16BE();
						break;
					case INIT_CARRY:         // 6
						_actListArr[i][j].a6.timer = in.readSint16BE();
						_actListArr[i][j].a6.objNumb = in.readSint16BE();
						_actListArr[i][j].a6.carriedFl = (in.readByte() == 1) ? true : false;
						break;
					case INIT_HF_COORD:      // 7
						_actListArr[i][j].a7.timer = in.readSint16BE();
						_actListArr[i][j].a7.objNumb = in.readSint16BE();
						break;
					case NEW_SCREEN:         // 8
						_actListArr[i][j].a8.timer = in.readSint16BE();
						_actListArr[i][j].a8.screenIndex = in.readSint16BE();
						break;
					case INIT_OBJSTATE:      // 9
						_actListArr[i][j].a9.timer = in.readSint16BE();
						_actListArr[i][j].a9.objNumb = in.readSint16BE();
						_actListArr[i][j].a9.newState = in.readByte();
						break;
					case INIT_PATH:          // 10
						_actListArr[i][j].a10.timer = in.readSint16BE();
						_actListArr[i][j].a10.objNumb = in.readSint16BE();
						_actListArr[i][j].a10.newPathType = in.readSint16BE();
						_actListArr[i][j].a10.vxPath = in.readByte();
						_actListArr[i][j].a10.vyPath = in.readByte();
						break;
					case COND_R:             // 11
						_actListArr[i][j].a11.timer = in.readSint16BE();
						_actListArr[i][j].a11.objNumb = in.readSint16BE();
						_actListArr[i][j].a11.stateReq = in.readByte();
						_actListArr[i][j].a11.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a11.actFailIndex = in.readUint16BE();
						break;
					case TEXT:               // 12
						_actListArr[i][j].a12.timer = in.readSint16BE();
						_actListArr[i][j].a12.stringIndex = in.readSint16BE();
						break;
					case SWAP_IMAGES:        // 13
						_actListArr[i][j].a13.timer = in.readSint16BE();
						_actListArr[i][j].a13.obj1 = in.readSint16BE();
						_actListArr[i][j].a13.obj2 = in.readSint16BE();
						break;
					case COND_SCR:           // 14
						_actListArr[i][j].a14.timer = in.readSint16BE();
						_actListArr[i][j].a14.objNumb = in.readSint16BE();
						_actListArr[i][j].a14.screenReq = in.readSint16BE();
						_actListArr[i][j].a14.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a14.actFailIndex = in.readUint16BE();
						break;
					case AUTOPILOT:          // 15
						_actListArr[i][j].a15.timer = in.readSint16BE();
						_actListArr[i][j].a15.obj1 = in.readSint16BE();
						_actListArr[i][j].a15.obj2 = in.readSint16BE();
						_actListArr[i][j].a15.dx = in.readByte();
						_actListArr[i][j].a15.dy = in.readByte();
						break;
					case INIT_OBJ_SEQ:       // 16
						_actListArr[i][j].a16.timer = in.readSint16BE();
						_actListArr[i][j].a16.objNumb = in.readSint16BE();
						_actListArr[i][j].a16.seqIndex = in.readSint16BE();
						break;
					case SET_STATE_BITS:     // 17
						_actListArr[i][j].a17.timer = in.readSint16BE();
						_actListArr[i][j].a17.objNumb = in.readSint16BE();
						_actListArr[i][j].a17.stateMask = in.readSint16BE();
						break;
					case CLEAR_STATE_BITS:   // 18
						_actListArr[i][j].a18.timer = in.readSint16BE();
						_actListArr[i][j].a18.objNumb = in.readSint16BE();
						_actListArr[i][j].a18.stateMask = in.readSint16BE();
						break;
					case TEST_STATE_BITS:    // 19
						_actListArr[i][j].a19.timer = in.readSint16BE();
						_actListArr[i][j].a19.objNumb = in.readSint16BE();
						_actListArr[i][j].a19.stateMask = in.readSint16BE();
						_actListArr[i][j].a19.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a19.actFailIndex = in.readUint16BE();
						break;
					case DEL_EVENTS:         // 20
						_actListArr[i][j].a20.timer = in.readSint16BE();
						_actListArr[i][j].a20.actTypeDel = (action_t) in.readByte();
						break;
					case GAMEOVER:           // 21
						_actListArr[i][j].a21.timer = in.readSint16BE();
						break;
					case INIT_HH_COORD:      // 22
						_actListArr[i][j].a22.timer = in.readSint16BE();
						_actListArr[i][j].a22.objNumb = in.readSint16BE();
						break;
					case EXIT:               // 23
						_actListArr[i][j].a23.timer = in.readSint16BE();
						break;
					case BONUS:              // 24
						_actListArr[i][j].a24.timer = in.readSint16BE();
						_actListArr[i][j].a24.pointIndex = in.readSint16BE();
						break;
					case COND_BOX:           // 25
						_actListArr[i][j].a25.timer = in.readSint16BE();
						_actListArr[i][j].a25.objNumb = in.readSint16BE();
						_actListArr[i][j].a25.x1 = in.readSint16BE();
						_actListArr[i][j].a25.y1 = in.readSint16BE();
						_actListArr[i][j].a25.x2 = in.readSint16BE();
						_actListArr[i][j].a25.y2 = in.readSint16BE();
						_actListArr[i][j].a25.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a25.actFailIndex = in.readUint16BE();
						break;
					case SOUND:              // 26
						_actListArr[i][j].a26.timer = in.readSint16BE();
						_actListArr[i][j].a26.soundIndex = in.readSint16BE();
						break;
					case ADD_SCORE:          // 27
						_actListArr[i][j].a27.timer = in.readSint16BE();
						_actListArr[i][j].a27.objNumb = in.readSint16BE();
						break;
					case SUB_SCORE:          // 28
						_actListArr[i][j].a28.timer = in.readSint16BE();
						_actListArr[i][j].a28.objNumb = in.readSint16BE();
						break;
					case COND_CARRY:         // 29
						_actListArr[i][j].a29.timer = in.readSint16BE();
						_actListArr[i][j].a29.objNumb = in.readSint16BE();
						_actListArr[i][j].a29.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a29.actFailIndex = in.readUint16BE();
						break;
					case INIT_MAZE:          // 30
						_actListArr[i][j].a30.timer = in.readSint16BE();
						_actListArr[i][j].a30.mazeSize = in.readByte();
						_actListArr[i][j].a30.x1 = in.readSint16BE();
						_actListArr[i][j].a30.y1 = in.readSint16BE();
						_actListArr[i][j].a30.x2 = in.readSint16BE();
						_actListArr[i][j].a30.y2 = in.readSint16BE();
						_actListArr[i][j].a30.x3 = in.readSint16BE();
						_actListArr[i][j].a30.x4 = in.readSint16BE();
						_actListArr[i][j].a30.firstScreenIndex = in.readByte();
						break;
					case EXIT_MAZE:          // 31
						_actListArr[i][j].a31.timer = in.readSint16BE();
						break;
					case INIT_PRIORITY:      // 32
						_actListArr[i][j].a32.timer = in.readSint16BE();
						_actListArr[i][j].a32.objNumb = in.readSint16BE();
						_actListArr[i][j].a32.priority = in.readByte();
						break;
					case INIT_SCREEN:        // 33
						_actListArr[i][j].a33.timer = in.readSint16BE();
						_actListArr[i][j].a33.objNumb = in.readSint16BE();
						_actListArr[i][j].a33.screenIndex = in.readSint16BE();
						break;
					case AGSCHEDULE:         // 34
						_actListArr[i][j].a34.timer = in.readSint16BE();
						_actListArr[i][j].a34.actIndex = in.readUint16BE();
						break;
					case REMAPPAL:           // 35
						_actListArr[i][j].a35.timer = in.readSint16BE();
						_actListArr[i][j].a35.oldColorIndex = in.readSint16BE();
						_actListArr[i][j].a35.newColorIndex = in.readSint16BE();
						break;
					case COND_NOUN:          // 36
						_actListArr[i][j].a36.timer = in.readSint16BE();
						_actListArr[i][j].a36.nounIndex = in.readUint16BE();
						_actListArr[i][j].a36.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a36.actFailIndex = in.readUint16BE();
						break;
					case SCREEN_STATE:       // 37
						_actListArr[i][j].a37.timer = in.readSint16BE();
						_actListArr[i][j].a37.screenIndex = in.readSint16BE();
						_actListArr[i][j].a37.newState = in.readByte();
						break;
					case INIT_LIPS:          // 38
						_actListArr[i][j].a38.timer = in.readSint16BE();
						_actListArr[i][j].a38.lipsObjNumb = in.readSint16BE();
						_actListArr[i][j].a38.objNumb = in.readSint16BE();
						_actListArr[i][j].a38.dxLips = in.readByte();
						_actListArr[i][j].a38.dyLips = in.readByte();
						break;
					case INIT_STORY_MODE:    // 39
						_actListArr[i][j].a39.timer = in.readSint16BE();
						_actListArr[i][j].a39.storyModeFl = (in.readByte() == 1);
						break;
					case WARN:               // 40
						_actListArr[i][j].a40.timer = in.readSint16BE();
						_actListArr[i][j].a40.stringIndex = in.readSint16BE();
						break;
					case COND_BONUS:         // 41
						_actListArr[i][j].a41.timer = in.readSint16BE();
						_actListArr[i][j].a41.BonusIndex = in.readSint16BE();
						_actListArr[i][j].a41.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a41.actFailIndex = in.readUint16BE();
						break;
					case TEXT_TAKE:          // 42
						_actListArr[i][j].a42.timer = in.readSint16BE();
						_actListArr[i][j].a42.objNumb = in.readSint16BE();
						break;
					case YESNO:              // 43
						_actListArr[i][j].a43.timer = in.readSint16BE();
						_actListArr[i][j].a43.promptIndex = in.readSint16BE();
						_actListArr[i][j].a43.actYesIndex = in.readUint16BE();
						_actListArr[i][j].a43.actNoIndex = in.readUint16BE();
						break;
					case STOP_ROUTE:         // 44
						_actListArr[i][j].a44.timer = in.readSint16BE();
						break;
					case COND_ROUTE:         // 45
						_actListArr[i][j].a45.timer = in.readSint16BE();
						_actListArr[i][j].a45.routeIndex = in.readSint16BE();
						_actListArr[i][j].a45.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a45.actFailIndex = in.readUint16BE();
						break;
					case INIT_JUMPEXIT:      // 46
						_actListArr[i][j].a46.timer = in.readSint16BE();
						_actListArr[i][j].a46.jumpExitFl = (in.readByte() == 1);
						break;
					case INIT_VIEW:          // 47
						_actListArr[i][j].a47.timer = in.readSint16BE();
						_actListArr[i][j].a47.objNumb = in.readSint16BE();
						_actListArr[i][j].a47.viewx = in.readSint16BE();
						_actListArr[i][j].a47.viewy = in.readSint16BE();
						_actListArr[i][j].a47.direction = in.readSint16BE();
						break;
					case INIT_OBJ_FRAME:     // 48
						_actListArr[i][j].a48.timer = in.readSint16BE();
						_actListArr[i][j].a48.objNumb = in.readSint16BE();
						_actListArr[i][j].a48.seqIndex = in.readSint16BE();
						_actListArr[i][j].a48.frameIndex = in.readSint16BE();
						break;
					case OLD_SONG:           //49
						_actListArr[i][j].a49.timer = in.readSint16BE();
						_actListArr[i][j].a49.soundIndex = in.readUint16BE();
						break;
					default:
						error("Engine - Unknown action type encountered: %d", _actListArr[i][j].a0.actType);
					}
				}
				_actListArr[i][numSubElem].a0.actType = ANULL;
			}
		} else {
			for (int i = 0; i < numElem; i++) {
				numSubElem = in.readUint16BE();
				for (int j = 0; j < numSubElem; j++) {
					numSubAct = in.readByte();
					switch (numSubAct) {
					case ANULL:              // -1
						break;
					case ASCHEDULE:          // 0
						in.readSint16BE();
						in.readUint16BE();
						break;
					case START_OBJ:          // 1
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						break;
					case INIT_OBJXY:         // 2
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case PROMPT:             // 3
						in.readSint16BE();
						in.readSint16BE();
						numSubAct = in.readUint16BE();
						for (int k = 0; k < numSubAct; k++)
							in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						in.readByte();
						break;
					case BKGD_COLOR:         // 4
						in.readSint16BE();
						in.readUint32BE();
						break;
					case INIT_OBJVXY:        // 5
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case INIT_CARRY:         // 6
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						break;
					case INIT_HF_COORD:      // 7
						in.readSint16BE();
						in.readSint16BE();
						break;
					case NEW_SCREEN:         // 8
						in.readSint16BE();
						in.readSint16BE();
						break;
					case INIT_OBJSTATE:      // 9
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						break;
					case INIT_PATH:          // 10
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						in.readByte();
						break;
					case COND_R:             // 11
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case TEXT:               // 12
						in.readSint16BE();
						in.readSint16BE();
						break;
					case SWAP_IMAGES:        // 13
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case COND_SCR:           // 14
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case AUTOPILOT:          // 15
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						in.readByte();
						break;
					case INIT_OBJ_SEQ:       // 16
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case SET_STATE_BITS:     // 17
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case CLEAR_STATE_BITS:   // 18
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case TEST_STATE_BITS:    // 19
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case DEL_EVENTS:         // 20
						in.readSint16BE();
						in.readByte();
						break;
					case GAMEOVER:           // 21
						in.readSint16BE();
						break;
					case INIT_HH_COORD:      // 22
						in.readSint16BE();
						in.readSint16BE();
						break;
					case EXIT:               // 23
						in.readSint16BE();
						break;
					case BONUS:              // 24
						in.readSint16BE();
						in.readSint16BE();
						break;
					case COND_BOX:           // 25
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case SOUND:              // 26
						in.readSint16BE();
						in.readSint16BE();
						break;
					case ADD_SCORE:          // 27
						in.readSint16BE();
						in.readSint16BE();
						break;
					case SUB_SCORE:          // 28
						in.readSint16BE();
						in.readSint16BE();
						break;
					case COND_CARRY:         // 29
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case INIT_MAZE:          // 30
						in.readSint16BE();
						in.readByte();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						break;
					case EXIT_MAZE:          // 31
						in.readSint16BE();
						break;
					case INIT_PRIORITY:      // 32
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						break;
					case INIT_SCREEN:        // 33
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case AGSCHEDULE:         // 34
						in.readSint16BE();
						in.readUint16BE();
						break;
					case REMAPPAL:           // 35
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case COND_NOUN:          // 36
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case SCREEN_STATE:       // 37
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						break;
					case INIT_LIPS:          // 38
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						in.readByte();
						break;
					case INIT_STORY_MODE:    // 39
						in.readSint16BE();
						in.readByte();
						break;
					case WARN:               // 40
						in.readSint16BE();
						in.readSint16BE();
						break;
					case COND_BONUS:         // 41
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case TEXT_TAKE:          // 42
						in.readSint16BE();
						in.readSint16BE();
						break;
					case YESNO:              // 43
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case STOP_ROUTE:         // 44
						in.readSint16BE();
						break;
					case COND_ROUTE:         // 45
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case INIT_JUMPEXIT:      // 46
						in.readSint16BE();
						in.readByte();
						break;
					case INIT_VIEW:          // 47
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case INIT_OBJ_FRAME:     // 48
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case OLD_SONG:           //49
						in.readSint16BE();
						in.readUint16BE();
						break;
					default:
						error("Engine - Unknown action type encountered %d - variante %d pos %d.%d", numSubAct, varnt, i, j);
					}
				}
			}
		}
	}
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		if (varnt == _gameVariant) {
			_tunesNbr     = in.readByte();
			_soundSilence = in.readByte();
			_soundTest    = in.readByte();
		} else {
			in.readByte();
			in.readByte();
			in.readByte();
		}
	}

	//Read _defltTunes
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant) {
			_maxInvent = numElem;
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

	//Read LASTOBJ
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant)
			_numObj = numElem;
	}

	//Read kALnewscr used by maze (Hugo 2)
	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _gameVariant)
			_alNewscrIndex = numElem;
	}

	if (_gameVariant > 2) {
		_arrayFontSize[0] = in.readUint16BE();
		_arrayFont[0] = (byte *)malloc(sizeof(byte) * _arrayFontSize[0]);
		for (int j = 0; j < _arrayFontSize[0]; j++)
			_arrayFont[0][j] = in.readByte();

		_arrayFontSize[1] = in.readUint16BE();
		_arrayFont[1] = (byte *)malloc(sizeof(byte) * _arrayFontSize[1]);
		for (int j = 0; j < _arrayFontSize[1]; j++)
			_arrayFont[1][j] = in.readByte();

		_arrayFontSize[2] = in.readUint16BE();
		_arrayFont[2] = (byte *)malloc(sizeof(byte) * _arrayFontSize[2]);
		for (int j = 0; j < _arrayFontSize[2]; j++)
			_arrayFont[2][j] = in.readByte();
	} else {
		numElem = in.readUint16BE();
		for (int j = 0; j < numElem; j++)
			in.readByte();

		numElem = in.readUint16BE();
		for (int j = 0; j < numElem; j++)
			in.readByte();

		numElem = in.readUint16BE();
		for (int j = 0; j < numElem; j++)
			in.readByte();
	}
	return true;
}

char **HugoEngine::loadTextsVariante(Common::File &in, uint16 *arraySize) {
	int  numTexts;
	int  entryLen;
	int  len;
	char **res = 0;
	char *pos = 0;

	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numTexts = in.readUint16BE();
		entryLen = in.readUint16BE();
		pos = (char *)malloc(entryLen);
		if (varnt == _gameVariant) {
			if (arraySize)
				*arraySize = numTexts;
			res = (char **)malloc(sizeof(char *) * numTexts);
			res[0] = pos;
			in.read(res[0], entryLen);
			res[0] += DATAALIGNMENT;
		} else {
			in.read(pos, entryLen);
		}

		pos += DATAALIGNMENT;

		for (int i = 1; i < numTexts; i++) {
			pos -= 2;

			len = READ_BE_UINT16(pos);
			pos += 2 + len;

			if (varnt == _gameVariant)
				res[i] = pos;
		}
	}

	return res;
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

char ***HugoEngine::loadTextsArray(Common::File &in) {
	char ***resArray = 0;

	for (int varnt = 0; varnt < _numVariant; varnt++) {
		int numNouns = in.readUint16BE();
		if (varnt == _gameVariant) {
			resArray = (char ** *)malloc(sizeof(char **) * (numNouns + 1));
			resArray[numNouns] = 0;
		}
		for (int i = 0; i < numNouns; i++) {
			int numTexts = in.readUint16BE();
			int entryLen = in.readUint16BE();
			char *pos = (char *)malloc(entryLen);
			char **res = 0;
			if (varnt == _gameVariant) {
				res = (char **)malloc(sizeof(char *) * numTexts);
				res[0] = pos;
				in.read(res[0], entryLen);
				res[0] += DATAALIGNMENT;
			} else {
				in.read(pos, entryLen);
			}

			pos += DATAALIGNMENT;

			for (int j = 0; j < numTexts; j++) {
				if (varnt == _gameVariant)
					res[j] = pos;

				pos -= 2;
				int len = READ_BE_UINT16(pos);
				pos += 2 + len;
			}

			if (varnt == _gameVariant)
				resArray[i] = res;
		}
	}

	return resArray;
}

char **HugoEngine::loadTexts(Common::File &in) {
	int numTexts = in.readUint16BE();
	char **res = (char **)malloc(sizeof(char *) * numTexts);
	int entryLen = in.readUint16BE();
	char *pos = (char *)malloc(entryLen);

	in.read(pos, entryLen);

	pos += DATAALIGNMENT;
	res[0] = pos;

	for (int i = 1; i < numTexts; i++) {
		pos -= 2;
		int len = READ_BE_UINT16(pos);
		pos += 2 + len;
		res[i] = pos;
	}

	return res;
}

void HugoEngine::freeTexts(char **ptr) {
	if (!ptr)
		return;

	free(*ptr);
	free(ptr);
}

// Sets the playlist to be the default tune selection
void HugoEngine::initPlaylist(bool playlist[MAX_TUNES]) {
	debugC(1, kDebugEngine, "initPlaylist");

	for (int16 i = 0; i < MAX_TUNES; i++)
		playlist[i] = false;
	for (int16 i = 0; _defltTunes[i] != -1; i++)
		playlist[_defltTunes[i]] = true;
}

// Initialize the dynamic game status
void HugoEngine::initStatus() {
	debugC(1, kDebugEngine, "initStatus");
	_status.initSaveFl    = true;                   // Force initial save
	_status.storyModeFl   = false;                  // Not in story mode
	_status.gameOverFl    = false;                  // Hero not knobbled yet
	_status.recordFl      = false;                  // Not record mode
	_status.playbackFl    = false;                  // Not playback mode
	_status.demoFl        = false;                  // Not demo mode
	_status.textBoxFl     = false;                  // Not processing a text box
//	Strangerke - Not used ?
//	_status.mmtime        = false;                   // Multimedia timer support
	_status.lookFl        = false;                  // Toolbar "look" button
	_status.recallFl      = false;                  // Toolbar "recall" button
	_status.leftButtonFl  = false;                  // Left mouse button pressed
	_status.rightButtonFl = false;                  // Right mouse button pressed
	_status.newScreenFl   = false;                  // Screen not just loaded
	_status.jumpExitFl    = false;                  // Can't jump to a screen exit
	_status.godModeFl     = false;                  // No special cheats allowed
	_status.helpFl        = false;                  // Not calling WinHelp()
	_status.doQuitFl      = false;
	_status.path[0]       = 0;                      // Path to write files
	_status.saveSlot      = 0;                      // Slot to save/restore game
	_status.screenWidth   = 0;                      // Desktop screen width

	// Initialize every start of new game
	_status.tick            = 0;                    // Tick count
	_status.saveTick        = 0;                    // Time of last save
	_status.viewState       = V_IDLE;               // View state
	_status.inventoryState  = I_OFF;                // Inventory icon bar state
	_status.inventoryHeight = 0;                    // Inventory icon bar pos
	_status.inventoryObjId  = -1;                   // Inventory object selected (none)
	_status.routeIndex      = -1;                   // Hero not following a route
	_status.go_for          = GO_SPACE;             // Hero walking to space
	_status.go_id           = -1;                   // Hero not walking to anything
}

// Initialize default config values.  Must be done before Initialize().
// Reset needed to save config.cx,cy which get splatted during OnFileNew()
void HugoEngine::initConfig(inst_t action) {
	debugC(1, kDebugEngine, "initConfig(%d)", action);

	switch (action) {
	case INSTALL:
		_config.musicFl = true;                     // Music state initially on
		_config.soundFl = true;                     // Sound state initially on
		_config.turboFl = false;                    // Turbo state initially off
		_config.backgroundMusicFl = false;          // No music when inactive
		_config.musicVolume = 85;                   // Music volume %
		_config.soundVolume = 100;                  // Sound volume %
		initPlaylist(_config.playlist);             // Initialize default tune playlist

		file().readBootFile();    // Read startup structure
		break;
	case RESET:
		// Find first tune and play it
		for (int16 i = 0; i < MAX_TUNES; i++) {
			if (_config.playlist[i]) {
				sound().playMusic(i);
				break;
			}
		}

		file().initSavedGame();   // Initialize saved game
		break;
	case RESTORE:
		warning("Unhandled action RESTORE");
		break;
	}
}

void HugoEngine::initialize() {
	debugC(1, kDebugEngine, "initialize");

	_maze.enabledFl = false;
	_line[0] = '\0';

	sound().initSound();
	scheduler().initEventQueue();                   // Init scheduler stuff
	screen().initDisplay();                         // Create Dibs and palette
	file().openDatabaseFiles();                     // Open database files
	calcMaxScore();                                 // Initialise maxscore

	_rnd = new Common::RandomSource();
	g_eventRec.registerRandomSource(*_rnd, "hugo");

	_rnd->setSeed(42);                              // Kick random number generator

	switch (getGameType()) {
	case kGameTypeHugo1:
		_episode = "\"HUGO'S HOUSE OF HORRORS\"";
		_picDir = "";
		break;
	case kGameTypeHugo2:
		_episode = "\"Hugo's Mystery Adventure\"";
		_picDir = "hugo2/";
		break;
	case kGameTypeHugo3:
		_episode = "\"Hugo's Amazon Adventure\"";
		_picDir = "hugo3/";
		break;
	default:
		error("Unknown game");
	}
}

// Restore all resources before termination
void HugoEngine::shutdown() {
	debugC(1, kDebugEngine, "shutdown");

	file().closeDatabaseFiles();
	if (_status.recordFl || _status.playbackFl)
		file().closePlaybackFile();
	freeObjects();
}

void HugoEngine::readObjectImages() {
	debugC(1, kDebugEngine, "readObjectImages");

	for (int i = 0; i < _numObj; i++)
		file().readImage(i, &_objects[i]);
}

// Read the uif image file (inventory icons)
void HugoEngine::readUIFImages() {
	debugC(1, kDebugEngine, "readUIFImages");

	file().readUIFItem(UIF_IMAGES, screen().getGUIBuffer());   // Read all uif images
}

// Read scenery, overlay files for given screen number
void HugoEngine::readScreenFiles(int screenNum) {
	debugC(1, kDebugEngine, "readScreenFiles(%d)", screenNum);

	file().readBackground(screenNum);               // Scenery file
	memcpy(screen().getBackBuffer(), screen().getFrontBuffer(), sizeof(screen().getFrontBuffer()));// Make a copy
	file().readOverlay(screenNum, _boundary, BOUNDARY); // Boundary file
	file().readOverlay(screenNum, _overlay, OVERLAY);   // Overlay file
	file().readOverlay(screenNum, _ovlBase, OVLBASE);   // Overlay base file
}

// Update all object positions.  Process object 'local' events
// including boundary events and collisions
void HugoEngine::moveObjects() {
	debugC(4, kDebugEngine, "moveObjects");

	// If route mode enabled, do special route processing
	if (_status.routeIndex >= 0)
		route().processRoute();

	// Perform any adjustments to velocity based on special path types
	// and store all (visible) object baselines into the boundary file.
	// Don't store foreground or background objects
	for (int i = 0; i < _numObj; i++) {
		object_t *obj = &_objects[i];               // Get pointer to object
		seq_t *currImage = obj->currImagePtr;       // Get ptr to current image
		if (obj->screenIndex == *_screen_p) {
			switch (obj->pathType) {
			case CHASE:
			case CHASE2: {
				int8 radius = obj->radius;          // Default to object's radius
				if (radius < 0)                     // If radius infinity, use closer value
					radius = DX;

				// Allowable motion wrt boundary
				int dx = _hero->x + _hero->currImagePtr->x1 - obj->x - currImage->x1;
				int dy = _hero->y + _hero->currImagePtr->y2 - obj->y - currImage->y2 - 1;
				if (abs(dx) <= radius)
					obj->vx = 0;
				else
					obj->vx = (dx > 0) ? MIN(dx, obj->vxPath) : MAX(dx, -obj->vxPath);
				if (abs(dy) <= radius)
					obj->vy = 0;
				else
					obj->vy = (dy > 0) ? MIN(dy, obj->vyPath) : MAX(dy, -obj->vyPath);

				// Set first image in sequence (if multi-seq object)
				switch (obj->seqNumb) {
				case 4:
					if (!obj->vx) {                 // Got 4 directions
						if (obj->vx != obj->oldvx)  { // vx just stopped
							if (dy >= 0)
								obj->currImagePtr = obj->seqList[DOWN].seqPtr;
							else
								obj->currImagePtr = obj->seqList[_UP].seqPtr;
						}
					} else if (obj->vx != obj->oldvx) {
						if (dx > 0)
							obj->currImagePtr = obj->seqList[RIGHT].seqPtr;
						else
							obj->currImagePtr = obj->seqList[LEFT].seqPtr;
					}
					break;
				case 3:
				case 2:
					if (obj->vx != obj->oldvx) {    // vx just stopped
						if (dx > 0)                 // Left & right only
							obj->currImagePtr = obj->seqList[RIGHT].seqPtr;
						else
							obj->currImagePtr = obj->seqList[LEFT].seqPtr;
					}
					break;
				}

				if (obj->vx || obj->vy)
					obj->cycling = CYCLE_FORWARD;
				else {
					obj->cycling = NOT_CYCLING;
					boundaryCollision(obj);         // Must have got hero!
				}
				obj->oldvx = obj->vx;
				obj->oldvy = obj->vy;
				currImage = obj->currImagePtr;      // Get (new) ptr to current image
				break;
				}
			case WANDER2:
			case WANDER:
				if (!_rnd->getRandomNumber(3 * NORMAL_TPS)) {       // Kick on random interval
					obj->vx = _rnd->getRandomNumber(obj->vxPath << 1) - obj->vxPath;
					obj->vy = _rnd->getRandomNumber(obj->vyPath << 1) - obj->vyPath;

					// Set first image in sequence (if multi-seq object)
					if (obj->seqNumb > 1) {
						if (!obj->vx && (obj->seqNumb >= 4)) {
							if (obj->vx != obj->oldvx)  { // vx just stopped
								if (obj->vy > 0)
									obj->currImagePtr = obj->seqList[DOWN].seqPtr;
								else
									obj->currImagePtr = obj->seqList[_UP].seqPtr;
							}
						} else if (obj->vx != obj->oldvx) {
							if (obj->vx > 0)
								obj->currImagePtr = obj->seqList[RIGHT].seqPtr;
							else
								obj->currImagePtr = obj->seqList[LEFT].seqPtr;
						}
					}
					obj->oldvx = obj->vx;
					obj->oldvy = obj->vy;
					currImage = obj->currImagePtr;  // Get (new) ptr to current image
				}
				if (obj->vx || obj->vy)
					obj->cycling = CYCLE_FORWARD;
				break;
			default:
				; // Really, nothing
			}
			// Store boundaries
			if ((obj->cycling > ALMOST_INVISIBLE) && (obj->priority == FLOATING))
				storeBoundary(obj->x + currImage->x1, obj->x + currImage->x2, obj->y + currImage->y2);
		}
	}

	// Move objects, allowing for boundaries
	for (int i = 0; i < _numObj; i++) {
		object_t *obj = &_objects[i];                         // Get pointer to object
		if ((obj->screenIndex == *_screen_p) && (obj->vx || obj->vy)) {
			// Only process if it's moving

			// Do object movement.  Delta_x,y return allowed movement in x,y
			// to move as close to a boundary as possible without crossing it.
			seq_t *currImage = obj->currImagePtr;   // Get ptr to current image
			// object coordinates
			int x1 = obj->x + currImage->x1;        // Left edge of object
			int x2 = obj->x + currImage->x2;        // Right edge
			int y1 = obj->y + currImage->y1;        // Top edge
			int y2 = obj->y + currImage->y2;        // Bottom edge

			if ((obj->cycling > ALMOST_INVISIBLE) && (obj->priority == FLOATING))
				clearBoundary(x1, x2, y2);          // Clear our own boundary

			// Allowable motion wrt boundary
			int dx = deltaX(x1, x2, obj->vx, y2);
			if (dx != obj->vx) {
				// An object boundary collision!
				boundaryCollision(obj);
				obj->vx = 0;
			}

			int dy = deltaY(x1, x2, obj->vy, y2);
			if (dy != obj->vy) {
				// An object boundary collision!
				boundaryCollision(obj);
				obj->vy = 0;
			}

			if ((obj->cycling > ALMOST_INVISIBLE) && (obj->priority == FLOATING))
				storeBoundary(x1, x2, y2);          // Re-store our own boundary

			obj->x += dx;                           // Update object position
			obj->y += dy;

			// Don't let object go outside screen
			if (x1 < EDGE)
				obj->x = EDGE2;
			if (x2 > (XPIX - EDGE))
				obj->x = XPIX - EDGE2 - (x2 - x1);
			if (y1 < EDGE)
				obj->y = EDGE2;
			if (y2 > (YPIX - EDGE))
				obj->y = YPIX - EDGE2 - (y2 - y1);

			if ((obj->vx == 0) && (obj->vy == 0) && (obj->pathType != WANDER2) && (obj->pathType != CHASE2))
				obj->cycling = NOT_CYCLING;
		}
	}

	// Clear all object baselines from the boundary file.
	for (int i = 0; i < _numObj; i++) {
		object_t *obj = &_objects[i];               // Get pointer to object
		seq_t *currImage = obj->currImagePtr;       // Get ptr to current image
		if ((obj->screenIndex == *_screen_p) && (obj->cycling > ALMOST_INVISIBLE) && (obj->priority == FLOATING))
			clearBoundary(obj->oldx + currImage->x1, obj->oldx + currImage->x2, obj->oldy + currImage->y2);
	}

	// If maze mode is enabled, do special maze processing
	if (_maze.enabledFl)
		processMaze();
}

// Return maximum allowed movement (from zero to vx) such that object does
// not cross a boundary (either background or another object)
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

	y *= XBYTES;                                    // Offset into boundary file
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

// Similar to Delta_x, but for movement in y direction.  Special case of
// bytes at end of line segment; must only count boundary bits falling on
// line segment.
int HugoEngine::deltaY(int x1, int x2, int vy, int y) {
	debugC(3, kDebugEngine, "deltaY(%d, %d, %d, %d)", x1, x2, vy, y);

	if (vy == 0)
		return 0;                                   // Object stationary

	int inc = (vy > 0) ? 1 : -1;
	for (int j = y + inc; j != (y + vy + inc); j += inc) { //Search by byte
		for (int i = x1 >> 3; i <= x2 >> 3; i++) {
			int b = _boundary[j * XBYTES + i] | _objBound[j * XBYTES + i];
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

// Store a horizontal line segment in the object boundary file
void HugoEngine::storeBoundary(int x1, int x2, int y) {
	debugC(5, kDebugEngine, "storeBoundary(%d, %d, %d)", x1, x2, y);

	for (int i = x1 >> 3; i <= x2 >> 3; i++) {      // For each byte in line
		byte *b = &_objBound[y * XBYTES + i];       // get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b |= 0xff << ((i << 3) + 7 - x2);
		else if (i == x1 >> 3)                      // Adjust left end
			*b |= 0xff >> (x1 - (i << 3));
		else
			*b = 0xff;
	}
}

// Clear a horizontal line segment in the object boundary file
void HugoEngine::clearBoundary(int x1, int x2, int y) {
	debugC(5, kDebugEngine, "clearBoundary(%d, %d, %d)", x1, x2, y);

	for (int i = x1 >> 3; i <= x2 >> 3; i++) {      // For each byte in line
		byte *b = &_objBound[y * XBYTES + i];       // get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b &= ~(0xff << ((i << 3) + 7 - x2));
		else if (i == x1 >> 3)                      // Adjust left end
			*b &= ~(0xff >> (x1 - (i << 3)));
		else
			*b = 0;
	}
}

// Maze mode is enabled.  Check to see whether hero has crossed the maze
// bounding box, if so, go to the next room */
void HugoEngine::processMaze() {
	debugC(1, kDebugEngine, "processMaze");

	seq_t *currImage = _hero->currImagePtr;         // Get ptr to current image

	// hero coordinates
	int x1 = _hero->x + currImage->x1;              // Left edge of object
	int x2 = _hero->x + currImage->x2;              // Right edge
	int y1 = _hero->y + currImage->y1;              // Top edge
	int y2 = _hero->y + currImage->y2;              // Bottom edge

	if (x1 < _maze.x1) {
		// Exit west
		_actListArr[_alNewscrIndex][3].a8.screenIndex = *_screen_p - 1;
		_actListArr[_alNewscrIndex][0].a2.x = _maze.x2 - SHIFT - (x2 - x1);
		_actListArr[_alNewscrIndex][0].a2.y = _hero->y;
		_status.routeIndex = -1;
		scheduler().insertActionList(_alNewscrIndex);
	} else if (x2 > _maze.x2) {
		// Exit east
		_actListArr[_alNewscrIndex][3].a8.screenIndex = *_screen_p + 1;
		_actListArr[_alNewscrIndex][0].a2.x = _maze.x1 + SHIFT;
		_actListArr[_alNewscrIndex][0].a2.y = _hero->y;
		_status.routeIndex = -1;
		scheduler().insertActionList(_alNewscrIndex);
	} else if (y1 < _maze.y1 - SHIFT) {
		// Exit north
		_actListArr[_alNewscrIndex][3].a8.screenIndex = *_screen_p - _maze.size;
		_actListArr[_alNewscrIndex][0].a2.x = _maze.x3;
		_actListArr[_alNewscrIndex][0].a2.y = _maze.y2 - SHIFT - (y2 - y1);
		_status.routeIndex = -1;
		scheduler().insertActionList(_alNewscrIndex);
	} else if (y2 > _maze.y2 - SHIFT / 2) {
		// Exit south
		_actListArr[_alNewscrIndex][3].a8.screenIndex = *_screen_p + _maze.size;
		_actListArr[_alNewscrIndex][0].a2.x = _maze.x4;
		_actListArr[_alNewscrIndex][0].a2.y = _maze.y1 + SHIFT;
		_status.routeIndex = -1;
		scheduler().insertActionList(_alNewscrIndex);
	}
}

// Compare function for the quicksort.  The sort is to order the objects in
// increasing vertical position, using y+y2 as the baseline
// Returns -1 if ay2 < by2 else 1 if ay2 > by2 else 0
int HugoEngine::y2comp(const void *a, const void *b) {
	debugC(6, kDebugEngine, "y2comp");

	const object_t *p1 = &s_Engine->_objects[*(const byte *)a];
	const object_t *p2 = &s_Engine->_objects[*(const byte *)b];

	if (p1 == p2)
		// Why does qsort try the same indexes?
		return 0;

	if (p1->priority == BACKGROUND)
		return -1;

	if (p2->priority == BACKGROUND)
		return 1;

	if (p1->priority == FOREGROUND)
		return 1;

	if (p2->priority == FOREGROUND)
		return -1;

	int ay2 = p1->y + p1->currImagePtr->y2;
	int by2 = p2->y + p2->currImagePtr->y2;

	return ay2 - by2;
}

// Draw all objects on screen as follows:
// 1. Sort 'FLOATING' objects in order of y2 (base of object)
// 2. Display new object frames/positions in dib
// Finally, cycle any animating objects to next frame
void HugoEngine::updateImages() {
	debugC(5, kDebugEngine, "updateImages");

	// Initialise the index array to visible objects in current screen
	int  num_objs = 0;
	byte objindex[MAX_OBJECTS];                // Array of indeces to objects

	for (int i = 0; i < _numObj; i++) {
		object_t *obj = &_objects[i];
		if ((obj->screenIndex == *_screen_p) && (obj->cycling >= ALMOST_INVISIBLE))
			objindex[num_objs++] = i;
	}

	// Sort the objects into increasing y+y2 (painter's algorithm)
	qsort(objindex, num_objs, sizeof(objindex[0]), y2comp);

	// Add each visible object to display list
	for (int i = 0; i < num_objs; i++) {
		object_t *obj = &_objects[objindex[i]];
		// Count down inter-frame timer
		if (obj->frameTimer)
			obj->frameTimer--;

		if (obj->cycling > ALMOST_INVISIBLE) {      // Only if visible
			switch (obj->cycling) {
			case NOT_CYCLING:
				screen().displayFrame(obj->x, obj->y, obj->currImagePtr, obj->priority == OVEROVL);
				break;
			case CYCLE_FORWARD:
				if (obj->frameTimer)                // Not time to see next frame yet
					screen().displayFrame(obj->x, obj->y, obj->currImagePtr, obj->priority == OVEROVL);
				else
					screen().displayFrame(obj->x, obj->y, obj->currImagePtr->nextSeqPtr, obj->priority == OVEROVL);
				break;
			case CYCLE_BACKWARD: {
				seq_t *seqPtr = obj->currImagePtr;
				if (!obj->frameTimer) {             // Show next frame
					while (seqPtr->nextSeqPtr != obj->currImagePtr)
						seqPtr = seqPtr->nextSeqPtr;
				}
				screen().displayFrame(obj->x, obj->y, seqPtr, obj->priority == OVEROVL);
				break;
				}
			default:
				break;
			}
		}
	}

	// Cycle any animating objects
	for (int i = 0; i < num_objs; i++) {
		object_t *obj = &_objects[objindex[i]];
		if (obj->cycling != INVISIBLE) {
			// Only if it's visible
			if (obj->cycling == ALMOST_INVISIBLE)
				obj->cycling = INVISIBLE;

			// Now Rotate to next picture in sequence
			switch (obj->cycling) {
			case NOT_CYCLING:
				break;
			case CYCLE_FORWARD:
				if (!obj->frameTimer) {
					// Time to step to next frame
					obj->currImagePtr = obj->currImagePtr->nextSeqPtr;
					// Find out if this is last frame of sequence
					// If so, reset frame_timer and decrement n_cycle
					if (obj->frameInterval || obj->cycleNumb) {
						obj->frameTimer = obj->frameInterval;
						for (int j = 0; j < obj->seqNumb; j++) {
							if (obj->currImagePtr->nextSeqPtr == obj->seqList[j].seqPtr) {
								if (obj->cycleNumb) { // Decr cycleNumb if Non-continous
									if (!--obj->cycleNumb)
										obj->cycling = NOT_CYCLING;
								}
							}
						}
					}
				}
				break;
			case CYCLE_BACKWARD: {
				if (!obj->frameTimer) {
					// Time to step to prev frame
					seq_t *seqPtr = obj->currImagePtr;
					while (obj->currImagePtr->nextSeqPtr != seqPtr)
						obj->currImagePtr = obj->currImagePtr->nextSeqPtr;
					// Find out if this is first frame of sequence
					// If so, reset frame_timer and decrement n_cycle
					if (obj->frameInterval || obj->cycleNumb) {
						obj->frameTimer = obj->frameInterval;
						for (int j = 0; j < obj->seqNumb; j++) {
							if (obj->currImagePtr == obj->seqList[j].seqPtr) {
								if (obj->cycleNumb){ // Decr cycleNumb if Non-continous
									if (!--obj->cycleNumb)
										obj->cycling = NOT_CYCLING;
								}
							}
						}
					}
				}
				break;
				}
			default:
				break;
			}
			obj->oldx = obj->x;
			obj->oldy = obj->y;
		}
	}
}

// Return object index of the topmost object under the cursor, or -1 if none
// Objects are filtered if not "useful"
int16 HugoEngine::findObject(uint16 x, uint16 y) {
	debugC(3, kDebugEngine, "findObject(%d, %d)", x, y);

	int16     objIndex = -1;                        // Index of found object
	uint16    y2Max = 0;                            // Greatest y2
	object_t *obj = _objects;
	// Check objects on screen
	for (int i = 0; i < _numObj; i++, obj++) {
		// Object must be in current screen and "useful"
		if (obj->screenIndex == *_screen_p && (obj->genericCmd || obj->objValue || obj->cmdIndex)) {
			seq_t *curImage = obj->currImagePtr;
			// Object must have a visible image...
			if (curImage != 0 && obj->cycling != INVISIBLE) {
				// If cursor inside object
				if (x >= (uint16)obj->x && x <= obj->x + curImage->x2 && y >= (uint16)obj->y && y <= obj->y + curImage->y2) {
					// If object is closest so far
					if (obj->y + curImage->y2 > y2Max) {
						y2Max = obj->y + curImage->y2;
						objIndex = i;               // Found an object!
					}
				}
			} else {
				// ...or a dummy object that has a hotspot rectangle
				if (curImage == 0 && obj->vxPath != 0 && !obj->carriedFl) {
					// If cursor inside special rectangle
					if ((int16)x >= obj->oldx && (int16)x < obj->oldx + obj->vxPath && (int16)y >= obj->oldy && (int16)y < obj->oldy + obj->vyPath) {
						// If object is closest so far
						if (obj->oldy + obj->vyPath - 1 > (int16)y2Max) {
							y2Max = obj->oldy + obj->vyPath - 1;
							objIndex = i;           // Found an object!
						}
					}
				}
			}
		}
	}
	return objIndex;
}

// Find a clear space around supplied object that hero can walk to
bool HugoEngine::findObjectSpace(object_t *obj, int16 *destx, int16 *desty) {
	debugC(1, kDebugEngine, "findObjectSpace(obj, %d, %d)", *destx, *desty);

	seq_t *curImage = obj->currImagePtr;
	int16 y = obj->y + curImage->y2 - 1;

	bool foundFl = true;
	// Try left rear corner
	for (int16 x = *destx = obj->x + curImage->x1; x < *destx + HERO_MAX_WIDTH; x++) {
		if (BOUND(x, y))
			foundFl = false;
	}

	if (!foundFl) {                                 // Try right rear corner
		foundFl = true;
		for (int16 x = *destx = obj->x + curImage->x2 - HERO_MAX_WIDTH + 1; x <= obj->x + (int16)curImage->x2; x++) {
			if (BOUND(x, y))
				foundFl = false;
		}
	}

	if (!foundFl) {                                 // Try left front corner
		foundFl = true;
		y += 2;
		for (int16 x = *destx = obj->x + curImage->x1; x < *destx + HERO_MAX_WIDTH; x++) {
			if (BOUND(x, y))
				foundFl = false;
		}
	}

	if (!foundFl) {                                 // Try right rear corner
		foundFl = true;
		for (int16 x = *destx = obj->x + curImage->x2 - HERO_MAX_WIDTH + 1; x <= obj->x + (int16)curImage->x2; x++) {
			if (BOUND(x, y))
				foundFl = false;
		}
	}

	*desty = y;
	return foundFl;
}

// Search background command list for this screen for supplied object.
// Return first associated verb (not "look") or 0 if none found.
char *HugoEngine::useBG(char *name) {
	debugC(1, kDebugEngine, "useBG(%s)", name);

	objectList_t p = _backgroundObjects[*_screen_p];
	for (int i = 0; *_arrayVerbs[p[i].verbIndex]; i++) {
		if ((name == _arrayNouns[p[i].nounIndex][0] &&
		     p[i].verbIndex != _look) &&
		    ((p[i].roomState == DONT_CARE) || (p[i].roomState == _screenStates[*_screen_p])))
			return _arrayVerbs[p[i].verbIndex][0];
	}

	return 0;
}

// If status.objid = -1, pick up objid, else use status.objid on objid,
// if objid can't be picked up, use it directly
void HugoEngine::useObject(int16 objId) {
	debugC(1, kDebugEngine, "useObject(%d)", objId);

	char *verb;                                     // Background verb to use directly
	object_t *obj = &_objects[objId];               // Ptr to object
	if (_status.inventoryObjId == -1) {
		// Get or use objid directly
		if ((obj->genericCmd & TAKE) || obj->objValue)  // Get collectible item
			sprintf(_line, "%s %s", _arrayVerbs[_take][0], _arrayNouns[obj->nounIndex][0]);
		else if (obj->genericCmd & LOOK)            // Look item
			sprintf(_line, "%s %s", _arrayVerbs[_look][0], _arrayNouns[obj->nounIndex][0]);
		else if (obj->genericCmd & DROP)            // Drop item
			sprintf(_line, "%s %s", _arrayVerbs[_drop][0], _arrayNouns[obj->nounIndex][0]);
		else if (obj->cmdIndex != 0)                // Use non-collectible item if able
			sprintf(_line, "%s %s", _arrayVerbs[_cmdList[obj->cmdIndex][1].verbIndex][0], _arrayNouns[obj->nounIndex][0]);
		else if ((verb = useBG(_arrayNouns[obj->nounIndex][0])) != 0)
			sprintf(_line, "%s %s", verb, _arrayNouns[obj->nounIndex][0]);
		else
			return;                                 // Can't use object directly
	} else {
		// Use status.objid on objid
		// Default to first cmd verb
		sprintf(_line, "%s %s %s", _arrayVerbs[_cmdList[_objects[_status.inventoryObjId].cmdIndex][1].verbIndex][0], _arrayNouns[_objects[_status.inventoryObjId].nounIndex][0], _arrayNouns[obj->nounIndex][0]);

		// Check valid use of objects and override verb if necessary
		for (uses_t *use = _uses; use->objId != _numObj; use++) {
			if (_status.inventoryObjId == use->objId) {
				// Look for secondary object, if found use matching verb
				bool foundFl = false;
				for (target_t *target = use->targets; _arrayNouns[target->nounIndex] != 0; target++)
					if (_arrayNouns[target->nounIndex][0] == _arrayNouns[obj->nounIndex][0]) {
						foundFl = true;
						sprintf(_line, "%s %s %s", _arrayVerbs[target->verbIndex][0], _arrayNouns[_objects[_status.inventoryObjId].nounIndex][0], _arrayNouns[obj->nounIndex][0]);
					}

				// No valid use of objects found, print failure string
				if (!foundFl) {
					// Deselect dragged icon if inventory not active
					if (_status.inventoryState != I_ACTIVE)
						_status.inventoryObjId  = -1;
					Utils::Box(BOX_ANY, "%s", _textData[use->dataIndex]);
					return;
				}
			}
		}
	}

	if (_status.inventoryState == I_ACTIVE)         // If inventory active, remove it
		_status.inventoryState = I_UP;
	_status.inventoryObjId  = -1;                   // Deselect any dragged icon
	parser().lineHandler();                         // and process command
}

// Issue "Look at <object>" command
// Note special case of swapped hero image
void HugoEngine::lookObject(object_t *obj) {
	debugC(1, kDebugEngine, "lookObject");

	if (obj == _hero)
		// Hero swapped - look at other
		obj = &_objects[_heroImage];

	parser().command("%s %s", _arrayVerbs[_look][0], _arrayNouns[obj->nounIndex][0]);
}

// Free all object images
void HugoEngine::freeObjects() {
	debugC(1, kDebugEngine, "freeObjects");

	// Nothing to do if not allocated yet
	if (_hero->seqList[0].seqPtr == 0)
		return;

	// Free all sequence lists and image data
	for (int i = 0; i < _numObj; i++) {
		object_t *obj = &_objects[i];
		for (int j = 0; j < obj->seqNumb; j++) {    // for each sequence
			seq_t *seq = obj->seqList[j].seqPtr;    // Free image
			if (seq == 0)                           // Failure during database load
				break;
			do {
				free(seq->imagePtr);
				seq = seq->nextSeqPtr;
			} while (seq != obj->seqList[j].seqPtr);
			free(seq);                              // Free sequence record
		}
	}
}

// Add action lists for this screen to event queue
void HugoEngine::screenActions(int screenNum) {
	debugC(1, kDebugEngine, "screenActions(%d)", screenNum);

	uint16 *screenAct = _screenActs[screenNum];
	if (screenAct) {
		for (int i = 0; screenAct[i]; i++)
			scheduler().insertActionList(screenAct[i]);
	}
}

// Set the new screen number into the hero object and any carried objects
void HugoEngine::setNewScreen(int screenNum) {
	debugC(1, kDebugEngine, "setNewScreen(%d)", screenNum);

	*_screen_p = screenNum;                             // HERO object
	for (int i = HERO + 1; i < _numObj; i++) {          // Any others
		if (_objects[i].carriedFl)                      // being carried
			_objects[i].screenIndex = screenNum;
	}
}

// An object has collided with a boundary.  See if any actions are required
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
					scheduler().insertActionList(hotspot->actIndex);
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
			radius = DX * 2;
		if ((abs(dx) <= radius) && (abs(dy) <= radius))
			scheduler().insertActionList(obj->actIndex);
	}
}

// Add up all the object values and all the bonus points
void HugoEngine::calcMaxScore() {
	debugC(1, kDebugEngine, "calcMaxScore");

	for (int i = 0; i < _numObj; i++)
		_maxscore += _objects[i].objValue;

	for (int i = 0; i < _numBonuses; i++)
		_maxscore += _points[i].score;
}

// Exit game, advertise trilogy, show copyright
void HugoEngine::endGame() {
	debugC(1, kDebugEngine, "endGame");

	if (!_boot.registered)
		Utils::Box(BOX_ANY, "%s", _textEngine[kEsAdvertise]);
	Utils::Box(BOX_ANY, "%s\n%s", _episode, COPYRIGHT);
	_status.viewState = V_EXIT;
}

} // End of namespace Hugo
