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

#include "igor/igor.h"

namespace Igor {

static const uint8 PART_12_DATA_ANIM_1[10] = { 0, 1, 2, 1, 2, 1, 2, 1, 2, 3 };

static const uint8 PART_12_DATA_ANIM_2[16] = { 0, 1, 2, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 5, 6, 7 };

void IgorEngine::PART_12_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_12_EXEC_ACTION %d", action);
	switch (action) {
	case 101:
		PART_12_ACTION_101();
		break;
	case 102:
		ADD_DIALOGUE_TEXT(201, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 103:
		if (_objectsState[44] == 0) {
			ADD_DIALOGUE_TEXT(203, 2);
		} else {
			ADD_DIALOGUE_TEXT(205, 1);
		}
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104:
		PART_12_ACTION_104();
		break;
	case 105:
		PART_12_ACTION_105();
		break;
	case 106:
		ADD_DIALOGUE_TEXT(207, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107:
		ADD_DIALOGUE_TEXT(206, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 108:
		PART_12_ACTION_108();
		break;
	default:
		error("PART_12_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_12_ACTION_101() {
	for (int i = 9; i <= 10; ++i) {
		_roomObjectAreasTable[i].area = 3;
	}
	--_walkDataLastIndex;
	buildWalkPath(272, 94, 196, 64);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	_gameState.unk10 = 1;
	_gameTicks = 0;
	do {
		if (compareGameTick(1, 16)) {
			if (_walkDataCurrentIndex > _walkDataLastIndex) {
				_gameState.igorMoving = false;
				_walkDataLastIndex = _walkDataCurrentIndex;
			}
			if (_gameState.igorMoving) {
				moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
				++_walkDataCurrentIndex;
			}
		}
		if (compareGameTick(2, 32) && _gameState.unk10 < 8) {
			if (_objectsState[44] == 0) {
				const uint8 *src = _animFramesBuffer + 0x761 + READ_LE_UINT16(_animFramesBuffer + 0x1107 + _gameState.unk10 * 2) - 1;
				decodeAnimFrame(src, _screenVGA, true);
			}
			++_gameState.unk10;
		}
		waitForTimer();
	} while (_gameState.igorMoving || _gameState.unk10 != 8);
	_currentPart = 40;
}

void IgorEngine::PART_12_ACTION_104() {
	_walkCurrentFrame = 2;
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[33390]].area = _roomObjectAreasTable[_screenLayer2[34686]].area;
	buildWalkPath(126, 108, 110, 104);
	_roomObjectAreasTable[_screenLayer2[33390]].area = 0;
	_walkDataCurrentIndex = 1;
	moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
	waitForTimer(35);
	playSound(13, 1);
	_walkDataCurrentIndex = _walkDataLastIndex;
	const uint8 *src = _animFramesBuffer + 0x395B + READ_LE_UINT16(_animFramesBuffer + 0xA444) - 1;
	decodeAnimFrame(src, _screenVGA);
	decodeAnimFrame(src, _screenLayer1);
	moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
	waitForTimer(35);
	_walkDataCurrentIndex = _walkDataLastIndex - 1;
	src = _animFramesBuffer + 0x395B + READ_LE_UINT16(_animFramesBuffer + 0xA446) - 1;
	decodeAnimFrame(src, _screenVGA);
	decodeAnimFrame(src, _screenLayer1);
	_walkData[_walkDataCurrentIndex].xPosChanged = 1;
	_walkData[_walkDataCurrentIndex].yPosChanged = 1;
	moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
	waitForTimer(35);
	_walkData[0].setPos(127, 108, 4, 0);
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkData[0].scaleWidth = 50;
	_walkData[0].xPosChanged = 1;
	_walkData[0].dxPos = 10;
	_walkData[0].yPosChanged = 1;
	_walkData[0].dyPos = 3;
	_walkData[0].scaleHeight = 50;
	_walkDataCurrentIndex = 0;
	_walkDataLastIndex = 1;
	src = _animFramesBuffer + 0x395B + READ_LE_UINT16(_animFramesBuffer + 0xA448) - 1;
	decodeAnimFrame(src, _screenVGA);
	decodeAnimFrame(src, _screenLayer1);
	moveIgor(4, 0);
	PART_12_HANDLE_DIALOGUE_CHURCHMAN();
	for (int i = 4; i >= 1; --i) {
		src = _animFramesBuffer + 0x395B + READ_LE_UINT16(_animFramesBuffer + 0xA43E + i * 2) - 1;
		decodeAnimFrame(src, _screenVGA);
		if (i > 1) {
			waitForTimer(35);
		} else {
			playSound(14, 1);
		}
	}
	src = _animFramesBuffer + 0x395B + READ_LE_UINT16(_animFramesBuffer + 0xA440) - 1;
	decodeAnimFrame(src, _screenVGA);
	PART_12_HELPER_1(255);
}

void IgorEngine::PART_12_ACTION_105() {
	if (_objectsState[44] == 1) {
		ADD_DIALOGUE_TEXT(216, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		return;
	}
	memset(_screenVGA + 46080, 0, 17920);
	_inventoryInfo[_inventoryInfo[62] - 1] = 0;
	_inventoryInfo[62] = 0;
	packInventory();
	if (_inventoryInfo[72] > _inventoryInfo[73]) {
		_inventoryInfo[72] = _inventoryOffsetTable[(_inventoryInfo[73] - 1) / 7];
	}
	const int animOffset = (_objectsState[45] == 0) ? 0x2543 : 0xBF1E;
	for (int i = 1; i <= 9; ++i) {
		const int offset = 30149;
		for (int j = 0; j <= 48; ++j) {
			const uint8 *src = _animFramesBuffer + animOffset + PART_12_DATA_ANIM_1[i] * 1715 + j * 35 + 0xF94C;
			memcpy(_screenVGA + j * 320 + offset, src, 35);
		}
		waitForTimer(30);
	}
	ADD_DIALOGUE_TEXT(209, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	for (int i = 9; i <= 10; ++i) {
		_roomObjectAreasTable[i].area = 3;
	}
	--_walkDataLastIndex;
	buildWalkPath(95, 142, 196, 64);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_gameState.unk10 = 1;
	_gameTicks = 0;
	do {
		if (compareGameTick(2, 32) && _gameState.unk10 < 8) {
			const uint8 *src = _animFramesBuffer + 0x761 + READ_LE_UINT16(_animFramesBuffer + 0x1107 + _gameState.unk10 * 2) - 1;
			decodeAnimFrame(src, _screenVGA, true);
			++_gameState.unk10;
		}
		waitForTimer();
	} while (_gameState.unk10 != 8);
	--_walkDataLastIndex;
	buildWalkPath(196, 64, 95, 142);
	PART_12_HELPER_2();
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	_gameTicks = 0;
	do {
		if (compareGameTick(1, 16)) {
			if (_walkDataCurrentIndex > _walkDataLastIndex) {
				_gameState.igorMoving = false;
				_walkDataLastIndex = _walkDataCurrentIndex;
			}
			if (_gameState.igorMoving) {
				moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
				++_walkDataCurrentIndex;
			}
		}
		if (compareGameTick(2, 32)) {
			_gameState.unk10 = getRandomNumber(2) + 7;
			const uint8 *src = _animFramesBuffer + 0x761 + READ_LE_UINT16(_animFramesBuffer + 0x1107 + _gameState.unk10 * 2) - 1;
			decodeAnimFrame(src, _screenVGA, true);
		}
		waitForTimer();
	} while (_gameState.igorMoving);
	decodeAnimFrame(_animFramesBuffer + 0x761 + READ_LE_UINT16(_animFramesBuffer + 0x1117) - 1, _screenVGA, true);
	if (_objectsState[45] == 0) {
		for (int i = 1; i <= 15; ++i) {
			const uint8 *src = _animFramesBuffer + 0x1119 + READ_LE_UINT16(_animFramesBuffer + 0x2532 + PART_12_DATA_ANIM_2[i] * 2) - 1;
			decodeAnimFrame(src, _screenVGA, true);
			waitForTimer(45);
		}
	} else {
		for (int i = 1; i <= 15; ++i) {
			const uint8 *src = _animFramesBuffer + 0xA47A + READ_LE_UINT16(_animFramesBuffer + 0xBF0D + PART_12_DATA_ANIM_2[i] * 2) - 1;
			decodeAnimFrame(src, _screenVGA, true);
			waitForTimer(45);
		}
	}
	drawVerbsPanel();
	redrawVerb(kVerbWalk, true);
	_currentAction.verb = kVerbWalk;
	addObjectToInventory(24, 59);
	_objectsState[44] = 1;
	PART_12_HELPER_1(255);
	ADD_DIALOGUE_TEXT(211, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
}

void IgorEngine::PART_12_ACTION_108() {
	_walkCurrentFrame = 2;
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[28555]].area = _roomObjectAreasTable[_screenLayer2[34686]].area;
	buildWalkPath(126, 108, 75, 89);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	_gameState.unk10 = 1;
	_gameTicks = 0;
	do {
		if (compareGameTick(1, 16)) {
			if (_walkDataCurrentIndex > _walkDataLastIndex) {
				_gameState.igorMoving = false;
				_walkDataLastIndex = _walkDataCurrentIndex;
			}
			if (_gameState.igorMoving) {
				moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
				++_walkDataCurrentIndex;
			}
		}
		if (compareGameTick(2, 32) && _gameState.unk10 < 8) {
			if (_objectsState[44] == 0) {
				const uint8 *src = _animFramesBuffer + 0x761 + READ_LE_UINT16(_animFramesBuffer + 0x1107 + _gameState.unk10 * 2) - 1;
				decodeAnimFrame(src, _screenVGA, true);
			}
			++_gameState.unk10;
		}
		waitForTimer();
	} while (_gameState.igorMoving || _gameState.unk10 != 8);
	_currentPart = 130;
}

void IgorEngine::PART_12_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(1) && getRandomNumber(15) == 0) {
		switch (getRandomNumber(4)) {
		case 0:
			playSound(21, 1);
			break;
		case 1:
			playSound(22, 1);
			break;
		case 2:
			playSound(23, 1);
			break;
		case 3:
			playSound(18, 1);
			break;
		}
	}
}

void IgorEngine::PART_12_UPDATE_DIALOGUE_CHURCHMAN(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_12_HELPER_8();
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_12_HELPER_10(6 + getRandomNumber(6));
		break;
	case kUpdateDialogueAnimStanding:
		PART_12_HELPER_8();
		break;
	}
}

void IgorEngine::PART_12_HANDLE_DIALOGUE_CHURCHMAN() {
	loadDialogueData(DLG_OutsideChurch);
	_updateDialogue = &IgorEngine::PART_12_UPDATE_DIALOGUE_CHURCHMAN;
	handleDialogue(95, 55, 51, 28, 63);
	_updateDialogue = 0;
}

void IgorEngine::PART_12_HELPER_1(int num) {
	if (num == 2 || num == 255) {
		if (_objectsState[45] == 0) {
			_roomActionsTable[169] = 104;
			_roomActionsTable[175] = 104;
			_roomActionsTable[177] = 104;
			_roomActionsTable[179] = 104;
			_roomActionsTable[181] = 104;
			_roomActionsTable[185] = 104;
			_roomActionsTable[187] = 104;
			_roomActionsTable[170] = 3;
			_roomActionsTable[176] = 3;
			_roomActionsTable[178] = 3;
			_roomActionsTable[180] = 3;
			_roomActionsTable[182] = 3;
			_roomActionsTable[186] = 3;
			_roomActionsTable[188] = 3;
		} else {
			PART_12_HELPER_9();
			_roomActionsTable[169] = 108;
			_roomActionsTable[175] = 106;
			_roomActionsTable[177] = 2;
			_roomActionsTable[179] = 11;
			_roomActionsTable[181] = 107;
			_roomActionsTable[185] = 35;
			_roomActionsTable[187] = 35;
			_roomActionsTable[170] = 1;
			_roomActionsTable[176] = 1;
			_roomActionsTable[178] = 1;
			_roomActionsTable[180] = 1;
			_roomActionsTable[182] = 1;
			_roomActionsTable[186] = 1;
			_roomActionsTable[188] = 1;
		}
	}
}

void IgorEngine::PART_12_HELPER_2() {
	for (int i = 9; i <= 10; ++i) {
		_roomObjectAreasTable[i].area = 0;
	}
}

void IgorEngine::PART_12_HELPER_3() {
	decodeAnimFrame(_animFramesBuffer, _screenVGA, true);
}

void IgorEngine::PART_12_HELPER_4() {
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	fadeInPalette(768);
	playSound(13, 1);
	for (int i = 1; i <= 5; ++i) {
		const uint8 *src = _animFramesBuffer + 0x395B + READ_LE_UINT16(_animFramesBuffer + 0xA43E + i * 2) - 1;
		decodeAnimFrame(src, _screenVGA, true);
		if (_objectsState[44] == 0) {
			src = _animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0x74B + i * 2) - 1;
			decodeAnimFrame(src, _screenVGA, true);
		}
		waitForTimer(30);
	}
	if (_objectsState[44] == 0) {
		for (int i = 6; i <= 10; ++i) {
			const uint8 *src = _animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0x74B + i * 2) - 1;
			decodeAnimFrame(src, _screenVGA, true);
			waitForTimer(30);
		}
	}
	ADD_DIALOGUE_TEXT(212, 2);
	ADD_DIALOGUE_TEXT(214, 1);
	ADD_DIALOGUE_TEXT(215, 1);
	SET_DIALOGUE_TEXT(1, 3);
	_updateDialogue = &IgorEngine::PART_12_UPDATE_DIALOGUE_CHURCHMAN;
	startCutsceneDialogue(95, 55, 51, 28, 63);
	waitForEndOfCutsceneDialogue(95, 55, 51, 28, 63);
	_updateDialogue = 0;
	for (int i = 12; i <= 29; ++i){
		const uint8 *src = _animFramesBuffer + 0x395B + READ_LE_UINT16(_animFramesBuffer + 0xA43E + i * 2) - 1;
		decodeAnimFrame(src, _screenVGA, true);
		waitForTimer(30);
	}
	if (_objectsState[44] == 0) {
		for (int i = 1; i <= 8; ++i) {
			const uint8 *src = _animFramesBuffer + 0x761 + READ_LE_UINT16(_animFramesBuffer + 0x1107 + i * 2) - 1;
			decodeAnimFrame(src, _screenVGA, true);
			waitForTimer(30);
		}
	}
	waitForTimer(255);
	fadeOutPalette(768);
	_objectsState[45] = 1;
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	_currentPart = 40;
}

void IgorEngine::PART_12_HELPER_5() {
	_walkData[0].setPos(196, 64, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(196, 64, 238, 104);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	_gameState.unk10 = 1;
	waitForTimer(15);
	_gameTicks = 0;
	do {
		if (compareGameTick(1, 16)) {
			if (_walkDataCurrentIndex > _walkDataLastIndex) {
				_gameState.igorMoving = false;
				_walkDataLastIndex = _walkDataCurrentIndex;
			}
			if (_gameState.igorMoving) {
				moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
				++_walkDataCurrentIndex;
			}
		}
		if (compareGameTick(2, 32) && _gameState.unk10 < 11) {
			if (_objectsState[44] == 0) {
				const uint8 *src = _animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0x74B + _gameState.unk10 * 2) - 1;
				decodeAnimFrame(src, _screenVGA, true);
			}
			++_gameState.unk10;
		}
		waitForTimer();
	} while (_gameState.igorMoving || _gameState.unk10 != 11);
}

void IgorEngine::PART_12_HELPER_6() {
	playMusic(2);
	PART_12_HELPER_1(255);
	_walkData[0].setPos(75, 89, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_roomObjectAreasTable[_screenLayer2[28555]].area = _roomObjectAreasTable[_screenLayer2[34686]].area;
	buildWalkPath(75, 89, 126, 108);
	_roomObjectAreasTable[_screenLayer2[28555]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	_gameState.unk10 = 1;
	waitForTimer(15);
	_gameTicks = 0;
	do {
		if (compareGameTick(1, 16)) {
			if (_walkDataCurrentIndex > _walkDataLastIndex) {
				_gameState.igorMoving = false;
				_walkDataLastIndex = _walkDataCurrentIndex;
			}
			if (_gameState.igorMoving) {
				moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
				++_walkDataCurrentIndex;
			}
		}
		if (compareGameTick(2, 32) && _gameState.unk10 < 11) {
			if (_objectsState[44] == 0) {
				const uint8 *src = _animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0x74B + _gameState.unk10 * 2) - 1;
				decodeAnimFrame(src, _screenVGA, true);
			}
			++_gameState.unk10;
		}
		waitForTimer();
	} while (_gameState.igorMoving || _gameState.unk10 != 11);
}

void IgorEngine::PART_12_HELPER_8() {
	decodeAnimFrame(_animFramesBuffer + 0x395B + READ_LE_UINT16(_animFramesBuffer + 0xA454) - 1, _screenVGA, true);
}

void IgorEngine::PART_12_HELPER_9() {
	decodeAnimFrame(_animFramesBuffer + 0x395B + READ_LE_UINT16(_animFramesBuffer + 0xA442) - 1, _screenLayer1, true);
}

void IgorEngine::PART_12_HELPER_10(int frame) {
	decodeAnimFrame(_animFramesBuffer + 0x395B + READ_LE_UINT16(_animFramesBuffer + 0xA43E + frame * 2) - 1, _screenVGA, true);
}

void IgorEngine::PART_12() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_OutsideChurch, IMG_OutsideChurch, BOX_OutsideChurch, MSK_OutsideChurch, TXT_OutsideChurch);
	static const int anm[] = { FRM_OutsideChurch1, FRM_OutsideChurch2, FRM_OutsideChurch3, FRM_OutsideChurch4, FRM_OutsideChurch5, FRM_OutsideChurch6, FRM_OutsideChurch7, FRM_OutsideChurch8, FRM_OutsideChurch9, FRM_OutsideChurch10, FRM_OutsideChurch11, FRM_OutsideChurch12, 0 };
	loadAnimData(anm);
	loadActionData(DAT_OutsideChurch);
	_roomDataOffsets = PART_12_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(89, 0, 275, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_12_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_12_UPDATE_ROOM_BACKGROUND;
	PART_12_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	if (_objectsState[44] == 0) {
		PART_12_HELPER_3();
	}
	if (_currentPart == 122) {
		PART_12_HELPER_4();
		return;
	}
	_currentAction.verb = kVerbWalk;
	fadeInPalette(768);
	if (_currentPart == 120) {
		PART_12_HELPER_5();
	}
	if (_currentPart == 121) {
		PART_12_HELPER_6();
		if (_objectsState[106] == 1) {
			ADD_DIALOGUE_TEXT(216, 2);
			ADD_DIALOGUE_TEXT(218, 1);
			SET_DIALOGUE_TEXT(1, 2);
			startIgorDialogue();
			waitForEndOfIgorDialogue();
			--_walkDataLastIndex;
			buildWalkPath(126, 108, 196, 64);
			_walkData[_walkDataLastIndex].frameNum = 0;
			_walkDataCurrentIndex = 1;
			_gameState.igorMoving = true;
			waitForIgorMove();
			_currentPart = 40;
			goto PART_12_EXIT;
		}
	}
	PART_12_HELPER_2();
	enterPartLoop();
	while (_currentPart >= 120 && _currentPart <= 122) {
		runPartLoop();
	}
PART_12_EXIT:
	leavePartLoop();
	fadeOutPalette(624);
}

} // namespace Igor
