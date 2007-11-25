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

static const uint8 PART_31_ANIM_DATA_1[] = { 0, 4, 5, 4, 5, 4, 5, 4, 5, 6 };

static const uint8 PART_31_ANIM_DATA_2[] = { 0, 4, 5, 4, 5, 4, 5, 1, 2, 3 };

void IgorEngine::PART_31_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_currentPart = 251;
		break;
	case 102:
		PART_31_ACTION_102();
		break;
	case 103:
		PART_31_ACTION_103();
		break;
	case 104:
		if (_objectsState[72] == 0) {
			EXEC_MAIN_ACTION(14);
		} else {
			ADD_DIALOGUE_TEXT(205, 1);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 105:
		ADD_DIALOGUE_TEXT(201, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106:
		PART_31_ACTION_106();
		break;
	case 107:
		ADD_DIALOGUE_TEXT(203, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 108:
		_currentPart = 240;
		break;
	case 109:
		ADD_DIALOGUE_TEXT(204, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 110:
		PART_31_ACTION_110();
		break;
	case 111:
		ADD_DIALOGUE_TEXT(206, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 112:
		ADD_DIALOGUE_TEXT(208, 1);
		ADD_DIALOGUE_TEXT(209, 2);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 113:
		ADD_DIALOGUE_TEXT(211, 1);
		ADD_DIALOGUE_TEXT(212, 1);
		ADD_DIALOGUE_TEXT(213, 1);
		SET_DIALOGUE_TEXT(1, 3);
		startIgorDialogue();
		break;
	case 114:
		ADD_DIALOGUE_TEXT(214, 1);
		ADD_DIALOGUE_TEXT(215, 1);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 115:
		ADD_DIALOGUE_TEXT(220, 3);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 116:
		ADD_DIALOGUE_TEXT(218, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	default:
		error("PART_31_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_31_ACTION_102() {
	if (_objectsState[72] == 0) {
		return;
	}
	_roomObjectAreasTable[_screenLayer2[33712]].area = 2;
	--_walkDataLastIndex;
	buildWalkPath(102, 120, 112, 105);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_roomObjectAreasTable[_screenLayer2[20207]].area = 2;
	--_walkDataLastIndex;
	buildWalkPath(112, 105, 47, 63);
	for (int i = 1; i <= _walkDataLastIndex; ++i) {
		_walkData[i].posNum = i;
	}
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 320;
}

void IgorEngine::PART_31_ACTION_103() {
	if (_objectsState[72] == 1) {
		EXEC_MAIN_ACTION(11);
		return;
	}
	playSound(49, 1);
	const int offset = 20250;
	for (int i = 1; i <= 9; ++i) {
		for (int j = 0; j <= 57; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + PART_31_ANIM_DATA_1[i] * 1856 + j * 32 + 0x384, 32);
		}
		if (i < 9) {
			waitForTimer(30);
		}
	}
	ADD_DIALOGUE_TEXT(202, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
}

void IgorEngine::PART_31_ACTION_106() {
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[36326]].area = 2;
	buildWalkPath(166, 120, 166, 113);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_walkDataCurrentIndex = 0;
	for (int i = 9; i >= 0; --i) {
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		WalkData *wd = &_walkData[0];
		wd->setPos(166, 113, 1, _walkCurrentFrame);
		WalkData::setNextFrame(1, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 30;
		wd->scaleWidth = i * 3 + 23;
		wd->xPosChanged = 1;
		wd->dxPos = 0;
		wd->yPosChanged = 1;
		wd->dyPos = 3;
		wd->scaleHeight = 50;
		moveIgor(wd->posNum, wd->frameNum);
		waitForTimer(15);
	}
	_currentPart = 301;
}

void IgorEngine::PART_31_ACTION_110() {
	if (_objectsState[72] == 1) {
		EXEC_MAIN_ACTION(11);
		return;
	}
	const int offset = 20250;
	for (int i = 1; i <= 9; ++i) {
		for (int j = 0; j <= 57; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + PART_31_ANIM_DATA_2[i] * 1856 + j * 32 + 0x384, 32);
		}
		if (i == 9) {
			playSound(13, 1);
		} else if (i < 8) {
			waitForTimer(30);
		} else if (i == 8) {
			waitForTimer(10);
		}
	}
	_objectsState[72] = 1;
	PART_31_HELPER_1(1);
}

void IgorEngine::PART_31_UPDATE_ROOM_BACKGROUND() {
	int xPos, pos;
	if (_gameState.igorMoving) {
		xPos = _walkData[_walkDataCurrentIndex - 1].x;
		pos = _walkData[_walkDataCurrentIndex - 1].posNum;
	} else {
		xPos = _walkData[_walkDataLastIndex - 1].x;
		pos = _walkData[_walkDataLastIndex - 1].posNum;
	}
	if (xPos < 160 && compareGameTick(3, 8)) {
		if (pos == 2) {
			if (_gameState.igorMoving) {
				if (_gameState.unk10 == 1 || _gameState.unk10 == 2) {
					_gameState.unk10 = 3;
					PART_31_HELPER_2(_gameState.unk10);
					return;
				} else if (_gameState.unk10 == 3) {
					_gameState.unk10 = 4;
					PART_31_HELPER_2(_gameState.unk10);
					return;
				} else if (_gameState.unk10 == 4) {
					if (getRandomNumber(30) == 0) {
						_gameState.unk10 = 5;
						PART_31_HELPER_2(_gameState.unk10);
						_gameState.counter[4] = 0;
						return;
					}
				} else if (_gameState.unk10 == 5) {
					if (_gameState.counter[4] == 20) {
						_gameState.unk10 = 4;
						PART_31_HELPER_2(_gameState.unk10);
						return;
					} else {
						++_gameState.counter[4];
					}
				}
			}
		} else {
			if (_gameState.unk10 == 4 || _gameState.unk10 == 5) {
				_gameState.unk10 = 3;
				PART_31_HELPER_2(_gameState.unk10);
				return;
			}
			if (_gameState.unk10 == 1 || _gameState.unk10 == 3) {
				_gameState.unk10 = 2;
				PART_31_HELPER_2(_gameState.unk10);
				return;
			}
		}
	}
	if (xPos > 159 && xPos < 251) {
		if (compareGameTick(3, 32)) {
			if (_gameState.unk10 == 2) {
				_gameState.unk10 = 1;
			} else {
				_gameState.unk10 = 2;
			}
			PART_31_HELPER_2(_gameState.unk10);
			return;
		}
	}
	if (xPos > 250 && compareGameTick(3, 16)) {
		if (_gameState.unk10 == 1) {
			_gameState.unk10 = 2;
		} else {
			_gameState.unk10 = 1;
		}
		PART_31_HELPER_2(_gameState.unk10);
	}
}

void IgorEngine::PART_31_HELPER_1(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[72] == 0) {
			_roomActionsTable[74] = 6;
		} else {
			PART_31_HELPER_9();
			_roomActionsTable[74] = 7;
		}
	}
}

void IgorEngine::PART_31_HELPER_2(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 42; ++i) {
		for (int j = 0; j <= 30; ++j) {
			int offset = (i + 88) * 320 + j + 288;
			uint8 color = _screenVGA[offset];
			if (color < 192 || (color > 207 && color != 240 && color != 241)) {
				color = _animFramesBuffer[0x310F + frame * 1333 + i * 31 + j];
			}
			_screenTempLayer[100 * i + j] = color;
		}
	}
	for (int i = 0; i <= 42; ++i) {
		const int offset = i * 320 + 28448;
		memcpy(_screenVGA + offset, _screenTempLayer + i * 100, 31);
		memcpy(_screenLayer1 + offset, _animFramesBuffer + 0x310F + frame * 1333 + i * 31, 31);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_31_HELPER_3() {
	_walkData[0].setPos(0, 137, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 15;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPath(0, 137, 30, 137);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_31_HELPER_4() {
	_walkData[0].setPos(47, 63, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 30;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[20207]].area = 2;
	_roomObjectAreasTable[_screenLayer2[33712]].area = 2;
	buildWalkPath(47, 63, 112, 105);
	for (int i = 1; i <= _walkDataCurrentIndex; ++i) {
		_walkData[i].posNum = 2;
	}
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	buildWalkPath(112, 105, 102, 125);
	_roomObjectAreasTable[_screenLayer2[20207]].area = 0;
	_roomObjectAreasTable[_screenLayer2[33712]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_31_HELPER_5() {
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	_walkCurrentPos = 3;
	for (int i = 0; i <= 9; ++i) {
		WalkData *wd = &_walkData[0];
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		wd->setPos(166, 113, 3, _walkCurrentFrame);
		WalkData::setNextFrame(3, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 30;
		wd->scaleWidth = i * 3 + 23;
		wd->xPosChanged = 1;
		wd->dxPos = 0;
		wd->yPosChanged = 1;
		wd->dyPos = 3;
		wd->scaleHeight = 50;
		moveIgor(wd->posNum, wd->frameNum);
		waitForTimer(15);
	}
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
	_roomObjectAreasTable[_screenLayer2[36326]].area = 2;
	--_walkDataLastIndex;
	buildWalkPath(166, 113, 140, 123);
	_roomObjectAreasTable[_screenLayer2[36326]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_31_HELPER_6() {
	_walkData[0].setPos(319, 138, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPath(319, 138, 289, 138);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_31_HELPER_9() {
	const int offset = 20256;
	for (int i = 0; i <= 52; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + i * 26 + 0x562, 26);
	}
}

void IgorEngine::PART_31() {
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeStairsSecondFloor, IMG_CollegeStairsSecondFloor, BOX_CollegeStairsSecondFloor, MSK_CollegeStairsSecondFloor, TXT_CollegeStairsSecondFloor);
	static const int anm[] = { FRM_CollegeStairsSecondFloor1, FRM_CollegeStairsSecondFloor2, FRM_CollegeStairsSecondFloor3, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeStairsSecondFloor);
	_roomDataOffsets = PART_31_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_31_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_31_UPDATE_ROOM_BACKGROUND;
	PART_31_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	_gameState.unk10 = 4;
	PART_31_HELPER_2(_gameState.unk10);
	fadeInPalette(768);
	if (_currentPart == 310) {
		PART_31_HELPER_3();
	} else if (_currentPart == 311) {
		PART_31_HELPER_4();
	} else if (_currentPart == 312) {
		PART_31_HELPER_5();
	} else if (_currentPart == 313) {
		PART_31_HELPER_6();
	}
	enterPartLoop();
	while (_currentPart >= 310 && _currentPart <= 313) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

} // namespace Igor
