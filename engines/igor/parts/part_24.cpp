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

void IgorEngine::PART_24_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_currentPart = 313;
		break;
	case 102:
		PART_24_ACTION_102();
		break;
	case 103:
		ADD_DIALOGUE_TEXT(201, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104:
		PART_24_ACTION_104();
		break;
	case 105:
		PART_24_ACTION_105();
		break;
	case 106:
		ADD_DIALOGUE_TEXT(202, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107:
		PART_24_ACTION_107();
		break;
	case 108:
		ADD_DIALOGUE_TEXT(203, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 109:
		ADD_DIALOGUE_TEXT(204, 1);
		ADD_DIALOGUE_TEXT(205, 1);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 110:
		ADD_DIALOGUE_TEXT(206, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 111:
		ADD_DIALOGUE_TEXT(208, 2);
		ADD_DIALOGUE_TEXT(210, 2);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 112:
		ADD_DIALOGUE_TEXT(212, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	default:
		error("PART_24_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_24_ACTION_102() {
	if (_objectsState[67] != 0) {
		_roomObjectAreasTable[_screenLayer2[37188]].area = 1;
		--_walkDataLastIndex;
		buildWalkPathSimple(76, 128, 68, 116);
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
		_currentPart = 360;
	}
}

void IgorEngine::PART_24_ACTION_104() {
	if (_objectsState[67] == 1) {
		executeAction(11);
		return;
	}
	for (int i = 1; i <= 2; ++i) {
		const int offset = 23104;
		for (int j = 0; j <= 56; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + i * 1938 + j * 34 + 0x69C, 34);
		}
		if (i == 2) {
			playSound(13, 1);
		} else if (i == 1) {
			waitForTimer(100);
		}
	}
	_objectsState[67] = 1;
	PART_24_HELPER_1(1);
}

void IgorEngine::PART_24_ACTION_105() {
	if (_objectsState[67] == 0) {
		executeAction(14);
		return;
	}
	for (int i = 3; i <= 4; ++i) {
		const int offset = 23104;
		for (int j = 0; j <= 56; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + i * 1938 + j * 34 + 0x69C, 34);
		}
		if (i == 4) {
			playSound(14, 1);
		} else if (i == 3) {
			waitForTimer(100);
		}
	}
	_objectsState[67] = 0;
	PART_24_HELPER_1(1);
}

void IgorEngine::PART_24_ACTION_107() {
	_roomObjectAreasTable[_screenLayer2[38719]].area = 1;
	--_walkDataLastIndex;
	buildWalkPathSimple(266, 123, 319, 120);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 330;
}

void IgorEngine::PART_24_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(29) || compareGameTick(61)) {
		if (_gameState.unk10 == _gameState.counter[4]) {
			_gameState.counter[4] = getRandomNumber(90) + 10;
			_gameState.unk10 = 0;
			if (_gameState.unkF) {
				PART_24_HELPER_3(1);
			} else {
				PART_24_HELPER_3(2);
			}
			_gameState.unkF = !_gameState.unkF;
		} else {
			++_gameState.unk10;
		}
		if (_gameState.unkF) {
			PART_24_HELPER_3(getRandomNumber(6) + 1);
		} else {
			PART_24_HELPER_2(getRandomNumber(6) + 1);
		}
	}
}

void IgorEngine::PART_24_HELPER_1(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[67] == 0) {
			PART_24_HELPER_4();
			_roomActionsTable[2] = 6;
		} else {
			PART_24_HELPER_5();
			_roomActionsTable[2] = 7;
		}
	}
}

void IgorEngine::PART_24_HELPER_2(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 48; ++i) {
		for (int j = 0; j <= 22; ++j) {
			int offset = (i + 76) * 320 + j + 149;
			uint8 color = _screenVGA[offset];
			if ((color >= 192 && color <= 207) || color == 240 || color == 241) {
				_screenTempLayer[i * 100 + j] = _screenVGA[offset];
			} else {
				_screenTempLayer[i * 100 + j] = _animFramesBuffer[frame * 1127 + i * 23 + j + 0x3E3B];
			}
		}
	}
	int offset = 24469;
	for (int i = 0; i <= 48; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _screenTempLayer + i * 100, 23);
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + frame * 1127 + i * 23 + 0x3E3B, 23);
	}
	if (_gameState.dialogueTextRunning) {
		memcpy(_screenTextLayer + 23040, _screenLayer1 + _dialogueDirtyRectY, _dialogueDirtyRectSize);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_24_HELPER_3(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 42; ++i) {
		for (int j = 0; j <= 21; ++j) {
			int offset = (i + 80) * 320 + j + 173;
			uint8 color = _screenVGA[offset];
			if ((color >= 192 && color <= 207) || color == 240 || color == 241) {
				_screenTempLayer[i * 100 + j] = _screenVGA[offset];
			} else {
				_screenTempLayer[i * 100 + j] = _animFramesBuffer[frame * 946 + i * 22 + j + 0x28C4];
			}
		}
	}
	int offset = 25773;
	for (int i = 0; i <= 42; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _screenTempLayer + i * 100, 22);
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + frame * 946 + i * 22 + 0x28C4, 22);
	}
	if (_gameState.dialogueTextRunning) {
		memcpy(_screenTextLayer + 23040, _screenLayer1 + _dialogueDirtyRectY, _dialogueDirtyRectSize);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_24_HELPER_4() {
	const int offset = 22785;
	for (int i = 0; i <= 54; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + i * 33, 33);
	}
}

void IgorEngine::PART_24_HELPER_5() {
	const int offset = 22785;
	for (int i = 0; i <= 54; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + i * 33 + 0x717, 33);
	}
}

void IgorEngine::PART_24_HELPER_7() {
	_walkData[0].setPos(0, 138, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 15;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(0, 138, 30, 140);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_24_HELPER_8() {
	_walkData[0].setPos(319, 123, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	_roomObjectAreasTable[_screenLayer2[39679]].area = 1;
	buildWalkPathSimple(319, 123, 260, 123);
	_roomObjectAreasTable[_screenLayer2[39679]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_24_HELPER_9() {
	_walkData[0].setPos(68, 116, 3, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	_roomObjectAreasTable[_screenLayer2[37188]].area = 1;
	buildWalkPathSimple(68, 116, 78, 130);
	_roomObjectAreasTable[_screenLayer2[37188]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_24() {
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeCorridorSharonMichael, IMG_CollegeCorridorSharonMichael, BOX_CollegeCorridorSharonMichael, MSK_CollegeCorridorSharonMichael, TXT_CollegeCorridorSharonMichael);
	static const int anm[] = { FRM_CollegeCorridorSharonMichael1, FRM_CollegeCorridorSharonMichael2, FRM_CollegeCorridorSharonMichael3, FRM_CollegeCorridorSharonMichael4, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorSharonMichael);
	_roomDataOffsets = PART_24_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_24_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_24_UPDATE_ROOM_BACKGROUND;
	PART_24_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeInPalette(768);
	if (_currentPart == 240) {
		PART_24_HELPER_7();
	} else if (_currentPart == 241) {
		PART_24_HELPER_8();
	} else if (_currentPart == 242) {
		PART_24_HELPER_9();
	}
	enterPartLoop();
	while (_currentPart >= 240 && _currentPart <= 242) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

} // namespace Igor
