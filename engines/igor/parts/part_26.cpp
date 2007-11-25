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

static const uint8 PART_26_ANIM_DATA_1[12] = { 6, 0, 2, 3, 4, 5, 6, 5, 4, 3, 2, 0 };

void IgorEngine::PART_26_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		ADD_DIALOGUE_TEXT(202, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 102:
		ADD_DIALOGUE_TEXT(203, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 103:
		PART_26_ACTION_103();
		break;
	case 104:
		PART_26_ACTION_104();
		break;
	case 105:
		ADD_DIALOGUE_TEXT(224, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106:
		_currentPart = 250;
		break;
	case 107:
		PART_26_ACTION_107();
		break;
	case 108:
		ADD_DIALOGUE_TEXT(216, 2);
		ADD_DIALOGUE_TEXT(218, 1);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 109:
		ADD_DIALOGUE_TEXT(219, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 110:
		ADD_DIALOGUE_TEXT(220, 2);
		ADD_DIALOGUE_TEXT(222, 1);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 111:
		ADD_DIALOGUE_TEXT(223, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	default:
		error("PART_26_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_26_ACTION_103() {
	if (_objectsState[69] == 1) {
		EXEC_MAIN_ACTION(11);
		return;
	}
	for (int i = 1; i <= 2; ++i) {
		const int offset = 19870;
		for (int j = 0; j <= 65; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + 0x16C0 + i * 2706 + j * 41, 41);
		}
		if (i == 2) {
			playSound(13, 1);
		} else if (i == 1) {
			waitForTimer(100);
		}
	}
	_objectsState[69] = 1;
	PART_25_HELPER_1(1);
}

void IgorEngine::PART_26_ACTION_104() {
	if (_objectsState[69] == 0) {
		EXEC_MAIN_ACTION(14);
		return;
	}
	for (int i = 3; i <= 4; ++i) {
		const int offset = 19870;
		for (int j = 0; j <= 65; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + i * 2706 + j * 41 + 0x16C0, 41);
			if (i == 4) {
				playSound(14, 1);
			}
			if (i == 3) {
				waitForTimer(100);
			}
		}
	}
	_objectsState[69] = 0;
	PART_25_HELPER_1(1);
}

void IgorEngine::PART_26_ACTION_107() {
	if (_objectsState[69] != 0) {
		_roomObjectAreasTable[_screenLayer2[36800]].area = 1;
		--_walkDataLastIndex;
		buildWalkPath(62, 127, 0, 115);
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
		_currentPart = 160;
	}
}

void IgorEngine::PART_26_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(29) || compareGameTick(61)) {
		if (_gameState.unk10 > 2) {
			if (_gameState.unk10 == 11) {
				_gameState.unk10 = 2;
			}
			PART_26_HELPER_7(PART_26_ANIM_DATA_1[_gameState.unk10]);
			if (_gameState.unk10 > 2) {
				++_gameState.unk10;
			}
		} else if (_gameState.unk10 == 1) {
			_gameState.unk10 = 2;
			PART_26_HELPER_7(_gameState.unk10);
		} else if (getRandomNumber(25) == 0) {
			_gameState.unk10 = 1;
			PART_26_HELPER_7(_gameState.unk10);
		} else if (getRandomNumber(30) == 0) {
			_gameState.unk10 = 3;
		}
	}
}

void IgorEngine::PART_26_HELPER_1(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[69] == 0) {
			PART_26_HELPER_2();
			_roomActionsTable[0x1E] = 6;
		} else {
			PART_26_HELPER_3();
			_roomActionsTable[0x1E] = 7;
		}
	}
}

void IgorEngine::PART_26_HELPER_2() {
	const int offset = 19870;
	for (int i = 0; i <= 64; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + i * 25, 25);
	}
}

void IgorEngine::PART_26_HELPER_3() {
	const int offset = 19870;
	for (int i = 0; i <= 64; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + 0x659 + i * 25, 25);
	}
}

void IgorEngine::PART_26_HELPER_4() {
	_walkData[0].setPos(319, 125, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(319, 125, 269, 140);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_26_HELPER_5() {
	_walkData[0].setPos(0, 115, 3, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 2;
	_walkData[0].clipWidth = 30;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[36800]].area = 1;
	buildWalkPath(0, 115, 70, 127);
	_roomObjectAreasTable[_screenLayer2[36800]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	if (_objectsState[65] == 3) {
		ADD_DIALOGUE_TEXT(212, 1);
		ADD_DIALOGUE_TEXT(213, 2);
		ADD_DIALOGUE_TEXT(215, 1);
		SET_DIALOGUE_TEXT(1, 3);
		startIgorDialogue();
		_objectsState[65] = 4;
		drawVerbsPanel();
		redrawVerb(kVerbWalk, true);
		_currentAction.verb = kVerbWalk;
		drawInventory(_inventoryInfo[72], 0);
		playSound(51, 1);
		PART_26_HELPER_1(255);
	}
}

void IgorEngine::PART_26_HELPER_7(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 39; ++i) {
		for (int j = 0; j <= 21; ++j) {
			int offset = (i + 91) * 320 + j + 169;
			uint8 color = _screenVGA[offset];
			if (color < 192 || (color > 207 && color != 240 && color != 241)) {
				color = _animFramesBuffer[0x942 + frame * 880 + i * 22 + j];
			}
			_screenTempLayer[100 * i + j] = color;
		}
	}
	for (int i = 0; i <= 39; ++i) {
		const int offset = i * 320 + 29289;
		memcpy(_screenVGA + offset, _screenTempLayer + i * 100, 22);
		memcpy(_screenLayer1 + offset, _animFramesBuffer + 0x942 + frame * 880 + i * 22, 22);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_26() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeCorridorMissBarrymore, IMG_CollegeCorridorMissBarrymore, BOX_CollegeCorridorMissBarrymore, MSK_CollegeCorridorMissBarrymore, TXT_CollegeCorridorMissBarrymore);
	static const int anm[] = { FRM_CollegeCorridorMissBarrymore1, FRM_CollegeCorridorMissBarrymore2, FRM_CollegeCorridorMissBarrymore3, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorMissBarrymore);
	_roomDataOffsets = PART_26_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(14, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_26_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_26_UPDATE_ROOM_BACKGROUND;
	PART_26_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	_gameState.unk10 = true;
	fadeInPalette(768);
	if (_currentPart == 260) {
		PART_26_HELPER_4();
	} else {
		PART_26_HELPER_5();
	}
	enterPartLoop();
	while (_currentPart >= 260 && _currentPart <= 261) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

} // namespace Igor
