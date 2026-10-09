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


void IgorEngine::PART_25_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_currentPart = 260;
		break;
	case 102:
		switch (getRandomNumber(2) + 1) {
		case 1:
			igorSay(204, 2, 1273);
			break;
		case 2:
			igorSay({ { 207, 1, 1274 }, { 208, 1, 1275 }, { 209, 1, 1276 } });
			break;
		case 3:
			igorSay({ { 210, 1, 1277 }, { 211, 1, 1278 } });
			break;
		}
		break;
	case 103:
		igorSay(201, 1, 1271);
		break;
	case 104: // walk to board: nothing happens on arrival
		break;
	case 105:
		PART_25_ACTION_105();
		break;
	case 106:
		igorSay(203, 1, 1272);
		break;
	case 107:
		PART_25_ACTION_107();
		break;
	case 108:
		PART_25_ACTION_108();
		break;
	case 109:
		_currentPart = 310;
		break;
	default:
		error("PART_25_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_25_ACTION_105() {
	if (_objectsState[68] != 0) {
		_roomObjectAreasTable[_screenLayer2[34370]].area = 1;
		--_walkDataLastIndex;
		buildWalkPath(133, 123, 130, 107);
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
		_currentPart = 180;
	}
}

void IgorEngine::PART_25_ACTION_107() {
	if (_objectsState[68] == 1) {
		EXEC_MAIN_ACTION(11);
		return;
	}
	for (int i = 1; i <= 2; ++i) {
		const int offset = 21881;
		drawAnimRect(offset, i * 1736 + 0x290, 31, 56);
		if (i == 2) {
			playSound(13, 1);
		}
		if (i == 1) {
			waitForTimer(100);
		}
	}
	_objectsState[68] = 1;
	PART_25_HELPER_1(1);
}

void IgorEngine::PART_25_ACTION_108() {
	if (_objectsState[68] == 0) {
		EXEC_MAIN_ACTION(14);
		return;
	}
	for (int i = 3; i <= 4; ++i) {
		const int offset = 21881;
		drawAnimRect(offset, i * 1736 + 0x290, 31, 56);
		if (i == 4) {
			playSound(14, 1);
		}
		if (i == 3) {
			waitForTimer(100);
		}
	}
	_objectsState[68] = 0;
	PART_25_HELPER_1(1);
}

void IgorEngine::PART_25_HELPER_1(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[68] == 0) {
			PART_25_HELPER_5();
			_roomActionsTable[31] = 6;
		} else {
			PART_25_HELPER_7();
			_roomActionsTable[31] = 7;
		}
	}
}

void IgorEngine::PART_25_HELPER_2() {
	_walkData[0].setPos(0, 130, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 15;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(0, 130, 60, 135);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_25_HELPER_3() {
	_walkData[0].setPos(319, 138, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(319, 138, 289, 142);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_25_HELPER_4() {
	_walkData[0].setPos(130, 107, 3, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[34370]].area = 1;
	buildWalkPath(130, 107, 135, 125);
	_roomObjectAreasTable[_screenLayer2[34370]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_25_HELPER_5() {
	const int offset = 22208;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer, 23, 23, 52);
}

void IgorEngine::PART_25_HELPER_7() {
	const int offset = 22208;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + 0x4AC, 23, 23, 52);
}

void IgorEngine::PART_25() {
	playMusic(2);
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeCorridorAnnouncementBoard, IMG_CollegeCorridorAnnouncementBoard, BOX_CollegeCorridorAnnouncementBoard, MSK_CollegeCorridorAnnouncementBoard, TXT_CollegeCorridorAnnouncementBoard);
	static const int anm[] = { FRM_CollegeCorridorAnnouncementBoard1, FRM_CollegeCorridorAnnouncementBoard2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorAnnouncementBoard);
	_roomDataOffsets = PART_25_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_25_EXEC_ACTION);
	PART_25_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeIn(768);
	if (_currentPart == 250) {
		PART_25_HELPER_2();
	} else if (_currentPart == 251) {
		PART_25_HELPER_3();
	} else if (_currentPart == 252) {
		PART_25_HELPER_4();
	}
	enterPartLoop();
	while (_currentPart >= 250 && _currentPart <= 252) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOut(624);
}


} // End of namespace Igor
