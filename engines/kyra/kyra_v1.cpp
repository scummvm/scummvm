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

#include "kyra/kyra_v1.h"

#include "common/file.h"
#include "common/events.h"
#include "common/system.h"
#include "common/savefile.h"

#include "gui/message.h"

#include "kyra/resource.h"
#include "kyra/screen.h"
#include "kyra/script.h"
#include "kyra/seqplayer.h"
#include "kyra/sound.h"
#include "kyra/sprites.h"
#include "kyra/wsamovie.h"
#include "kyra/animator_v1.h"
#include "kyra/text.h"
#include "kyra/debugger.h"
#include "kyra/timer.h"

namespace Kyra {

KyraEngine_v1::KyraEngine_v1(OSystem *system, const GameFlags &flags)
	: KyraEngine(system, flags) {
	_flags = flags;

	_seq_Forest = _seq_KallakWriting = _seq_KyrandiaLogo = _seq_KallakMalcolm =
	_seq_MalcolmTree = _seq_WestwoodLogo = _seq_Demo1 = _seq_Demo2 = _seq_Demo3 =
	_seq_Demo4 = 0;
	
	_seq_WSATable = _seq_CPSTable = _seq_COLTable = _seq_textsTable = 0;
	_seq_WSATable_Size = _seq_CPSTable_Size = _seq_COLTable_Size = _seq_textsTable_Size = 0;
	
	_roomFilenameTable = _characterImageTable = 0;
	_roomFilenameTableSize = _characterImageTableSize = 0;
	_itemList = _takenList = _placedList = _droppedList = _noDropList = 0;
	_itemList_Size = _takenList_Size = _placedList_Size = _droppedList_Size = _noDropList_Size = 0;
	_putDownFirst = _waitForAmulet = _blackJewel = _poisonGone = _healingTip = 0;
	_putDownFirst_Size = _waitForAmulet_Size = _blackJewel_Size = _poisonGone_Size = _healingTip_Size = 0;
	_thePoison = _fluteString = _wispJewelStrings = _magicJewelString = _flaskFull = _fullFlask = 0;
	_thePoison_Size = _fluteString_Size = _wispJewelStrings_Size = 0;
	_magicJewelString_Size = _flaskFull_Size = _fullFlask_Size = 0;
	
	_defaultShapeTable = 0;
	_healingShapeTable = _healingShape2Table = 0;
	_defaultShapeTableSize = _healingShapeTableSize = _healingShape2TableSize = 0;
	_posionDeathShapeTable = _fluteAnimShapeTable = 0;
	_posionDeathShapeTableSize = _fluteAnimShapeTableSize = 0;
	_winterScrollTable = _winterScroll1Table = _winterScroll2Table = 0;
	_winterScrollTableSize = _winterScroll1TableSize = _winterScroll2TableSize = 0;
	_drinkAnimationTable = _brandonToWispTable = _magicAnimationTable = _brandonStoneTable = 0;
	_drinkAnimationTableSize = _brandonToWispTableSize = _magicAnimationTableSize = _brandonStoneTableSize = 0;
	memset(&_specialPalettes, 0, sizeof(_specialPalettes));
	_debugger = 0;
	_sprites = 0;
	_animator = 0;
	_seq = 0;
	_npcScriptData = 0;
	_scriptMain = 0;
	_scriptClickData = 0;
	_scriptClick = 0;
	_characterList = 0;
	_movFacingTable = 0;
	memset(_shapes, 0, sizeof(_shapes));
	memset(_movieObjects, 0, sizeof(_movieObjects));
	_finalA = _finalB = _finalC = 0;
	_endSequenceBackUpRect = 0;
	memset(_panPagesTable, 0, sizeof(_panPagesTable));
	_npcScriptData = _scriptClickData = 0;
	_scrollUpButton.process0PtrShape = _scrollUpButton.process1PtrShape = _scrollUpButton.process2PtrShape = 0;
	_scrollDownButton.process0PtrShape = _scrollDownButton.process1PtrShape = _scrollDownButton.process2PtrShape = 0;
	memset(_sceneAnimTable, 0, sizeof(_sceneAnimTable));
	_currHeadShape = 0;

	_curSfxFile = _curMusicTheme = 0;

	memset(&_itemBkgBackUp, 0, sizeof(_itemBkgBackUp));
}

KyraEngine_v1::~KyraEngine_v1() {
	for (int i = 0; i < ARRAYSIZE(_movieObjects); ++i) {
		if (_movieObjects[i])
			_movieObjects[i]->close();
		delete _movieObjects[i];
		_movieObjects[i] = 0;
	}

	closeFinalWsa();
	if (_scriptInterpreter) {
		_scriptInterpreter->unloadScript(_npcScriptData);
		_scriptInterpreter->unloadScript(_scriptClickData);
	}

	Common::clearAllSpecialDebugLevels();

	delete _screen;
	delete _debugger;
	delete _sprites;
	delete _animator;
	delete _seq;
		
	delete _npcScriptData;
	delete _scriptMain;
	
	delete _scriptClickData;
	delete _scriptClick;
	
	delete [] _characterList;
	
	delete [] _movFacingTable;

	delete [] _scrollUpButton.process0PtrShape;
	delete [] _scrollUpButton.process1PtrShape;
	delete [] _scrollUpButton.process2PtrShape;
	delete [] _scrollDownButton.process0PtrShape;
	delete [] _scrollDownButton.process1PtrShape;
	delete [] _scrollDownButton.process2PtrShape;

	delete [] _itemBkgBackUp[0];
	delete [] _itemBkgBackUp[1];	

	for (int i = 0; i < ARRAYSIZE(_shapes); ++i) {
		if (_shapes[i] != 0) {
			delete [] _shapes[i];
			for (int i2 = 0; i2 < ARRAYSIZE(_shapes); i2++) {
				if (_shapes[i2] == _shapes[i] && i2 != i) {
					_shapes[i2] = 0;
				}
			}
			_shapes[i] = 0;
		}
	}

	for (int i = 0; i < ARRAYSIZE(_sceneAnimTable); ++i)
		delete [] _sceneAnimTable[i];
}

int KyraEngine_v1::init() {
	_screen = new Screen_v1(this, _system);
	assert(_screen);
	if (!_screen->init())
		error("_screen->init() failed");

	KyraEngine::init();

	_sprites = new Sprites(this, _system);
	assert(_sprites);
	_seq = new SeqPlayer(this, _system);
	assert(_seq);
	_animator = new ScreenAnimator(this, _system);
	assert(_animator);
	_animator->init(5, 11, 12);
	assert(*_animator);

	initStaticResource();
	
	if (_flags.platform == Common::kPlatformFMTowns)
		_sound->setSoundFileList(_soundFilesTowns, _soundFilesTownsCount);
	else
		_sound->setSoundFileList(_soundFiles, _soundFilesCount);
	
	if (!_sound->init())
		error("Couldn't init sound");

	_sound->setVolume(255);
	_sound->loadSoundFile(0);

	setupTimers();
	setupButtonData();
	setupMenu();

	_paletteChanged = 1;
	_currentCharacter = 0;
	_characterList = new Character[11];
	assert(_characterList);
	memset(_characterList, 0, sizeof(Character)*11);

	for (int i = 0; i < 11; ++i)
		memset(_characterList[i].inventoryItems, 0xFF, sizeof(_characterList[i].inventoryItems));

	_characterList[0].sceneId = 5;
	_characterList[0].height = 48;
	_characterList[0].facing = 3;
	_characterList[0].currentAnimFrame = 7;
	
	_npcScriptData = new ScriptData;
	memset(_npcScriptData, 0, sizeof(ScriptData));
	assert(_npcScriptData);
	_npcScript = new ScriptState;
	assert(_npcScript);
	memset(_npcScript, 0, sizeof(ScriptState));
	
	_scriptMain = new ScriptState;
	assert(_scriptMain);
	memset(_scriptMain, 0, sizeof(ScriptState));
	
	_scriptClickData = new ScriptData;
	assert(_scriptClickData);
	memset(_scriptClickData, 0, sizeof(ScriptData));
	_scriptClick = new ScriptState;
	assert(_scriptClick);
	memset(_scriptClick, 0, sizeof(ScriptState));
	
	_debugger = new Debugger_v1(this);
	assert(_debugger);	
	memset(_shapes, 0, sizeof(_shapes));

	for (int i = 0; i < ARRAYSIZE(_movieObjects); ++i)
		_movieObjects[i] = createWSAMovie();

	memset(_flagsTable, 0, sizeof(_flagsTable));

	_abortWalkFlag = false;
	_abortWalkFlag2 = false;
	_talkingCharNum = -1;
	_charSayUnk3 = -1;
	memset(_currSentenceColor, 0, 3);
	_startSentencePalIndex = -1;
	_fadeText = false;

	_cauldronState = 0;
	_crystalState[0] = _crystalState[1] = -1;

	_brandonStatusBit = 0;
	_brandonStatusBit0x02Flag = _brandonStatusBit0x20Flag = 10;
	_brandonPosX = _brandonPosY = -1;
	_deathHandler = 0xFF;
	_poisonDeathCounter = 0;
	
	memset(_itemTable, 0, sizeof(_itemTable));
	memset(_exitList, 0xFFFF, sizeof(_exitList));
	_exitListPtr = 0;
	_pathfinderFlag = _pathfinderFlag2 = 0;
	_lastFindWayRet = 0;
	_sceneChangeState = _loopFlag2 = 0;

	_movFacingTable = new int[150];
	assert(_movFacingTable);
	_movFacingTable[0] = 8;

	registerDefaultSettings();
	readSettings();

	_skipFlag = false;

	_marbleVaseItem = -1;
	memset(_foyerItemTable, -1, sizeof(_foyerItemTable));
	_mouseState = _itemInHand = -1;
	_handleInput = false;
	
	_currentRoom = 0xFFFF;
	_scenePhasingFlag = 0;
	_lastProcessedItem = 0;
	_lastProcessedItemHeight = 16;
	
	_unkScreenVar1 = 1;
	_unkScreenVar2 = 0;
	_unkScreenVar3 = 0;
	_unkAmuletVar = 0;
	
	_endSequenceNeedLoading = 1;
	_malcolmFlag = 0;
	_beadStateVar = 0;
	_endSequenceSkipFlag = 0;
	_unkEndSeqVar2 = 0;
	_endSequenceBackUpRect = 0;
	_unkEndSeqVar4 = 0;
	_unkEndSeqVar5 = 0;
	_lastDisplayedPanPage = 0;
	memset(_panPagesTable, 0, sizeof(_panPagesTable));
	_finalA = _finalB = _finalC = 0;
	memset(&_kyragemFadingState, 0, sizeof(_kyragemFadingState));	
	_kyragemFadingState.gOffset = 0x13;
	_kyragemFadingState.bOffset = 0x13;

	_mousePressFlag = false;
	
	_menuDirectlyToLoad = false;

	_lastMusicCommand = 0;
	
	return 0;
}

int KyraEngine_v1::go() {
	if (_res->getFileSize("6.FNT"))
		_screen->loadFont(Screen::FID_6_FNT, "6.FNT");
	_screen->loadFont(Screen::FID_8_FNT, "8FAT.FNT");
	_screen->setScreenDim(0);

	_abortIntroFlag = false;

	if (_flags.isDemo) {
		seq_demo();
	} else {
		setGameFlag(0xF3);
		setGameFlag(0xFD);
		setGameFlag(0xEF);
		seq_intro();
		if (_quitFlag)
			return 0;
		if (_skipIntroFlag && _abortIntroFlag)
			resetGameFlag(0xEF);
		startup();
		resetGameFlag(0xEF);
		mainLoop();
	}
	return 0;
}


void KyraEngine_v1::startup() {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::startup()");
	static const uint8 colorMap[] = { 0, 0, 0, 0, 12, 12, 12, 0, 0, 0, 0, 0 };
	_screen->setTextColorMap(colorMap);
//	_screen->setFont(Screen::FID_6_FNT);
	_screen->setAnimBlockPtr(3750);
	memset(_sceneAnimTable, 0, sizeof(_sceneAnimTable));
	loadMouseShapes();
	_currentCharacter = &_characterList[0];
	for (int i = 1; i < 5; ++i)
		_animator->setCharacterDefaultFrame(i);
	for (int i = 5; i <= 10; ++i)
		setCharactersPositions(i);
	_animator->setCharactersHeight();
	resetBrandonPoisonFlags();
	_screen->_curPage = 0;
	// XXX
	for (int i = 0; i < 12; ++i) {
		int size = _screen->getRectSize(3, 24);
		_shapes[361+i] = new byte[size];
	}

	_itemBkgBackUp[0] = new uint8[_screen->getRectSize(3, 24)];
	memset(_itemBkgBackUp[0], 0, _screen->getRectSize(3, 24));
	_itemBkgBackUp[1] = new uint8[_screen->getRectSize(4, 32)];
	memset(_itemBkgBackUp[1], 0, _screen->getRectSize(4, 32));

	for (int i = 0; i < _roomTableSize; ++i) {
		for (int item = 0; item < 12; ++item) {
			_roomTable[i].itemsTable[item] = 0xFF;
			_roomTable[i].itemsXPos[item] = 0xFFFF;
			_roomTable[i].itemsYPos[item] = 0xFF;
			_roomTable[i].needInit[item] = 0;
		}
	}

	loadCharacterShapes();
	loadSpecialEffectShapes();
	loadItems();
	loadButtonShapes();
	initMainButtonList();
	loadMainScreen();
	setupTimers();
	_screen->loadPalette("PALETTE.COL", _screen->_currentPalette);

	// XXX
	_animator->initAnimStateList();
	setCharactersInDefaultScene();

	if (!_scriptInterpreter->loadScript("_STARTUP.EMC", _npcScriptData, &_opcodes))
		error("Could not load \"_STARTUP.EMC\" script");
	_scriptInterpreter->initScript(_scriptMain, _npcScriptData);

	if (!_scriptInterpreter->startScript(_scriptMain, 0))
		error("Could not start script function 0 of script \"_STARTUP.EMC\"");

	while (_scriptInterpreter->validScript(_scriptMain))
		_scriptInterpreter->runScript(_scriptMain);
	
	_scriptInterpreter->unloadScript(_npcScriptData);

	if (!_scriptInterpreter->loadScript("_NPC.EMC", _npcScriptData, &_opcodes))
		error("Could not load \"_NPC.EMC\" script");
	
	snd_playTheme(1);
	enterNewScene(_currentCharacter->sceneId, _currentCharacter->facing, 0, 0, 1);
	
	if (_abortIntroFlag && _skipIntroFlag) {
		_menuDirectlyToLoad = true;
		_screen->setMouseCursor(1, 1, _shapes[0]);
		_screen->showMouse();
		buttonMenuCallback(0);
		_menuDirectlyToLoad = false;
	} else
		saveGame(getSavegameFilename(0), "New game");
}

void KyraEngine_v1::mainLoop() {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::mainLoop()");

	while (!_quitFlag) {
		int32 frameTime = (int32)_system->getMillis();
		_skipFlag = false;

		if (_currentCharacter->sceneId == 210) {
			updateKyragemFading();
			if (seq_playEnd() && _deathHandler != 8)
				break;
		}
		
		if (_deathHandler != 0xFF) {
			snd_playWanderScoreViaMap(0, 1);
			snd_playSoundEffect(49);
			_screen->hideMouse();
			_screen->setMouseCursor(1, 1, _shapes[0]);
			destroyMouseItem();
			_screen->showMouse();
			buttonMenuCallback(0);
			_deathHandler = 0xFF;
		}
		
		if ((_brandonStatusBit & 2) && _brandonStatusBit0x02Flag)
			_animator->animRefreshNPC(0);

		if ((_brandonStatusBit & 0x20) && _brandonStatusBit0x20Flag) {
			_animator->animRefreshNPC(0);
			_brandonStatusBit0x20Flag = 0;
		}
		
		_screen->showMouse();

		processButtonList(_buttonList);
		updateMousePointer();
		_timer->update();
		updateTextFade();

		_handleInput = true;
		delay((frameTime + _gameSpeed) - _system->getMillis(), true, true);
		_handleInput = false;

		_sound->process();
	}
}

void KyraEngine_v1::delayUntil(uint32 timestamp, bool updateTimers, bool update, bool isMainLoop) {
	while (_system->getMillis() < timestamp && !_quitFlag) {
		if (updateTimers)
			_timer->update();

		if (timestamp - _system->getMillis() >= 10)
			delay(10, update, isMainLoop);
	}
}

void KyraEngine_v1::delay(uint32 amount, bool update, bool isMainLoop) {
	Common::Event event;
	char saveLoadSlot[20];
	char savegameName[14];

	uint32 start = _system->getMillis();
	do {
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode >= '1' && event.kbd.keycode <= '9' && 
						(event.kbd.flags == Common::KBD_CTRL || event.kbd.flags == Common::KBD_ALT) && isMainLoop) {
					sprintf(saveLoadSlot, "%s.00%d", _targetName.c_str(), event.kbd.keycode - '0');
					if (event.kbd.flags == Common::KBD_CTRL)
						loadGame(saveLoadSlot);
					else {
						sprintf(savegameName, "Quicksave %d",  event.kbd.keycode - '0');
						saveGame(saveLoadSlot, savegameName);
					}
				} else if (event.kbd.flags == Common::KBD_CTRL) {
					if (event.kbd.keycode == 'd')
						_debugger->attach();
					else if (event.kbd.keycode == 'q')
						_quitFlag = true;
				} else if (event.kbd.keycode == '.')
						_skipFlag = true;
				else if (event.kbd.keycode == Common::KEYCODE_RETURN || event.kbd.keycode == Common::KEYCODE_SPACE || event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_abortIntroFlag = true;
					_skipFlag = true;
				}

				break;
			case Common::EVENT_MOUSEMOVE:
				_animator->_updateScreen = true;
				break;
			case Common::EVENT_QUIT:
				quitGame();
				break;
			case Common::EVENT_LBUTTONDOWN:
				_mousePressFlag = true;
				break;
			case Common::EVENT_LBUTTONUP:
				_mousePressFlag = false;

				if (_abortWalkFlag2) 
					_abortWalkFlag = true;

				if (_handleInput) {
					_handleInput = false;
					processInput();
					_handleInput = true;
				} else
					_skipFlag = true;

				break;
			default:
				break;
			}
		}

		if (_debugger->isAttached())
			_debugger->onFrame();

		if (update) {
			_sprites->updateSceneAnims();
			_animator->updateAllObjectShapes();
			updateTextFade();
			updateMousePointer();
		}

		if (_currentCharacter && _currentCharacter->sceneId == 210 && update)
			updateKyragemFading();

		if (_skipFlag && !_abortIntroFlag && !queryGameFlag(0xFE))
			_skipFlag = false;
			
		if (amount > 0 && !_skipFlag && !_quitFlag)
			_system->delayMillis(10);

		if (_skipFlag)
			_sound->voiceStop();
	} while (!_skipFlag && _system->getMillis() < start + amount && !_quitFlag);
}

void KyraEngine_v1::waitForEvent() {
	bool finished = false;
	Common::Event event;

	while (!finished && !_quitFlag) {
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				finished = true;
				break;
			case Common::EVENT_QUIT:
				quitGame();
				break;
			case Common::EVENT_LBUTTONDOWN:
				finished = true;
				_skipFlag = true;
				break;
			default:
				break;
			}
		}

		if (_debugger->isAttached())
			_debugger->onFrame();

		_system->delayMillis(10);
	}
}

void KyraEngine_v1::delayWithTicks(int ticks) {
	uint32 nextTime = _system->getMillis() + ticks * _tickLength;

	while (_system->getMillis() < nextTime) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();

		if (_currentCharacter->sceneId == 210) {
			updateKyragemFading();
			seq_playEnd();
		}

		if (_skipFlag)
			break;

		if (nextTime - _system->getMillis() >= 10)
			delay(10);
	}
}

#pragma mark -
#pragma mark - Animation/shape specific code
#pragma mark -

void KyraEngine_v1::setupShapes123(const Shape *shapeTable, int endShape, int flags) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::setupShapes123(%p, %d, %d)", (const void *)shapeTable, endShape, flags);

	for (int i = 123; i <= 172; ++i)
		_shapes[i] = 0;

	uint8 curImage = 0xFF;
	int curPageBackUp = _screen->_curPage;
	_screen->_curPage = 8;	// we are using page 8 here in the original page 2 was backuped and then used for this stuff
	int shapeFlags = 2;
	if (flags)
		shapeFlags = 3;
	for (int i = 123; i < 123+endShape; ++i) {
		uint8 newImage = shapeTable[i-123].imageIndex;
		if (newImage != curImage && newImage != 0xFF) {
			assert(_characterImageTable);
			_screen->loadBitmap(_characterImageTable[newImage], 8, 8, 0);
			curImage = newImage;
		}
		_shapes[i] = _screen->encodeShape(shapeTable[i-123].x<<3, shapeTable[i-123].y, shapeTable[i-123].w<<3, shapeTable[i-123].h, shapeFlags);
		assert(i-7 < _defaultShapeTableSize);
		_defaultShapeTable[i-7].xOffset = shapeTable[i-123].xOffset;
		_defaultShapeTable[i-7].yOffset = shapeTable[i-123].yOffset;
		_defaultShapeTable[i-7].w = shapeTable[i-123].w;
		_defaultShapeTable[i-7].h = shapeTable[i-123].h;
	}
	_screen->_curPage = curPageBackUp;
}

void KyraEngine_v1::freeShapes123() {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::freeShapes123()");

	for (int i = 123; i <= 172; ++i) {
		delete [] _shapes[i];
		_shapes[i] = 0;
	}
}

#pragma mark -
#pragma mark - Misc stuff
#pragma mark -

Movie *KyraEngine_v1::createWSAMovie() {
	if (_flags.platform == Common::kPlatformAmiga)
		return new WSAMovieAmiga(this);

	return new WSAMovieV1(this);
}

void KyraEngine_v1::setBrandonPoisonFlags(int reset) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::setBrandonPoisonFlags(%d)", reset);
	_brandonStatusBit |= 1;

	if (reset)
		_poisonDeathCounter = 0;

	for (int i = 0; i < 0x100; ++i)
		_brandonPoisonFlagsGFX[i] = i;

	_brandonPoisonFlagsGFX[0x99] = 0x34;
	_brandonPoisonFlagsGFX[0x9A] = 0x35;
	_brandonPoisonFlagsGFX[0x9B] = 0x37;
	_brandonPoisonFlagsGFX[0x9C] = 0x38;
	_brandonPoisonFlagsGFX[0x9D] = 0x2B;
}

void KyraEngine_v1::resetBrandonPoisonFlags() {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::resetBrandonPoisonFlags()");
	_brandonStatusBit = 0;

	for (int i = 0; i < 0x100; ++i)
		_brandonPoisonFlagsGFX[i] = i;
}

#pragma mark -
#pragma mark - Input
#pragma mark -

void KyraEngine_v1::processInput() {
	Common::Point mouse = getMousePos();
	int xpos = mouse.x;
	int ypos = mouse.y;

	debugC(9, kDebugLevelMain, "KyraEngine_v1::processInput(%d, %d)", xpos, ypos);
	_abortWalkFlag2 = false;

	if (processInputHelper(xpos, ypos))
		return;

	uint8 item = findItemAtPos(xpos, ypos);
	if (item == 0xFF) {
		_changedScene = false;
		int handled = clickEventHandler(xpos, ypos);
		if (_changedScene || handled)
			return;
	} 
	
	// XXX _deathHandler specific
	if (ypos <= 158) {
		uint16 exit = 0xFFFF;
		if (xpos < 12) {
			exit = _walkBlockWest;
		} else if (xpos >= 308) {
			exit = _walkBlockEast;
		} else if (ypos >= 136) {
			exit = _walkBlockSouth;
		} else if (ypos < 12) {
			exit = _walkBlockNorth;
		}
		
		if (exit != 0xFFFF) {
			_abortWalkFlag2 = true;
			handleSceneChange(xpos, ypos, 1, 1);
			_abortWalkFlag2 = false;
			return;
		} else {
			int script = checkForNPCScriptRun(xpos, ypos);
			if (script >= 0) {
				runNpcScript(script);
				return;
			}
			if (_itemInHand != -1) {
				if (ypos < 155) {
					if (hasClickedOnExit(xpos, ypos)) {
						_abortWalkFlag2 = true;
						handleSceneChange(xpos, ypos, 1, 1);
						_abortWalkFlag2 = false;
						return;
					}
					dropItem(0, _itemInHand, xpos, ypos, 1);
				}
			} else {
				if (ypos <= 155) {
					_abortWalkFlag2 = true;
					handleSceneChange(xpos, ypos, 1, 1);
					_abortWalkFlag2 = false;
				}
			}
		}
	}	
}

int KyraEngine_v1::processInputHelper(int xpos, int ypos) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::processInputHelper(%d, %d)", xpos, ypos);
	uint8 item = findItemAtPos(xpos, ypos);
	if (item != 0xFF) {
		if (_itemInHand == -1) {
			_screen->hideMouse();
			_animator->animRemoveGameItem(item);
			snd_playSoundEffect(53);
			assert(_currentCharacter->sceneId < _roomTableSize);
			Room *currentRoom = &_roomTable[_currentCharacter->sceneId];
			int item2 = currentRoom->itemsTable[item];
			currentRoom->itemsTable[item] = 0xFF;
			setMouseItem(item2);
			assert(_itemList && _takenList);
			updateSentenceCommand(_itemList[item2], _takenList[0], 179);
			_itemInHand = item2;
			_screen->showMouse();
			clickEventHandler2();
			return 1;
		} else {
			exchangeItemWithMouseItem(_currentCharacter->sceneId, item);
			return 1;
		}
	}
	return 0;
}

int KyraEngine_v1::clickEventHandler(int xpos, int ypos) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::clickEventHandler(%d, %d)", xpos, ypos);
	_scriptInterpreter->initScript(_scriptClick, _scriptClickData);
	_scriptClick->regs[1] = xpos;
	_scriptClick->regs[2] = ypos;
	_scriptClick->regs[3] = 0;
	_scriptClick->regs[4] = _itemInHand;
	_scriptInterpreter->startScript(_scriptClick, 1);

	while (_scriptInterpreter->validScript(_scriptClick))
		_scriptInterpreter->runScript(_scriptClick);

	return _scriptClick->regs[3];
}

void KyraEngine_v1::updateMousePointer(bool forceUpdate) {
	int shape = 0;
	
	int newMouseState = 0;
	int newX = 0;
	int newY = 0;
	Common::Point mouse = getMousePos();
	if (mouse.y <= 158) {
		if (mouse.x >= 12) {
			if (mouse.x >= 308) {
				if (_walkBlockEast == 0xFFFF) {
					newMouseState = -2;
				} else {
					newMouseState = -5;
					shape = 3;
					newX = 7;
					newY = 5;
				}
			} else if (mouse.y >= 136) {
				if (_walkBlockSouth == 0xFFFF) {
					newMouseState = -2;
				} else {
					newMouseState = -4;
					shape = 4;
					newX = 5;
					newY = 7;
				}
			} else if (mouse.y < 12) {
				if (_walkBlockNorth == 0xFFFF) {
					newMouseState = -2;
				} else {
					newMouseState = -6;
					shape = 2;
					newX = 5;
					newY = 1;
				}
			}
		} else {
			if (_walkBlockWest == 0xFFFF) {
				newMouseState = -2;
			} else {
				newMouseState = -3;
				newX = 1;
				newY = shape = 5;
			}
		}
	}
	
	if (mouse.x >= _entranceMouseCursorTracks[0] && mouse.y >= _entranceMouseCursorTracks[1]
		&& mouse.x <= _entranceMouseCursorTracks[2] && mouse.y <= _entranceMouseCursorTracks[3]) {
		switch (_entranceMouseCursorTracks[4]) {
		case 0:
			newMouseState = -6;
			shape = 2;
			newX = 5;
			newY = 1;
			break;

		case 2:
			newMouseState = -5;
			shape = 3;
			newX = 7;
			newY = 5;
			break;

		case 4:
			newMouseState = -4;
			shape = 4;
			newX = 5;
			newY = 7;
			break;

		case 6:
			newMouseState = -3;
			shape = 5;
			newX = 1;
			newY = 5;
			break;

		default:
			break;
		}
	}
	
	if (newMouseState == -2) {
		shape = 6;
		newX = 4;
		newY = 4;
	}
	
	if ((newMouseState && _mouseState != newMouseState) || (newMouseState && forceUpdate)) {
		_mouseState = newMouseState;
		_screen->hideMouse();
		_screen->setMouseCursor(newX, newY, _shapes[shape]);
		_screen->showMouse();
	}
	
	if (!newMouseState) {
		if (_mouseState != _itemInHand || forceUpdate) {
			if (mouse.y > 158 || (mouse.x >= 12 && mouse.x < 308 && mouse.y < 136 && mouse.y >= 12) || forceUpdate) {
				_mouseState = _itemInHand;
				_screen->hideMouse();
				if (_itemInHand == -1) {
					_screen->setMouseCursor(1, 1, _shapes[0]);
				} else {
					_screen->setMouseCursor(8, 15, _shapes[216+_itemInHand]);
				}
				_screen->showMouse();
			}
		}
	}
}

bool KyraEngine_v1::hasClickedOnExit(int xpos, int ypos) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::hasClickedOnExit(%d, %d)", xpos, ypos);
	if (xpos < 16 || xpos >= 304)
		return true;

	if (ypos < 8)
		return true;

	if (ypos < 136 || ypos > 155)
		return false;

	return true;
}

void KyraEngine_v1::clickEventHandler2() {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::clickEventHandler2()");

	Common::Point mouse = getMousePos();

	_scriptInterpreter->initScript(_scriptClick, _scriptClickData);
	_scriptClick->regs[0] = _currentCharacter->sceneId;
	_scriptClick->regs[1] = mouse.x;
	_scriptClick->regs[2] = mouse.y;
	_scriptClick->regs[4] = _itemInHand;
	_scriptInterpreter->startScript(_scriptClick, 6);
	
	while (_scriptInterpreter->validScript(_scriptClick))
		_scriptInterpreter->runScript(_scriptClick);
}

int KyraEngine_v1::checkForNPCScriptRun(int xpos, int ypos) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::checkForNPCScriptRun(%d, %d)", xpos, ypos);
	int returnValue = -1;
	const Character *currentChar = _currentCharacter;
	int charLeft = 0, charRight = 0, charTop = 0, charBottom = 0;
	
	int scaleFactor = _scaleTable[currentChar->y1];
	int addX = (((scaleFactor*8)*3)>>8)>>1;
	int addY = ((scaleFactor*3)<<4)>>8;
	
	charLeft = currentChar->x1 - addX;
	charRight = currentChar->x1 + addX;
	charTop = currentChar->y1 - addY;
	charBottom = currentChar->y1;
	
	if (xpos >= charLeft && charRight >= xpos && charTop <= ypos && charBottom >= ypos)
		return 0;
	
	if (xpos > 304 || xpos < 16)
		return -1;
	
	for (int i = 1; i < 5; ++i) {
		currentChar = &_characterList[i];
		
		if (currentChar->sceneId != _currentCharacter->sceneId)
			continue;
			
		charLeft = currentChar->x1 - 12;
		charRight = currentChar->x1 + 11;
		charTop = currentChar->y1 - 48;
		// if (!i) {
		// 	charBottom = currentChar->y2 - 16;
		// } else {
		charBottom = currentChar->y1;
		// }
		
		if (xpos < charLeft || xpos > charRight || ypos < charTop || charBottom < ypos)
			continue;
		
		if (returnValue != -1) {
			if (currentChar->y1 >= _characterList[returnValue].y1)
				returnValue = i;
		} else {
			returnValue = i;
		}
	}
	
	return returnValue;
}

void KyraEngine_v1::runNpcScript(int func) {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::runNpcScript(%d)", func);
	_scriptInterpreter->initScript(_npcScript, _npcScriptData);
	_scriptInterpreter->startScript(_npcScript, func);
	_npcScript->regs[0] = _currentCharacter->sceneId;
	_npcScript->regs[4] = _itemInHand;
	_npcScript->regs[5] = func;
	
	while (_scriptInterpreter->validScript(_npcScript))
		_scriptInterpreter->runScript(_npcScript);
}

void KyraEngine_v1::checkAmuletAnimFlags() {
	debugC(9, kDebugLevelMain, "KyraEngine_v1::checkSpecialAnimFlags()");

	if (_brandonStatusBit & 2) {
		seq_makeBrandonNormal2();
		_timer->setCountdown(19, 300);
	}

	if (_brandonStatusBit & 0x20) {
		seq_makeBrandonNormal();
		_timer->setCountdown(19, 300);
	}
}

typedef Functor1Mem<ScriptState*, int, KyraEngine_v1> OpcodeV1;
#define Opcode(x) OpcodeV1(this, &KyraEngine_v1::x)
void KyraEngine_v1::setupOpcodeTable() {
	static const OpcodeV1 opcodeTable[] = {
		// 0x00
		Opcode(o1_magicInMouseItem),
		Opcode(o1_characterSays),
		Opcode(o1_pauseTicks),
		Opcode(o1_drawSceneAnimShape),
		// 0x04
		Opcode(o1_queryGameFlag),
		Opcode(o1_setGameFlag),
		Opcode(o1_resetGameFlag),
		Opcode(o1_runNPCScript),
		// 0x08
		Opcode(o1_setSpecialExitList),
		Opcode(o1_blockInWalkableRegion),
		Opcode(o1_blockOutWalkableRegion),
		Opcode(o1_walkPlayerToPoint),
		// 0x0c
		Opcode(o1_dropItemInScene),
		Opcode(o1_drawAnimShapeIntoScene),
		Opcode(o1_createMouseItem),
		Opcode(o1_savePageToDisk),
		// 0x10
		Opcode(o1_sceneAnimOn),
		Opcode(o1_sceneAnimOff),
		Opcode(o1_getElapsedSeconds),
		Opcode(o1_mouseIsPointer),
		// 0x14
		Opcode(o1_destroyMouseItem),
		Opcode(o1_runSceneAnimUntilDone),
		Opcode(o1_fadeSpecialPalette),
		Opcode(o1_playAdlibSound),
		// 0x18
		Opcode(o1_playAdlibScore),
		Opcode(o1_phaseInSameScene),
		Opcode(o1_setScenePhasingFlag),
		Opcode(o1_resetScenePhasingFlag),
		// 0x1c
		Opcode(o1_queryScenePhasingFlag),
		Opcode(o1_sceneToDirection),
		Opcode(o1_setBirthstoneGem),
		Opcode(o1_placeItemInGenericMapScene),
		// 0x20
		Opcode(o1_setBrandonStatusBit),
		Opcode(o1_pauseSeconds),
		Opcode(o1_getCharactersLocation),
		Opcode(o1_runNPCSubscript),
		// 0x24
		Opcode(o1_magicOutMouseItem),
		Opcode(o1_internalAnimOn),
		Opcode(o1_forceBrandonToNormal),
		Opcode(o1_poisonDeathNow),
		// 0x28
		Opcode(o1_setScaleMode),
		Opcode(o1_openWSAFile),
		Opcode(o1_closeWSAFile),
		Opcode(o1_runWSAFromBeginningToEnd),
		// 0x2c
		Opcode(o1_displayWSAFrame),
		Opcode(o1_enterNewScene),
		Opcode(o1_setSpecialEnterXAndY),
		Opcode(o1_runWSAFrames),
		// 0x30
		Opcode(o1_popBrandonIntoScene),
		Opcode(o1_restoreAllObjectBackgrounds),
		Opcode(o1_setCustomPaletteRange),
		Opcode(o1_loadPageFromDisk),
		// 0x34
		Opcode(o1_customPrintTalkString),
		Opcode(o1_restoreCustomPrintBackground),
		Opcode(o1_hideMouse),
		Opcode(o1_showMouse),
		// 0x38
		Opcode(o1_getCharacterX),
		Opcode(o1_getCharacterY),
		Opcode(o1_changeCharactersFacing),
		Opcode(o1_copyWSARegion),
		// 0x3c
		Opcode(o1_printText),
		Opcode(o1_random),
		Opcode(o1_loadSoundFile),
		Opcode(o1_displayWSAFrameOnHidPage),
		// 0x40
		Opcode(o1_displayWSASequentialFrames),
		Opcode(o1_drawCharacterStanding),
		Opcode(o1_internalAnimOff),
		Opcode(o1_changeCharactersXAndY),
		// 0x44
		Opcode(o1_clearSceneAnimatorBeacon),
		Opcode(o1_querySceneAnimatorBeacon),
		Opcode(o1_refreshSceneAnimator),
		Opcode(o1_placeItemInOffScene),
		// 0x48
		Opcode(o1_wipeDownMouseItem),
		Opcode(o1_placeCharacterInOtherScene),
		Opcode(o1_getKey),
		Opcode(o1_specificItemInInventory),
		// 0x4c
		Opcode(o1_popMobileNPCIntoScene),
		Opcode(o1_mobileCharacterInScene),
		Opcode(o1_hideMobileCharacter),
		Opcode(o1_unhideMobileCharacter),
		// 0x50
		Opcode(o1_setCharactersLocation),
		Opcode(o1_walkCharacterToPoint),
		Opcode(o1_specialEventDisplayBrynnsNote),
		Opcode(o1_specialEventRemoveBrynnsNote),
		// 0x54
		Opcode(o1_setLogicPage),
		Opcode(o1_fatPrint),
		Opcode(o1_preserveAllObjectBackgrounds),
		Opcode(o1_updateSceneAnimations),
		// 0x58
		Opcode(o1_sceneAnimationActive),
		Opcode(o1_setCharactersMovementDelay),
		Opcode(o1_getCharactersFacing),
		Opcode(o1_bkgdScrollSceneAndMasksRight),
		// 0x5c
		Opcode(o1_dispelMagicAnimation),
		Opcode(o1_findBrightestFireberry),
		Opcode(o1_setFireberryGlowPalette),
		Opcode(o1_setDeathHandlerFlag),
		// 0x60
		Opcode(o1_drinkPotionAnimation),
		Opcode(o1_makeAmuletAppear),
		Opcode(o1_drawItemShapeIntoScene),
		Opcode(o1_setCharactersCurrentFrame),
		// 0x64
		Opcode(o1_waitForConfirmationMouseClick),
		Opcode(o1_pageFlip),
		Opcode(o1_setSceneFile),
		Opcode(o1_getItemInMarbleVase),
		// 0x68
		Opcode(o1_setItemInMarbleVase),
		Opcode(o1_addItemToInventory),
		Opcode(o1_intPrint),
		Opcode(o1_shakeScreen),
		// 0x6c
		Opcode(o1_createAmuletJewel),
		Opcode(o1_setSceneAnimCurrXY),
		Opcode(o1_poisonBrandonAndRemaps),
		Opcode(o1_fillFlaskWithWater),
		// 0x70
		Opcode(o1_getCharactersMovementDelay),
		Opcode(o1_getBirthstoneGem),
		Opcode(o1_queryBrandonStatusBit),
		Opcode(o1_playFluteAnimation),
		// 0x74
		Opcode(o1_playWinterScrollSequence),
		Opcode(o1_getIdolGem),
		Opcode(o1_setIdolGem),
		Opcode(o1_totalItemsInScene),
		// 0x78
		Opcode(o1_restoreBrandonsMovementDelay),
		Opcode(o1_setMousePos),
		Opcode(o1_getMouseState),
		Opcode(o1_setEntranceMouseCursorTrack),
		// 0x7c
		Opcode(o1_itemAppearsOnGround),
		Opcode(o1_setNoDrawShapesFlag),
		Opcode(o1_fadeEntirePalette),
		Opcode(o1_itemOnGroundHere),
		// 0x80
		Opcode(o1_queryCauldronState),
		Opcode(o1_setCauldronState),
		Opcode(o1_queryCrystalState),
		Opcode(o1_setCrystalState),
		// 0x84
		Opcode(o1_setPaletteRange),
		Opcode(o1_shrinkBrandonDown),
		Opcode(o1_growBrandonUp),
		Opcode(o1_setBrandonScaleXAndY),
		// 0x88
		Opcode(o1_resetScaleMode),
		Opcode(o1_getScaleDepthTableValue),
		Opcode(o1_setScaleDepthTableValue),
		Opcode(o1_message),
		// 0x8c
		Opcode(o1_checkClickOnNPC),
		Opcode(o1_getFoyerItem),
		Opcode(o1_setFoyerItem),
		Opcode(o1_setNoItemDropRegion),
		// 0x90
		Opcode(o1_walkMalcolmOn),
		Opcode(o1_passiveProtection),
		Opcode(o1_setPlayingLoop),
		Opcode(o1_brandonToStoneSequence),
		// 0x94
		Opcode(o1_brandonHealingSequence),
		Opcode(o1_protectCommandLine),
		Opcode(o1_pauseMusicSeconds),
		Opcode(o1_resetMaskRegion),
		// 0x98
		Opcode(o1_setPaletteChangeFlag),
		Opcode(o1_fillRect),
		Opcode(o1_vocUnload),
		Opcode(o1_vocLoad),
		// 0x9c
		Opcode(o1_dummy)
	};
	
	for (int i = 0; i < ARRAYSIZE(opcodeTable); ++i)
		_opcodes.push_back(&opcodeTable[i]);
}
#undef Opcode

} // end of namespace Kyra

