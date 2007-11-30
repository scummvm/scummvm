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

void IgorEngine::PART_27_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_currentPart = 231;
		break;
	case 102:
		if (_objectsState[85] == 0) {
			ADD_DIALOGUE_TEXT(216, 2);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
			_objectsState[85] = 1;
			PART_27_HELPER_1(255);
		} else {
			ADD_DIALOGUE_TEXT(201, 1);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 103:
		ADD_DIALOGUE_TEXT(203, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(204, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		ADD_DIALOGUE_TEXT(205, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106:
		PART_27_ACTION_106();
		break;
	case 107:
		PART_27_ACTION_107();
		break;
	case 108:
		PART_27_ACTION_108();
		break;
	case 109:
		ADD_DIALOGUE_TEXT(215, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 110:
		PART_27_ACTION_110();
		break;
	case 111:
		ADD_DIALOGUE_TEXT(222, 1);
		ADD_DIALOGUE_TEXT(223, 2);
		ADD_DIALOGUE_TEXT(225, 4);
		SET_DIALOGUE_TEXT(1, 3);
		startIgorDialogue();
		break;
	case 112:
		ADD_DIALOGUE_TEXT(221, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 113:
		_currentPart = 150;
		break;
	default:
		error("PART_27_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_27_ACTION_106() {
	if (_objectsState[84] == 1) {
		EXEC_MAIN_ACTION(11);
		return;
	}
	if (_objectsState[5] == 0) {
		ADD_DIALOGUE_TEXT(206, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		return;
	}
	if (_objectsState[5] == 1) {
		ADD_DIALOGUE_TEXT(209, 3);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		_objectsState[5] = 2;
		return;
	}
	const int offset = 21810;
	for (int i = 2; i <= 3; ++i) {
		for (int j = 0; j <= 48; ++j) {
			memcpy(_screenVGA + i * 320 + offset, _animFramesBuffer + 0x3A0 + i * 1568 + j * 32, 32);
		}
		if (i == 3) {
			playSound(3, 1);
		}
		if (i == 2) {
			waitForTimer(100);
		}
	}
	_objectsState[84] = 1;
	PART_27_HELPER_1(1);
}

void IgorEngine::PART_27_ACTION_107() {
	if (_objectsState[84] == 0) {
		EXEC_MAIN_ACTION(14);
		return;
	}
	const int offset = 21810;
	for (int i = 2; i >= 1; --i) {
		for (int j = 0; j <= 48; ++j) {
			memcpy(_screenVGA + i * 320 + offset, _animFramesBuffer + 0x3A0 + i * 1568 + j * 32, 32);
		}
		if (i == 2) {
			playSound(14, 1);
			waitForTimer(100);
		}
	}
	_objectsState[84] = 0;
	PART_27_HELPER_1(1);
}

void IgorEngine::PART_27_ACTION_108() {
	if (_inventoryInfo[58] > 0 || _objectsState[42] == 2) {
		ADD_DIALOGUE_TEXT(208, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		return;
	}
	ADD_DIALOGUE_TEXT(207, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	const int offset = 25012;
	for (int i = 1; i <= 2; ++i) {
		for (int j = 0; j <= 29; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + i * 630 + j * 21 + 0x19AA, 21);
		}
		if (i == 1) {
			waitForTimer(100);
		}
	}
	addObjectToInventory(23, 58);
	PART_27_HELPER_1(1);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
}

void IgorEngine::PART_27_ACTION_110() {
	ADD_DIALOGUE_TEXT(212, 1);
	ADD_DIALOGUE_TEXT(213, 2);
	SET_DIALOGUE_TEXT(1, 2);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	const int offset = 25012;
	for (int i = 1; i <= 2; ++i) {
		for (int j = 0; j <= 48; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + 0x19AA + i * 630 + j * 21, 21);
		}
		if (i == 1) {
			waitForTimer(100);
		}
	}
	removeObjectFromInventory(55);
	PART_27_HELPER_1(255);
	_objectsState[107] = 1;
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
}

void IgorEngine::PART_27_HELPER_1(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[84] == 0) {
			PART_27_HELPER_3();
			_roomActionsTable[3] = 6;
			_roomObjectAreasTable[6].object = 3;
		} else {
			PART_27_HELPER_4();
			_roomActionsTable[3] = 7;
			_roomObjectAreasTable[6].object = 4;
		}
	}
	if (num == 2 || num == 255) {
		if (_objectsState[85] == 0) {
			_roomObjectAreasTable[5].object = 2;
			_roomObjectAreasTable[6].object = 2;
		} else {
			_roomObjectAreasTable[5].object = 3;
		}
	}
}

void IgorEngine::PART_27_HELPER_2() {
	_walkData[0].setPos(0, 132, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(0, 132, 30, 132);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_27_HELPER_3() {
	const int offset = 21816;
	for (int i = 0; i <= 47; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + i * 26, 26);
	}
}

void IgorEngine::PART_27_HELPER_4() {
	const int offset = 21816;
	for (int i = 0; i <= 47; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + i * 26 + 0x4E0, 26);
	}
}

void IgorEngine::PART_27_HELPER_5() {
	_walkData[0].setPos(270, 134, 4, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(270, 134, 140, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_27() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeLockers, IMG_CollegeLockers, BOX_CollegeLockers, MSK_CollegeLockers, TXT_CollegeLockers);
	static const int anm[] = { FRM_CollegeLockers1, FRM_CollegeLockers2, FRM_CollegeLockers3, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeLockers);
	_roomDataOffsets = PART_27_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_27_EXEC_ACTION);
	PART_27_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeInPalette(768);
	if (_currentPart == 270) {
		PART_27_HELPER_2();
	} else {
		PART_27_HELPER_5();
	}
	enterPartLoop();
	while (_currentPart == 270 || _currentPart == 271) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

} // namespace Igor
