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

static const uint8 PART_23_ANIM_DATA_1[4] = { 0, 4, 1, 11 };

void IgorEngine::PART_23_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_currentPart = 280;
		break;
	case 102:
		ADD_DIALOGUE_TEXT(201, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		PART_23_ACTION_105();
		break;
	case 106:
		ADD_DIALOGUE_TEXT(205, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107:
		PART_23_ACTION_107();
		break;
	case 108:
		PART_23_ACTION_108();
		break;
	case 109:
		_currentPart = 300;
		break;
	case 110:
		ADD_DIALOGUE_TEXT(203, 1);
		ADD_DIALOGUE_TEXT(204, 1);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 111:
		ADD_DIALOGUE_TEXT(208, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 112:
		ADD_DIALOGUE_TEXT(210, 1);
		ADD_DIALOGUE_TEXT(211, 1);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 113:
		ADD_DIALOGUE_TEXT(212, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	default:
		error("PART_23_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_23_ACTION_105() {
	if (_objectsState[66] != 0) {
		_roomObjectAreasTable[_screenLayer2[38633]].area = 1;
		--_walkDataLastIndex;
		buildWalkPathSimple(233, 132, 233, 120);
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
		_currentPart = 190;
	}
}

void IgorEngine::PART_23_ACTION_107() {
	if (_objectsState[66] == 1) {
		EXEC_MAIN_ACTION(11);
	} else {
		const int offset = 23901;
		for (int i = 2; i <= 3; ++i) {
			for (int j = 0; j <= 53; ++j) {
				memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + i * 1620 + j * 30 + 0x895, 30);
			}
		}
		playSound(13, 1);
		waitForTimer(100);
		_objectsState[66] = 1;
		PART_23_HELPER_1(1);
	}
}

void IgorEngine::PART_23_ACTION_108() {
	if (_objectsState[66] == 0) {
		EXEC_MAIN_ACTION(14);
	} else {
		const int offset = 23901;
		for (int i = 2; i <= 3; ++i) {
			for (int j = 0; j <= 53; ++j) {
				memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + PART_23_ANIM_DATA_1[i] * 1620 + j * 30 + 0x895, 30);
			}
		}
		playSound(14, 1);
		waitForTimer(100);
		_objectsState[66] = 0;
		PART_23_HELPER_1(1);
	}
}

void IgorEngine::PART_23_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(29) || compareGameTick(61)) {
		int rnd = getRandomNumber(11);
		if (rnd <= 4) {
			_gameState.unk10 = 1;
		} else if (rnd <= 9) {
			_gameState.unk10 = 2;
		} else if (rnd == 10) {
			if (_gameState.unk10 < 3) {
				_gameState.unk10 = getRandomNumber(2) + 3;
			} else {
				_gameState.unk10 = getRandomNumber(2) + 1;
			}
		}
		PART_23_HELPER_7(_gameState.unk10);
	}
}

void IgorEngine::PART_23_HELPER_1(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[66] == 0) {
			PART_23_HELPER_2(1);
			_roomActionsTable[4] = 6;
		} else {
			PART_23_HELPER_2(2);
			_roomActionsTable[4] = 7;
		}
	}
	PART_23_HELPER_3();
}

void IgorEngine::PART_23_HELPER_2(int frame) {
	const int offset = 23907;
	for (int i = 0; i <= 53; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + (frame - 1) * 1296 + i * 24, 24);
	}
}

void IgorEngine::PART_23_HELPER_3() {
	const int offset = 25763;
	for (int i = 0; i <= 48; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + i * 25 + 0xA20, 25);
	}
}

void IgorEngine::PART_23_HELPER_4() {
	_walkData[0].setPos(233, 120, 3, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[38633]].area = 1;
	buildWalkPathSimple(233, 120, 233, 137);
	_roomObjectAreasTable[_screenLayer2[38633]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	if (_objectsState[59] == 0) {
		for (int i = 0; i <= 53; ++i) {
			for (int j = 0; j <= 23; ++j) {
				uint8 color = _screenVGA[(i + 74) * 320 + j + 227];
				if (color >= 0xC0 && color <= 0xCF) {
					_screenTempLayer[i * 100 + j] = color;
				} else {
					_screenTempLayer[i * 100 + j] = _animFramesBuffer[i * 24 + j];
				}
			}
		}
		for (int i = 0; i <= 53; ++i) {
			memcpy(_screenVGA + i * 320 + 23907, _screenTempLayer + i * 100, 24);
		}
		playSound(14, 1);
		_objectsState[66] = 0;
		PART_23_HELPER_1(1);
		ADD_DIALOGUE_TEXT(206, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
	}
}

void IgorEngine::PART_23_HELPER_5() {
	_walkData[0].setPos(0, 134, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 15;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(0, 134, 100, 140);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_23_HELPER_6() {
	_walkData[0].setPos(319, 134, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(319, 134, 289, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_23_HELPER_7(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 18; ++i) {
		for (int j = 0; j <= 14; ++j) {
			int offset = (i + 80) * 320 + j + 162;
			uint8 color = _screenVGA[offset];
			if ((color >= 192 && color <= 207) || color == 240 || color == 241) {
				_screenTempLayer[i * 100 + j] = color;
			} else {
				_screenTempLayer[i * 100 + j] = _animFramesBuffer[frame * 285 + i * 15 + j + 0x271C];
			}
		}
	}
	int offset = 25762;
	for (int i = 0; i <= 18; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _screenTempLayer + i * 100, 15);
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + frame * 285 + i * 15 + 0x271C, 15);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_23() {
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeCorridorLucas, IMG_CollegeCorridorLucas, BOX_CollegeCorridorLucas, MSK_CollegeCorridorLucas, TXT_CollegeCorridorLucas);
	static const int anm[] = { FRM_CollegeCorridorLucas1, FRM_CollegeCorridorLucas2, FRM_CollegeCorridorLucas3, FRM_CollegeCorridorLucas4, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorLucas);
	_roomDataOffsets = PART_23_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(15, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_23_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_23_UPDATE_ROOM_BACKGROUND;
	PART_23_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeInPalette(768);
	if (_currentPart == 230) {
		PART_23_HELPER_5();
	} else if (_currentPart == 231) {
		PART_23_HELPER_6();
	} else if (_currentPart == 232) {
		PART_23_HELPER_4();
	}
	enterPartLoop();
	while (_currentPart >= 230 && _currentPart <= 232) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

} // namespace Igor
