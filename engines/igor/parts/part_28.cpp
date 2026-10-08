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
#include "igor/igor.h"


namespace Igor {


void IgorEngine::PART_28_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_currentPart = 230;
		break;
	case 102:
		ADD_DIALOGUE_TEXT(201, 1, 1226);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 103:
		ADD_DIALOGUE_TEXT(202, 1, 1227);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(203, 1, 1228);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		ADD_DIALOGUE_TEXT(204, 1, 1229);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106:
		if (_objectsState[111] == 0) {
			ADD_DIALOGUE_TEXT(205, 1, 1230);
			ADD_DIALOGUE_TEXT(206, 1, 1231);
			SET_DIALOGUE_TEXT(1, 2);
			startIgorDialogue();
		} else {
			ADD_DIALOGUE_TEXT(234, 1, 1253);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 107:
		ADD_DIALOGUE_TEXT(237, 1, 1256);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 108:
		PART_28_ACTION_108();
		break;
	case 109:
		PART_28_ACTION_109();
		break;
	case 110:
		ADD_DIALOGUE_TEXT(228, 1, 1248);
		ADD_DIALOGUE_TEXT(229, 1, 1249);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 111:
		if (_objectsState[3] == 0) {
			ADD_DIALOGUE_TEXT(226, 1, 1246);
			SET_DIALOGUE_TEXT(1, 1);
		} else {
			ADD_DIALOGUE_TEXT(227, 1, 1247);
			SET_DIALOGUE_TEXT(1, 1);
		}
		startIgorDialogue();
		break;
	case 112:
		ADD_DIALOGUE_TEXT(235, 1, 1254);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 113:
		ADD_DIALOGUE_TEXT(236, 1, 1255);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	default:
		error("PART_28_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_28_ACTION_108() {
	if (_objectsState[111] == 1) {
		ADD_DIALOGUE_TEXT(218, 1, 1241);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		ADD_DIALOGUE_TEXT(211, 1, 1235);
		ADD_DIALOGUE_TEXT(212, 1, 1236);
		ADD_DIALOGUE_TEXT(213, 2, 1237);
		ADD_DIALOGUE_TEXT(215, 1, 1238);
		ADD_DIALOGUE_TEXT(216, 1, 1239);
		SET_DIALOGUE_TEXT(1, 5);
		startCutsceneDialogue(182, 81, 63, 17, 17);
		_updateDialogue = &IgorEngine::PART_28_UPDATE_DIALOGUE_CAROLINE;
		waitForEndOfCutsceneDialogue(182, 81, 63, 17, 17);
		ADD_DIALOGUE_TEXT(217, 1, 1240);
		SET_DIALOGUE_TEXT(1, 5);
		startCutsceneDialogue(182, 81, 63, 17, 17);
		waitForEndOfCutsceneDialogue(182, 81, 63, 17, 17);
		_updateDialogue = 0;
		--_walkDataLastIndex;
		buildWalkPath(213, 131, 160, 140);
		_walkDataCurrentIndex = 1;
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkData[_walkDataLastIndex].posNum = 3;
		_gameState.igorMoving = true;
		waitForIgorMove();
		ADD_DIALOGUE_TEXT(222, 2, 1244);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		return;
	}
	if (_objectsState[87] < 2) {
		ADD_DIALOGUE_TEXT(207, 1, 1232);
		SET_DIALOGUE_TEXT(1, 1);
	} else {
		ADD_DIALOGUE_TEXT(218, 1, 1241);
		SET_DIALOGUE_TEXT(1, 1);
	}
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	if (_objectsState[87] < 2) {
		ADD_DIALOGUE_TEXT(208, 1, 1233);
		SET_DIALOGUE_TEXT(1, 1);
	} else {
		ADD_DIALOGUE_TEXT(219, 2, 1242);
		ADD_DIALOGUE_TEXT(221, 1, 1243);
		SET_DIALOGUE_TEXT(1, 2);
	}
	startCutsceneDialogue(182, 81, 63, 17, 17);
	_updateDialogue = &IgorEngine::PART_28_UPDATE_DIALOGUE_CAROLINE;
	waitForEndOfCutsceneDialogue(182, 81, 63, 17, 17);
	_updateDialogue = 0;
}

void IgorEngine::PART_28_ACTION_109() {
	for (int i = 1; i <= 7; ++i) {
		const int offset = 25067;
		for (int j = 0; j <= 34; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + i * 1085 + j * 31 + 0x5EC9, 31);
		}
		if (i < 7) {
			waitForTimer(45);
		}
	}
	addObjectToInventory(22, 57);
	_objectsState[87] = 1;
	PART_28_HELPER_1(255);
	ADD_DIALOGUE_TEXT(230, 1, 1250);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
}

void IgorEngine::PART_28_UPDATE_DIALOGUE_CAROLINE(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_28_HELPER_5(7);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_28_HELPER_5(getRandomNumber(4) + 7);
		break;
	}
}

void IgorEngine::PART_28_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(61)) {
		if (_gameState.unk10 > 2) {
			PART_28_HELPER_5(_gameState.unk10);
			if (_gameState.unk10 < 6) {
				++_gameState.unk10;
			} else {
				_gameState.unk10 = 1;
			}
		} else {
			PART_28_HELPER_5(_gameState.unk10);
			if (_gameState.unk10 == 1) {
				_gameState.unk10 = 2;
			} else {
				_gameState.unk10 = 1;
			}
			if (getRandomNumber(7) == 0) {
				_gameState.unk10 = 3;
			}
		}
	}
}

void IgorEngine::PART_28_HELPER_1(int num) {
	WRITE_LE_UINT16(_roomActionsTable + 0x56, 42133);
	if (num == 2 || num == 255) {
		if (_objectsState[87] == 0) {
			PART_28_HELPER_8(1);
		} else {
			PART_28_HELPER_8(0);
			_roomObjectAreasTable[7].object = 0;
		}
	}
}

void IgorEngine::PART_28_HELPER_2() {
	const int offset = 27374;
	for (int i = 0; i <= 43; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + i * 30, 30);
	}
}

void IgorEngine::PART_28_HELPER_3() {
	_walkData[0].setPos(319, 142, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(319, 142, 289, 142);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_28_HELPER_5(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 43; ++i) {
		for (int j = 0; j <= 29; ++j) {
			int offset = (i + 85) * 320 + j + 174;
			uint8 color = _screenVGA[offset];
			if ((color >= 192 && color <= 207) || color == 240 || color == 241) {
				_screenTempLayer[i * 100 + j] = _screenVGA[offset];
			} else {
				_screenTempLayer[i * 100 + j] = _animFramesBuffer[(frame - 1) * 1320 + i * 30 + j];
			}
		}
	}
	int offset = 27374;
	for (int i = 0; i <= 43; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _screenTempLayer + i * 100, 30);
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + (frame - 1) * 1320 + i * 30, 30);
	}
	if (_gameState.dialogueTextRunning) {
		memcpy(_screenTextLayer + 23040, _screenLayer1 + _dialogueDirtyRectY, _dialogueDirtyRectSize);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_28_HELPER_6() {
	setupDefaultPalette();
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	PART_28_HELPER_1(255);
	fadeIn(768);
	_walkData[0].setPos(319, 142, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(319, 142, 214, 130);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkData[_walkDataLastIndex].posNum = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	ADD_DIALOGUE_TEXT(209, 2, 1234);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	waitForTimer(255);
	ADD_DIALOGUE_TEXT(231, 1, 1251);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	--_walkDataLastIndex;
	buildWalkPath(214, 130, 214, 140);
	_walkDataCurrentIndex = 1;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
	ADD_DIALOGUE_TEXT(224, 2, 1245);
	ADD_DIALOGUE_TEXT(232, 2, 1252);
	SET_DIALOGUE_TEXT(1, 2);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
}

void IgorEngine::PART_28_HELPER_8(int frame) {
	const int offset = 33387;
	for (int i = 0; i <= 8; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + frame * 135 + i * 15 + 0x61F8, 15);
	}
}

void IgorEngine::PART_28() {
	playMusic(2);
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeCorridorCaroline, IMG_CollegeCorridorCaroline, BOX_CollegeCorridorCaroline, MSK_CollegeCorridorCaroline, TXT_CollegeCorridorCaroline);
	static const int anm[] = { FRM_CollegeCorridorCaroline1, FRM_CollegeCorridorCaroline2, FRM_CollegeCorridorCaroline3, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorCaroline);
	_roomDataOffsets = PART_28_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(15, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_28_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_28_UPDATE_ROOM_BACKGROUND;
	PART_28_HELPER_2();
	PART_28_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	// cseg123:2E04 -- the original's only write to the background-animation frame
	// counter (s3:0xEB2C) is an unconditional set to 1 in this room's setup, made
	// before the state dispatch below. PART_28_HELPER_5() forms its source address
	// as animBase + frame * 1320 + i * 30 + j - 1320 (cseg123:0B7-0D5), so the frame
	// is 1-based and a counter of 0 reads 1320 bytes before _animFramesBuffer.
	// Without this the first background update reads out of bounds.
	if (!restoreRoomAfterLoad()) {
		_gameState.unk10 = 1;
		_currentAction.verb = kVerbWalk;
		if (_currentPart == 280) {
			fadeIn(768);
			PART_28_HELPER_3();
		} else if (_currentPart == 281) {
			PART_28_HELPER_6();
		}
	}
	enterPartLoop();
	while ((_currentPart == 280 || _currentPart == 281) && !_gameStateLoaded) {
		runPartLoop();
	}
	if (!_gameStateLoaded && _objectsState[87] == 1) {
		_objectsState[87] = 2;
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}
} // End of namespace Igor
