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

#include "kyra/kyra.h"
#include "kyra/kyra_v2.h"
#include "kyra/screen.h"
#include "kyra/resource.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"
#include "kyra/script.h"
#include "kyra/text_v2.h"
#include "kyra/timer.h"
#include "kyra/debugger.h"

#include "common/system.h"
#include "common/config-manager.h"

namespace Kyra {

KyraEngine_v2::KyraEngine_v2(OSystem *system, const GameFlags &flags) : KyraEngine(system, flags), _updateFunctor(this, &KyraEngine_v2::update) {
	memset(_defaultShapeTable, 0, sizeof(_defaultShapeTable));
	_mouseSHPBuf = 0;
	_debugger = 0;
	_screen = 0;
	_text = 0;

	_seqProcessedString = 0;
	_activeWSA = 0;
	_activeText = 0;
	_seqWsa = 0;
	_sequences = 0;
	_nSequences = 0;
	_demoShapeDefs = 0;
	_sequenceSoundList = 0;

	_showCredits = false;

	_gamePlayBuffer = 0;
	_cCodeBuffer = _optionsBuffer = _chapterBuffer = 0;

	_overwriteSceneFacing = false;
	_mainCharX = _mainCharY = -1;
	_drawNoShapeFlag = false;
	_charPalEntry = 0;
	_itemInHand = -1;
	_unkSceneScreenFlag1 = false;
	_noScriptEnter = true;
	_currentChapter = 0;
	_newChapterFile = 1;
	_oldTalkFile = -1;
	_currentTalkFile = 0;
	_lastSfxTrack = -1;
	_handItemSet = -1;
	_lastProcessedSceneScript = 0;
	_specialSceneScriptRunFlag = false;
	memset(_animObjects, 0, sizeof(_animObjects));
	_unkHandleSceneChangeFlag = false;
	_pathfinderFlag = 0;
	_mouseX = _mouseY = 0;
	_newShapeCount = 0;
	_newShapeFiledata = 0;

	_vocHigh = -1;
	_chatVocHigh = -1;
	_chatVocLow = -1;
	_chatText = 0;
	_chatObject = -1;
	_lastIdleScript = -1;

	_timChatText = 0;
	_timChatObject = -1;

	_currentTalkSections.STATim = NULL;
	_currentTalkSections.TLKTim = NULL;
	_currentTalkSections.ENDTim = NULL;

	memset(&_invWsa, 0, sizeof(_invWsa));
	_itemAnimTable = 0;
	_nextAnimItem = 0;

	_colorCodeFlag1 = 0;
	_colorCodeFlag2 = -1;
	_scriptCountDown = 0;
	_dbgPass = 0;

	_gamePlayBuffer = 0;
	_unkBuf500Bytes = 0;
	_screenBuffer = 0;
	_inventorySaved = false;
	_unkBuf200kByte = 0;
	memset(&_defaultShapeTable, 0, sizeof(_defaultShapeTable));
	memset(&_sceneShapeTable, 0, sizeof(_sceneShapeTable));
	memset(&_sceneScriptData, 0, sizeof(_sceneScriptData));

	_talkObjectList = 0;
	_shapeDescTable = 0;
	_gfxBackUpRect = 0;
	_sceneList = 0;
	memset(&_sceneAnimMovie, 0, sizeof(_sceneAnimMovie));
	memset(&_wsaSlots, 0, sizeof(_wsaSlots));
	memset(&_buttonShapes, 0, sizeof(_buttonShapes));

	_inventoryButtons = _buttonList = 0;

	_dlgBuffer = 0;
	_conversationState = new int8*[19];
	for (int i = 0; i < 19; i++)
		_conversationState[i] = new int8[14];
	_npcTalkChpIndex = _npcTalkDlgIndex = -1;
	_mainCharacter.dlgIndex = 0;
	setNewDlgIndex(-1);

	_deathHandler = -1;

	_bookMaxPage = 6;
	_bookCurPage = 0;
	_bookNewPage = 0;
	_bookBkgd = 0;

	_cauldronState = 0;
	_cauldronUseCount = 0;
	memset(_cauldronStateTables, 0, sizeof(_cauldronStateTables));

	_menuDirectlyToLoad = false;
}

KyraEngine_v2::~KyraEngine_v2() {
	cleanup();
	seq_uninit();

	if (_sequences)
		delete [] _sequences;
	if (_nSequences)
		delete [] _nSequences;

	delete [] _mouseSHPBuf;
	delete _screen;
	delete _text;
	delete _gui;
	_text = 0;
	delete _debugger;
	delete _invWsa.wsa;

	if (_sequenceSoundList) {
		for (int i = 0; i < _sequenceSoundListSize; i++) {
			if (_sequenceSoundList[i])
				delete [] _sequenceSoundList[i];
		}
		delete [] _sequenceSoundList;
		_sequenceSoundList = NULL;
	}

	if (_dlgBuffer)
		delete [] _dlgBuffer;
	for (int i = 0; i < 19; i++)
		delete [] _conversationState[i];
	delete [] _conversationState;
}

Movie *KyraEngine_v2::createWSAMovie() {
	return new WSAMovieV2(this);
}

int KyraEngine_v2::init() {
	_screen = new Screen_v2(this, _system);
	assert(_screen);
	if (!_screen->init())
		error("_screen->init() failed");

	KyraEngine::init();
	initStaticResource();

	_debugger = new Debugger_v2(this);
	assert(_debugger);
	_text = new TextDisplayer_v2(this, _screen);
	assert(_text);
	_gui = new GUI_v2(this);
	assert(_gui);

	if (_flags.isDemo && !_flags.isTalkie) {
		_screen->loadFont(_screen->FID_8_FNT, "FONT9P.FNT");
	} else {
		_screen->loadFont(_screen->FID_6_FNT, "6.FNT");
		_screen->loadFont(_screen->FID_8_FNT, "8FAT.FNT");
		_screen->loadFont(_screen->FID_BOOKFONT_FNT, "BOOKFONT.FNT");
	}
	_screen->loadFont(_screen->FID_GOLDFONT_FNT, "GOLDFONT.FNT");

	_screen->setAnimBlockPtr(3504);
	_screen->setScreenDim(0);

	if (!_sound->init())
		error("Couldn't init sound");

	_abortIntroFlag = false;

	if (_sequenceStrings) {
		for (int i = 0; i < 33; i++)
			_sequenceStringsDuration[i] = (int) strlen(_sequenceStrings[i]) * 8;
	}

	// No mouse display in demo
	if (_flags.isDemo && !_flags.isTalkie)
		return 0;

	tim_setupOpcodes();

	_mouseSHPBuf = _res->fileData("PWGMOUSE.SHP", 0);
	assert(_mouseSHPBuf);

	for (int i = 0; i < 2; i++) {
		_defaultShapeTable[i] = _screen->getPtrToShape(_mouseSHPBuf, i);
		assert(_defaultShapeTable[i]);
	}

	_screen->setMouseCursor(0, 0, _defaultShapeTable[0]);
	return 0;
}

int KyraEngine_v2::go() {
	if (_gameToLoad == -1) {
		if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)
			seq_showStarcraftLogo();

		if (_flags.isDemo && !_flags.isTalkie) {
			seq_playSequences(kSequenceDemoVirgin, kSequenceDemoFisher);
			_menuChoice = 4;
		} else {
			seq_playSequences(kSequenceVirgin, kSequenceZanfaun);
		}
	} else {
		_menuChoice = 1;
	}

	_res->unloadAllPakFiles();

	if (_menuChoice != 4) {
		// load just the pak files needed for ingame
		_res->loadPakFile(StaticResource::staticDataFilename());
		if (_flags.platform == Common::kPlatformPC && _flags.isTalkie)
			_res->loadFileList("FILEDATA.FDT");
		else
			_res->loadFileList(_ingamePakList, _ingamePakListSize);
	}

	_menuDirectlyToLoad = (_menuChoice == 3) ? true : false;

	if (_menuChoice & 1) {
		startup();
		if (!quit())
			runLoop();
		cleanup();
		
		if (_showCredits)
			seq_playSequences(kSequenceFunters, kSequenceFrash);
	}

	return 0;
}

void KyraEngine_v2::startup() {
	_sound->setSoundList(&_soundData[kMusicIngame]);
	// The track map is exactly the same
	// for FM-TOWNS and DOS
	_trackMap = _dosTrackMap;
	_trackMapSize = _dosTrackMapSize;

	_screen->_curPage = 0;
	delete [] _mouseSHPBuf;
	_mouseSHPBuf = 0;

	memset(_defaultShapeTable, 0, sizeof(_defaultShapeTable));
	memset(_sceneShapeTable, 0, sizeof(_sceneShapeTable));
	_gamePlayBuffer = new uint8[46080];
	_unkBuf500Bytes = new uint8[500];

	loadMouseShapes();
	loadItemShapes();

	_screen->setMouseCursor(0, 0, getShapePtr(0));

	_screenBuffer = new uint8[64000];
	_unkBuf200kByte = new uint8[200000];

	loadChapterBuffer(_newChapterFile);

	loadCCodeBuffer("C_CODE.XXX");

	if (_flags.isTalkie) {
		loadOptionsBuffer("OPTIONS.XXX");

		showMessageFromCCode(265, 150, 0);
		_screen->updateScreen();
		openTalkFile(0);
		_currentTalkFile = 1;
		openTalkFile(1);
	} else {
		_optionsBuffer = _cCodeBuffer;
	}

	showMessage(0, 207);

	_screen->setShapePages(5, 3);

	memset(&_mainCharacter, 0, sizeof(_mainCharacter));
	_mainCharacter.height = 0x30;
	_mainCharacter.facing = 4;
	_mainCharacter.animFrame = 0x12;
	memset(_mainCharacter.inventory, -1, sizeof(_mainCharacter.inventory));

	memset(_sceneAnims, 0, sizeof(_sceneAnims));
	for (int i = 0; i < ARRAYSIZE(_sceneAnimMovie); ++i)
		_sceneAnimMovie[i] = new WSAMovieV2(this);
	memset(_wsaSlots, 0, sizeof(_wsaSlots));
	for (int i = 0; i < ARRAYSIZE(_wsaSlots); ++i)
		_wsaSlots[i] = new WSAMovieV2(this);

	_screen->_curPage = 0;

	_talkObjectList = new TalkObject[72];
	memset(_talkObjectList, 0, sizeof(TalkObject)*72);
	_shapeDescTable = new ShapeDesc[55];
	memset(_shapeDescTable, 0, sizeof(ShapeDesc)*55);

	for (int i = 9; i <= 32; ++i) {
		_shapeDescTable[i-9].width = 30;
		_shapeDescTable[i-9].height = 55;
		_shapeDescTable[i-9].xAdd = -15;
		_shapeDescTable[i-9].yAdd = -50;
	}

	for (int i = 19; i <= 24; ++i) {
		_shapeDescTable[i-9].width = 53;
		_shapeDescTable[i-9].yAdd = -51;
	}

	_gfxBackUpRect = new uint8[_screen->getRectSize(32, 32)];
	_itemList = new Item[30];
	memset(_itemList, 0, sizeof(Item)*30);
	loadButtonShapes();
	resetItemList();
	_loadedZTable = 1;
	loadZShapes(_loadedZTable);
	initInventoryButtonList();
	setupLangButtonShapes();
	loadInventoryShapes();

	_res->loadFileToBuf("PALETTE.COL", _screen->_currentPalette, 0x300);
	_screen->loadBitmap("_PLAYFLD.CPS", 3, 3, 0);
	_screen->copyPage(3, 0);
	_screen->showMouse();
	_screen->hideMouse();

	clearAnimObjects();

	for (int i = 0; i < 19; ++i)
		memset(_conversationState[i], -1, sizeof(int8)*14);
	clearCauldronTable();
	memset(_inputColorCode, -1, sizeof(_inputColorCode));
	memset(_newSceneDlgState, 0, sizeof(_newSceneDlgState));
	memset(_hiddenItems, -1, sizeof(_hiddenItems));
	for (int i = 0; i < 23; ++i)
		resetCauldronStateTable(i);

	_sceneList = new SceneDesc[86];
	memset(_sceneList, 0, sizeof(SceneDesc)*86);
	_sceneListSize = 86;
	runStartScript(1, 0);
	loadNPCScript();

	if (_gameToLoad == -1) {
		snd_playWanderScoreViaMap(52, 1);
		enterNewScene(_mainCharacter.sceneId, _mainCharacter.facing, 0, 0, 1);
		saveGame(getSavegameFilename(0), "New Game");
	} else {
		loadGame(getSavegameFilename(_gameToLoad));
	}

	_screen->showMouse();

	if (_menuDirectlyToLoad)
		(*_inventoryButtons[0].buttonCallback)(&_inventoryButtons[0]);

	setNextIdleAnimTimer();
	//XXX
	setWalkspeed(_configWalkspeed);
}

void KyraEngine_v2::runLoop() {
	_screen->updateScreen();

	_quitFlag = false;
	_runFlag = true;
	while (!_quitFlag && _runFlag) {
		if (_deathHandler >= 0) {
			removeHandItem();
			delay(5);
			_drawNoShapeFlag = 0;
			_gui->optionsButton(0);
			_deathHandler = -1;
		}

		if (_system->getMillis() > _nextIdleAnim)
			showIdleAnim();

		if (queryGameFlag(0x159)) {
			dinoRide();
			resetGameFlag(0x159);
		}

		if (queryGameFlag(0x124) && !queryGameFlag(0x125)) {
			_mainCharacter.animFrame = 32;
			enterNewScene(39, -1, 0, 0, 0);
		}

		if (queryGameFlag(0xD8)) {
			resetGameFlag(0xD8);
			if (_mainCharacter.sceneId == 34) {
				if (queryGameFlag(0xD1)) {
					initTalkObject(28);
					npcChatSequence(getTableString(0xFA, _cCodeBuffer, 1), 28, 0x83, 0xFA);
					deinitTalkObject(28);
					enterNewScene(35, 4, 0, 0, 0);
				} else if (queryGameFlag(0xD0)) {
					initTalkObject(29);
					npcChatSequence(getTableString(0xFB, _cCodeBuffer, 1), 29, 0x83, 0xFB);
					deinitTalkObject(29);
					enterNewScene(33, 6, 0, 0, 0);
				}
			}
		}

		int inputFlag = checkInput(_buttonList, true);
		removeInputTop();

		update();

		if (inputFlag == 198 || inputFlag == 199) {
			_unk3 = _handItemSet;
			handleInput(_mouseX, _mouseY);
		}

		//if (queryGameFlag(0x1EE) && inputFlag)
		//	sub_13B19(inputFlag);

		_system->delayMillis(10);
	}
}

void KyraEngine_v2::handleInput(int x, int y) {
	setNextIdleAnimTimer();
	if (_unk5) {
		_unk5 = 0;
		return;
	}

	if (!_screen->isMouseVisible())
		return;

	if (_unk3 == -2) {
		snd_playSoundEffect(13);
		return;
	}

	setNextIdleAnimTimer();

	if (x <= 6 || x >= 312 || y <= 6 || y >= 135) {
		bool exitOk = false;
		assert(_unk3 + 6 >= 0);
		switch (_unk3 + 6) {
		case 0:
			if (_sceneExit1 != 0xFFFF)
				exitOk = true;
			break;

		case 1:
			if (_sceneExit2 != 0xFFFF)
				exitOk = true;
			break;

		case 2:
			if (_sceneExit3 != 0xFFFF)
				exitOk = true;
			break;

		case 3:
			if (_sceneExit4 != 0xFFFF)
				exitOk = true;
			break;

		default:
			break;
		}

		if (exitOk) {
			inputSceneChange(x, y, 1, 1);
			return;
		}
	}

	if (checkCharCollision(x, y) >= 0 && _unk3 >= -1) {
		runSceneScript2();
		return;
	} else if (pickUpItem(x, y)) {
		return;
	} else {
		int skipHandling = 0;

		if (checkItemCollision(x, y) == -1) {
			resetGameFlag(0x1EF);
			skipHandling = handleInputUnkSub(x, y) ? 1 : 0;

			if (queryGameFlag(0x1EF)) {
				resetGameFlag(0x1EF);
				return;
			}

			if (_unk5) {
				_unk5 = 0;
				return;
			}
		}

		if (_deathHandler > -1)
			skipHandling = 1;

		if (skipHandling)
			return;

		if (checkCharCollision(x, y) >= 0) {
			runSceneScript2();
			return;
		}

		if (_itemInHand >= 0) {
			if (y > 136)
				return;

			dropItem(0, _itemInHand, x, y, 1);
		} else {
			if (_unk3 == -2 || y > 135)
				return;

			if (!_unk5) {
				inputSceneChange(x, y, 1, 1);
				return;
			}

			_unk5 = 0;
		}
	}
}

bool KyraEngine_v2::handleInputUnkSub(int x, int y) {
	if (y > 143 || _deathHandler > -1 || queryGameFlag(0x164))
		return false;

	if (_handItemSet <= -3 && findItem(_mainCharacter.sceneId, 13) >= 0) {
		updateCharFacing();
		objectChat(getTableString(0xFC, _cCodeBuffer, 1), 0, 0x83, 0xFC);
		return true;
	} else {
		_scriptInterpreter->initScript(&_sceneScriptState, &_sceneScriptData);

		_sceneScriptState.regs[1] = x;
		_sceneScriptState.regs[2] = y;
		_sceneScriptState.regs[3] = 0;
		_sceneScriptState.regs[4] = _itemInHand;

		_scriptInterpreter->startScript(&_sceneScriptState, 1);

		while (_scriptInterpreter->validScript(&_sceneScriptState))
			_scriptInterpreter->runScript(&_sceneScriptState);

		//XXXsys_unkKeyboad (flush? wait? whatever...)

		if (queryGameFlag(0x1ED)) {
			_sound->beginFadeOut();
			_screen->fadeToBlack();
			_showCredits = true;
			_runFlag = false;
		}

		return _sceneScriptState.regs[3] != 0;
	}
}

void KyraEngine_v2::update() {
	updateInput();

	refreshAnimObjectsIfNeed();
	updateMouse();
	updateSpecialSceneScripts();
	_timer->update();
	updateItemAnimations();
	updateInvWsa();
	fadeMessagePalette();
	_screen->updateScreen();
}

void KyraEngine_v2::updateWithText() {
	updateInput();

	updateMouse();
	fadeMessagePalette();
	updateSpecialSceneScripts();
	_timer->update();
	updateItemAnimations();
	updateInvWsa();
	restorePage3();
	drawAnimObjects();

	if (textEnabled() && _chatText) {
		int pageBackUp = _screen->_curPage;
		_screen->_curPage = 2;
		objectChatPrintText(_chatText, _chatObject);
		_screen->_curPage = pageBackUp;
	}

	refreshAnimObjects(0);
	_screen->updateScreen();
}

void KyraEngine_v2::updateMouse() {
	int shapeIndex = 0;
	int type = 0;
	int xOffset = 0, yOffset = 0;
	Common::Point mouse = getMousePos();

	if (mouse.y <= 145) {
		if (mouse.x <= 6) {
			if (_sceneExit4 != 0xFFFF) {
				type = -3;
				shapeIndex = 4;
				xOffset = 1;
				yOffset = 5;
			} else {
				type = -2;
			}
		} else if (mouse.x >= 312) {
			if (_sceneExit2 != 0xFFFF) {
				type = -5;
				shapeIndex = 2;
				xOffset = 7;
				yOffset = 5;
			} else {
				type = -2;
			}
		} else if (mouse.y >= 135) {
			if (_sceneExit3 != 0xFFFF) {
				type = -4;
				shapeIndex = 3;
				xOffset = 5;
				yOffset = 10;
			} else {
				type = -2;
			}
		} else if (mouse.y <= 6) {
			if (_sceneExit1 != 0xFFFF) {
				type = -6;
				shapeIndex = 1;
				xOffset = 5;
				yOffset = 1;
			} else {
				type = -2;
			}
		}
	}

	for (int i = 0; i < _specialExitCount; ++i) {
		if (checkSpecialSceneExit(i, mouse.x, mouse.y)) {
			switch (_specialExitTable[20+i]) {
			case 0:
				type = -6;
				shapeIndex = 1;
				xOffset = 5;
				yOffset = 1;
				break;

			case 2:
				type = -5;
				shapeIndex = 2;
				xOffset = 7;
				yOffset = 5;
				break;

			case 4:
				type = -4;
				shapeIndex = 3;
				xOffset = 5;
				yOffset = 7;
				break;

			case 6:
				type = -3;
				shapeIndex = 4;
				xOffset = 1;
				yOffset = 5;
				break;

			default:
				break;
			}
		}
	}

	if (type == -2) {
		shapeIndex = 5;
		xOffset = 5;
		yOffset = 9;
	}

	if (type != 0 && _handItemSet != type && _screen->isMouseVisible()) {
		_mouseState = _handItemSet = type;
		_screen->hideMouse();
		_screen->setMouseCursor(xOffset, yOffset, getShapePtr(shapeIndex));
		_screen->showMouse();
	}

	if (type == 0 && _handItemSet != _itemInHand && _screen->isMouseVisible()) {
		if ((mouse.y > 145) || (mouse.x > 6 && mouse.x < 312 && mouse.y > 6 && mouse.y < 135)) {
			_mouseState = 0;
			_handItemSet = _itemInHand;
			_screen->hideMouse();
			if (_itemInHand == -1)
				_screen->setMouseCursor(0, 0, getShapePtr(0));
			else
				_screen->setMouseCursor(8, 15, getShapePtr(_itemInHand+64));
			_screen->showMouse();
		}
	}
}

void KyraEngine_v2::updateInput() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
			_quitFlag = true;
			break;

		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == '.' || event.kbd.keycode == Common::KEYCODE_ESCAPE)
				_eventList.push_back(Event(event, true));
			else if (event.kbd.keycode == 'q' && event.kbd.flags == Common::KBD_CTRL)
				_quitFlag = true;
			else
				_eventList.push_back(event);
			break;

		case Common::EVENT_LBUTTONUP:
			_eventList.push_back(Event(event, true));
			break;

		case Common::EVENT_MOUSEMOVE:
			_eventList.push_back(event);
			break;

		default:
			break;
		}
	}
}

int KyraEngine_v2::checkInput(Button *buttonList, bool mainLoop) {
	updateInput();

	int keys = 0;

	while (_eventList.size()) {
		Common::Event event = *_eventList.begin();
		bool breakLoop = false;

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode >= '1' && event.kbd.keycode <= '9' &&
					(event.kbd.flags == Common::KBD_CTRL || event.kbd.flags == Common::KBD_ALT) && mainLoop) {
				const char *saveLoadSlot = getSavegameFilename(event.kbd.keycode - '0');

				if (event.kbd.flags == Common::KBD_CTRL) {
					loadGame(saveLoadSlot);
					_eventList.clear();
					breakLoop = true;
				} else {
					char savegameName[14];
					sprintf(savegameName, "Quicksave %d",  event.kbd.keycode - '0');
					saveGame(saveLoadSlot, savegameName);
				}
			} else if (event.kbd.flags == Common::KBD_CTRL) {
				if (event.kbd.keycode == 'd')
					_debugger->attach();
			}
			break;

		case Common::EVENT_MOUSEMOVE: {
			Common::Point pos = getMousePos();
			_mouseX = pos.x;
			_mouseY = pos.y;
			_screen->updateScreen();
			} break;

		case Common::EVENT_LBUTTONUP: {
			Common::Point pos = getMousePos();
			_mouseX = pos.x;
			_mouseY = pos.y;
			keys = 199;
			breakLoop = true;
			} break;

		default:
			break;
		}

		if (_debugger->isAttached())
			_debugger->onFrame();

		if (breakLoop)
			break;

		_eventList.erase(_eventList.begin());
	}

	return _gui->processButtonList(buttonList, keys | 0x8000);
}

void KyraEngine_v2::removeInputTop() {
	if (!_eventList.empty())
		_eventList.erase(_eventList.begin());
}

bool KyraEngine_v2::skipFlag() const {
	for (Common::List<Event>::const_iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
		if (i->causedSkip)
			return true;
	}
	return false;
}

void KyraEngine_v2::resetSkipFlag(bool removeEvent) {
	for (Common::List<Event>::iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
		if (i->causedSkip) {
			if (removeEvent)
				_eventList.erase(i);
			else
				i->causedSkip = false;
			return;
		}
	}
}

void KyraEngine_v2::delay(uint32 amount, bool updateGame, bool isMainLoop) {
	uint32 start = _system->getMillis();
	do {
		if (updateGame) {
			if (_chatText)
				updateWithText();
			else
				update();
		} else {
			updateInput();
		}

		if (amount > 0)
			_system->delayMillis(amount > 10 ? 10 : amount);
	} while (!skipFlag() && _system->getMillis() < start + amount && !_quitFlag);
}

void KyraEngine_v2::cleanup() {
	delete [] _inventoryButtons; _inventoryButtons = 0;

	delete [] _gamePlayBuffer; _gamePlayBuffer = 0;
	delete [] _unkBuf500Bytes; _unkBuf500Bytes = 0;
	delete [] _screenBuffer; _screenBuffer = 0;
	delete [] _unkBuf200kByte; _unkBuf200kByte = 0;

	resetNewShapes(_newShapeCount, _newShapeFiledata);
	_newShapeFiledata = 0;
	_newShapeCount = 0;

	for (int i = 0; i < ARRAYSIZE(_defaultShapeTable); ++i) {
		delete [] _defaultShapeTable[i];
		_defaultShapeTable[i] = 0;
	}
	freeSceneShapePtrs();

	if (_optionsBuffer != _cCodeBuffer)
		delete [] _optionsBuffer;
	_optionsBuffer = 0;
	delete [] _cCodeBuffer; _cCodeBuffer = 0;
	delete [] _chapterBuffer; _chapterBuffer = 0;

	delete [] _talkObjectList; _talkObjectList = 0;
	delete [] _shapeDescTable; _shapeDescTable = 0;

	delete [] _gfxBackUpRect; _gfxBackUpRect = 0;

	delete [] _sceneList; _sceneList = 0;

	for (int i = 0; i < ARRAYSIZE(_sceneAnimMovie); ++i) {
		delete _sceneAnimMovie[i];
		_sceneAnimMovie[i] = 0;
	}
	for (int i = 0; i < ARRAYSIZE(_wsaSlots); ++i) {
		delete _wsaSlots[i];
		_wsaSlots[i] = 0;
	}
	for (int i = 0; i < ARRAYSIZE(_buttonShapes); ++i) {
		delete [] _buttonShapes[i];
		_buttonShapes[i] = 0;
	}
}

#pragma mark - Localization

void KyraEngine_v2::loadCCodeBuffer(const char *file) {
	char tempString[13];
	strcpy(tempString, file);
	changeFileExtension(tempString);

	delete [] _cCodeBuffer;
	_cCodeBuffer = _res->fileData(tempString, 0);
}

void KyraEngine_v2::loadOptionsBuffer(const char *file) {
	char tempString[13];
	strcpy(tempString, file);
	changeFileExtension(tempString);

	delete [] _optionsBuffer;
	_optionsBuffer = _res->fileData(tempString, 0);
}

void KyraEngine_v2::loadChapterBuffer(int chapter) {
	char tempString[14];

	static const char *chapterFilenames[] = {
		"CH1.XXX", "CH2.XXX", "CH3.XXX", "CH4.XXX", "CH5.XXX"
	};

	assert(chapter >= 1 && chapter <= ARRAYSIZE(chapterFilenames));
	strcpy(tempString, chapterFilenames[chapter-1]);
	changeFileExtension(tempString);

	delete [] _chapterBuffer;
	_chapterBuffer = _res->fileData(tempString, 0);
	_currentChapter = chapter;
}

void KyraEngine_v2::changeFileExtension(char *buffer) {
	while (*buffer != '.')
		++buffer;

	++buffer;
	strcpy(buffer, _languageExtension[_lang]);
}

uint8 *KyraEngine_v2::getTableEntry(uint8 *buffer, int id) {
	return buffer + READ_LE_UINT16(buffer + (id<<1));
}

char *KyraEngine_v2::getTableString(int id, uint8 *buffer, int decode) {
	char *string = (char*)getTableEntry(buffer, id);

	if (decode && _flags.lang != Common::JA_JPN) {
		decodeString1(string, _internStringBuf);
		decodeString2(_internStringBuf, _internStringBuf);
		string = _internStringBuf;
	}

	return string;
}

const char *KyraEngine_v2::getChapterString(int id) {
	if (_currentChapter != _newChapterFile)
		loadChapterBuffer(_newChapterFile);

	return getTableString(id, _chapterBuffer, 1);
}

int KyraEngine_v2::decodeString1(const char *src, char *dst) {
	static const uint8 decodeTable1[] = {
		0x20, 0x65, 0x74, 0x61, 0x69, 0x6E, 0x6F, 0x73, 0x72, 0x6C, 0x68,
		0x63, 0x64, 0x75, 0x70, 0x6D
	};

	static const uint8 decodeTable2[] = {
		0x74, 0x61, 0x73, 0x69, 0x6F, 0x20, 0x77, 0x62, 0x20, 0x72, 0x6E,
		0x73, 0x64, 0x61, 0x6C, 0x6D, 0x68, 0x20, 0x69, 0x65, 0x6F, 0x72,
		0x61, 0x73, 0x6E, 0x72, 0x74, 0x6C, 0x63, 0x20, 0x73, 0x79, 0x6E,
		0x73, 0x74, 0x63, 0x6C, 0x6F, 0x65, 0x72, 0x20, 0x64, 0x74, 0x67,
		0x65, 0x73, 0x69, 0x6F, 0x6E, 0x72, 0x20, 0x75, 0x66, 0x6D, 0x73,
		0x77, 0x20, 0x74, 0x65, 0x70, 0x2E, 0x69, 0x63, 0x61, 0x65, 0x20,
		0x6F, 0x69, 0x61, 0x64, 0x75, 0x72, 0x20, 0x6C, 0x61, 0x65, 0x69,
		0x79, 0x6F, 0x64, 0x65, 0x69, 0x61, 0x20, 0x6F, 0x74, 0x72, 0x75,
		0x65, 0x74, 0x6F, 0x61, 0x6B, 0x68, 0x6C, 0x72, 0x20, 0x65, 0x69,
		0x75, 0x2C, 0x2E, 0x6F, 0x61, 0x6E, 0x73, 0x72, 0x63, 0x74, 0x6C,
		0x61, 0x69, 0x6C, 0x65, 0x6F, 0x69, 0x72, 0x61, 0x74, 0x70, 0x65,
		0x61, 0x6F, 0x69, 0x70, 0x20, 0x62, 0x6D
	};

	int size = 0;
	uint cChar = 0;
	while ((cChar = *src++) != 0) {
		if (cChar & 0x80) {
			cChar &= 0x7F;
			int index = (cChar & 0x78) >> 3;
			*dst++ = decodeTable1[index];
			++size;
			assert(cChar < sizeof(decodeTable2));
			cChar = decodeTable2[cChar];
		}

		*dst++ = cChar;
		++size;
	}

	*dst++ = 0;
	return size;
}

void KyraEngine_v2::decodeString2(const char *src, char *dst) {
	if (!src || !dst)
		return;

	char out = 0;
	while ((out = *src) != 0) {
		if (*src == 0x1B) {
			++src;
			out = *src + 0x7F;
		}
		*dst++ = out;
		++src;
	}

	*dst = 0;
}

#pragma mark -

void KyraEngine_v2::showMessageFromCCode(int id, int16 palIndex, int) {
	const char *string = getTableString(id, _cCodeBuffer, 1);
	showMessage(string, palIndex);
}

void KyraEngine_v2::showMessage(const char *string, int16 palIndex) {
	_shownMessage = string;
	_screen->hideMouse();
	_screen->fillRect(0, 190, 319, 199, 0xCF);

	if (string) {
		if (palIndex != -1 || _fadeMessagePalette) {
			palIndex *= 3;
			memcpy(_messagePal, _screen->_currentPalette + palIndex, 3);
			memmove(_screen->_currentPalette + 765, _screen->_currentPalette + palIndex, 3);
			_screen->setScreenPalette(_screen->_currentPalette);
		}

		int x = _text->getCenterStringX(string, 0, 320);
		_text->printText(string, x, 190, 255, 207, 0);

		setTimer1DelaySecs(7);
	}

	_fadeMessagePalette = false;
	_screen->showMouse();
}

void KyraEngine_v2::showChapterMessage(int id, int16 palIndex) {
	showMessage(getChapterString(id), palIndex);
}

void KyraEngine_v2::updateCommandLineEx(int str1, int str2, int16 palIndex) {
	char buffer[0x51];
	char *src = buffer;

	strcpy(src, getTableString(str1, _cCodeBuffer, 1));

	if (_flags.lang != Common::JA_JPN) {
		while (*src != 0x20)
			++src;
		++src;
		*src = toupper(*src);
	}

	strcpy((char*)_unkBuf500Bytes, src);

	if (str2 > 0) {
		if (_flags.lang != Common::JA_JPN)
			strcat((char*)_unkBuf500Bytes, " ");
		strcat((char*)_unkBuf500Bytes, getTableString(str2, _cCodeBuffer, 1));
	}

	showMessage((char*)_unkBuf500Bytes, palIndex);
}

void KyraEngine_v2::fadeMessagePalette() {
	if (!_fadeMessagePalette)
		return;

	bool updatePalette = false;
	for (int i = 0; i < 3; ++i) {
		if (_messagePal[i] >= 4) {
			_messagePal[i] -= 4;
			updatePalette = true;
		} else if (_messagePal[i] != 0) {
			_messagePal[i] = 0;
			updatePalette = true;
		}
	}

	if (updatePalette) {
		memcpy(_screen->getPalette(0) + 765, _messagePal, 3);
		_screen->setScreenPalette(_screen->getPalette(0));
	} else {
		_fadeMessagePalette = false;
	}
}

#pragma mark -

void KyraEngine_v2::loadMouseShapes() {
	_screen->loadBitmap("_MOUSE.CSH", 3, 3, 0);

	for (int i = 0; i <= 8; ++i) {
		_defaultShapeTable[i] = _screen->makeShapeCopy(_screen->getCPagePtr(3), i);
		assert(_defaultShapeTable[i]);
	}
}

void KyraEngine_v2::loadItemShapes() {
	_screen->loadBitmap("_ITEMS.CSH", 3, 3, 0);

	for (int i = 64; i <= 239; ++i) {
		_defaultShapeTable[i] = _screen->makeShapeCopy(_screen->getCPagePtr(3), i-64);
		assert(_defaultShapeTable[i]);
	}

	_res->loadFileToBuf("_ITEMHT.DAT", _itemHtDat, sizeof(_itemHtDat));
	assert(_res->getFileSize("_ITEMHT.DAT") == sizeof(_itemHtDat));

	_screen->_curPage = 0;
}

void KyraEngine_v2::loadZShapes(int shapes) {
	char file[10];
	strcpy(file, "_ZX.SHP");

	_loadedZTable = shapes;
	file[2] = '0' + shapes;

	uint8 *data = _res->fileData(file, 0);
	for (int i = 9; i <= 32; ++i) {
		delete [] _defaultShapeTable[i];
		_defaultShapeTable[i] = _screen->makeShapeCopy(data, i-9);
		assert(_defaultShapeTable[i]);
	}
	delete [] data;

	_loadedZTable = shapes;
}

void KyraEngine_v2::loadInventoryShapes() {
	int curPageBackUp = _screen->_curPage;
	_screen->_curPage = 2;

	_screen->loadBitmap("_PLAYALL.CPS", 3, 3, 0);

	for (int i = 0; i < 10; ++i) {
		_defaultShapeTable[240+i] = _screen->encodeShape(_inventoryX[i], _inventoryY[i], 16, 16, 0);
		assert(_defaultShapeTable[240+i]);
	}

	_screen->_curPage = curPageBackUp;
}

void KyraEngine_v2::runStartScript(int script, int unk1) {
	char filename[14];
	strcpy(filename, "_START0X.EMC");
	filename[7] = script + '0';

	ScriptData scriptData;
	ScriptState scriptState;

	_scriptInterpreter->loadScript(filename, &scriptData, &_opcodes);
	_scriptInterpreter->initScript(&scriptState, &scriptData);
	scriptState.regs[6] = unk1;
	_scriptInterpreter->startScript(&scriptState, 0);
	while (_scriptInterpreter->validScript(&scriptState))
		_scriptInterpreter->runScript(&scriptState);
	_scriptInterpreter->unloadScript(&scriptData);
}

void KyraEngine_v2::loadNPCScript() {
	char filename[12];
	strcpy(filename, "_NPC.EMC");

	if (_flags.platform != Common::kPlatformPC || _flags.isTalkie) {
		switch (_lang) {
		case 0:
			filename[5] = 'E';
			break;

		case 1:
			filename[5] = 'F';
			break;

		case 2:
			filename[5] = 'G';
			break;

		case 3:
			filename[5] = 'J';
			break;

		default:
			break;
		};
	}

	_scriptInterpreter->loadScript(filename, &_npcScriptData, &_opcodes);
}

void KyraEngine_v2::runTemporaryScript(const char *filename, int unk1, int unk2, int newShapes, int shapeUnload) {
	memset(&_temporaryScriptData, 0, sizeof(_temporaryScriptData));
	memset(&_temporaryScriptState, 0, sizeof(_temporaryScriptState));

	if (!_scriptInterpreter->loadScript(filename, &_temporaryScriptData, &_opcodesTemporary))
		error("couldn't load temporary script '%s'", filename);

	_scriptInterpreter->initScript(&_temporaryScriptState, &_temporaryScriptData);
	_scriptInterpreter->startScript(&_temporaryScriptState, 0);

	_newShapeFlag = -1;

	while (_scriptInterpreter->validScript(&_temporaryScriptState))
		_scriptInterpreter->runScript(&_temporaryScriptState);

	uint8 *fileData = 0;

	if (newShapes) {
		_newShapeFiledata = _res->fileData(_newShapeFilename, 0);
		assert(_newShapeFiledata);
	}

	fileData = _newShapeFiledata;

	if (!fileData)
		return;

	if (newShapes)
		_newShapeCount = initNewShapes(fileData);

	processNewShapes(unk1, unk2);

	if (shapeUnload) {
		resetNewShapes(_newShapeCount, fileData);
		_newShapeCount = 0;
		_newShapeFiledata = 0;
	}

	_scriptInterpreter->unloadScript(&_temporaryScriptData);
}

#pragma mark -

void KyraEngine_v2::resetScaleTable() {
	Common::set_to(_scaleTable, _scaleTable + ARRAYSIZE(_scaleTable), 0x100);
}

void KyraEngine_v2::setScaleTableItem(int item, int data) {
	if (item >= 1 || item <= 15)
		_scaleTable[item-1] = (data << 8) / 100;
}

int KyraEngine_v2::getScale(int x, int y) {
	return _scaleTable[_screen->getLayer(x, y) - 1];
}

void KyraEngine_v2::setDrawLayerTableEntry(int entry, int data) {
	if (entry >= 1 || entry <= 15)
		_drawLayerTable[entry-1] = data;
}

int KyraEngine_v2::getDrawLayer(int x, int y) {
	int layer = _screen->getLayer(x, y);
	layer = _drawLayerTable[layer-1];
	if (layer < 0)
		layer = 0;
	else if (layer >= 7)
		layer = 6;
	return layer;
}

void KyraEngine_v2::restorePage3() {
	_screen->copyBlockToPage(2, 0, 0, 320, 144, _gamePlayBuffer);
}

void KyraEngine_v2::backUpPage0() {
	if (_screenBuffer) {
		_screen->hideMouse();
		memcpy(_screenBuffer, _screen->getCPagePtr(0), 64000);
		_screen->showMouse();
	}
}

void KyraEngine_v2::restorePage0() {
	restorePage3();
	if (_screenBuffer)
		_screen->copyBlockToPage(0, 0, 0, 320, 200, _screenBuffer);
}

void KyraEngine_v2::updateCharPal(int unk1) {
	static bool unkVar1 = false;

	if (!_useCharPal)
		return;

	int layer = _screen->getLayer(_mainCharacter.x1, _mainCharacter.y1);
	int palEntry = _charPalTable[layer];

	if (palEntry != _charPalEntry && unk1) {
		const uint8 *src = &_scenePal[(palEntry << 4) * 3];
		uint8 *ptr = _screen->getPalette(0) + 336;
		for (int i = 0; i < 48; ++i) {
			*ptr -= (*ptr - *src) >> 1;
			++ptr;
			++src;
		}
		_screen->setScreenPalette(_screen->getPalette(0));
		unkVar1 = true;
		_charPalEntry = palEntry;
	} else if (unkVar1 || !unk1) {
		memcpy(_screen->getPalette(0) + 336, &_scenePal[(palEntry << 4) * 3], 48);
		_screen->setScreenPalette(_screen->getPalette(0));
		unkVar1 = false;
	}
}

void KyraEngine_v2::setCharPalEntry(int entry, int value) {
	if (entry > 15 || entry < 1)
		entry = 1;
	if (value > 8 || value < 0)
		value = 0;
	_charPalTable[entry] = value;
	_useCharPal = 1;
	_charPalEntry = 0;
}

int KyraEngine_v2::inputSceneChange(int x, int y, int unk1, int unk2) {
	bool refreshNPC = false;
	uint16 curScene = _mainCharacter.sceneId;
	_pathfinderFlag = 15;

	if (!_unkHandleSceneChangeFlag) {
		if (_unk3 == -3) {
			if (_sceneList[curScene].exit4 != 0xFFFF) {
				x = 4;
				y = _sceneEnterY4;
				_pathfinderFlag = 7;
			}
		} else if (_unk3 == -5) {
			if (_sceneList[curScene].exit2 != 0xFFFF) {
				x = 316;
				y = _sceneEnterY2;
				_pathfinderFlag = 7;
			}
		} else if (_unk3 == -6) {
			if (_sceneList[curScene].exit1 != 0xFFFF) {
				x = _sceneEnterX1;
				y = _sceneEnterY1 - 2;
				_pathfinderFlag = 14;
			}
		} else if (_unk3 == -4) {
			if (_sceneList[curScene].exit3 != 0xFFFF) {
				x = _sceneEnterX3;
				y = 147;
				_pathfinderFlag = 11;
			}
		}
	}

	int strId = 0;
	int vocH = _flags.isTalkie ? 131 : -1;

	if (_pathfinderFlag) {
		if (findItem(curScene, 13) >= 0 && _unk3 <= -3) {			
			strId = 252;
		} else if (_itemInHand == 72) {
			strId = 257;
		} else if (findItem(curScene, 72) >= 0 && _unk3 <= -3) {
			strId = 256;
		} else if (getInventoryItemSlot(72) != -1 && _unk3 <= -3) {
			strId = 257;
		}
	}

	if (strId) {
		updateCharFacing();
		objectChat(getTableString(strId, _cCodeBuffer, 1), 0, vocH, strId);
		_pathfinderFlag = 0;
		return 0;
	}

	if (ABS(_mainCharacter.x1 - x) < 4 && ABS(_mainCharacter.y1 - y) < 2) {
		_pathfinderFlag = 0;
		return 0;
	}

	int curX = _mainCharacter.x1 & ~3;
	int curY = _mainCharacter.y1 & ~1;
	int dstX = x & ~3;
	int dstY = y & ~1;

	int wayLength = findWay(curX, curY, dstX, dstY, _movFacingTable, 600);
	_pathfinderFlag = 0;
	_timer->disable(5);

	if (wayLength != 0 && wayLength != 0x7D00)
		refreshNPC = (trySceneChange(_movFacingTable, unk1, unk2) != 0);

	int charLayer = _screen->getLayer(_mainCharacter.x1, _mainCharacter.y1);
	if (_layerFlagTable[charLayer] != 0 && !queryGameFlag(0x163)) {
		if (queryGameFlag(0x164)) {
			_screen->hideMouse();
			_timer->disable(5);
			runTemporaryScript("_ZANBURN.EMC", 0, 1, 1, 0);
			_deathHandler = 7;
			snd_playWanderScoreViaMap(0x53, 1);
		} else {
			objectChat(getTableString(0xFD, _cCodeBuffer, 1), 0, 0x83, 0xFD);
			setGameFlag(0x164);
			_timer->enable(5);
			_timer->setCountdown(5, 120);
		}
	} else if (queryGameFlag(0x164)) {
		objectChat(getTableString(0xFE, _cCodeBuffer, 1), 0, 0x83, 0xFE);
		resetGameFlag(0x164);
		_timer->disable(5);
	}

	if (refreshNPC)
		enterNewSceneUnk2(0);

	_pathfinderFlag = 0;
	return refreshNPC;
}

bool KyraEngine_v2::checkSpecialSceneExit(int num, int x, int y) {
	if (_specialExitTable[0+num] > x || _specialExitTable[5+num] > y ||
		_specialExitTable[10+num] < x || _specialExitTable[15+num] < y)
		return 0;
	return 1;
}

void KyraEngine_v2::moveCharacter(int facing, int x, int y) {
	_mainCharacter.facing = facing;
	x &= ~3;
	y &= ~1;

	_screen->hideMouse();
	switch (facing) {
	case 0:
		while (y < _mainCharacter.y1)
			updateCharPosWithUpdate();
		break;

	case 2:
		while (_mainCharacter.x1 < x)
			updateCharPosWithUpdate();
		break;

	case 4:
		while (y > _mainCharacter.y1)
			updateCharPosWithUpdate();
		break;

	case 6:
		while (_mainCharacter.x1 > x)
			updateCharPosWithUpdate();
		break;

	default:
		break;
	}

	_screen->showMouse();
}

int KyraEngine_v2::updateCharPos(int *table) {
	static uint32 nextUpdate = 0;
	static const int updateX[] = { 0, 4, 4, 4, 0, -4, -4, -4 };
	static const int updateY[] = { -2, -2, 0, 2, 2, 2, 0, -2 };

	if (_system->getMillis() < nextUpdate)
		return 0;

	int facing = _mainCharacter.facing;
	_mainCharacter.x1 += updateX[facing];
	_mainCharacter.y1 += updateY[facing];
	updateCharAnimFrame(0, table);
	nextUpdate = _system->getMillis() + _timer->getDelay(0) * _tickLength;
	return 1;
}

void KyraEngine_v2::updateCharPosWithUpdate() {
	updateCharPos(0);
	update();
}

void KyraEngine_v2::updateCharAnimFrame(int charId, int *table) {
	static int unkTable1[] = { 0, 0 };
	static const int unkTable2[] = { 17, 0 };
	static const int unkTable3[] = { 10, 0 };
	static const int unkTable4[] = { 24, 0 };
	static const int unkTable5[] = { 19, 0 };
	static const int unkTable6[] = { 21, 0 };
	static const int unkTable7[] = { 31, 0 };
	static const int unkTable8[] = { 26, 0 };

	Character *character = &_mainCharacter;
	++character->animFrame;

	int facing = character->facing;

	if (table) {
		if (table[0] != table[-1] && table[-1] == table[1]) {
			facing = getOppositeFacingDirection(table[-1]);
			table[0] = table[-1];
		}
	}

	if (!facing) {
		++unkTable1[charId];
	} else if (facing == 4) {
		++unkTable1[charId+1];
	} else if (facing == 7 || facing == 1 || facing == 5 || facing == 3) {
		if (facing == 7 || facing == 1) {
			if (unkTable1[charId] > 2)
				facing = 0;
		} else {
			if (unkTable1[charId+1] > 2)
				facing = 4;
		}

		unkTable1[charId] = 0;
		unkTable1[charId+1] = 0;
	}

	if (facing == 0) {
		if (character->animFrame < unkTable8[charId])
			character->animFrame = unkTable8[charId];

		if (character->animFrame > unkTable7[charId])
			character->animFrame = unkTable8[charId];
	} else if (facing == 4) {
		if (character->animFrame < unkTable5[charId])
			character->animFrame = unkTable5[charId];

		if (character->animFrame > unkTable4[charId])
			character->animFrame = unkTable5[charId];
	} else {
		if (character->animFrame > unkTable5[charId])
			character->animFrame = unkTable6[charId];

		if (character->animFrame == unkTable2[charId])
			character->animFrame = unkTable3[charId];

		if (character->animFrame > unkTable2[charId])
			character->animFrame = unkTable3[charId] + 2;
	}

	updateCharacterAnim(charId);
}

int KyraEngine_v2::checkCharCollision(int x, int y) {
	int scale1 = 0, scale2 = 0, scale3 = 0;
	int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	scale1 = getScale(_mainCharacter.x1, _mainCharacter.y1);
	scale2 = (scale1 * 24) >> 8;
	scale3 = (scale1 * 48) >> 8;

	x1 = _mainCharacter.x1 - (scale2 >> 1);
	x2 = _mainCharacter.x1 + (scale2 >> 1);
	y1 = _mainCharacter.y1 - scale3;
	y2 = _mainCharacter.y1;

	if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
		return 0;

	return -1;
}

int KyraEngine_v2::initNewShapes(uint8 *filedata) {
	const int lastEntry = MIN(_newShapeLastEntry, 31);
	for (int i = 0; i < lastEntry; ++i) {
		_defaultShapeTable[33+i] = _screen->getPtrToShape(filedata, i);
		ShapeDesc *desc = &_shapeDescTable[24+i];
		desc->xAdd = _newShapeXAdd;
		desc->yAdd = _newShapeYAdd;
		desc->width = _newShapeWidth;
		desc->height = _newShapeHeight;
	}
	return lastEntry;
}

void KyraEngine_v2::processNewShapes(int unk1, int unk2) {
	setCharacterAnimDim(_newShapeWidth, _newShapeHeight);

	_scriptInterpreter->initScript(&_temporaryScriptState, &_temporaryScriptData);
	_scriptInterpreter->startScript(&_temporaryScriptState, 1);

	resetSkipFlag();

	while (_scriptInterpreter->validScript(&_temporaryScriptState)) {
		_temporaryScriptExecBit = false;
		while (_scriptInterpreter->validScript(&_temporaryScriptState) && !_temporaryScriptExecBit)
			_scriptInterpreter->runScript(&_temporaryScriptState);

		if (_newShapeAnimFrame < 0)
			continue;

		_mainCharacter.animFrame = _newShapeAnimFrame + 33;
		updateCharacterAnim(0);
		if (_chatText)
			updateWithText();
		else
			update();

		uint32 delayEnd = _system->getMillis() + _newShapeDelay * _tickLength;

		while (!skipFlag() && _system->getMillis() < delayEnd) {
			// XXX skipFlag handling, unk1 seems to make a scene not skipable

			if (_chatText)
				updateWithText();
			else
				update();

			delay(10);
		}
	}

	if (unk2) {
		if (_newShapeFlag >= 0) {
			_mainCharacter.animFrame = _newShapeFlag + 33;
			updateCharacterAnim(0);
			if (_chatText)
				updateWithText();
			else
				update();
		}

		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
		updateCharacterAnim(0);
	}

	resetSkipFlag();

	_newShapeFlag = -1;
	resetCharacterAnimDim();
}

void KyraEngine_v2::resetNewShapes(int count, uint8 *filedata) {
	Common::set_to(_defaultShapeTable+33, _defaultShapeTable+33+count, (uint8*)0);
	delete [] filedata;
	setNextIdleAnimTimer();
}

void KyraEngine_v2::setNextIdleAnimTimer() {
	_nextIdleAnim = _system->getMillis() + _rnd.getRandomNumberRng(10, 15) * 60 * _tickLength;
}

void KyraEngine_v2::showIdleAnim() {
	static const uint8 scriptMinTable[] = {
		0x00, 0x05, 0x07, 0x08, 0x00, 0x09, 0x0A, 0x0B, 0xFF, 0x00
	};

	static const uint8 scriptMaxTable[] = {
		0x04, 0x06, 0x07, 0x08, 0x04, 0x09, 0x0A, 0x0B, 0xFF, 0x00
	};

	if (queryGameFlag(0x159) && _flags.isTalkie)
		return;

	static bool scriptAnimation = false;
	if (!scriptAnimation && _flags.isTalkie) {
		scriptAnimation = true;
		zanthRandomIdleChat();
	} else {
		scriptAnimation = false;
		if (_loadedZTable > 8)
			return;

		int scriptMin = scriptMinTable[_loadedZTable-1];
		int scriptMax = scriptMaxTable[_loadedZTable-1];
		int script = 0;

		if (scriptMin < scriptMax) {
			do {
				script = _rnd.getRandomNumberRng(scriptMin, scriptMax);
			} while (script == _lastIdleScript);
		} else {
			script = scriptMin;
		}

		runIdleScript(script);
		_lastIdleScript = script;
	}
}

void KyraEngine_v2::runIdleScript(int script) {
	if (script < 0 || script >= 12)
		script = 0;

	if (_mainCharacter.animFrame != 18) {
		setNextIdleAnimTimer();
	} else {
		// FIXME: move this to staticres.cpp?
		static const char *idleScriptFiles[] = {
			"_IDLHAIR.EMC", "_IDLDUST.EMC", "_IDLLEAN.EMC", "_IDLDIRT.EMC", "_IDLTOSS.EMC", "_IDLNOSE.EMC",
			"_IDLBRSH.EMC", "_Z3IDLE.EMC", "_Z4IDLE.EMC", "_Z6IDLE.EMC", "_Z7IDLE.EMC", "_Z8IDLE.EMC"
		};

		runTemporaryScript(idleScriptFiles[script], 1, 1, 1, 1);
	}
}

#pragma mark -

void KyraEngine_v2::backUpGfxRect24x24(int x, int y) {
	_screen->copyRegionToBuffer(_screen->_curPage, x, y, 24, 24, _gfxBackUpRect);
}

void KyraEngine_v2::restoreGfxRect24x24(int x, int y) {
	_screen->copyBlockToPage(_screen->_curPage, x, y, 24, 24, _gfxBackUpRect);
}

void KyraEngine_v2::backUpGfxRect32x32(int x, int y) {
	_screen->copyRegionToBuffer(_screen->_curPage, x, y, 32, 32, _gfxBackUpRect);
}

void KyraEngine_v2::restoreGfxRect32x32(int x, int y) {
	_screen->copyBlockToPage(_screen->_curPage, x, y, 32, 32, _gfxBackUpRect);
}

#pragma mark -

void KyraEngine_v2::openTalkFile(int newFile) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v2::openTalkFile(%d)", newFile);
	char talkFilename[16];

	if (_oldTalkFile > 0) {
		sprintf(talkFilename, "CH%dVOC.TLK", _oldTalkFile);
		_res->unloadPakFile(talkFilename);
		_oldTalkFile = -1;
	}

	if (newFile == 0) {
		strcpy(talkFilename, "ANYTALK.TLK");
		_res->loadPakFile(talkFilename);
	} else {
		sprintf(talkFilename, "CH%dVOC.TLK", newFile);
		_res->loadPakFile(talkFilename);
	}

	_oldTalkFile = newFile;
}

void KyraEngine_v2::snd_playVoiceFile(int id) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v2::snd_playVoiceFile(%d)", id);
	char vocFile[9];
	assert(id >= 0 && id <= 9999999);
	sprintf(vocFile, "%07d", id);
	if (_sound->voiceFileIsPresent(vocFile)) {
		snd_stopVoice();

		while (!_sound->voicePlay(vocFile)) {
			updateWithText();
			_system->delayMillis(10);
		}

		_speechFile = vocFile;
	}
}

void KyraEngine_v2::snd_loadSoundFile(int id) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v2::snd_loadSoundFile(%d)", id);
	if (id < 0 || !_trackMap)
		return;

	assert(id < _trackMapSize);
	int file = _trackMap[id*2];
	_curSfxFile = _curMusicTheme = file;
	_sound->loadSoundFile(file);
}

void KyraEngine_v2::playVoice(int high, int low) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v2::playVoice(%d, %d)", high, low);
	if (!_flags.isTalkie)
		return;
	int vocFile = high * 10000 + low * 10;
	snd_playVoiceFile(vocFile);
}

void KyraEngine_v2::snd_playSoundEffect(int track) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "KyraEngine_v2::snd_playSoundEffect(%d)", track);

	if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98) {
		if (track == 10)
			track = _lastSfxTrack;

		if (track == 10 || track == -1)
			return;

		_lastSfxTrack = track;
	}

	int16 vocIndex = (int16)READ_LE_UINT16(&_ingameSoundIndex[track * 2]);
	if (vocIndex != -1)
		_sound->voicePlay(_ingameSoundList[vocIndex], true);
	else if (_flags.platform == Common::kPlatformPC)
		// TODO ?? Maybe there is a way to let users select whether they want
		// voc, midi or adl sfx (even though it makes no sense to choose anything but voc).
		// For now this is used as a fallback only (if no voc file exists).
		KyraEngine::snd_playSoundEffect(track);
}

#pragma mark -

void KyraEngine_v2::loadInvWsa(const char *filename, int run, int delayTime, int page, int sfx, int sFrame, int flags) {
	int wsaFlags = 1;
	if (flags)
		wsaFlags |= 2;

	if (!_invWsa.wsa)
		_invWsa.wsa = new WSAMovieV2(this);

	if (!_invWsa.wsa->open(filename, wsaFlags, 0))
		error("Couldn't open inventory WSA file '%s'", filename);

	_invWsa.curFrame = 0;
	_invWsa.lastFrame = _invWsa.wsa->frames();

	_invWsa.x = _invWsa.wsa->xAdd();
	_invWsa.y = _invWsa.wsa->yAdd();
	_invWsa.w = _invWsa.wsa->width();
	_invWsa.h = _invWsa.wsa->height();
	_invWsa.x2 = _invWsa.x + _invWsa.w - 1;
	_invWsa.y2 = _invWsa.y + _invWsa.h - 1;

	_invWsa.delay = delayTime;
	_invWsa.page = page;
	_invWsa.sfx = sfx;

	_invWsa.specialFrame = sFrame;

	if (_invWsa.page)
		_screen->copyRegion(_invWsa.x, _invWsa.y, _invWsa.x, _invWsa.y, _invWsa.w, _invWsa.h, 0, _invWsa.page, Screen::CR_NO_P_CHECK);

	_invWsa.running = true;
	_invWsa.timer = _system->getMillis();

	if (run) {
		while (_invWsa.running && !skipFlag() && !_quitFlag) {
			update();
			_system->delayMillis(10);
		}

		if (skipFlag()) {
			resetSkipFlag();
			displayInvWsaLastFrame();
		}
	}
}

void KyraEngine_v2::closeInvWsa() {
	_invWsa.wsa->close();
	delete _invWsa.wsa;
	_invWsa.wsa = 0;
	_invWsa.running = false;
}

void KyraEngine_v2::updateInvWsa() {
	if (!_invWsa.running || !_invWsa.wsa)
		return;

	if (_invWsa.timer > _system->getMillis())
		return;

	_invWsa.wsa->setX(0);
	_invWsa.wsa->setY(0);
	_invWsa.wsa->setDrawPage(_invWsa.page);
	_invWsa.wsa->displayFrame(_invWsa.curFrame, 0, 0, 0);

	if (_invWsa.page)
		_screen->copyRegion(_invWsa.x, _invWsa.y, _invWsa.x, _invWsa.y, _invWsa.w, _invWsa.h, _invWsa.page, 0, Screen::CR_NO_P_CHECK);

	_invWsa.timer = _system->getMillis() + _invWsa.delay * _tickLength;

	++_invWsa.curFrame;
	if (_invWsa.curFrame >= _invWsa.lastFrame)
		displayInvWsaLastFrame();

	if (_invWsa.curFrame == _invWsa.specialFrame)
		snd_playSoundEffect(_invWsa.sfx);

	if (_invWsa.sfx == -2) {
		switch (_invWsa.curFrame) {
		case 9: case 27: case 40:
			snd_playSoundEffect(0x39);
			break;

		case 18: case 34: case 44:
			snd_playSoundEffect(0x33);
			break;

		case 48:
			snd_playSoundEffect(0x38);
			break;

		default:
			break;
		}
	}
}

void KyraEngine_v2::displayInvWsaLastFrame() {
	if (!_invWsa.wsa)
		return;

	_invWsa.wsa->setX(0);
	_invWsa.wsa->setY(0);
	_invWsa.wsa->setDrawPage(_invWsa.page);
	_invWsa.wsa->displayFrame(_invWsa.lastFrame-1, 0, 0, 0);

	if (_invWsa.page)
		_screen->copyRegion(_invWsa.x, _invWsa.y, _invWsa.x, _invWsa.y, _invWsa.w, _invWsa.h, _invWsa.page, 0, Screen::CR_NO_P_CHECK);

	closeInvWsa();

	int32 countdown = _rnd.getRandomNumberRng(45, 80);
	_timer->setCountdown(2, countdown * 60);
}

#pragma mark -

void KyraEngine_v2::setCauldronState(uint8 state, bool paletteFade) {
	memcpy(_screen->getPalette(2), _screen->getPalette(0), 768);
	Common::SeekableReadStream *file = _res->getFileStream("_POTIONS.PAL");
	if (!file)
		error("Couldn't load cauldron palette");
	file->seek(state*18, SEEK_SET);
	file->read(_screen->getPalette(2)+723, 18);
	delete file;
	file = 0;

	if (paletteFade) {
		snd_playSoundEffect((state == 0) ? 0x6B : 0x66);
		_screen->fadePalette(_screen->getPalette(2), 0x4B, &_updateFunctor);
	} else {
		_screen->setScreenPalette(_screen->getPalette(2));
		_screen->updateScreen();
	}

	memcpy(_screen->getPalette(0)+723, _screen->getPalette(2)+723, 18);
	_cauldronState = state;
	_cauldronUseCount = 0;
	//if (state == 5)
	// sub_27149();
}

void KyraEngine_v2::clearCauldronTable() {
	Common::set_to(_cauldronTable, _cauldronTable+ARRAYSIZE(_cauldronTable), -1);
}

void KyraEngine_v2::addFrontCauldronTable(int item) {
	for (int i = 23; i >= 0; --i)
		_cauldronTable[i+1] = _cauldronTable[i];
	_cauldronTable[0] = item;
}

void KyraEngine_v2::cauldronItemAnim(int item) {
	const int x = 282;
	const int y = 135;
	const int mouseDstX = (x + 7) & (~1);
	const int mouseDstY = (y + 15) & (~1);
	int mouseX = _mouseX & (~1);
	int mouseY = _mouseY & (~1);

	while (mouseY != mouseDstY) {
		if (mouseY < mouseDstY)
			mouseY += 2;
		else if (mouseY > mouseDstY)
			mouseY -= 2;
		uint32 waitEnd = _system->getMillis() + _tickLength;
		setMousePos(mouseX, mouseY);
		_system->updateScreen();
		delayUntil(waitEnd);
	}

	while (mouseX != mouseDstX) {
		if (mouseX < mouseDstX)
			mouseX += 2;
		else if (mouseX > mouseDstX)
			mouseX -= 2;
		uint32 waitEnd = _system->getMillis() + _tickLength;
		setMousePos(mouseX, mouseY);
		_system->updateScreen();
		delayUntil(waitEnd);
	}

	if (itemIsFlask(item)) {
		setHandItem(19);
		delayUntil(_system->getMillis()+_tickLength*30);
		setHandItem(18);
	} else {
		_screen->hideMouse();
		backUpGfxRect32x32(x, y);
		uint8 *shape = getShapePtr(item+64);

		int curY = y;
		for (int i = 0; i < 12; i += 2, curY += 2) {
			restoreGfxRect32x32(x, y);
			uint32 waitEnd = _system->getMillis() + _tickLength;
			_screen->drawShape(0, shape, x, curY, 0, 0);
			_screen->updateScreen();
			delayUntil(waitEnd);
		}

		snd_playSoundEffect(0x17);
		
		for (int i = 16; i > 0; i -= 2, curY += 2) {
			_screen->setNewShapeHeight(shape, i);
			restoreGfxRect32x32(x, y);
			uint32 waitEnd = _system->getMillis() + _tickLength;
			_screen->drawShape(0, shape, x, curY, 0, 0);
			_screen->updateScreen();
			delayUntil(waitEnd);
		}

		restoreGfxRect32x32(x, y);
		_screen->resetShapeHeight(shape);
		removeHandItem();
		_screen->showMouse();
	}
}

bool KyraEngine_v2::updateCauldron() {
	for (int i = 0; i < 23; ++i) {
		const int16 *curStateTable = _cauldronStateTables[i];
		if (*curStateTable == -2)
			continue;

		int cauldronState = i;
		int16 cauldronTable[25];
		memcpy(cauldronTable, _cauldronTable, sizeof(cauldronTable));

		while (*curStateTable != -2) {
			int stateValue = *curStateTable++;
			int j = 0;
			for (; j < 25; ++j) {
				int val = cauldronTable[j];

				switch (val) {
				case 68:
					val = 70;
					break;

				case 133:
				case 167:
					val = 119;
					break;

				case 130:
				case 143:
				case 100:
					val = 12;
					break;

				case 132:
				case 65:
				case 69:
				case 74:
					val = 137;
					break;

				case 157:
					val = 134;
					break;

				default:
					break;
				}

				if (val == stateValue) {
					cauldronTable[j] = -1;
					j = 26;
				}
			}

			if (j == 25)
				cauldronState = -1;
		}

		if (cauldronState >= 0) {
			showMessage(0, 0xCF);
			setCauldronState(cauldronState, true);
			if (cauldronState == 7)
				objectChat(getTableString(0xF2, _cCodeBuffer, 1), 0, 0x83, 0xF2);
			clearCauldronTable();
			return true;
		}
	}

	return false;
}

void KyraEngine_v2::cauldronRndPaletteFade() {
	showMessage(0, 0xCF);
	int index = _rnd.getRandomNumberRng(0x0F, 0x16);
	Common::SeekableReadStream *file = _res->getFileStream("_POTIONS.PAL");
	if (!file)
		error("Couldn't load cauldron palette");
	file->seek(index*18, SEEK_SET);
	file->read(_screen->getPalette(0)+723, 18);
	snd_playSoundEffect(0x6A);
	_screen->fadePalette(_screen->getPalette(0), 0x1E, &_updateFunctor);
	file->seek(0, SEEK_SET);
	file->read(_screen->getPalette(0)+723, 18);
	delete file;
	_screen->fadePalette(_screen->getPalette(0), 0x1E, &_updateFunctor);
}

void KyraEngine_v2::resetCauldronStateTable(int idx) {
	for (int i = 0; i < 7; ++i)
		_cauldronStateTables[idx][i] = -2;
}

bool KyraEngine_v2::addToCauldronStateTable(int data, int idx) {
	for (int i = 0; i < 7; ++i) {
		if (_cauldronStateTables[idx][i] == -2) {
			_cauldronStateTables[idx][i] = data;
			return true;
		}
	}
	return false;
}

void KyraEngine_v2::listItemsInCauldron() {
	int itemsInCauldron = 0;
	for (int i = 0; i < 25; ++i) {
		if (_cauldronTable[i] != -1)
			++itemsInCauldron;
		else
			break;
	}

	if (!itemsInCauldron) {
		if (!_cauldronState)
			objectChat(getTableString(0xF4, _cCodeBuffer, 1), 0, 0x83, 0xF4);
		else
			objectChat(getTableString(0xF3, _cCodeBuffer, 1), 0, 0x83, 0xF3);
	} else {
		objectChat(getTableString(0xF7, _cCodeBuffer, 1), 0, 0x83, 0xF7);

		char buffer[80];
		for (int i = 0; i < itemsInCauldron-1; ++i) {
			char *str = buffer;
			strcpy(str, getTableString(_cauldronTable[i]+54, _cCodeBuffer, 1));
			if (_lang == 1) {
				if (*str == 37)
					str += 2;
			}
			strcpy((char*)_unkBuf500Bytes, "...");
			strcat((char*)_unkBuf500Bytes, str);
			strcat((char*)_unkBuf500Bytes, "...");
			objectChat((const char*)_unkBuf500Bytes, 0, 0x83, _cauldronTable[i]+54);
		}

		char *str = buffer;
		strcpy(str, getTableString(_cauldronTable[itemsInCauldron-1]+54, _cCodeBuffer, 1));
		if (_lang == 1) {
			if (*str == 37)
				str += 2;
		}
		strcpy((char*)_unkBuf500Bytes, "...");
		strcat((char*)_unkBuf500Bytes, str);
		strcat((char*)_unkBuf500Bytes, ".");
		objectChat((const char*)_unkBuf500Bytes, 0, 0x83, _cauldronTable[itemsInCauldron-1]+54);
	}
}

#pragma mark -

void KyraEngine_v2::dinoRide() {
	_mainCharX = _mainCharY = -1;

	setGameFlag(0x15A);
	enterNewScene(41, -1, 0, 0, 0);
	resetGameFlag(0x15A);

	setGameFlag(0x15B);
	enterNewScene(39, -1, 0, 0, 0);
	resetGameFlag(0x15B);

	setGameFlag(0x16F);

	setGameFlag(0x15C);
	enterNewScene(42, -1, 0, 0, 0);
	resetGameFlag(0x15C);

	setGameFlag(0x15D);
	enterNewScene(39, -1, 0, 0, 0);
	resetGameFlag(0x15D);

	setGameFlag(0x15E);
	enterNewScene(40, -1, 0, 0, 0);
	resetGameFlag(0x15E);

	_mainCharX = 262;
	_mainCharY = 28;
	_mainCharacter.facing = 5;
	_mainCharacter.animFrame = _characterFrameTable[5];
	enterNewScene(39, 4, 0, 0, 0);
	setHandItem(0x61);
	_screen->showMouse();
	resetGameFlag(0x159);
}

#pragma mark -

void KyraEngine_v2::registerDefaultSettings() {
	KyraEngine::registerDefaultSettings();

	// Most settings already have sensible defaults. This one, however, is
	// specific to the Kyra engine.
	ConfMan.registerDefault("walkspeed", 5);
}

} // end of namespace Kyra




