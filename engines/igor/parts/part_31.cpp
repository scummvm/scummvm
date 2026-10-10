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
		PART_31_ACTION_103_openDoor();
		break;
	case 104:
		if (_objectsState[72] == 0) {
			EXEC_MAIN_ACTION(14);
		} else {
			igorSay(205, 1, 885);
		}
		break;
	case 105:
		igorSay(201, 1, 881);
		break;
	case 106:
		PART_31_ACTION_106_goDownstairs();
		break;
	case 107:
		igorSay(203, 1, 883);
		break;
	case 108:
		_currentPart = 240;
		break;
	case 109:
		igorSay(204, 1, 884);
		break;
	case 110:
		PART_31_ACTION_110_openDoor2();
		break;
	case 111:
		igorSay(206, 2, 886);
		break;
	case 112:
		igorSay({ { 208, 1, 887 }, { 209, 2, 888 } });
		break;
	case 113:
		igorSay({ { 211, 1, 889 }, { 212, 1, 890 }, { 213, 1, 891 } });
		break;
	case 114:
		igorSay({ { 214, 1, 892 }, { 215, 1, 893 } });
		break;
	case 115:
		igorSay(220, 3, 895);
		break;
	case 116:
		igorSay(218, 2, 894);
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
		_walkData[i].posNum = 4;
	}
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 320;
}

void IgorEngine::PART_31_ACTION_103_openDoor() {
	if (_objectsState[72] == 1) {
		EXEC_MAIN_ACTION(11);
		return;
	}
	playSound(49, 1);
	const int offset = 20250;
	for (int i = 1; i <= 9; ++i) {
		drawAnimRect(offset, PART_31_ANIM_DATA_1[i] * 1856 + 0x384, 32, 58);
		if (i < 9) {
			waitForTimer(30);
		}
	}
	igorSay(202, 1, 882);
}

void IgorEngine::PART_31_ACTION_106_goDownstairs() {
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

void IgorEngine::PART_31_ACTION_110_openDoor2() {
	if (_objectsState[72] == 1) {
		EXEC_MAIN_ACTION(11);
		return;
	}
	const int offset = 20250;
	for (int i = 1; i <= 9; ++i) {
		drawAnimRect(offset, PART_31_ANIM_DATA_2[i] * 1856 + 0x384, 32, 58);
		if (i == 9) {
			playSound(13, 1);
		} else if (i < 8) {
			waitForTimer(30);
		} else if (i == 8) {
			waitForTimer(10);
		}
	}
	_objectsState[72] = 1;
	PART_31_HELPER_1_OBJECT_STATE(1);
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
					PART_31_HELPER_2_drawJohnnyFrame(_gameState.unk10);
					return;
				} else if (_gameState.unk10 == 3) {
					_gameState.unk10 = 4;
					PART_31_HELPER_2_drawJohnnyFrame(_gameState.unk10);
					return;
				} else if (_gameState.unk10 == 4) {
					if (getRandomNumber(29) == 0) {
						_gameState.unk10 = 5;
						PART_31_HELPER_2_drawJohnnyFrame(_gameState.unk10);
						_gameState.counter[4] = 0;
						return;
					}
				} else if (_gameState.unk10 == 5) {
					if (_gameState.counter[4] == 20) {
						_gameState.unk10 = 4;
						PART_31_HELPER_2_drawJohnnyFrame(_gameState.unk10);
						return;
					} else {
						++_gameState.counter[4];
					}
				}
			}
		} else {
			if (_gameState.unk10 == 4 || _gameState.unk10 == 5) {
				_gameState.unk10 = 3;
				PART_31_HELPER_2_drawJohnnyFrame(_gameState.unk10);
				return;
			}
			if (_gameState.unk10 == 1 || _gameState.unk10 == 3) {
				_gameState.unk10 = 2;
				PART_31_HELPER_2_drawJohnnyFrame(_gameState.unk10);
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
			PART_31_HELPER_2_drawJohnnyFrame(_gameState.unk10);
			return;
		}
	}
	if (xPos > 250 && compareGameTick(3, 16)) {
		if (_gameState.unk10 == 1) {
			_gameState.unk10 = 2;
		} else {
			_gameState.unk10 = 1;
		}
		PART_31_HELPER_2_drawJohnnyFrame(_gameState.unk10);
	}
}

void IgorEngine::PART_31_HELPER_1_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[72] == 0) {
			_roomActionsTable[74] = 6;
		} else {
			PART_31_HELPER_9_drawOpenDoor();
			_roomActionsTable[74] = 7;
		}
	}
}

void IgorEngine::PART_31_HELPER_2_drawJohnnyFrame(int frame) {
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

void IgorEngine::PART_31_HELPER_3_enterFromLeft() {
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

void IgorEngine::PART_31_HELPER_4_enterFromAstronomyLab() {
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

void IgorEngine::PART_31_HELPER_5_enterFromDownstairs() {
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

void IgorEngine::PART_31_HELPER_6_enterFromRight() {
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

void IgorEngine::PART_31_HELPER_9_drawOpenDoor() {
	const int offset = 20256;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + 0x562, 26, 26, 53);
}

void IgorEngine::PART_31() {
	playMusic(2);
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeStairsSecondFloor, IMG_CollegeStairsSecondFloor, BOX_CollegeStairsSecondFloor, MSK_CollegeStairsSecondFloor, TXT_CollegeStairsSecondFloor);
	static const int anm[] = { FRM_CollegeStairsSecondFloor1, FRM_CollegeStairsSecondFloor2, FRM_CollegeStairsSecondFloor3, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeStairsSecondFloor);
	_roomDataOffsets = PART_31_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_31_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_31_UPDATE_ROOM_BACKGROUND;
	PART_31_HELPER_1_OBJECT_STATE(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	_gameState.unk10 = 4;
	PART_31_HELPER_2_drawJohnnyFrame(_gameState.unk10);
	fadeIn(768);
	if (_currentPart == 310) {
		PART_31_HELPER_3_enterFromLeft();
	} else if (_currentPart == 311) {
		PART_31_HELPER_4_enterFromAstronomyLab();
	} else if (_currentPart == 312) {
		PART_31_HELPER_5_enterFromDownstairs();
	} else if (_currentPart == 313) {
		PART_31_HELPER_6_enterFromRight();
	}
	enterPartLoop();
	while (_currentPart >= 310 && _currentPart <= 313) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOut(624);
}

} // End of namespace Igor
