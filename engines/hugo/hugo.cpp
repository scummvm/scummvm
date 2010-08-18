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
#include "common/events.h"
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
#include "hugo/sound.h"
#include "hugo/intro.h"

#include "engines/util.h"

namespace Hugo {

HugoEngine *HugoEngine::s_Engine = NULL;

overlay_t HugoEngine::_boundary;
overlay_t HugoEngine::_overlay;
overlay_t HugoEngine::_ovlBase;
overlay_t HugoEngine::_objBound;

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

	_fileManager = new FileManager(*this);
	_scheduler = new Scheduler(*this);
	_screen = new Screen(*this);
	_mouseHandler = new MouseHandler(*this);
	_inventoryHandler = new InventoryHandler(*this);
	_parser = new Parser(*this);
	_route = new Route(*this);
	_soundHandler = new SoundHandler(*this);

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

	bool doQuitFl = false;

	while (!doQuitFl) {
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
				doQuitFl = true;
				break;
			default:
				break;
			}
		}
	}
	return Common::kNoError;
}

void HugoEngine::initMachine() {
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
		gameStatus.viewState = V_INTRO;
		break;
	case V_INTRO:                                   // Do any game-dependant preamble
		if (intro().introPlay())    {               // Process intro screen
			scheduler().newScreen(0);               // Initialize first screen
			gameStatus.viewState = V_PLAY;
		}
		break;
	case V_PLAY:                                    // Playing game
		parser().charHandler();                     // Process user cmd input
		moveObjects();                              // Process object movement
		scheduler().runScheduler();                 // Process any actions
		screen().displayList(D_RESTORE);            // Restore previous background
		updateImages();                             // Draw into _frontBuffer, compile display list
		mouse().mouseHandler();                     // Mouse activity - adds to display list
		parser().drawStatusText();
		screen().displayList(D_DISPLAY);            // Blit the display list to screen
		break;
	case V_INVENT:                                  // Accessing inventory
		inventory().runInventory();                 // Process Inventory state machine
		break;
	case V_EXIT:                                    // Game over or user exited
		gameStatus.viewState = V_IDLE;
		break;
	}
}

bool HugoEngine::loadHugoDat() {
	Common::File in;
	int numElem, numSubElem, numSubAct;
	char buf[256];
	int majVer, minVer;

	in.open("hugo.dat");

	if (!in.isOpen()) {
		Common::String errorMessage = "You're missing the 'hugo.dat' file. Get it from the ScummVM website";
		GUIErrorMessage(errorMessage);
		warning("%s", errorMessage.c_str());
		return false;
	}

	// Read header
	in.read(buf, 4);
	buf[4] = '\0';

	if (strcmp(buf, "HUGO")) {
		Common::String errorMessage = "File 'hugo.dat' is corrupt. Get it from the ScummVM website";
		GUIErrorMessage(errorMessage);
		warning("%s", errorMessage.c_str());
		return false;
	}

	majVer = in.readByte();
	minVer = in.readByte();

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
	for (int i = 0; i < _paletteSize; i++) {
		_palette[i] = in.readByte();
	}

	// Read textEngine
	_textEngine = loadTexts(in);

	// Read textIntro
	_textIntro = loadTexts(in);

	// Read x_intro and y_intro
	_introXSize = in.readUint16BE();
	_introX = (byte *)malloc(sizeof(byte) * _introXSize);
	_introY = (byte *)malloc(sizeof(byte) * _introXSize);
	for (int i = 0; i < _introXSize; i++) {
		_introX[i] = in.readByte();
		_introY[i] = in.readByte();
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
					in.readUint16BE();;
					in.readUint16BE();;
					in.readSint16BE();;
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
				if (numSubElem == 0)
					_screenActs[i] = 0;
				else {
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
	_hero = &_objects[HERO];                    // This always points to hero
	_screen_p = &(_objects[HERO].screenIndex);  // Current screen is hero's
	_heroImage = HERO;                          // Current in use hero image

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
	uint16 *resRow = 0;
	uint16 dummy, numRows, numElems;

	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numRows = in.readUint16BE();
		if (varnt == _gameVariant) {
			resArray = (uint16 **)malloc(sizeof(uint16 *) * (numRows + 1));
			resArray[numRows] = 0;
		}
		for (int i = 0; i < numRows; i++) {
			numElems = in.readUint16BE();
			if (varnt == _gameVariant) {
				resRow = (uint16 *)malloc(sizeof(uint16) * numElems);
				for (int j = 0; j < numElems; j++)
					resRow[j] = in.readUint16BE();
				resArray[i] = resRow;
			} else {
				for (int j = 0; j < numElems; j++)
					dummy = in.readUint16BE();
			}
		}
	}
	return resArray;
}

char ***HugoEngine::loadTextsArray(Common::File &in) {
	int  numNouns;
	int  numTexts;
	int  entryLen;
	int  len;
	char ***resArray = 0;
	char **res = 0;
	char *pos = 0;

	for (int varnt = 0; varnt < _numVariant; varnt++) {
		numNouns = in.readUint16BE();
		if (varnt == _gameVariant) {
			resArray = (char ** *)malloc(sizeof(char **) * (numNouns + 1));
			resArray[numNouns] = 0;
		}
		for (int i = 0; i < numNouns; i++) {
			numTexts = in.readUint16BE();
			entryLen = in.readUint16BE();
			pos = (char *)malloc(entryLen);
			if (varnt == _gameVariant) {
				res = (char **)malloc(sizeof(char *) * numTexts);
				res[0] = pos;
				in.read(res[0], entryLen);
			} else {
				in.read(pos, entryLen);
			}

			pos += DATAALIGNMENT;

			for (int j = 0; j < numTexts; j++) {
				if (varnt == _gameVariant)
					res[j] = pos;

				pos -= 2;
				len = READ_BE_UINT16(pos);
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
	int entryLen;
	char *pos = 0;
	int len;

	entryLen = in.readUint16BE();
	pos = (char *)malloc(entryLen);

	in.read(pos, entryLen);

	pos += DATAALIGNMENT;
	res[0] = pos;

	for (int i = 1; i < numTexts; i++) {
		pos -= 2;
		len = READ_BE_UINT16(pos);
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


} // End of namespace Hugo
