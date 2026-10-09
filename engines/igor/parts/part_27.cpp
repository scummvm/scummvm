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


void IgorEngine::PART_27_EXEC_ACTION(int action) {
	switch (action) {
	case 101: // go to right
		_currentPart = 211;
		break;
	case 102: // look at lockers
		if (_objectsState[85] == 0) {
			igorSay(216, 2, 1173);
			_objectsState[85] = 1;
			PART_27_HELPER_1(255);
		} else {
			igorSay(201, 1, 1162);
		}
		break;
	case 103: // open lockers
		igorSay(203, 1, 1163);
		break;
	case 104: // close lockers
		igorSay(204, 1, 1164);
		break;
	case 105: // look at philips locker
		igorSay(205, 1, 1165);
		break;
	case 106:
		PART_27_ACTION_106_openPhilipLocker();
		break;
	case 107:
		PART_27_ACTION_107();
		break;
	case 108:
		PART_27_ACTION_108();
		break;
	case 109:
		igorSay(215, 1, 1172);
		break;
	case 110:
		PART_27_ACTION_110();
		break;
	case 111:
		igorSay(218, 2, 1174);
		break;
	case 112: // look at plaque
		igorSay(221, 1, 1176);
		break;
	case 113: // go to library
		_currentPart = 150;
		break;
	default:
		error("PART_27_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_27_ACTION_106_openPhilipLocker() {
	if (_objectsState[84] == 1) {
		EXEC_MAIN_ACTION(11);
		return;
	}
	if (_objectsState[5] == 0) {
		igorSay(206, 1, 1166);
		return;
	}
	if (_objectsState[5] == 1) {
		igorSayAndWait(209, 3, 1169);
		_objectsState[5] = 2;
	}
	const int offset = 21810;
	for (int i = 2; i <= 3; ++i) {
		drawAnimRect(offset, 0x3A0 + i * 1568, 32, 49);
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
		drawAnimRect(offset, 0x3A0 + i * 1568, 32, 49);
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
		igorSay(208, 1, 1168);
		return;
	}
	igorSayAndWait(207, 1, 1167);
	const int offset = 25012;
	for (int i = 1; i <= 2; ++i) {
		drawAnimRect(offset, i * 630 + 0x19AA, 21, 30);
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
	igorSayAndWait({ { 212, 1, 1170 }, { 213, 2, 1171 } });
	const int offset = 25012;
	for (int i = 1; i <= 2; ++i) {
		drawAnimRect(offset, 0x19AA + i * 630, 21, 49);
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
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer, 26, 26, 48);
}

void IgorEngine::PART_27_HELPER_4() {
	const int offset = 21816;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + 0x4E0, 26, 26, 48);
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
	playMusic(2);
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
	fadeIn(768);
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
	fadeOut(624);
}

} // End of namespace Igor
