/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/config-manager.h"
#include "common/system.h"

#include "touche/touche.h"
#include "touche/graphics.h"

namespace Touche {

ToucheEngine::ToucheEngine(OSystem *system, Common::Language language)
	: Engine(system), _language(language) {

	_talkTextMode = kTalkModeVoiceAndText;

	_saveLoadCurrentPage = 0;
	_saveLoadCurrentSlot = 0;
	_hideInventoryTexts = false;

	_screenRect = Common::Rect(640, 400);
	_roomAreaRect = Common::Rect(640, 352);
	clearDirtyRects();

	_defaultSoundPriority = 0;
	_snd_midiContext.unk2 = 0;
	_snd_midiContext.unkA = 1;
	_snd_midiContext.unkB = 0;
	_snd_midiContext.volume = 0;
	_snd_midiContext.unkF = 0;
	_snd_midiContext.currentVolume = 175;
	_playSoundCounter = 0;

	_processRandomPaletteCounter = 0;

	_roomNeedRedraw = false;
	_fastWalkMode = false;

	_currentObjectNum = -1;
	_objectDescriptionNum = 0;
	_speechPlaying = false;

	_roomNeedRedraw	= false;
	_fullRedrawCounter = 0;
	_redrawScreenCounter1 = 0;
	memset(_paletteBuffer, 0, sizeof(_paletteBuffer));

	setupOpcodes();
	setupRect();
	setupUIRect();

	Common::addSpecialDebugLevel(kDebugEngine,   "Engine",   "Engine debug level");
	Common::addSpecialDebugLevel(kDebugGraphics, "Graphics", "Graphics debug level");
	Common::addSpecialDebugLevel(kDebugResource, "Resource", "Resource debug level");
	Common::addSpecialDebugLevel(kDebugOpcodes,  "Opcodes",  "Opcodes debug level");
	Common::addSpecialDebugLevel(kDebugUserIntf, "UserIntf", "UserInterface debug level");
}

ToucheEngine::~ToucheEngine() {
	Common::clearAllSpecialDebugLevels();
}

int ToucheEngine::init() {
	_system->beginGFXTransaction();
		initCommonGFX(true);
		_system->initSize(640, 400);
	_system->endGFXTransaction();

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	return 0;
}

int ToucheEngine::go() {
	res_openDataFile();
	res_allocateTables();
	res_loadSpriteImage(18, _menuKitData);
	res_loadImageHelper(_menuKitData, _currentImageWidth, _currentImageHeight);
	res_loadSpriteImage(19, _convKitData);
	res_loadImageHelper(_convKitData, _currentImageWidth, _currentImageHeight);

	mainLoop();

	res_deallocateTables();
	res_closeDataFile();

	return 0;
}

void ToucheEngine::restart() {
	_displayQuitDialog = false;

	memset(_flagsTable, 0, sizeof(_flagsTable));

	_currentKeyCharNum = 0;
	initKeyChars(-1);

	for (int i = 0; i < NUM_SEQUENCES; ++i) {
		_sequenceEntryTable[i].sprNum = -1;
		_sequenceEntryTable[i].seqNum = -1;
	}

	_disabledInputCounter = 0;
	_currentCursorObject = 0;
	setCursor(0);

	_waitingSetKeyCharNum1 = -1;
	_waitingSetKeyCharNum2 = -1;
	_waitingSetKeyCharNum3 = -1;

	_currentEpisodeNum = 0;
	_newEpisodeNum = ConfMan.getInt("boot_param");
	if (_newEpisodeNum == 0) {
		_newEpisodeNum = 90;
	}

	_newMusicNum = 0;
	_currentMusicNum = 0;

	_newSoundNum = 0;
	_newSoundDelay = 0;
	_newSoundPriority = 0;

	_flagsTable[176] = 0;
	_keyCharsTable[0].money = 25;
	_currentAmountOfMoney = 0;

	_giveItemToKeyCharNum = 0;
	_giveItemToObjectNum = 0;
	_giveItemToCounter = 0;

	clearAreaTable();
	clearAnimationTable();

	initInventoryObjectsTable();
	initInventoryLists();
	drawInventory(0, 1);

	_talkListEnd = 0;
	_talkListCurrent = 0;
	_talkTextRectDefined = false;
	_talkTextDisplayed = false;
	_talkTextInitialized = false;
	_skipTalkText = false;
	_talkTextSpeed = 0;
	_keyCharTalkCounter = 0;
	_talkTableLastTalkingKeyChar = -1;
	_talkTableLastOtherKeyChar = -1;
	_talkTableLastStringNum = -1;
	_objectDescriptionNum = 0;
	memset(_talkTable, 0, sizeof(_talkTable));

	_conversationChoicesUpdated = false;
	_conversationReplyNum = -1;
	_conversationEnded = false;
	_conversationNum = 0;
	_drawCharacterConversionRepeatCounter = 0;
	_currentConversation = 0;
	_disableConversationScript = false;
	_conversationAreaCleared = false;
	memset(_conversationChoicesTable, 0, sizeof(_conversationChoicesTable));

	_flagsTable[901] = 1;
	if (_language == Common::FR_FRA) {
		_flagsTable[621] = 1;
	}
}

void ToucheEngine::mainLoop() {
	restart();
	_inp_mousePos.x = 640 / 2;
	_inp_mousePos.y = 352 / 2;
	_inp_mouseButtonClicked = false;
	_inp_mouseButtonPressed = false;
	_system->warpMouse(_inp_mousePos.x, _inp_mousePos.y);
	setPalette(0, 255, 0, 0, 0);
#ifdef NORMAL_GAME_SPEED
	const int cycleDelay = 1000 / (1193180 / 32768);
#else
	const int cycleDelay = 10;
#endif
	uint32 frameTimeStamp = _system->getMillis();
	for (uint32 cycleCounter = 0; _flagsTable[611] == 0; ++cycleCounter) {
		if ((cycleCounter & 3) == 0) {
			runCycle();
		}
		if ((cycleCounter & 2) == 0) {
			fadePaletteFromFlags();
 		}
		int delay = _system->getMillis() - frameTimeStamp;
		delay = cycleDelay - delay;
		if (delay < 1) {
			delay = 1;
		}
		_system->delayMillis(delay);
		frameTimeStamp = _system->getMillis();
	}
}

void ToucheEngine::processEvents() {
	OSystem::Event event;
	while (_system->pollEvent(event)) {
		switch (event.type) {
		case OSystem::EVENT_QUIT:
			_flagsTable[611] = 1;
			break;
		case OSystem::EVENT_KEYDOWN:
			_flagsTable[600] = event.kbd.keycode;
			if (event.kbd.keycode == 27) { // ESC
				if (_displayQuitDialog) {
					_flagsTable[611] = ui_displayQuitDialog();
				}
			}
			if (event.kbd.keycode == 286) { // F5
				if (_flagsTable[618] == 0 && !_hideInventoryTexts) {
					ui_handleOptions(0);
				}
			}
			if (event.kbd.keycode == 290) { // F9
				_fastWalkMode = true;
			}
			if (event.kbd.keycode == 291) { // F10
				_fastWalkMode = false;
			}
			if (event.kbd.ascii == 't') {
				++_talkTextMode;
				if (_talkTextMode == kTalkModeCount) {
					_talkTextMode = 0;
				}
				ui_displayTextMode(-(92 + _talkTextMode));
			}
			if (event.kbd.ascii == 'd') {
				// enable debugging stuff ?
				_flagsTable[777] = 1;
			}
			if (event.kbd.ascii == ' ') {
				updateKeyCharTalk(2);
			}
			break;
		case OSystem::EVENT_MOUSEMOVE:
			_inp_mousePos.x = event.mouse.x;
			_inp_mousePos.y = event.mouse.y;
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			_inp_mousePos.x = event.mouse.x;
			_inp_mousePos.y = event.mouse.y;
			_inp_mouseButtonClicked = true;
			break;
		case OSystem::EVENT_LBUTTONUP:
			_inp_mousePos.x = event.mouse.x;
			_inp_mousePos.y = event.mouse.y;
			break;
		case OSystem::EVENT_RBUTTONDOWN:
			_inp_mousePos.x = event.mouse.x;
			_inp_mousePos.y = event.mouse.y;
			_inp_mouseButtonPressed = true;
			break;
		case OSystem::EVENT_RBUTTONUP:
			_inp_mousePos.x = event.mouse.x;
			_inp_mousePos.y = event.mouse.y;
			_inp_mouseButtonPressed = false;
			break;
		default:
			break;
		}
	}
}

void ToucheEngine::runCycle() {
	debugC(9, kDebugEngine, "ToucheEngine::runCycle()");
	if (_flagsTable[290]) {
		changePaletteRange();
	}
	if (_flagsTable[270]) {
		playSoundInRange();
	}
	if (_conversationEnded) {
		_disabledInputCounter = 0;
		_fullRedrawCounter = 1;
		_roomAreaRect.setHeight(352);
		_hideInventoryTexts = false;
		_conversationEnded = false;
		drawInventory(_currentKeyCharNum, 1);
	}
	if (_giveItemToCounter == 1) {
		_fullRedrawCounter = 1;
		drawInventory(_giveItemToObjectNum, 1);
		++_giveItemToCounter;
	}
	if (_giveItemToCounter == -1) {
		_giveItemToCounter = 0;
		_roomAreaRect.setHeight(320);
		_keyCharsTable[_giveItemToKeyCharNum].flags &= ~kScriptPaused;
	}
	setupNewEpisode();
	startNewMusic();
	startNewSound();
	updateSpeech();
	handleConversation();
	if (scrollRoom(_currentKeyCharNum)) {
	 	_fullRedrawCounter |= 1;
	}
	redrawRoom();
	clearDirtyRects();
	processAreaTable();
	clearAreaTable();
	updateRoomRegions();
	if (_flagsTable[612] != 0) {
		_flagsTable[613] = getRandomNumber(_flagsTable[612]);
	}
	processEvents();
	sortKeyChars();
	for (int i = 0; i < NUM_KEYCHARS; ++i) {
		runKeyCharScript(&_keyCharsTable[i]);
	}
	if (_roomNeedRedraw) {
		scrollRoom(_currentKeyCharNum);
		redrawRoom();
		_roomNeedRedraw = false;
	}
	updateSpeech();
	for (int i = 0; i < NUM_KEYCHARS; ++i) {
		waitForKeyCharPosition(i);
	}
	redrawBackground();
	waitForKeyCharsSet();
	handleMouseInput(0);
	for (int i = 0; i < NUM_KEYCHARS; ++i) {
		drawKeyChar(&_keyCharsTable[i]);
	}
	processAnimationTable();
	updateKeyCharTalk(0);
	updateDirtyScreenAreas();
	++_flagsTable[295];
	++_flagsTable[296];
	++_flagsTable[297];
	if (_flagsTable[298]) {
		--_flagsTable[298];
	}
	if (_flagsTable[299]) {
		--_flagsTable[299];
	}
}

int16 ToucheEngine::getRandomNumber(int max) {
	assert(max > 0);
	return _rnd.getRandomNumber(max - 1);
}

void ToucheEngine::changePaletteRange() {
	if (_processRandomPaletteCounter) {
		--_processRandomPaletteCounter;
	} else {
		int scale = _flagsTable[291] + getRandomNumber(_flagsTable[292]);
		setPalette(0, 240, scale, scale, scale);
		_processRandomPaletteCounter = _flagsTable[293] + getRandomNumber(_flagsTable[294]);
	}
}

void ToucheEngine::playSoundInRange() {
	if (_playSoundCounter != 0) {
		--_playSoundCounter;
	} else {
		int16 flag = getRandomNumber(_flagsTable[270]);
		int16 num = _flagsTable[273 + flag];
		res_loadSound(0, num);
		_playSoundCounter = _flagsTable[271] + getRandomNumber(_flagsTable[272]);
	}
}

void ToucheEngine::resetSortedKeyCharsTable() {
	for (int i = 0; i < NUM_KEYCHARS; ++i) {
		_sortedKeyCharsTable[i] = &_keyCharsTable[i];
	}
}

void ToucheEngine::setupEpisode(int num) {
	debugC(9, kDebugEngine, "ToucheEngine::setupEpisode() num=%d", num);
	res_stopSpeech();
	resetTalkingVars();
	res_loadSpeech(-1);
	_currentObjectNum = -1;
	if (num != -1) {
		_updatedRoomAreasTable[0] = 1;
		clearAreaTable();
		initKeyChars(-1);
		for (int i = 200; i < 300; ++i) {
			_flagsTable[i] = 0;
		}
		_flagsTable[291] = 240;
		_flagsTable[292] = 16;
		_flagsTable[293] = 0;
		_flagsTable[294] = 1;
		_currentEpisodeNum = num;
		if (_flagsTable[911] != 0) {
			// load scripts from external data files
		}
		debug(0, "Setting up episode %d\n", num);
		res_loadProgram(num);
		_disabledInputCounter = 0;
	}
	res_decodeProgramData();
	_roomAreaRect.setHeight(352);
	_disableConversationScript = false;
	_hideInventoryTexts = false;
	_conversationEnded = false;
	clearRoomArea();
	drawInventory(_currentKeyCharNum, 1);
}

void ToucheEngine::setupNewEpisode() {
	debugC(9, kDebugEngine, "ToucheEngine::setupNewEpisode() _newEpisodeNum=%d", _newEpisodeNum);
	if (_newEpisodeNum) {
		if (_newEpisodeNum == 91) {
			_displayQuitDialog = true;
		}
//		flushDigitalSounds();
		setupEpisode(_newEpisodeNum);
		runCurrentKeyCharScript(1);
		_newEpisodeNum = 0;
		resetSortedKeyCharsTable();
	}
}

void ToucheEngine::drawKeyChar(KeyChar *key) {
	debugC(9, kDebugEngine, "ToucheEngine::drawKeyChar()");
	if (key->num != 0) {
		Common::Rect r(key->prevBoundingRect);
		r.extend(key->boundingRect);
//		r.clip(_roomAreaRect);
//		addToDirtyRect(r);
	}
}

void ToucheEngine::sortKeyChars() {
	debugC(9, kDebugEngine, "ToucheEngine::sortKeyChars()");
	for (int i = 0; i < NUM_KEYCHARS; ++i) {
		bool hasSwapped = false;
		for (int j = 0; j < NUM_KEYCHARS - 1; ++j) {
			KeyChar *key1 = _sortedKeyCharsTable[j];
			KeyChar *key2 = _sortedKeyCharsTable[j + 1];
			if (key1->num != 0 && key2->num != 0) {
				if (key1->zPos > key2->zPos) {
					SWAP(_sortedKeyCharsTable[j], _sortedKeyCharsTable[j + 1]);
					hasSwapped = true;
				} else if (key1->zPos == key2->zPos && key1->yPos > key2->yPos) {
					SWAP(_sortedKeyCharsTable[j], _sortedKeyCharsTable[j + 1]);
					hasSwapped = true;
				}
			} else if (key1->num != 0) {
				SWAP(_sortedKeyCharsTable[j], _sortedKeyCharsTable[j + 1]);
				hasSwapped = true;
			}
		}
		if (!hasSwapped) {
			break;
		}
	}
}

void ToucheEngine::runKeyCharScript(KeyChar *key) {
	debugC(9, kDebugEngine, "ToucheEngine::runKeyCharScript() keyChar=%d", key - _keyCharsTable);
	if (key->scriptDataOffset != 0 && (key->flags & (kScriptStopped | kScriptPaused)) == 0) {
		int16 scriptParam = key->num - 1;
		int16 *prevStackDataPtr = _script.stackDataPtr;
		_script.stackDataPtr = key->scriptStackPtr;
		uint16 prevDataOffset = _script.dataOffset;
		_script.dataOffset = key->scriptDataOffset;
		_script.quitFlag = 0;
		while (_script.quitFlag == 0) {
			executeScriptOpcode(scriptParam);
		}
		switch (_script.quitFlag) {
		case 1: // restart
			key->scriptDataOffset = key->scriptDataStartOffset;
			key->scriptStackPtr = &key->scriptStackTable[39];
			break;
		case 3: // pause
			key->flags |= kScriptPaused;
			key->flags &= ~kScriptStopped;
			key->scriptDataOffset = _script.dataOffset;
			key->scriptStackPtr = _script.stackDataPtr;
			break;
		default: // stop
			key->flags |= kScriptStopped;
			key->flags &= ~kScriptPaused;
			key->scriptDataOffset = 0;
			break;
		}
		_script.dataOffset = prevDataOffset;
		_script.stackDataPtr = prevStackDataPtr;
	}
}

void ToucheEngine::runCurrentKeyCharScript(int mode) {
	debugC(9, kDebugEngine, "ToucheEngine::runCurrentKeyCharScript() _currentKeyCharNum=%d mode=%d", _currentKeyCharNum, mode);
	KeyChar *key = &_keyCharsTable[_currentKeyCharNum];
	if (mode == 1) {
		_script.dataOffset = 0;
		_script.stackDataPtr = key->scriptStackPtr;
	}
	if (mode != 0) {
		while (_script.quitFlag == 0) {
			executeScriptOpcode(0);
		}
		if (mode == 1) {
			centerScreenToKeyChar(_currentKeyCharNum);
		}
		if (_script.quitFlag == 3) {
			key->flags |= kScriptPaused;
			key->flags &= ~kScriptStopped;
			key->scriptDataOffset = _script.dataOffset;
			key->scriptStackPtr = _script.stackDataPtr;
		}
	}
	handleMouseInput(1);
}

void ToucheEngine::executeScriptOpcode(int16 param) {
	debugC(9, kDebugEngine, "executeScriptOpcode(%d) offset=%04X", param, _script.dataOffset);
	_script.keyCharNum = param;
	_script.opcodeNum = _script.readNextByte();
	if (_script.opcodeNum < NUM_OPCODES) {
		OpcodeProc op = _opcodesTable[_script.opcodeNum];
		if (op) {
			(this->*op)();
			return;
		}
	}
	error("Invalid opcode 0x%X", _script.opcodeNum);
}

void ToucheEngine::initKeyChars(int keyChar) {
	debugC(9, kDebugEngine, "ToucheEngine::initKeyChars() keyChar=%d", keyChar);
	int indexStart, indexEnd;
	if (keyChar == -1) {
		indexStart = 0;
		indexEnd = NUM_KEYCHARS;
	} else {
		indexStart = keyChar;
		indexEnd = keyChar + 1;
	}
	Common::Rect defaultKeyCharRect(10, 10, 11, 11);
	for (int i = indexStart; i < indexEnd; ++i) {
		KeyChar *key = &_keyCharsTable[i];
		if (keyChar != -1 && key->num != 0) {
			Area keyCharArea;
			keyCharArea.r = key->prevBoundingRect;
			keyCharArea.r.extend(key->boundingRect);
			keyCharArea.srcX = _flagsTable[614] + keyCharArea.r.left;
			keyCharArea.srcY = _flagsTable[615] + keyCharArea.r.top;
			addToAreaTable(&keyCharArea);
		}
		key->num = 0;
		key->strNum = 0;
		key->textColor = 253;
		key->currentAnimCounter = 0;
		key->currentAnimSpeed = 0;
		key->currentAnim = 0;
		key->framesListCount = 0;
		key->currentFrame = 0;
		key->anim1Start = 0;
		key->anim1Count = 1;
		key->anim2Start = 0;
		key->anim2Count = 1;
		key->anim3Start = 0;
		key->anim3Count = 1;
		key->facingDirection = 0;
		key->sequenceDataOffset = 0;
		key->walkDataNum = 0;
		key->walkPointsList[0] = -1;
		key->walkPointsListCount = 0;
		key->delay = 0;
		key->waitingKeyChar = -1;
		key->flags = 0;
		key->scriptDataOffset = 0;
		key->scriptStackPtr = &key->scriptStackTable[39];
		key->xPos = 10;
		// like the original interpreter, don't reset yPos here. Doing so causes
		// glitches during the introduction for example (talk texts get displayed
		// at the wrong coordinates).
		key->boundingRect = defaultKeyCharRect;
		key->prevBoundingRect = defaultKeyCharRect;
	}
}

void ToucheEngine::setKeyCharTextColor(int keyChar, uint16 color) {
	debugC(9, kDebugEngine, "ToucheEngine::setKeyCharTextColor(%d) color=%d", keyChar, color);
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	_keyCharsTable[keyChar].textColor = color;
}

void ToucheEngine::waitForKeyCharPosition(int keyChar) {
	debugC(9, kDebugEngine, "ToucheEngine::waitForKeyCharPosition(%d)", keyChar);
	KeyChar *key = _sortedKeyCharsTable[keyChar];
	if (key->num != 0) {
		key->prevBoundingRect = key->boundingRect;
		moveKeyChar(_offscreenBuffer, 640, key);
		key->boundingRect = _moveKeyCharRect;
		if (key->delay != 0) {
			--key->delay;
			if (key->delay == 0) {
				key->flags &= ~kScriptPaused;
			}
			return;
		}
		if (key->waitingKeyChar == -1) {
			return;
		}
		KeyChar *nextKey = &_keyCharsTable[key->waitingKeyChar];
		if (nextKey->currentAnim != key->waitingKeyCharPosTable[0] &&
		    nextKey->pointsDataNum != key->waitingKeyCharPosTable[1] &&
		    nextKey->walkDataNum != key->waitingKeyCharPosTable[2]) {
			return;
		}
		key->flags &= ~kScriptPaused;
		key->waitingKeyChar = -1;
	}
}

void ToucheEngine::setKeyCharBox(int keyChar, int value) {
	debugC(9, kDebugEngine, "ToucheEngine::setKeyCharBox(%d) value=%d", keyChar, value);
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	key->prevPointsDataNum = key->pointsDataNum = value;
	key->xPosPrev = key->xPos = _programPointsTable[value].x;
	key->yPosPrev = key->yPos = _programPointsTable[value].y;
	key->zPosPrev = key->zPos = _programPointsTable[value].z;
	key->prevWalkDataNum = key->walkDataNum = findWalkDataNum(value, 10000);
}

void ToucheEngine::setKeyCharFrame(int keyChar, int16 type, int16 value1, int16 value2) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	switch (type) {
	case 0:
		key->anim2Start = value1;
		key->anim2Count = value2;
		key->anim3Start = value1;
		key->anim3Count = value2;
		break;
	case 1:
		if (value2 != 0) {
			value2 = getRandomNumber(value2);
		}
		key->framesList[key->framesListCount] = value1 + value2;
		++key->framesListCount;
		key->framesListCount &= 15;
		break;
	case 2:
		key->anim1Start = value1;
		key->anim1Count = value2;
		break;
	case 3:
		key->currentAnim = value1;
		key->currentAnimSpeed = 0;
		key->currentAnimCounter = 0;
		break;
	case 4:
		key->anim3Start = value1;
		key->anim3Count = value2;
		break;
	}
}

void ToucheEngine::setKeyCharFacingDirection(int keyChar, int16 dir) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	_keyCharsTable[keyChar].facingDirection = dir;
}

void ToucheEngine::initKeyCharScript(int keyChar, int16 spriteNum, int16 seqDataIndex, int16 seqDataOffs) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	key->num = keyChar + 1;
	key->spriteNum = spriteNum;
	key->sequenceDataIndex = seqDataIndex;
	key->sequenceDataOffset = seqDataOffs;
	key->scriptDataStartOffset = findProgramKeyCharScriptOffset(keyChar);
	key->scriptDataOffset = key->scriptDataStartOffset;
}

uint16 ToucheEngine::findProgramKeyCharScriptOffset(int keyChar) const {
	for (uint i = 0; i < _programKeyCharScriptOffsetTable.size(); ++i) {
		if (_programKeyCharScriptOffsetTable[i].keyChar == keyChar) {
			return _programKeyCharScriptOffsetTable[i].offset;
		}
	}
	return 0;
}

bool ToucheEngine::scrollRoom(int keyChar) {
	if (_flagsTable[616] != 0) {
		return 0;
	}
	KeyChar *key = &_keyCharsTable[keyChar];
	bool needRedraw = false;

	// vertical scrolling
	int prevRoomDy = _flagsTable[615];
	_flagsTable[615] = key->yPos + 32 - 400 / 2;
	int roomHeight;
	if (_hideInventoryTexts) {
		roomHeight = 352;
	} else {
		roomHeight = (_flagsTable[606] != 0) ? 400 : 352;
		_roomAreaRect.setHeight(roomHeight);
	}
	_flagsTable[615] = CLIP<int16>(_flagsTable[615], 0, _currentBitmapHeight - roomHeight);
	if (_flagsTable[615] != prevRoomDy) {
		needRedraw = true;
	}

	// horizontal scrolling
	int prevRoomDx = _flagsTable[614];
	if (key->xPos > prevRoomDx + 480) {
		int dx = key->xPos - (prevRoomDx + 480);
		prevRoomDx += dx;
	} else if (key->xPos < prevRoomDx + 160) {
		int dx = prevRoomDx + 160 - key->xPos;
		prevRoomDx -= dx;
		if (prevRoomDx < 0) {
			prevRoomDx = 0;
		}
	}
	prevRoomDx = CLIP<int16>(prevRoomDx, 0, _roomWidth - 640);
	if (_flagsTable[614] != prevRoomDx) {
		_flagsTable[614] = prevRoomDx;
		return true;
	}
	if (_screenOffset.x == 0) {
		return needRedraw;
	}
	int scrollDx = _screenOffset.x - _flagsTable[614];
	if (scrollDx < -4) {
		scrollDx = -4;
	} else if (scrollDx > 4) {
		scrollDx = 4;
	}
	_flagsTable[614] += scrollDx;

	if (_screenOffset.x == _flagsTable[614]) {
		_screenOffset.x = 0;
	}
	return true;
}

void ToucheEngine::drawIcon(int x, int y, int num) {
	res_loadImage(num, _iconData);
	Graphics::copyRect(_offscreenBuffer, 640, x, y,
	  _iconData, 58, 0, 0,
	  58, 42,
	  Graphics::kTransparent);
}

void ToucheEngine::centerScreenToKeyChar(int keyChar) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	_flagsTable[614] = key->xPos - 640 / 2;
	_flagsTable[615] = key->yPos - 400 / 2;
	_flagsTable[615] = CLIP<int16>(_flagsTable[615], 0, _currentBitmapHeight - 352);
	scrollRoom(keyChar);
}

void ToucheEngine::waitForKeyCharsSet() {
	if (_waitingSetKeyCharNum2 != -1) {
		KeyChar *key = &_keyCharsTable[_waitingSetKeyCharNum2];
		if (key->framesListCount == key->currentFrame && key->currentAnim == key->anim2Start) {
			key = &_keyCharsTable[_waitingSetKeyCharNum1];
			if (key->framesListCount == key->currentFrame && key->currentAnim == key->anim2Start) {
				key = &_keyCharsTable[_waitingSetKeyCharNum3];
				_waitingSetKeyCharNum2 = -1;
				key->flags &= ~kScriptPaused;
			}
		}
	}
}

void ToucheEngine::redrawRoom() {
	if (_currentBitmapWidth == 0 || _currentBitmapHeight == 0) {
		return;
	}
	int w = 640;
	if (_flagsTable[614] < 0 || _flagsTable[614] > _currentBitmapWidth - w) {
		error("Invalid room_x_offset = %d (w=%d, room_w=%d)", _flagsTable[614], w, _currentBitmapWidth);
	}
	int h = (_flagsTable[606] != 0) ? 400 : _roomAreaRect.height();
	if (_flagsTable[615] < 0 || _flagsTable[615] > _currentBitmapHeight - h) {
		error("Invalid room_y_offset = %d (h=%d, room_h=%d)", _flagsTable[615], h, _currentBitmapHeight);
	}
	uint8 *dst = _offscreenBuffer;
	const uint8 *src = _backdropBuffer + _flagsTable[615] * _currentBitmapWidth + _flagsTable[614];
	while (h--) {
		memcpy(dst, src, w);
		dst += w;
		src += _currentBitmapWidth;
	}
}

void ToucheEngine::fadePalette(int firstColor, int lastColor, int scale, int scaleInc, int fadingStepsCount) {
	for (int i = 0; i < fadingStepsCount; ++i) {
		scale += scaleInc;
		if (scale > 255) {
			scale = 0;
		} else if (scale < 0) {
			scale = 0;
		}
		setPalette(firstColor, lastColor, scale, scale, scale);
	}
}

void ToucheEngine::fadePaletteFromFlags() {
	if (_flagsTable[603]) {
		setPalette(_flagsTable[607], _flagsTable[608], _flagsTable[605], _flagsTable[605], _flagsTable[605]);
		if (_flagsTable[603] > 0) {
			if (_flagsTable[605] >= _flagsTable[609]) {
				_flagsTable[603] = 0;
			}
		} else {
			if (_flagsTable[605] <= _flagsTable[610]) {
				_flagsTable[603] = 0;
			}
		}
		_flagsTable[605] += _flagsTable[603];
		if (_flagsTable[605] < 0) {
			_flagsTable[605] = 0;
		} else if (_flagsTable[605] > 255) {
			_flagsTable[605] = 255;
		}
	}
}

static uint8 *getKeyCharFrameData(uint8 *p, uint16 dir1, uint16 dir2, uint16 dir3, uint8 **dst, int16 sequence_num) {
	uint8 *src;
	uint16 offs, num1;

	// spriteData
	// LE16 offset to "sprite copy" data
	// LE16 offset to 4 * 2 * 10 offsets : "sprite info" offset
	// LE16 data offset
	// LE16 ?
	offs = READ_LE_UINT16(p + sequence_num * 8 + 2);
	offs = READ_LE_UINT16(p + offs + dir1 * 4); // facing
	offs = READ_LE_UINT16(p + offs + dir2 * 2); // top/bottom
	src = p + offs + dir3 * 10; // current frame anim ?
	*dst = src;
	// LE16 : if 0x8000 -> offset "sprite copy" data num
	// LE16 : dx
	// LE16 : dy
	// LE16 : dz

	num1 = READ_LE_UINT16(src) & 0x7FFF;
	offs = READ_LE_UINT16(p + sequence_num * 8 + 0);
	offs = READ_LE_UINT16(p + offs + num1 * 2);
	return p + offs;
	// LE16 : srcX
	// LE16 : srcY
	// LE16 : flags (vflip, hflip)
}

void ToucheEngine::moveKeyChar(uint8 *dst, int dstPitch, KeyChar *key) {
	int16 keyChar = key->num - 1;
	int16 walkDataNum = key->walkDataNum;
	int16 clippingRectNum = 0;
	if (walkDataNum != -1) {
		clippingRectNum = _programWalkTable[walkDataNum].clippingRect;
	}
	Common::Rect clippingRect(_programRectsTable[clippingRectNum]);
	clippingRect.translate(-_flagsTable[614], -_flagsTable[615]);
	if (key->flags & 0x8000) {
		clippingRect.moveTo(clippingRect.left, 352);
	}
	clippingRect.clip(_roomAreaRect);
	SpriteData *spr = &_spritesTable[key->spriteNum];
	int x1 =  30000, y1 =  30000;
	int x2 = -30000, y2 = -30000;
	int16 keyCharDirection = _flagsTable[266];
	if (keyCharDirection == 0) {
		keyCharDirection = key->facingDirection;
	}
	int16 facingDirection = keyCharDirection;
	uint8 *sequenceDataBase = _sequenceDataTable[key->sequenceDataIndex];
	uint8 *sequenceData = sequenceDataBase;

	uint16 frameDirFlag = READ_LE_UINT16(sequenceData + key->sequenceDataOffset * 8 + 4);
	if (frameDirFlag) {
		sequenceData += frameDirFlag & ~1;
	}

	uint8 *frameData;
	uint8 *frameDataBase = getKeyCharFrameData(sequenceDataBase, key->currentAnim, facingDirection, key->currentAnimCounter, &frameData, key->sequenceDataOffset);
	uint16 frameFlag = READ_LE_UINT16(frameData); frameData += 2;
	uint16 walkDx = READ_LE_UINT16(frameData); frameData += 2;
	uint16 walkDy = READ_LE_UINT16(frameData); frameData += 2;
	uint16 walkDz = READ_LE_UINT16(frameData); frameData += 2;

	if (key->currentAnimSpeed <= 0) {
		key->currentAnimSpeed = READ_LE_UINT16(frameData);
	}
	--key->currentAnimSpeed;
	if (key->currentAnimSpeed <= 0) {
		++key->currentAnimCounter;
	}
	if (_fastWalkMode) {
		walkDx *= 2;
		walkDy *= 2;
		walkDz *= 2;
	}
	updateKeyCharWalkPath(key, walkDx, walkDy, walkDz);
	int posX = key->xPos;
	int posY = key->yPos;
	int posZ = key->zPos;
	if (frameFlag & 0x8000) {
		changeKeyCharFrame(key, keyChar);
	}
	posX -= _flagsTable[614];
	posY -= _flagsTable[615];
	if (posZ == 160) { // draw sprite frames without rescaling
		while (1) {
			int dstX = (int16)READ_LE_UINT16(frameDataBase); frameDataBase += 2;
			if (dstX == 10000) {
				_moveKeyCharRect = Common::Rect(x1, y1, x2 + spr->w, y2 + spr->h);
				break;
			}
			int dstY = (int16)READ_LE_UINT16(frameDataBase); frameDataBase += 2;

			if (facingDirection == 3) {
				dstX = -dstX - spr->w;
			}
			dstX += posX;
			dstY += posY;
			x1 = MIN(dstX, x1);
			x2 = MAX(dstX, x2);
			y1 = MIN(dstY, y1);
			y2 = MAX(dstY, y2);

			int frameDir = READ_LE_UINT16(frameDataBase); frameDataBase += 2;
//			assert((frameDir & 0x4000) == 0); // hflipped
			bool vflipped = (frameDir & 0x8000) != 0;

			frameDir &= 0xFFF;
			if (frameDirFlag) {
				frameDir = READ_LE_UINT16(sequenceData + frameDir * 2);
			}
			if (keyChar == 0) {
				if (_directionsTable[frameDir] <= _flagsTable[176]) {
					continue;
				}
			}
			if (frameDir == 0x800) {
				continue;
			}

			assert(spr->w != 0);
			int framesPerLine = spr->bitmapWidth / spr->w;
			assert(framesPerLine != 0);
			const int srcOffsX = spr->w * (frameDir % framesPerLine);
			const int srcOffsY = spr->h * (frameDir / framesPerLine);

			Area copyRegion(dstX, dstY, spr->w, spr->h);
			copyRegion.srcX = 0;
			copyRegion.srcY = 0;
			if (!copyRegion.clip(clippingRect)) {
				continue;
			}

			if (facingDirection == 3) {
				vflipped = !vflipped;
			}

			uint8 *dstCur = dst + copyRegion.r.top * dstPitch + copyRegion.r.left;
			const int spr_y1 = srcOffsY + copyRegion.srcY;
			const int spr_x1 = srcOffsX + copyRegion.srcX;
			const uint8 *srcSpr = spr->ptr + spr_y1 * spr->bitmapWidth + spr_x1;
			for (int h = 0; h < copyRegion.r.height(); ++h) {
				for (int w = 0; w < copyRegion.r.width(); ++w) {
					uint8 color = vflipped ? srcSpr[spr->w - 1 - w] : srcSpr[w];
					if (color != 0) {
						dstCur[w] = color;
					}
				}
				srcSpr += spr->bitmapWidth;
				dstCur += dstPitch;
			}
		}
	} else { // draw sprite frames with rescaling
		y2 = posY;
		int clippingRect_x1 = clippingRect.left;
		int clippingRect_y1 = clippingRect.top;
		int clippingRect_x2 = clippingRect.right;
		int clippingRect_y2 = clippingRect.bottom;
		buildSpriteScalingTable(160, posZ);
		while (1) {
			int dstX = (int16)READ_LE_UINT16(frameDataBase); frameDataBase += 2;
			if (dstX == 10000) {
				_moveKeyCharRect = Common::Rect(x1, y1, x2 + 1, y2 + 1);
				break;
			}
			int dstY = (int16)READ_LE_UINT16(frameDataBase); frameDataBase += 2;

			int frameDir = READ_LE_UINT16(frameDataBase); frameDataBase += 2;
//			assert((frameDir & 0x4000) == 0); // hflipped
			bool vflipped = (frameDir & 0x8000) != 0;

			frameDir &= 0xFFF;
			if (frameDirFlag) {
				frameDir = READ_LE_UINT16(sequenceData + frameDir * 2);
			}
			if (keyChar == 0) {
				if (_directionsTable[frameDir] <= _flagsTable[176]) {
					continue;
				}
			}
			if (frameDir == 0x800) {
				continue;
			}

			assert(spr->w != 0);
			int framesPerLine = spr->bitmapWidth / spr->w;
			assert(framesPerLine != 0);
			const int srcOffsX = spr->w * (frameDir % framesPerLine);
			const int srcOffsY = spr->h * (frameDir / framesPerLine);
			const uint8 *srcSpr = spr->ptr + srcOffsY * spr->bitmapWidth + srcOffsX;

			assert(dstY >= -500 && dstY < 500);
			int scalingIndex = _spriteScalingIndex[500 + dstY];
			int16 *yScaledTable = &_spriteScalingTable[scalingIndex];
			int sprScaledY = posY + scalingIndex - 500;
			y1 = MIN(y1, sprScaledY);

			if (facingDirection == 3) {
				dstX = -dstX;
			}
			assert(dstX >= -500 && dstX < 500);
			scalingIndex = _spriteScalingIndex[500 + dstX];
			int sprScaledX = posX + scalingIndex - 500;
			int16 *xScaledTable = &_spriteScalingTable[scalingIndex];
			x1 = MIN(x1, sprScaledX);
			x2 = MAX(x2, sprScaledX);

			uint8 *dstCur = dst + sprScaledY * dstPitch + sprScaledX;

			uint8 *dstStart = dstCur;
			int sprStartY = 0;
			while (1) {
				int sprCurY = *yScaledTable - dstY; ++yScaledTable;
				if (sprCurY >= spr->h) {
					break;
				}
				sprStartY = sprCurY - sprStartY;
				while (sprStartY != 0) {
					srcSpr += spr->bitmapWidth;
					--sprStartY;
				}
				sprStartY = sprCurY;

				int16 *scalingTable = xScaledTable;
				int spr_x2 = sprScaledX;
				dstCur = dstStart;
				if (sprScaledY < clippingRect_y1 || sprScaledY >= clippingRect_y2) {
					continue;
				}
				if (facingDirection != 3) {
					while (1) {
						int spr_x1 = *scalingTable - dstX; ++scalingTable;
						if (spr_x1 >= spr->w || spr_x2 >= clippingRect_x2) {
							break;
						}
						if (spr_x2 >= clippingRect_x1) {
							uint8 color = vflipped ? srcSpr[spr->w - 1 - spr_x1] : srcSpr[spr_x1];
							if (color != 0) {
								*dstCur = color;
							}
						}
						++spr_x2;
						++dstCur;
					}
					x2 = MAX(x2, spr_x2);
				} else {
					while (1) {
						int spr_x1 = dstX - *scalingTable; --scalingTable;
						if (spr_x1 >= spr->w || spr_x2 < clippingRect_x1) {
							break;
						}
						if (spr_x2 < clippingRect_x2) {
							uint8 color = vflipped ? srcSpr[spr->w - 1 - spr_x1] : srcSpr[spr_x1];
							if (color != 0) {
								*dstCur = color;
							}
						}
						--spr_x2;
						--dstCur;
					}
					x1 = MIN(x1, spr_x2);
				}
				++sprScaledY;
				dstStart += dstPitch;
			}
		}
	}
	if (walkDataNum != -1) {
		if (_flagsTable[604] == 0) {
			int area1 = _programWalkTable[walkDataNum].area1;
			if (area1 != 0) {
				findAndRedrawRoomRegion(area1);
			}
			int area2 = _programWalkTable[walkDataNum].area2;
			if (area2 != 0) {
				findAndRedrawRoomRegion(area2);
			}
		}
	}
}

void ToucheEngine::changeKeyCharFrame(KeyChar *key, int keyChar) {
	key->currentAnimSpeed = 0;
	key->currentAnimCounter = 0;
	if (key->currentAnim != 1) {
		int16 animStart, animCount;
		if (_currentObjectNum == keyChar && _flagsTable[901] == 1) {
			animStart = key->anim1Start;
			animCount = key->anim1Count;
		} else if (key->framesListCount != key->currentFrame) {
			animStart = key->framesList[key->currentFrame];
			++key->currentFrame;
			key->currentFrame &= 15;
			animCount = 0;
		} else {
			animStart = key->anim2Start;
			animCount = key->anim2Count;
			if (key->currentAnim >= animStart && key->currentAnim < animStart + animCount) {
				int rnd = getRandomNumber(100);
				if (key->flags & 0x10) {
					if (rnd >= 50 && rnd <= 55) {
						KeyChar *followingKey = &_keyCharsTable[key->followingKeyCharNum];
						int16 num = followingKey->pointsDataNum;
						if (num != 0 && followingKey->currentWalkBox != -1 && num != key->followingKeyCharPos) {
							key->followingKeyCharPos = num;
							sortPointsData(-1, num);
							buildWalkPointsList(key->num - 1);
						}
					}
				} else {
					if (rnd >= 50 && rnd <= 51) {
						animStart = key->anim3Start;
						animCount = key->anim3Count;
					}
				}
			}
		}
		if (animCount != 0) {
			animCount = getRandomNumber(animCount);
		}
		key->currentAnim = animStart + animCount;
	}
}

void ToucheEngine::setKeyCharRandomFrame(KeyChar *key) {
	key->currentAnimSpeed = 0;
	key->currentAnim = key->anim2Start + getRandomNumber(key->anim2Count);
	key->currentAnimCounter = 0;
}

void ToucheEngine::setKeyCharMoney() {
	_keyCharsTable[_currentKeyCharNum].money = _currentAmountOfMoney;
	drawAmountOfMoneyInInventory();
}

const char *ToucheEngine::getString(int num) const {
	if (num < 0) {
		return (const char *)_textData + READ_LE_UINT32(_textData - num * 4);
	} else {
		return (const char *)_programTextDataPtr + READ_LE_UINT32(_programTextDataPtr + num * 4);
	}
}

int ToucheEngine::getStringWidth(int m, int num) const {
	int w = 0;
	const char *str = getString(num);
	switch (m) {
	case 16:
		w = Graphics::getStringWidth16(str);
		break;
	}
	return w;
}

void ToucheEngine::drawString(uint8 *dst, int dstPitch, int m, uint16 color, int x, int y, int16 num) {
	if (num) {
		const char *str = getString(num);
		switch (m) {
		case 16:
			Graphics::drawString16(dst, dstPitch, color, x, y, str);
			break;
		}
	}
}

void ToucheEngine::drawGameString(int m, uint16 color, int x1, int y, const char *str) {
	int x, w;
	switch (m) {
	case 16:
		w = Graphics::getStringWidth16(str);
		x = x1 - w / 2;
		if (x + w >= 640) {
			x = 640 - w - 1;
		}
		while (*str) {
			char chr = *str++;
			if (chr == '\\') {
				y += 16;
				w = Graphics::getStringWidth16(str);
				x = x1 - w / 2;
			} else {
				if (x < 0) {
					x = 0;
				}
				x += Graphics::drawChar16(_offscreenBuffer, 640, chr, x, y, color);
			}
		}
		break;
	}
}

int ToucheEngine::restartKeyCharScriptOnAction(int action, int obj1, int obj2) {
	debugC(9, kDebugEngine, "ToucheEngine::restartKeyCharScriptOnAction(%d, %d, %d)", action, obj1, obj2);
	for (uint i = 0; i < _programActionScriptOffsetTable.size(); ++i) {
		const ProgramActionScriptOffsetData *pasod = &_programActionScriptOffsetTable[i];
		if (pasod->object1 == obj1 && pasod->action == action && pasod->object2 == obj2) {
			debug(0, "Found matching action i=%d %d,%d,%d\n", i, pasod->action, pasod->object1, pasod->object2);
			KeyChar *key = &_keyCharsTable[_currentKeyCharNum];
			key->scriptDataOffset = pasod->offset;
			key->scriptStackPtr = &key->scriptStackTable[39];
			key->flags &= ~(kScriptStopped | kScriptPaused);
			return 1;
		}
	}
	return 0;
}

void ToucheEngine::buildSpriteScalingTable(int z1, int z2) {
	debugC(9, kDebugEngine, "ToucheEngine::buildSpriteScalingTable(%d, %d)", z1, z2);
	if (z2 > 500) {
		z2 = 500;
	}
	if (z2 == 0) {
		z2 = 1;
	}

	const int scaleInc = z1 * 256 / z2;
	int scaleSum = 0;
	for (int i = 0; i < z2; ++i) {
		int value = scaleSum >> 8;
		assert(i < 500);
		_spriteScalingTable[500 + i] =  value;
		_spriteScalingTable[500 - i] = -value;
		scaleSum += scaleInc;
	}

	const int16 *p = &_spriteScalingTable[500];
	int16 z1_s = *p++;
	int16 z2_s = *p++;
	for (int i = 0, j = 0; j < z1; ++i) {
		while (z2_s != z1_s) {
			++z1_s;
			assert(j < 500);
			_spriteScalingIndex[500 + j] = i + 500;
			_spriteScalingIndex[500 - j] = 500 - i;
			if (j++ >= z1) {
				break;
			}
		}
		z1_s = z2_s;
		z2_s = *p++;
	}
}

void ToucheEngine::drawSpriteOnBackdrop(int num, int x, int y) {
	assert(num >= 0 && num < NUM_SPRITES);
	SpriteData *spr = &_spritesTable[num];
	Graphics::copyRect(_backdropBuffer, _currentBitmapWidth, x, y,
	  spr->ptr, spr->bitmapWidth, 0, 0,
	  spr->bitmapWidth, spr->bitmapHeight);
}

void ToucheEngine::updateTalkFrames(int keyChar) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	if (key->currentAnim >= key->anim1Start && key->currentAnim < key->anim1Start + key->anim1Count) {
		key->currentAnim = key->anim2Start;
		key->currentAnimCounter = 0;
		key->currentAnimSpeed = 0;
	}
}

void ToucheEngine::setKeyCharTalkingFrame(int keyChar) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	if (key->currentAnim != 1) {
		key->currentAnim = key->anim1Start;
		key->currentAnimCounter = 0;
		key->currentAnimSpeed = 0;
	}
}

void ToucheEngine::lockUnlockHitBox(int num, int lock) {
	for (uint i = 0; i < _programHitBoxTable.size(); ++i) {
		if (_programHitBoxTable[i].item == num) {
			if (lock) {
				_programHitBoxTable[i].hitBoxes[0].top |= 0x4000;
			} else {
				_programHitBoxTable[i].hitBoxes[0].top &= ~0x4000;
			}
		}
	}
}

void ToucheEngine::drawHitBoxes() {
	for (uint i = 0; i < _programHitBoxTable.size(); ++i) {
		uint16 state = _programHitBoxTable[i].state;
		if (state & 0x8000) {
			_programHitBoxTable[i].state = state & 0x7FFF;
			addToDirtyRect(_programHitBoxTable[i].hitBoxes[1]);
		}
	}
}

void ToucheEngine::setCursor(int num) {
	debugC(9, kDebugEngine, "ToucheEngine::setCursor(%d)", num);
	_currentCursorObject = num;
	const int cursorW = 58;
	const int cursorH = 42;
	res_loadImage(num, _mouseData);
	_system->setMouseCursor(_mouseData, cursorW, cursorH, cursorW / 2, cursorH / 2, 0);
	_system->showMouse(true);
}

void ToucheEngine::updateCursor(int num) {
	debugC(9, kDebugEngine, "ToucheEngine::updateCursor(%d)", num);
	if (_currentCursorObject != 0) {
		if (_currentCursorObject != 1) {
			addItemToInventory(num, _currentCursorObject);
			drawInventory(num, 1);
		}
		setCursor(0);
	}
}

void ToucheEngine::handleMouseButtonClicked() {
	for (int i = 0; i < 13; ++i) {
		if (_inventoryAreasTable[i].contains(_inp_mousePos)) {
			switch (i) {
			case 0:
				_keyCharsTable[_currentKeyCharNum].money += _currentAmountOfMoney;
				_currentAmountOfMoney = 0;
				ui_handleOptions(0);
				break;
			case 1:
				setKeyCharMoney();
				if (_currentCursorObject == 1) {
					setCursor(0);
				}
				break;
			case 2:
				if (_keyCharsTable[_currentKeyCharNum].money >= 10) {
					_keyCharsTable[_currentKeyCharNum].money -= 10;
					_currentAmountOfMoney += 10;
				}
				break;
			case 3:
				if (_keyCharsTable[_currentKeyCharNum].money != 0) {
					--_keyCharsTable[_currentKeyCharNum].money;
					++_currentAmountOfMoney;
					drawAmountOfMoneyInInventory();
				}
				break;
			case 4:
				if (_currentAmountOfMoney != 0) {
					updateCursor(_objectDescriptionNum);
					int money = _currentAmountOfMoney;
					_currentAmountOfMoney = 0;
					drawAmountOfMoneyInInventory();
					setCursor(1);
					_currentAmountOfMoney = money;
				}
				break;
			case 5:
				if (*_inventoryVar2 != 0) {
					*_inventoryVar2 -= 6;
					drawInventory(_objectDescriptionNum, 1);
				}
				break;
			case 12:
				if (_inventoryVar1[12 + *_inventoryVar2] != 0) {
					*_inventoryVar2 += 6;
					drawInventory(_objectDescriptionNum, 1);
				}
				break;
			default:
				if (i >= 6 && i <= 11) {
					int item = _inventoryVar1[i - 6 + *_inventoryVar2];
					_flagsTable[119] = _currentCursorObject;
					if (_currentCursorObject == 1) {
						setKeyCharMoney();
						_flagsTable[118] = _currentAmountOfMoney;
						_currentAmountOfMoney = 0;
					}
					if (item != 0 && _currentCursorObject != 0) {
						if (restartKeyCharScriptOnAction(-53, item | 0x1000, 0)) {
							updateCursor(_objectDescriptionNum);
							drawInventory(_objectDescriptionNum, 1);
						}
					} else {
						_inventoryVar1[i - 6 + *_inventoryVar2] = 0;
						if (_currentCursorObject != 0) {
							updateCursor(_objectDescriptionNum);
						}
						if (item != 0) {
							setCursor(item);
							packInventoryItems(0);
							packInventoryItems(1);
						}
						drawInventory(_objectDescriptionNum, 1);
					}
				}
				break;
			}
			break;
		}
	}
}

void ToucheEngine::handleMouseButtonPressed() {
	for (int pos = 0; pos < 13; ++pos) {
		const Common::Rect &r = _inventoryAreasTable[pos];
		if (r.contains(_inp_mousePos)) {
			if (pos >= 6 && pos <= 11) {
				int item = _inventoryVar1[pos - 6 + *_inventoryVar2] | 0x1000;
				for (uint i = 0; i < _programHitBoxTable.size(); ++i) {
					const ProgramHitBoxData *hitBox = &_programHitBoxTable[i];
					if (hitBox->item == item) {
						const int menuX = r.left + r.width() / 2;
						const int menuY = 352;
						int act = handleActionMenuUnderCursor(hitBox->actions, menuX, menuY, hitBox->str);
						if (act != 0) {
							restartKeyCharScriptOnAction(act, hitBox->item, 0);
						}
					}
				}
				break;
			}
		}
	}
}

void ToucheEngine::handleMouseInput(int flag) {
	if (_disabledInputCounter != 0 || _flagsTable[618] != 0) {
		_inp_mouseButtonPressed = false;
	}
	if (_inp_mousePos.y < _roomAreaRect.height()) {
		handleMouseInputRoomArea(flag);
	} else {
		handleMouseInputInventoryArea(flag);
	}
}

void ToucheEngine::handleMouseInputRoomArea(int flag) {
	if (_hideInventoryTexts && _conversationReplyNum != -1 && !_conversationAreaCleared) {
		drawConversationString(_conversationReplyNum, 0xD6);
	}
	if (_disabledInputCounter == 0 && !_hideInventoryTexts && _flagsTable[618] == 0) {
		bool itemSelected = false;
		bool stringDrawn = false;
		if (_conversationReplyNum != -1 && !_conversationAreaCleared && _giveItemToCounter == 0) {
			drawConversationString(_conversationReplyNum, 0xD6);
		}
		_conversationReplyNum = -1;
		int keyCharNewPosX = _flagsTable[614] + _inp_mousePos.x;
		int keyCharNewPosY = _flagsTable[615] + _inp_mousePos.y;
		for (uint i = 0; i < _programHitBoxTable.size(); ++i) {
			if (_programHitBoxTable[i].item & 0x1000) {
				break;
			}
			bool itemDisabled = false;
			Common::Rect *hitBox = &_programHitBoxTable[i].hitBoxes[0];
			int hitPosX = keyCharNewPosX;
			int hitPosY = keyCharNewPosY;
			int16 str = _programHitBoxTable[i].str;
			KeyChar *keyChar;
			switch (_programHitBoxTable[i].item & 0xF000) {
			case 0x1000:
				if (_inventoryItemsInfoTable[_programHitBoxTable[i].item & ~0x1000] != 0x20) {
					hitPosY = 10000;
				}
				break;
			case 0x2000:
				itemDisabled = true;
				break;
			case 0x4000:
				keyChar = &_keyCharsTable[_programHitBoxTable[i].item & ~0x4000];
				hitPosY = 10000;
				if (keyChar->num != 0) {
					if ((keyChar->flags & 0x4000) == 0) {
						if (keyChar->strNum != 0) {
							str = keyChar->strNum;
						}
						hitBox = &keyChar->prevBoundingRect;
						hitPosX = _inp_mousePos.x;
						hitPosY = _inp_mousePos.y;
					}
				}
				break;
			}
			if (_giveItemToCounter == 0 && !_hideInventoryTexts) {
				if (hitBox->contains(hitPosX, hitPosY)) {
					if (!itemDisabled) {
						if (_inp_mouseButtonClicked && _currentCursorObject != 0) {
							_inp_mouseButtonClicked = false;
							itemSelected = true;
							_flagsTable[119] = _currentCursorObject;
							if (_currentCursorObject == 1) {
								_flagsTable[118] = _currentAmountOfMoney;
								_currentAmountOfMoney = 0;
							}
							_inventoryItemsInfoTable[_currentCursorObject] = 0x20;
							setCursor(0);
							if (_giveItemToCounter == 0) {
								if (!restartKeyCharScriptOnAction(-53, _programHitBoxTable[i].item, 0)) {
									if (_flagsTable[119] == 1) {
										_currentAmountOfMoney = _flagsTable[118];
									} else {
										addItemToInventory(_currentKeyCharNum, _flagsTable[119]);
										drawInventory(_currentKeyCharNum, 1);
									}
									drawAmountOfMoneyInInventory();
								}
							} else {
								_flagsTable[117] = _programHitBoxTable[i].item - 1;
								_giveItemToCounter = -1;
							}
						}
						const char *strData = getString(str);
						int strPosY = _inp_mousePos.y - 22;
						if (_currentCursorObject != 0) {
							strPosY -= 8;
						}
						if (strPosY <= 0) {
							strPosY = 1;
						}
						int strWidth = getStringWidth(16, str);
						int strPosX = _inp_mousePos.x - strWidth / 2;
						strPosX = CLIP<int>(strPosX, 0, 640 - strWidth - 1);
						if (_talkTextSpeed != 0) {
							--_talkTextSpeed;
						}
						if (!stringDrawn && _talkTextSpeed == 0) {
							drawGameString(16, 0xFF, strPosX + strWidth / 2, strPosY, strData);
						}
						stringDrawn = true;
						Common::Rect redrawRect(strPosX, strPosY, strPosX + strWidth, strPosY + 16);
						if (_programHitBoxTable[i].state & 0x8000) {
							redrawRect.extend(_programHitBoxTable[i].hitBoxes[1]);
						}
						addToDirtyRect(redrawRect);
						_programHitBoxTable[i].hitBoxes[1] = Common::Rect(strPosX, strPosY, strPosX + strWidth, strPosY + 16);
						_programHitBoxTable[i].state |= 0x8000;
					}
					if (_inp_mouseButtonClicked) {
						_inp_mouseButtonClicked = false;
						if (_currentCursorObject != 0) {
							updateCursor(_currentKeyCharNum);
						} else {
							drawInventory(_currentKeyCharNum, 0);
							if (restartKeyCharScriptOnAction(-49, _programHitBoxTable[i].item, 0) == 0) {
								buildWalkPath(keyCharNewPosX, keyCharNewPosY, _currentKeyCharNum);
							}
						}
					} else {
						if (_inp_mouseButtonPressed && !itemDisabled && !itemSelected) {
							int act = handleActionMenuUnderCursor(_programHitBoxTable[i].actions, _inp_mousePos.x, _inp_mousePos.y, str);
							_inp_mouseButtonPressed = false;
							int16 facing = (keyCharNewPosX <= _keyCharsTable[_currentKeyCharNum].xPos) ? 3 : 0;
							_keyCharsTable[_currentKeyCharNum].facingDirection = facing;
							if (act != 0) {
								restartKeyCharScriptOnAction(act, _programHitBoxTable[i].item, 0);
							} else {
								act = _programHitBoxTable[i].talk;
								if (act != 0) {
									addToTalkTable(0, act, _currentKeyCharNum);
								}
							}
						}
					}
				} else if (_programHitBoxTable[i].state & 0x8000) {
					_programHitBoxTable[i].state &= 0x7FFF;
					addToDirtyRect(_programHitBoxTable[i].hitBoxes[1]);
				}
			}
		}
		if (_inp_mouseButtonClicked) {
			_inp_mouseButtonClicked = false;
			if (_currentCursorObject != 0) {
				if (_currentCursorObject != 1) {
					addItemToInventory(_currentKeyCharNum, _currentCursorObject);
					drawInventory(_objectDescriptionNum, 1);
				}
				setCursor(0);
			} else {
				drawInventory(_currentKeyCharNum, 0);
				buildWalkPath(keyCharNewPosX, keyCharNewPosY, _currentKeyCharNum);
			}
		}
	} else {
		if (flag) {
			drawHitBoxes();
		}
	}
}

void ToucheEngine::handleMouseInputInventoryArea(int flag) {
	if (flag) {
		drawHitBoxes();
	}
	if (_hideInventoryTexts && _giveItemToCounter == 0) {
		if (!_conversationAreaCleared) {
			if (_inp_mousePos.x >= 40) {
				if (_inp_mousePos.y >= 328) {
					int replyNum = (_inp_mousePos.y - 328) / 16;
					if (replyNum >= 4) {
						replyNum = 3;
					}
					if (replyNum != _conversationReplyNum) {
						if (_conversationReplyNum != -1) {
							drawConversationString(_conversationReplyNum, 0xD6);
						}
						drawConversationString(replyNum, 0xFF);
						_conversationReplyNum = replyNum;
					}
					if (_inp_mouseButtonClicked) {
						_inp_mouseButtonClicked = false;
						setupConversationScript(replyNum);
						_conversationReplyNum = -1;
					}
				}
			} else {
				if (_conversationReplyNum != -1 && !_conversationAreaCleared) {
					drawConversationString(_conversationReplyNum, 0xD6);
				}
				_conversationReplyNum = -1;
				if (_inp_mouseButtonClicked) {
					int replyNum = _inp_mousePos.y - _roomAreaRect.height();
					if (replyNum < 40) {
						drawCharacterConversationRepeat();
					} else {
						drawCharacterConversationRepeat2();
					}
					_inp_mouseButtonClicked = false;
				}
			}
		}
	} else if (_disabledInputCounter == 0 && !_hideInventoryTexts) {
		if (_inp_mouseButtonClicked) {
			handleMouseButtonClicked();
			_inp_mouseButtonClicked = false;
		}
		if (_inp_mouseButtonPressed) {
			handleMouseButtonPressed();
			_inp_mouseButtonPressed = false;
		}
	}
}

void ToucheEngine::scrollScreenToPos(int num) {
	_screenOffset.x = _programPointsTable[num].x - 640 / 2;
	_screenOffset.y = _programPointsTable[num].y - 400 / 2;
}

void ToucheEngine::clearRoomArea() {
	int h = (_flagsTable[606] != 0) ? 400 : _roomAreaRect.height();
	Graphics::fillRect(_offscreenBuffer, 640, 0, 0, 640, h, 0);
	updateEntireScreen();
}

void ToucheEngine::startNewMusic() {
	_snd_midiContext.unkA = _flagsTable[619] & 0xFF;
	if (_newMusicNum != 0 && _newMusicNum != _currentMusicNum) {
		_snd_midiContext.unkB = 3;
		if (_snd_midiContext.unkF != 0 && _snd_midiContext.unk2 != 0) {
			return;
		}
		_snd_midiContext.unkB = 0;
		res_loadMusic(_newMusicNum);
		_snd_midiContext.unk2 = 0;
		_newMusicNum = 0;
	}
}

void ToucheEngine::startNewSound() {
	if (_newSoundNum != 0) {
		if (_newSoundDelay == 0) {
			res_loadSound(_newSoundPriority, _newSoundNum);
			_newSoundNum = 0;
		} else {
			--_newSoundDelay;
		}
	}
}

void ToucheEngine::updateSpeech() {
	if (_speechPlaying) {
		if (!_mixer->isSoundHandleActive(_speechHandle)) {
			_speechPlaying = false;
			_defaultSoundPriority = 0;
		}
	}
}

int ToucheEngine::handleActionMenuUnderCursor(const int16 *actions, int offs, int y, int str) {
	if (*actions == 0 || _redrawScreenCounter1 != 0) {
		return -26;
	}
	int i;
	int16 actionsTable[10];
	int16 *currentAction = actionsTable;
	int drawY = 0;
	for (i = 0; i < 8; ++i) {
		int act = *actions++;
		if (act == 0) {
			break;
		}
		if (act != -49 && act != -53) {
			*currentAction++ = act;
			drawY = 1;
		}
	}
	if (drawY == 0) {
		return -26;
	}
	*currentAction = 0;
	int strW = getStringWidth(16, str);
	int h = 0;
	for (i = 0; i < 10; ++i) {
		if (actionsTable[i] == 0) {
			break;
		}
		++h;
		drawY = getStringWidth(16, actionsTable[i]);
		if (drawY > strW) {
			strW = drawY;
		}
	}
	int cursorW = strW + 28;
	int cursorPosX = CLIP<int16>(offs - cursorW / 2, 0, 640 - cursorW);
	offs = cursorPosX + 14;
	h *= 16;
	int cursorH = h + 28;
	int cursorPosY = CLIP<int16>(y - 24, 0, 352 - cursorH);
	y = cursorPosY + 24;
	_cursorObjectRect = Common::Rect(cursorPosX, cursorPosY, cursorPosX + cursorW, cursorPosY + cursorH);
	addToDirtyRect(_cursorObjectRect);

	Graphics::fillRect(_offscreenBuffer, 640, cursorPosX + 14, cursorPosY + 24, cursorW - 28, cursorH - 40, 0xF8);
	ui_drawActionsPanel(cursorPosX, cursorPosY, cursorW, cursorH);

	const char *strData = getString(str);
	drawGameString(16, 0xF8FF, offs + strW / 2, cursorPosY + 4, strData);
	for (i = 0; i < 10; ++i) {
		if (actionsTable[i] == 0) {
			break;
		}
		drawString(_offscreenBuffer, 640, 16, 0xF8F9, offs, y + i * 16, actionsTable[i]);
	}
	updateScreenArea(_offscreenBuffer, 640, cursorPosX, cursorPosY, cursorPosX, cursorPosY, cursorW, cursorH);

	_redrawScreenCounter1 = 2;
	Common::Rect rect(0, y, 640, y + h);
	i = -1;
	while (_inp_mouseButtonPressed) {
		if (rect.contains(_inp_mousePos)) {
			int c = (_inp_mousePos.y - y) / 16;
			if (c != i) {
				if (i >= 0) {
					drawY = y + i * 16;
					drawString(_offscreenBuffer, 640, 16, 0xF8F9, offs, drawY, actionsTable[i]);
					updateScreenArea(_offscreenBuffer, 640, offs, drawY, offs, drawY, strW, 16);
				}
				i = c;
				drawY = y + i * 16;
				drawString(_offscreenBuffer, 640, 16, 0xF8FF, offs, drawY, actionsTable[i]);
				updateScreenArea(_offscreenBuffer, 640, offs, drawY, offs, drawY, strW, 16);
			}
		} else if (i >= 0) {
			drawY = y + i * 16;
			drawString(_offscreenBuffer, 640, 16, 0xF8F9, offs, drawY, actionsTable[i]);
			updateScreenArea(_offscreenBuffer, 640, offs, drawY, offs, drawY, strW, 16);
			i = -1;
		}

		OSystem::Event event;
		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_QUIT:
				_flagsTable[611] = 1;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				_inp_mousePos.x = event.mouse.x;
				_inp_mousePos.y = event.mouse.y;
				break;
			case OSystem::EVENT_RBUTTONDOWN:
				_inp_mousePos.x = event.mouse.x;
				_inp_mousePos.y = event.mouse.y;
				_inp_mouseButtonPressed = true;
				break;
			case OSystem::EVENT_RBUTTONUP:
				_inp_mousePos.x = event.mouse.x;
				_inp_mousePos.y = event.mouse.y;
				_inp_mouseButtonPressed = false;
				break;
			default:
				break;
			}
		}
		_system->delayMillis(50);
	}

	const int action = (i >= 0) ? actionsTable[i] : -26;
	return action;
}

void ToucheEngine::redrawBackground() {
	for (uint i = 0; i < _programBackgroundTable.size(); ++i) {
		Area area = _programBackgroundTable[i].area;
		if (area.r.top != 20000) {
			area.r.translate(-_flagsTable[614], -_flagsTable[615]);
			if (_programBackgroundTable[i].type == 4) {
				int16 dx = _programBackgroundTable[i].offset - 640 / 2 - _flagsTable[614];
				dx *= _programBackgroundTable[i].scaleMul;
				dx /= _programBackgroundTable[i].scaleDiv;
				area.r.translate(dx, 0);
			}
			if (area.clip(_roomAreaRect)) {
				Graphics::copyRect(_offscreenBuffer, 640, area.r.left, area.r.top,
				  _backdropBuffer, _currentBitmapWidth, area.srcX, area.srcY,
				  area.r.width(), area.r.height(),
				  Graphics::kTransparent);
				addToDirtyRect(area.r);
			}
		}
	}
}

void ToucheEngine::processAreaTable() {
	debugC(9, kDebugEngine, "ToucheEngine::processAreaTable()");
//	for (int i = 0; i < _areaTableCount; ++i) {
//		Rect r(_areaTable[i].r);
//		if (rectClip(&_roomAreaRect, &r)) {
//			addToDirtyRect(&r);
//		}
//	}
}

void ToucheEngine::clearAreaTable() {
	debugC(9, kDebugEngine, "ToucheEngine::clearAreaTable()");
	_areaTableCount = 0;
}

void ToucheEngine::addToAreaTable(const Area *area) {
	debugC(9, kDebugEngine, "ToucheEngine::addToAreaTable()");
	assert(_areaTableCount < NUM_AREAS);
	_areaTable[_areaTableCount] = *area;
	++_areaTableCount;
}

void ToucheEngine::addRoomArea(int num, int flag) {
	debugC(9, kDebugEngine, "ToucheEngine::addRoomArea(%d, %d)", num, flag);
	if (_flagsTable[flag] == 20000) {
		Area area = _programBackgroundTable[num].area;
		area.r.translate(-_flagsTable[614], -_flagsTable[615]);
		if (area.clip(_roomAreaRect)) {
			addToAreaTable(&area);
		}
	}
	_programBackgroundTable[num].area.r.moveTo(_flagsTable[flag], _flagsTable[flag + 1]);
}

void ToucheEngine::updateRoomAreas(int num, int flags) {
	debugC(9, kDebugEngine, "ToucheEngine::updateRoomAreas(%d, %d)", num, flags);
	if (flags != -1) {
		int16 count = _updatedRoomAreasTable[0];
		++_updatedRoomAreasTable[0];
		if (count == 199) {
			_updatedRoomAreasTable[0] = 2;
			count = 1;
		}
		_updatedRoomAreasTable[count] = (uint8)num;
	}
	for (uint i = 0; i < _programAreaTable.size(); ++i) {
		if (_programAreaTable[i].id == num) {
			Area area = _programAreaTable[i].area;
			Graphics::copyRect(_backdropBuffer, _currentBitmapWidth, area.r.left, area.r.top,
			  _backdropBuffer, _currentBitmapWidth, area.srcX, area.srcY,
			  area.r.width(), area.r.height(),
			  Graphics::kTransparent);
			if (flags != 0) {
				area.r.translate(-_flagsTable[614], -_flagsTable[615]);
				if (area.clip(_roomAreaRect)) {
					addToAreaTable(&area);
				}
			}
		}
	}
}

void ToucheEngine::setRoomAreaState(int num, uint16 state) {
	debugC(9, kDebugEngine, "ToucheEngine::setRoomAreaState(%d, %d)", num, state);
	for (uint i = 0; i < _programAreaTable.size(); ++i) {
		if (_programAreaTable[i].id == num) {
			_programAreaTable[i].state = state;
		}
	}
}

void ToucheEngine::findAndRedrawRoomRegion(int num) {
	debugC(9, kDebugEngine, "ToucheEngine::findAndRedrawRoomRegion(%d)", num);
	for (uint i = 0; i < _programAreaTable.size(); ++i) {
		if (_programAreaTable[i].id == num) {
			redrawRoomRegion(i, 0);
			break;
		}
	}
}

void ToucheEngine::updateRoomRegions() {
	debugC(9, kDebugEngine, "ToucheEngine::updateRoomRegions()");
	if (_flagsTable[269] == 0) {
		uint i = 0;
		while (i < _programAreaTable.size() && _programAreaTable[i].id != 0) {
			switch (_programAreaTable[i].state) {
			case 0:
				++i;
				break;
			case 1:
				redrawRoomRegion(i + _programAreaTable[i].animNext, true);
				++_programAreaTable[i].animNext;
				if (_programAreaTable[i].animNext >= _programAreaTable[i].animCount) {
					_programAreaTable[i].animNext = 0;
				}
				i += _programAreaTable[i].animCount;
				break;
			case 3:
				redrawRoomRegion(i + _programAreaTable[i].animNext, true);
				++_programAreaTable[i].animNext;
				if (_programAreaTable[i].animNext >= _programAreaTable[i].animCount) {
					_programAreaTable[i].animNext = 0;
				}
				i += _programAreaTable[i].animCount + 1;
				break;
			}
		}
	}
}

void ToucheEngine::redrawRoomRegion(int num, bool markForRedraw) {
	debugC(9, kDebugEngine, "ToucheEngine::redrawRoomRegion(%d)", num);
	Area area = _programAreaTable[num].area;
	area.r.translate(-_flagsTable[614], -_flagsTable[615]);
	if (area.clip(_roomAreaRect)) {
		Graphics::copyRect(_offscreenBuffer, 640, area.r.left, area.r.top,
		  _backdropBuffer, _currentBitmapWidth, area.srcX, area.srcY,
		  area.r.width(), area.r.height(),
		  Graphics::kTransparent);
		if (markForRedraw) {
			addToDirtyRect(area.r);
		}
	}
}

void ToucheEngine::initInventoryObjectsTable() {
	for (int i = 0; i < NUM_INVENTORY_ITEMS; ++i) {
		_inventoryItemsInfoTable[i] = 0x20;
	}
}

void ToucheEngine::initInventoryLists() {
	memset(_inventoryList1, 0, sizeof(_inventoryList1));
	_inventoryList1[100] = -1;
	_inventoryListPtrs[0] = _inventoryList1;
	_inventoryListCount[3 * 0 + 0] = 0; // start offset
	_inventoryListCount[3 * 0 + 1] = 100; // max number of items
	_inventoryListCount[3 * 0 + 2] = 6; // items per inventory line

	memset(_inventoryList2, 0, sizeof(_inventoryList2));
	_inventoryList2[100] = -1;
	_inventoryListPtrs[1] = _inventoryList2;
	_inventoryListCount[3 * 1 + 0] = 0;
	_inventoryListCount[3 * 1 + 1] = 100;
	_inventoryListCount[3 * 1 + 2] = 6;

	memset(_inventoryList3, 0, sizeof(_inventoryList3));
	_inventoryList3[6] = -1;
	_inventoryListPtrs[2] = _inventoryList3;
	_inventoryListCount[3 * 2 + 0] = 0;
	_inventoryListCount[3 * 2 + 1] = 6;
	_inventoryListCount[3 * 2 + 2] = 6;
}

void ToucheEngine::drawInventory(int index, int flag) {
	if (_flagsTable[606] == 0) {
		if (index > 1) {
			index = 1;
		}
		if (_objectDescriptionNum == index && flag == 0) {
			return;
		}
		_inventoryVar1 = _inventoryListPtrs[index];
		_inventoryVar2 = &_inventoryListCount[index * 3];
		_objectDescriptionNum = index;
		uint8 *dst = _offscreenBuffer + 640 * 352;
		res_loadSpriteImage(index + 12, dst);
		res_loadImageHelper(dst, _currentImageWidth, _currentImageHeight);
		int firstObjNum = _inventoryVar2[0];
		for (int i = 0, x = 242; i < 6; ++i, x += 58) {
			int num = _inventoryVar1[firstObjNum + i];
			if (num == -1) {
				break;
			}
			if (num != 0) {
				drawIcon(x + 3, 353, num);
			}
		}
		drawAmountOfMoneyInInventory();
		updateScreenArea(_offscreenBuffer, 640, 0, 352, 0, 352, 640, 48);
	}
}

void ToucheEngine::drawAmountOfMoneyInInventory() {
	if (_flagsTable[606] == 0 && !_hideInventoryTexts) {
		char text[4];
		snprintf(text, sizeof(text)-1, "%d", _keyCharsTable[0].money);
		Graphics::fillRect(_offscreenBuffer, 640, 74, 354, 40, 16, 0xD2);
		drawGameString(16, 217, 94, 355, text);
		updateScreenArea(_offscreenBuffer, 640, 74, 354, 74, 354, 40, 16);
		Graphics::fillRect(_offscreenBuffer, 640, 150, 353, 40, 41, 0xD2);
		if (_currentAmountOfMoney != 0) {
			drawIcon(141, 348, 1);
			snprintf(text, sizeof(text)-1, "%d", _currentAmountOfMoney);
			drawGameString(16, 217, 170, 378, text);
		}
		updateScreenArea(_offscreenBuffer, 640, 150, 353, 150, 353, 40, 41);
	}
}

void ToucheEngine::packInventoryItems(int index) {
	int16 *p = _inventoryListPtrs[index];
	for (int i = 0; *p != -1; ++i, ++p) {
		if (p[0] == 0 && p[1] != -1) {
			p[0] = p[1];
			p[1] = 0;
		}
	}
}

void ToucheEngine::appendItemToInventoryList(int index) {
	int last = _inventoryListCount[index * 3 + 1] - 1;
	int16 *p = _inventoryListPtrs[index];
	if (p[last] != 0) {
		warning("Inventory %d Full", index);
	} else {
		for (int i = last; i > 0; --i) {
			p[i] = p[i - 1];
		}
		*p = 0;
	}
}

void ToucheEngine::addItemToInventory(int inventory, int16 item) {
	if (item == 0) {
		packInventoryItems(inventory);
	} else if (item == 1)  {
		_currentAmountOfMoney += _flagsTable[118];
		drawAmountOfMoneyInInventory();
	} else {
		appendItemToInventoryList(inventory);
		assert(inventory >= 0 && inventory < 3);
		int16 *p = _inventoryListPtrs[inventory];
		for (int i = 0; *p != -1; ++i, ++p) {
			if (*p == 0) {
				*p = item;
				_inventoryItemsInfoTable[item] = inventory | 0x10;
				packInventoryItems(0);
				packInventoryItems(1);
				break;
			}
		}
	}
}

void ToucheEngine::removeItemFromInventory(int inventory, int16 item) {
	if (item == 1) {
		_currentAmountOfMoney = 0;
		drawAmountOfMoneyInInventory();
	} else {
		assert(inventory >= 0 && inventory < 3);
		int16 *p = _inventoryListPtrs[inventory];
		for (int i = 0; *p != -1; ++i, ++p) {
			if (*p == item) {
				*p = 0;
				packInventoryItems(0);
				packInventoryItems(1);
				break;
			}
		}
	}
}

void ToucheEngine::changeInventoryItemState(int flag, int itemNum, int itemRnd, int inventoryItem) {
	const int rnd = getRandomNumber(100) + 1;
	if (inventoryItem) {
		itemNum = _keyCharsTable[_currentKeyCharNum].inventoryItems[itemNum];
	}
	if (_flagsTable[174]) {
		itemNum /= 2;
		_flagsTable[174] = 0;
	}
	int16 value;
	if (itemNum > itemRnd) {
		value = 1;
	} else if (rnd < itemNum / 6) {
		value = 0;
	} else if (rnd <= itemNum) {
		value = 1;
	} else if (rnd >= itemNum * 2) {
		value = 2;
	} else {
		value = 3;
	}
	_flagsTable[flag] = value;
}

void ToucheEngine::resetTalkingVars() {
	_talkListCurrent = 0;
	_talkListEnd = 0;
	_keyCharTalkCounter = 0;
	_talkTextRectDefined = false;
	_talkTextDisplayed = false;
	_skipTalkText = false;
	_talkTextInitialized = false;
	if (_speechPlaying) {
		res_stopSpeech();
	}
}

int ToucheEngine::updateKeyCharTalk(int skipFlag) {
	if (skipFlag != 0) {
		if (_speechPlaying) {
			res_stopSpeech();
		}
		if (_talkListEnd != _talkListCurrent) {
			_keyCharTalkCounter = 0;
			_talkTextInitialized = false;
			if (skipFlag == 2) {
				_skipTalkText = true;
			} else {
				_skipTalkText = false;
			}
		}
		return 0;
	}
	if (_talkListEnd == _talkListCurrent) {
		return 0;
	}
	int talkingKeyChar = _talkTable[_talkListCurrent].talkingKeyChar;
	int otherKeyChar = _talkTable[_talkListCurrent].otherKeyChar;
	KeyChar *key = &_keyCharsTable[talkingKeyChar];
	int x = key->xPos - _flagsTable[614];
	int y = key->yPos - _flagsTable[615] - (key->zPos / 2 + 16);
	int stringNum = _talkTable[_talkListCurrent].num;
	const char *stringData = getString(stringNum);
	int textWidth = getStringWidth(16, stringNum);
	if (!_talkTextInitialized && !_skipTalkText) {
		_keyCharTalkCounter = textWidth / 32 + 20;
		setKeyCharTalkingFrame(talkingKeyChar);
		res_loadSpeechSegment(stringNum);
		_talkTextInitialized = true;
	}
	if (_keyCharTalkCounter) {
		--_keyCharTalkCounter;
	}
	if (_speechPlaying) {
		_flagsTable[297] = 0;
		if (_talkTextMode == kTalkModeVoiceOnly) {
			_keyCharTalkCounter = 0;
		}
		_currentObjectNum = talkingKeyChar;
		if (_keyCharTalkCounter == 0) {
			_keyCharTalkCounter = 1;
		}
	}
	if (_talkTextMode == kTalkModeVoiceOnly) {
		if (_speechPlaying) {
			return 1;
		}
	}
	if (_keyCharTalkCounter != 0) {
		_currentObjectNum = talkingKeyChar;
		_talkTextDisplayed = true;
		int textHeight = 16;
		y -= 16;
		if (y < 0) {
			y = 1;
		} else if (y > 352) {
			y = 336;
		}
		if (textWidth > 200) {
			textWidth = 200;
			stringData = formatTalkText(16, &y, &textHeight, stringData);
		}
		x -= textWidth / 2;
		if (x < 0) {
			x = 0;
		}
		if (x + textWidth >= 640) {
			x = 640 - textWidth - 1;
		}
		drawGameString(16, key->textColor, x + textWidth / 2, y, stringData);
		_talkTextSpeed = 6;
		_talkTextRect = Common::Rect(x, y, x + textWidth, y + textHeight);
		if (_talkTextRectDefined) {
			_talkTextRect.extend(_talkTextRect2);
		}
		addToDirtyRect(_talkTextRect);
		_talkTextRect2 = Common::Rect(x, y, x + textWidth, y + textHeight);
		_talkTextRectDefined = true;
		_flagsTable[297] = 0;
	} else {
		updateTalkFrames(_currentObjectNum);
		_currentObjectNum = -1;
		if (_talkTextDisplayed) {
			addToDirtyRect(_talkTextRect2);
		}
		_talkTextInitialized = false;
		_skipTalkText = false;
		_talkTextRectDefined = false;
		++_talkListCurrent;
		if (_talkListCurrent == 16) {
			_talkListCurrent = 0;
		}
		if (otherKeyChar != -1) {
			_keyCharsTable[otherKeyChar].flags &= ~kScriptPaused;
		}
	}
	return 1;
}

const char *ToucheEngine::formatTalkText(int mode, int *y, int *h, const char *text) {
	int newLineWidth = 0;
	int lineWidth = 0;
	char *textBuffer = _talkTextBuffer;
	char *textLine = textBuffer;
	if (mode != 16) {
		return text;
	} else {
		while (*text) {
			char chr = *text++;
			int chrWidth = Graphics::getCharWidth16(chr);
			lineWidth += chrWidth;
			if (chr == ' ') {
				if (lineWidth + newLineWidth >= 200) {
					*textLine = '\\';
					newLineWidth = lineWidth - chrWidth;
					*y -= mode;
					*h += mode;
					lineWidth = chrWidth;
				} else {
					newLineWidth += lineWidth;
					lineWidth = chrWidth;
				}
				*textBuffer = ' ';
				textLine = textBuffer;
				textBuffer++;
			} else {
				*textBuffer++ = chr;
			}
		}
		if (newLineWidth + lineWidth >= 200) {
			*textLine = '\\';
			*y -= mode;
			*h += mode;
		}
		*textBuffer = '\0';
		if (*y < 0) {
			*y = 1;
		}
		return _talkTextBuffer;
	}
}

void ToucheEngine::addToTalkTable(int talkingKeyChar, int num, int otherKeyChar) {
	if (_talkListEnd != _talkListCurrent) {
		if (_talkTableLastTalkingKeyChar == talkingKeyChar &&
			_talkTableLastOtherKeyChar == otherKeyChar &&
			_talkTableLastStringNum == num) {
			return;
		}
	}
	_talkTableLastTalkingKeyChar = talkingKeyChar;
	_talkTableLastOtherKeyChar = otherKeyChar;
	_talkTableLastStringNum = num;

	removeFromTalkTable(otherKeyChar);

	assert(_talkListEnd < NUM_TALK_ENTRIES);
	TalkEntry *talkEntry = &_talkTable[_talkListEnd];
	talkEntry->talkingKeyChar = talkingKeyChar;
	talkEntry->otherKeyChar = otherKeyChar;
	talkEntry->num = num;

	++_talkListEnd;
	if (_talkListEnd == NUM_TALK_ENTRIES) {
		_talkListEnd = 0;
	}
}

void ToucheEngine::removeFromTalkTable(int keyChar) {
	debugC(9, kDebugEngine, "ToucheEngine::removeFromTalkTable(%d)", keyChar);
	int i = _talkListCurrent;
	while (i != _talkListEnd) {
		if (_talkTable[i].otherKeyChar == keyChar) {
			_talkTable[i].otherKeyChar = -1;
		}
		++i;
		i %= NUM_TALK_ENTRIES;
	}
}

void ToucheEngine::addConversationChoice(int16 num) {
	debugC(9, kDebugEngine, "ToucheEngine::addConversationChoice(%d)", num);
	_conversationChoicesUpdated = true;
	int16 msg = _programConversationTable[_currentConversation + num].msg;
	for (int i = 0; i < NUM_CONVERSATION_CHOICES; ++i) {
		if (_conversationChoicesTable[i].msg == msg) {
			break;
		}
		if (_conversationChoicesTable[i].msg == 0) {
			_conversationChoicesTable[i].msg = msg;
			_conversationChoicesTable[i].num = num;
			break;
		}
	}
}

void ToucheEngine::removeConversationChoice(int16 num) {
	debugC(9, kDebugEngine, "ToucheEngine::removeConversationChoice(%d)", num);
	for (int i = 0; i < NUM_CONVERSATION_CHOICES; ++i) {
		if (_conversationChoicesTable[i].num == num) {
			_conversationChoicesUpdated = true;
			for(; i < NUM_CONVERSATION_CHOICES - 1; ++i) {
				_conversationChoicesTable[i].num = _conversationChoicesTable[i + 1].num;
				_conversationChoicesTable[i].msg = _conversationChoicesTable[i + 1].msg;
			}
			break;
		}
	}
}

void ToucheEngine::runConversationScript(uint16 offset) {
	debugC(9, kDebugEngine, "ToucheEngine::runConversationScript() offset=0x%X", offset);
	_script.dataOffset = offset;
	_script.quitFlag = 0;
	runCurrentKeyCharScript(2);
}

void ToucheEngine::findConversationByNum(int16 num) {
	debugC(9, kDebugEngine, "ToucheEngine::findConversationByNum(%d)", num);
	for (uint i = 0; i < _programConversationTable.size(); ++i) {
		if (_programConversationTable[i].num == num) {
			clearConversationChoices();
			_currentConversation = i;
			runConversationScript(_programConversationTable[i].offset);
			break;
		}
	}
}

void ToucheEngine::clearConversationChoices() {
	debugC(9, kDebugEngine, "ToucheEngine::clearConversationChoices()");
	_conversationChoicesUpdated = true;
	for (int i = 0; i < NUM_CONVERSATION_CHOICES; ++i) {
		_conversationChoicesTable[i].num = 0;
		_conversationChoicesTable[i].msg = 0;
	}
	_drawCharacterConversionRepeatCounter = 0;
}

void ToucheEngine::drawCharacterConversationRepeat2() {
	if (_conversationChoicesTable[4 + _drawCharacterConversionRepeatCounter].msg != 0) {
		++_drawCharacterConversionRepeatCounter;
		drawCharacterConversation();
	}
}

void ToucheEngine::drawCharacterConversationRepeat() {
	if (_drawCharacterConversionRepeatCounter != 0) {
		--_drawCharacterConversionRepeatCounter;
		drawCharacterConversation();
	}
}

void ToucheEngine::drawCharacterConversation() {
	_conversationChoicesUpdated = false;
	if (!_disableConversationScript) {
		if (_conversationChoicesTable[0].msg == 0) {
			_conversationEnded = true;
			return;
		}
		if (_conversationChoicesTable[1].msg == 0) {
			setupConversationScript(0);
			return;
		}
	}
	ui_drawConversationPanel();
	for (int i = 0; i < 4; ++i) {
		drawString(_offscreenBuffer, 640, 16, 214, 42, 328 + i * 16, _conversationChoicesTable[_drawCharacterConversionRepeatCounter + i].msg);
	}
	updateScreenArea(_offscreenBuffer, 640, 0, 320, 0, 320, 640, 80);
	_conversationAreaCleared = false;
}

void ToucheEngine::drawConversationString(int num, uint16 color) {
	const int y = 328 + num * 16;
	drawString(_offscreenBuffer, 640, 16, color, 42, y, _conversationChoicesTable[num + _drawCharacterConversionRepeatCounter].msg);
	updateScreenArea(_offscreenBuffer, 640, 0, y, 0, y, 640, 16);
}

void ToucheEngine::clearConversationArea() {
	ui_drawConversationPanel();
	updateScreenArea(_offscreenBuffer, 640, 0, 320, 0, 320, 640, 80);
	_conversationAreaCleared = true;
}

void ToucheEngine::setupConversationScript(int num) {
	debugC(9, kDebugEngine, "ToucheEngine::setupConversationScript(%d)", num);
	if (num < 5 && _conversationChoicesTable[num].msg != 0) {
		num = _conversationChoicesTable[_drawCharacterConversionRepeatCounter + num].num;
		KeyChar *key = &_keyCharsTable[_currentKeyCharNum];
		key->scriptDataOffset = _programConversationTable[_currentConversation + num].offset;
		key->scriptStackPtr = &key->scriptStackTable[39];
		_drawCharacterConversionRepeatCounter = 0;
		removeConversationChoice(num);
		clearConversationArea();
	}
}

void ToucheEngine::handleConversation() {
	if (_conversationNum != 0) {
		findConversationByNum(_conversationNum);
		_conversationAreaCleared = false;
		drawCharacterConversation();
		_roomAreaRect.setHeight(320);
		_hideInventoryTexts = true;
		_conversationEnded = false;
		_conversationNum = 0;
	} else if (_hideInventoryTexts && _conversationAreaCleared) {
		if (_keyCharsTable[_currentKeyCharNum].scriptDataOffset == 0) {
			drawCharacterConversation();
		}
	} else if (!_conversationAreaCleared && _conversationChoicesUpdated) {
		drawCharacterConversation();
	}
}

static int getDirection(int x1, int y1, int z1, int x2, int y2, int z2) {
	int ret = -1;
	x2 -= x1;
	y2 -= y1;
	z2 -= z1;
	if (x2 == 0 && y2 == 0 && z2 == 0) {
		ret = -2;
	} else {
		if (ABS(x2) >= ABS(z2)) {
			if (ABS(x2) > ABS(y2)) {
				if (x2 > 0) {
					ret = 0;
				} else {
					ret = 3;
				}
			} else {
				if (y2 > 0) {
					ret = 1;
				} else {
					ret = 2;
				}
			}
		} else {
			if (z2 != 0) {
				if (z2 > 0) {
					ret = 1;
				} else {
					ret = 2;
				}
			} else {
				if (y2 > 0) {
					ret = 1;
				} else {
					ret = 2;
				}
			}
		}
	}
	return ret;
}

void ToucheEngine::buildWalkPointsList(int keyChar) {
	debugC(9, kDebugEngine, "ToucheEngine::buildWalkPointsList(%d)", keyChar);
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	uint16 curPos, pos1, pos2;
	if (key->pointsDataNum & 0x8000) {
		const ProgramWalkData *pwd = &_programWalkTable[(key->pointsDataNum & 0x7FFF)];
		if (_programPointsTable[pwd->point1].priority < _programPointsTable[pwd->point2].priority) {
			curPos = pwd->point1;
		} else {
			curPos = pwd->point2;
		}
	} else {
		curPos = key->pointsDataNum;
	}

	int16 posNum = _programPointsTable[curPos].priority;
	if (posNum == 32000) {
		return;
	}
	key->walkPointsList[0] = curPos;
	int16 walkPointsCount = 1;
	do {
		for (uint i = 0; i < _programWalkTable.size(); ++i) {
			if ((_programWalkTable[i].point1 & 0x4000) == 0) {
				pos1 = _programWalkTable[i].point1;
				pos2 = _programWalkTable[i].point2;
				if (pos1 == curPos && posNum > _programPointsTable[pos2].priority) {
					curPos = pos2;
					assert(walkPointsCount < 40);
					key->walkPointsList[walkPointsCount] = curPos;
					++walkPointsCount;
					posNum = _programPointsTable[pos2].priority;
					break;
				}
				if (pos2 == curPos && posNum > _programPointsTable[pos1].priority) {
					curPos = pos1;
					assert(walkPointsCount < 40);
					key->walkPointsList[walkPointsCount] = curPos;
					++walkPointsCount;
					posNum = _programPointsTable[pos1].priority;
					break;
				}
			}
		}
	} while (_programPointsTable[curPos].priority != 0);
	assert(walkPointsCount < 40);
	key->walkPointsList[walkPointsCount] = -1;

	key->xPosPrev = _programPointsTable[curPos].x;
	key->yPosPrev = _programPointsTable[curPos].y;
	key->zPosPrev = _programPointsTable[curPos].z;
	key->prevWalkDataNum = findWalkDataNum(curPos, -1);
	key->walkPointsListCount = 0;
	if (key->walkDataNum == -1) {
		return;
	}

	pos1 = _programWalkTable[key->walkDataNum].point1;
	pos2 = _programWalkTable[key->walkDataNum].point2;
	if (key->pointsDataNum == pos1) {
		if (key->walkPointsList[1] == pos2) {
			++key->walkPointsListCount;
		}
		return;
	}
	if (key->pointsDataNum == pos2) {
		if (key->walkPointsList[1] == pos1) {
			++key->walkPointsListCount;
		}
		return;
	}
}

int ToucheEngine::findWalkDataNum(int pointNum1, int pointNum2) {
	debugC(9, kDebugEngine, "ToucheEngine::findWalkDataNum(%d, %d)", pointNum1, pointNum2);
	if (pointNum1 != pointNum2) {
		for (uint i = 0; i < _programWalkTable.size(); ++i) {
			int p1 = _programWalkTable[i].point1 & 0xFFF;
			int p2 = _programWalkTable[i].point2 & 0xFFF;
			if (p1 == pointNum1) {
				if (p2 == pointNum2 || pointNum2 == 10000) {
					return i;
				}
			} else if (p2 == pointNum1) {
				if (p1 == pointNum2 || pointNum2 == 10000) {
					return i;
				}
			}
		}
	}
	return -1;
}

void ToucheEngine::changeWalkPath(int num1, int num2, int16 val) {
	debugC(9, kDebugEngine, "ToucheEngine::changeWalkPath(%d, %d)", num1, num2);
	int num = findWalkDataNum(num1, num2);
	if (num != -1) {
		_programWalkTable[num].area1 = val;
	}
}

void ToucheEngine::adjustKeyCharPosToWalkBox(KeyChar *key, int moveType) {
	const ProgramWalkData *pwd = &_programWalkTable[key->walkDataNum];

	const ProgramPointData *pts1 = &_programPointsTable[pwd->point1];
	int16 x1 = pts1->x;
	int16 y1 = pts1->y;
	int16 z1 = pts1->z;

	const ProgramPointData *pts2 = &_programPointsTable[pwd->point2];
	int16 x2 = pts2->x;
	int16 y2 = pts2->y;
	int16 z2 = pts2->z;

	int16 kx = key->xPos;
	int16 ky = key->yPos;
	int16 kz = key->zPos;

	int16 dx = x2 - x1;
	int16 dy = y2 - y1;
	int16 dz = z2 - z1;

	switch (moveType) {
	case 0:
		kx -= x1;
		if (dx != 0) {
			key->yPos = dy * kx / dx + y1;
			key->zPos = dz * kx / dx + z1;
		}
		break;
	case 1:
		ky -= y1;
		if (dy != 0) {
			key->xPos = dx * ky / dy + x1;
			key->zPos = dz * ky / dy + z1;
		}
		break;
	case 2:
		kz -= z1;
		if (dz != 0) {
			key->xPos = dx * kz / dz + x1;
			key->yPos = dy * kz / dz + y1;
		}
		break;
	}
}

void ToucheEngine::lockWalkPath(int num1, int num2) {
	debugC(9, kDebugEngine, "ToucheEngine::lockWalkPath(%d, %d)", num1, num2);
	const int num = findWalkDataNum(num1, num2);
	if (num != -1) {
		_programWalkTable[num].point1 |= 0x4000;
		_programWalkTable[num].point2 |= 0x4000;
	}
}

void ToucheEngine::unlockWalkPath(int num1, int num2) {
	debugC(9, kDebugEngine, "ToucheEngine::unlockWalkPath(%d, %d)", num1, num2);
	const int num = findWalkDataNum(num1, num2);
	if (num != -1) {
		_programWalkTable[num].point1 &= 0xFFF;
		_programWalkTable[num].point2 &= 0xFFF;
	}
}

void ToucheEngine::resetPointsData(int num) {
	debugC(9, kDebugEngine, "ToucheEngine::resetPointsData(%d)", num);
	for (uint i = 1; i < _programPointsTable.size(); ++i) {
		_programPointsTable[i].priority = num;
	}
}

bool ToucheEngine::sortPointsData(int num1, int num2) {
	debugC(9, kDebugEngine, "ToucheEngine::sortPointsData(%d, %d)", num1, num2);
	resetPointsData(32000);
	if (num1 == -1) {
		if (num2 == -1) {
			return false;
		}
		_programPointsTable[num2].priority = 0;
	} else {
		const int md1 = _programWalkTable[num1].point1;
		_programPointsTable[md1].priority = 0;
		const int md2 = _programWalkTable[num1].point2;
		_programPointsTable[md2].priority = 0;
	}
	bool quit = false;
	int priority = 1;
	while (!quit) {
		quit = true;
		for (uint i = 0; i < _programWalkTable.size(); ++i) {
			const int md1 = _programWalkTable[i].point1;
			const int md2 = _programWalkTable[i].point2;
			if ((md1 & 0x4000) == 0) {
				if (_programPointsTable[md1].priority == priority - 1 && _programPointsTable[md2].priority > priority) {
					_programPointsTable[md2].priority = priority;
					quit = false;
				}
				if (_programPointsTable[md2].priority == priority - 1 && _programPointsTable[md1].priority > priority) {
					_programPointsTable[md1].priority = priority;
					quit = false;
				}
			}
		}
		++priority;
	}
	return true;
}

void ToucheEngine::updateKeyCharWalkPath(KeyChar *key, int16 dx, int16 dy, int16 dz) {
	debugC(9, kDebugEngine, "ToucheEngine::updateKeyCharWalkPath(key=%d, dx=%d, dy=%d, dz=%d)", key - _keyCharsTable, dx, dy, dz);
	if (key->walkDataNum == -1) {
		return;
	}
	int16 kx = key->xPos;
	int16 ky = key->yPos;
	int16 kz = key->zPos;
	if (kz != 160) {
		if (dx != 0) {
			dx = dx * kz / 160;
			if (dx == 0) {
				dx = 1;
			}
		}
		if (dy != 0) {
			dy = dy * kz / 160;
			if (dy == 0) {
				dy = 1;
			}
		}
		if (dz != 0) {
			dz = dz * kz / 160;
			if (dz == 0) {
				dz = 1;
			}
		}
	}

	int16 curDirection = key->facingDirection;
	if (key->currentAnim > 1) {
		if (dx != 0 || dy != 0 || dz != 0) {
			if (curDirection == 3) {
				key->xPos -= dx;
			} else {
				key->xPos += dx;
			}
			key->xPosPrev = key->xPos;
		}
		return;
	}

	int16 xpos, ypos, zpos, walkPoint1, walkPoint2, newDirection, incDx, incDy, incDz;
	while (1) {
		walkPoint1 = key->walkPointsList[key->walkPointsListCount];
		walkPoint2 = key->walkPointsList[key->walkPointsListCount + 1];
		key->currentWalkBox = walkPoint1;
		if (walkPoint1 == -1) {
			xpos = key->xPosPrev;
			ypos = key->yPosPrev;
			zpos = key->zPosPrev;
			if (key->prevWalkDataNum != -1) {
				key->walkDataNum = key->prevWalkDataNum;
				key->prevWalkDataNum = -1;
			}
		} else {
			xpos = _programPointsTable[walkPoint1].x;
			ypos = _programPointsTable[walkPoint1].y;
			zpos = _programPointsTable[walkPoint1].z;
		}
		newDirection = getDirection(kx, ky, kz, xpos, ypos, zpos);
		if (newDirection < 0) {
			newDirection = curDirection;
		}
		if (newDirection != curDirection) {
			key->currentAnimCounter = 0;
			key->facingDirection = newDirection;
			return;
		}
		incDx = xpos - kx;
		incDy = ypos - ky;
		incDz = zpos - kz;
		if (incDz != 0 || incDy != 0 || incDx != 0) {
			break;
		}
		if (walkPoint1 == -1) {
			if (key->currentAnim == 1) {
				setKeyCharRandomFrame(key);
			}
			return;
		}
		key->prevPointsDataNum = key->pointsDataNum;
		key->pointsDataNum = walkPoint1;
		if (walkPoint2 == -1) {
			key->walkPointsList[0] = -1;
			key->walkPointsListCount = 0;
		} else {
			++key->walkPointsListCount;
			int16 walkDataNum = findWalkDataNum(walkPoint1, walkPoint2);
			if (walkDataNum != -1) {
				key->walkDataNum = walkDataNum;
			}
		}
	}

	if (key->currentAnim < 1) {
		key->currentAnimCounter = 0;
		key->currentAnim = 1;
		if (dx == 0 && dy == 0 && dz == 0) {
			return;
		}
	}

	switch (newDirection) {
	case 0:
	case 3:
		if (dx == 0) {
			return;
		}
		if (newDirection == 3) {
			dx = -dx;
		}
		if (ABS(dx) >= ABS(incDx)) {
			if (walkPoint1 != -1) {
				if (walkPoint2 == -1) {
					newDirection = getDirection(xpos, ypos, zpos, key->xPosPrev, key->yPosPrev, key->zPosPrev);
					if (key->prevWalkDataNum != -1) {
						key->walkDataNum = key->prevWalkDataNum;
						key->prevWalkDataNum = -1;
					}
				} else {
					newDirection = getDirection(xpos, ypos, zpos, _programPointsTable[walkPoint2].x, _programPointsTable[walkPoint2].y, _programPointsTable[walkPoint2].z);
					int16 walkDataNum = findWalkDataNum(walkPoint1, walkPoint2);
					if (walkDataNum != -1) {
						key->walkDataNum = walkDataNum;
					}
				}
				if (newDirection == -2) {
					key->xPos = xpos;
					key->yPos = ypos;
					key->zPos = zpos;
					setKeyCharRandomFrame(key);
					return;
				}
				if (newDirection < 0) {
					newDirection = curDirection;
				}
				key->prevPointsDataNum = key->pointsDataNum;
				key->pointsDataNum = walkPoint1;
				++key->walkPointsListCount;
				if (newDirection != curDirection) {
					key->facingDirection = newDirection;
					key->currentAnimCounter = 0;
					key->xPos = xpos;
					key->yPos = ypos;
					key->zPos = zpos;
					return;
				}
			} else {
				key->xPos = xpos;
				key->yPos = ypos;
				key->zPos = zpos;
				return;
			}
		}
		key->xPos += dx;
		adjustKeyCharPosToWalkBox(key, 0);
		break;
	case 1:
	case 2:
		if (ABS(dz) >= ABS(incDz) && incDz != 0) {
			if (walkPoint1 != -1) {
				if (walkPoint2 == -1) {
					newDirection = getDirection(xpos, ypos, zpos, key->xPosPrev, key->yPosPrev, key->zPosPrev);
				} else {
					newDirection = getDirection(xpos, ypos, zpos, _programPointsTable[walkPoint2].x, _programPointsTable[walkPoint2].y, _programPointsTable[walkPoint2].z);
					int16 walkDataNum = findWalkDataNum(walkPoint1, walkPoint2);
					if (walkDataNum != -1) {
						key->walkDataNum = walkDataNum;
					}
				}
				if (newDirection == -2) {
					key->xPos = xpos;
					key->yPos = ypos;
					key->zPos = zpos;
					setKeyCharRandomFrame(key);
					return;
				}
				if (newDirection < 0) {
					newDirection = curDirection;
				}
				key->prevPointsDataNum = key->pointsDataNum;
				key->pointsDataNum = walkPoint1;
				++key->walkPointsListCount;
				if (newDirection != curDirection) {
					key->facingDirection = newDirection;
					key->currentAnimCounter = 0;
					key->xPos = xpos;
					key->yPos = ypos;
					key->zPos = zpos;
					return;
				}
			} else {
				key->xPos = xpos;
				key->yPos = ypos;
				key->zPos = zpos;
				return;
			}
		}
		if (incDz != 0) {
			key->zPos += dz;
			adjustKeyCharPosToWalkBox(key, 2);
		} else {
			if (ABS(dz) < ABS(incDy)) {
				key->yPos += dz;
				adjustKeyCharPosToWalkBox(key, 1);
			} else {
				key->xPos = xpos;
				key->yPos = ypos;
				key->zPos = zpos;
			}
		}
		break;
	}
}

void ToucheEngine::markWalkPoints(int keyChar) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	int16 pointsDataNum = key->pointsDataNum;
	resetPointsData(0);
	if (pointsDataNum != -1) {
		_programPointsTable[pointsDataNum].priority = 1;
		bool quit = false;
		while (!quit) {
			quit = true;
			for (uint i = 0; i < _programWalkTable.size(); ++i) {
				int16 md1 = _programWalkTable[i].point1;
				int16 md2 = _programWalkTable[i].point2;
				if ((md1 & 0x4000) == 0) {
					if (_programPointsTable[md1].priority != 0 && _programPointsTable[md2].priority == 0) {
						_programPointsTable[md2].priority = 1;
						quit = false;
					}
					if (_programPointsTable[md2].priority != 0 && _programPointsTable[md1].priority == 0) {
						_programPointsTable[md1].priority = 1;
						quit = false;
					}
				}
			}
		}
	}
}

void ToucheEngine::buildWalkPath(int dstPosX, int dstPosY, int keyChar) {
	debugC(9, kDebugEngine, "ToucheEngine::buildWalkPath(x=%d, y=%d, key=%d)", dstPosX, dstPosY, keyChar);
	if (_currentEpisodeNum == 130) {
		return;
	}
	markWalkPoints(keyChar);

	int minDistance = 0x7D000000;
	int minPointsDataNum = -1;
	for (uint i = 1; i < _programPointsTable.size(); ++i) {
		if (_programPointsTable[i].priority != 0) {
			int dx = ABS<int>(_programPointsTable[i].x - dstPosX);
			int dy = ABS<int>(_programPointsTable[i].y - dstPosY);
			int distance = dx * dx + dy * dy;
			if (distance < minDistance) {
				minDistance = distance;
				minPointsDataNum = i;
			}
		}
	}

	minDistance = 32000;
	int minWalkDataNum = -1;
	for (uint i = 0; i < _programWalkTable.size(); ++i) {
		const ProgramWalkData *pwd = &_programWalkTable[i];
		if ((pwd->point1 & 0x4000) == 0) {
			int distance = 32000;
			ProgramPointData *pts1 = &_programPointsTable[pwd->point1];
			ProgramPointData *pts2 = &_programPointsTable[pwd->point2];
			if (pts1->priority != 0) {
				int dx = pts2->x - pts1->x;
				int dy = pts2->y - pts1->y;
				if (dx == 0) {
					if (dstPosY > MIN(pts2->y, pts1->y) && dstPosY < MAX(pts2->y, pts1->y)) {
						distance = ABS(dstPosX - pts1->x);
						if (distance <= 100) {
							distance *= distance;
						}
					}
				} else if (dy == 0) {
					if (dstPosX > MIN(pts2->x, pts1->x) && dstPosX < MAX(pts2->x, pts1->x)) {
						distance = ABS(dstPosY - pts1->y);
						if (distance <= 100) {
							distance *= distance;
						}
					}
				} else {
					if (dstPosY > MIN(pts2->y, pts1->y) && dstPosY < MAX(pts2->y, pts1->y) &&
						dstPosX > MIN(pts2->x, pts1->x) && dstPosX < MAX(pts2->x, pts1->x) ) {
						distance = (dstPosY - pts1->y) * dx - (dstPosX - pts1->x) * dy;
						distance = (dx * dx + dy * dy) / distance;
					}
				}
				if (distance < minDistance) {
					minDistance = distance;
					minWalkDataNum = i;
				}
			}
		}
	}
	if (!sortPointsData(minWalkDataNum, minPointsDataNum)) {
		return;
	}
	int dstPosZ;
	buildWalkPointsList(keyChar);
	KeyChar *key = &_keyCharsTable[keyChar];
	if (minWalkDataNum == -1) {
		dstPosX = _programPointsTable[minPointsDataNum].x;
		dstPosY = _programPointsTable[minPointsDataNum].y;
		dstPosZ = _programPointsTable[minPointsDataNum].z;
	} else {
		ProgramWalkData *pwd = &_programWalkTable[minWalkDataNum];
		ProgramPointData *pts1 = &_programPointsTable[pwd->point1];
		ProgramPointData *pts2 = &_programPointsTable[pwd->point2];
		int16 dx = pts2->x - pts1->x;
		int16 dy = pts2->y - pts1->y;
		int16 dz = pts2->z - pts1->z;
		if (ABS(dy) > ABS(dx)) {
			dstPosZ = pts2->z - (pts2->y - dstPosY) * dz / dy;
			dstPosX = pts2->x - (pts2->y - dstPosY) * dx / dy;
		} else {
			dstPosZ = pts2->z - (pts2->x - dstPosX) * dz / dx;
			dstPosY = pts2->y - (pts2->x - dstPosX) * dy / dx;
		}
		if (key->walkDataNum == key->prevWalkDataNum && key->walkPointsList[1] == -1) {
			if (key->walkPointsList[0] == _programWalkTable[minWalkDataNum].point1 || key->walkPointsList[0] == _programWalkTable[minWalkDataNum].point2) {
				++key->walkPointsListCount;
			}
		}
	}
	key->prevWalkDataNum = minWalkDataNum;
	key->xPosPrev = dstPosX;
	key->yPosPrev = dstPosY;
	key->zPosPrev = dstPosZ;
	if (_flagsTable[902] != 0) {
		Graphics::fillRect(_backdropBuffer, _currentBitmapWidth, dstPosX, dstPosY, 4, 4, 0xFC);
	}
}

void ToucheEngine::addToAnimationTable(int num, int posNum, int keyChar, int delayCounter) {
	for (int i = 0; i < NUM_ANIMATION_ENTRIES; ++i) {
		AnimationEntry *anim = &_animationTable[i];
		if (anim->num == 0) {
			anim->num = num;
			anim->delayCounter = delayCounter;
			anim->posNum = posNum;
			int16 xPos, yPos, x2Pos, y2Pos;
			if (posNum >= 0) {
				assert(posNum >= 0 && posNum < NUM_KEYCHARS);
				xPos = _keyCharsTable[posNum].xPos;
				yPos = _keyCharsTable[posNum].yPos - 50;
			} else {
				posNum = -posNum;
				xPos = _programPointsTable[posNum].x;
				yPos = _programPointsTable[posNum].y;
			}
			xPos -= _flagsTable[614];
			yPos -= _flagsTable[615];
			assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
			x2Pos = _keyCharsTable[keyChar].xPos - _flagsTable[614];
			y2Pos = _keyCharsTable[keyChar].yPos - _flagsTable[615] - 50;
			xPos -= x2Pos;
			yPos -= y2Pos;
			xPos /= 8;
			yPos /= 8;
			anim->x = x2Pos;
			anim->y = y2Pos;
			anim->dx = xPos;
			anim->dy = yPos;
			anim->displayCounter = 8;
			anim->displayRect.left = -1;
		}
	}
}

void ToucheEngine::copyAnimationImage(int dstX, int dstY, int w, int h, const uint8 *src, int srcX, int srcY, int fillColor) {
	Area copyRegion(dstX, dstY, w, h);
	copyRegion.srcX = srcX;
	copyRegion.srcY = srcY;
	if (copyRegion.clip(_screenRect)) {
		if (fillColor != -1) {
			Graphics::copyMask(_offscreenBuffer, 640, copyRegion.r.left, copyRegion.r.top,
			  src, 58, copyRegion.srcX, copyRegion.srcY,
			  copyRegion.r.width(), copyRegion.r.height(),
			  (uint8)fillColor);
		} else {
			Graphics::copyRect(_offscreenBuffer, 640, copyRegion.r.left, copyRegion.r.top,
			  src, 58, copyRegion.srcX, copyRegion.srcY,
			  copyRegion.r.width(), copyRegion.r.height(),
			  Graphics::kTransparent);
		}
	}
}

void ToucheEngine::drawAnimationImage(AnimationEntry *anim) {
	if (anim->displayRect.left != -1) {
		addToDirtyRect(anim->displayRect);
	}
	int x = anim->x;
	int y = anim->y;
	int dx = -anim->dx;
	int dy = -anim->dy;

	int displayRectX1 = 30000;
	int displayRectY1 = 30000;
	int displayRectX2 = -30000;
	int displayRectY2 = -30000;

	dx /= 3;
	dy /= 3;

	res_loadImage(anim->num, _iconData);
	int color = 0xCF;

	x += dx * 5 - 29;
	y += dy * 5 - 21;
	dx = -dx;
	dy = -dy;
	for (int i = 0; i < 6; ++i) {
		if (i == 5) {
			color = -1;
		}
		copyAnimationImage(x, y, 58, 42, _iconData, 0, 0, color);
		--color;
		displayRectX1 = MIN(x, displayRectX1);
		displayRectX2 = MAX(x, displayRectX2);
		displayRectY1 = MIN(y, displayRectY1);
		displayRectY2 = MAX(y, displayRectY2);
		x += dx;
		y += dy;
	}
	anim->displayRect = Common::Rect(displayRectX1, displayRectY1, displayRectX2 + 58, displayRectY2 + 42);
//	if (rectClip(&_roomAreaRect, &anim->displayRect)) {
//		addToDirtyRect(&anim->displayRect);
//	}
}

void ToucheEngine::processAnimationTable() {
	for (int i = 0; i < NUM_ANIMATION_ENTRIES; ++i) {
		AnimationEntry *anim = &_animationTable[i];
		if (anim->num != 0) {
			if (anim->displayCounter == 0) {
				anim->num = 0;
				if (anim->displayRect.left != -1) {
					addToDirtyRect(anim->displayRect);
				}
			} else {
				if (anim->delayCounter != 0) {
					--anim->delayCounter;
				} else {
					anim->x += anim->dx;
					anim->y += anim->dy;
					drawAnimationImage(anim);
					--anim->displayCounter;
				}
			}
		}
	}
}

void ToucheEngine::clearAnimationTable() {
	memset(_animationTable, 0, sizeof(_animationTable));
}

void ToucheEngine::addToDirtyRect(const Common::Rect &r) {
	// XXX
}

void ToucheEngine::clearDirtyRects() {
	// XXX
}

void ToucheEngine::setPalette(int firstColor, int colorCount, int rScale, int gScale, int bScale) {
	uint8 pal[256 * 4];
	for (int i = firstColor; i < firstColor + colorCount; ++i) {
		int r = _paletteBuffer[i * 4 + 0];
		r = (r * rScale) >> 8;
		pal[i * 4 + 0] = (uint8)r;

		int g = _paletteBuffer[i * 4 + 1];
		g = (g * gScale) >> 8;
		pal[i * 4 + 1] = (uint8)g;

		int b = _paletteBuffer[i * 4 + 2];
		b = (b * bScale) >> 8;
		pal[i * 4 + 2] = (uint8)b;

		pal[i * 4 + 3] = 0;
	}
	_system->setPalette(&pal[firstColor * 4], firstColor, colorCount);
}

void ToucheEngine::copyPaletteColor(int srcColorIndex, int dstColorIndex) {
	memcpy(&_paletteBuffer[dstColorIndex * 4], &_paletteBuffer[srcColorIndex * 4], 4);
}

void ToucheEngine::updateScreenArea(const uint8 *src, int srcPitch, int srcX, int srcY, int dstX, int dstY, int w, int h) {
	_system->copyRectToScreen(src + srcY * srcPitch + srcX, srcPitch, dstX, dstY, w, h);
	_system->updateScreen();
}

void ToucheEngine::updateEntireScreen() {
	int h = (_flagsTable[606] != 0) ? 400 : 352;
	_system->copyRectToScreen(_offscreenBuffer, 640, 0, 0, 640, h);
	_system->updateScreen();
}

void ToucheEngine::updateDirtyScreenAreas() {
	// XXX
	updateScreenArea(_offscreenBuffer, 640, 0, 0, 0, 0, 640, 400);
	if (_fullRedrawCounter) {
//		updateEntireScreen();
		--_fullRedrawCounter;
	} else {
//		for (int i = 0; i < _dirtyRectsCount; ++i) {
//			Common::Rect *r = &_dirtyRects[i];
//			updateScreenArea(_offscreenBuffer, 640, r->x, r->y, r->x, r->y, r->w, r->h);
//		}
		if (_redrawScreenCounter1) {
			--_redrawScreenCounter1;
//			updateScreenArea(_offscreenBuffer, 640, _cursorObjectRect.x, _cursorObjectRect.y, _cursorObjectRect.x, _cursorObjectRect.y, _cursorObjectRect.w, _cursorObjectRect.h);
		}
	}
}

void ToucheEngine::updatePalette() {
	_system->setPalette(_paletteBuffer, 0, 256);
}

} // namespace Touche
