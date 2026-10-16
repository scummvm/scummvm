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

static const int kMensDoorCell = 114 * 320 + 181;
static const int kMensDoorPictureOffset = 18729;
static const int kMensDoorAnimOffset = 18728;
static const int kMensDoorFramesBase = 0x5FC;
static const int kMensDoorFrameSize = 0x762;
static const int kMensDoorClosedPictureSize = 29 * 59;

void IgorEngine::part25FloppyExecuteAction(int action) {
	switch (action) {
	case 101: // exit left
		_currentPart = 260;
		break;
	case 102: // look at lockers
		igorSay(201, 2, 0);
		break;
	case 103: // open lockers
		igorSay(203, 1, 0);
		break;
	case 104: // close lockers
		igorSay(204, 1, 0);
		break;
	case 105:
		part25FloppyEnterMensToilet();
		break;
	case 106: // look at door
		igorSay(205, 1, 0);
		break;
	case 107:
		part25FloppyOpenDoor();
		break;
	case 108:
		part25FloppyCloseDoor();
		break;
	case 109: // exit right
		_currentPart = 310;
		break;
	default:
		error("part25FloppyExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part25FloppyEnterMensToilet() {
	if (_objectsState[kMensRoomDoorStateIndex] == 0) {
		return;
	}
	_roomObjectAreasTable[_screenLayer2[kMensDoorCell]].area = 2;
	--_walkDataLastIndex;
	buildWalkPathSimple(180, 120, 181, 114);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 180;
}

void IgorEngine::part25FloppyOpenDoor() {
	if (_objectsState[kMensRoomDoorStateIndex] == 1) {
		partMainExecuteAction(11);
		return;
	}
	for (int i = 1; i <= 3; ++i) {
		drawAnimRect(kMensDoorAnimOffset, kMensDoorFramesBase + i * kMensDoorFrameSize, 30, 63);
		if (i == 3) {
			playSound(13, 1);
		}
		waitForTimer(127);
	}
	_objectsState[kMensRoomDoorStateIndex] = 1;
	part25FloppyApplyObjectState(1);
}

void IgorEngine::part25FloppyCloseDoor() {
	if (_objectsState[kMensRoomDoorStateIndex] == 0) {
		partMainExecuteAction(14);
		return;
	}
	static const uint8 kCloseDoorFrames[3] = { 4, 2, 1 };
	for (int i = 1; i <= 3; ++i) {
		drawAnimRect(kMensDoorAnimOffset, kMensDoorFramesBase + kCloseDoorFrames[i - 1] * kMensDoorFrameSize, 30, 63);
		if (i == 2) {
			playSound(14, 1);
		}
		waitForTimer(127);
	}
	_objectsState[kMensRoomDoorStateIndex] = 0;
	part25FloppyApplyObjectState(1);
}

void IgorEngine::part25FloppyApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[kMensRoomDoorStateIndex] == 0) { // door state
			copyArea(_screenLayer1, kMensDoorPictureOffset, 320, _animFramesBuffer, 29, 29, 59);
			_roomActionsTable[31] = 6;
		} else {
			copyArea(_screenLayer1, kMensDoorPictureOffset, 320, _animFramesBuffer + kMensDoorClosedPictureSize, 29, 29, 59);
			_roomActionsTable[31] = 7;
		}
	}
}

void IgorEngine::part25FloppyEnterFromLeft() {
	_walkData[0].setPos(0, 134, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 15;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPathSimple(0, 134, 30, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part25FloppyEnterFromRight() {
	_walkData[0].setPos(319, 134, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPathSimple(319, 134, 289, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part25FloppyEnterFromMensToilet() {
	_walkData[0].setPos(181, 114, 3, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[kMensDoorCell]].area = 2;
	buildWalkPathSimple(181, 114, 181, 125);
	_roomObjectAreasTable[_screenLayer2[kMensDoorCell]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part25Floppy() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeCorridorAnnouncementBoard, IMG_CollegeCorridorAnnouncementBoard, BOX_CollegeCorridorAnnouncementBoard, MSK_CollegeCorridorAnnouncementBoard, TXT_CollegeCorridorAnnouncementBoard);
	static const int anm[] = { FRM_CollegeCorridorAnnouncementBoardFloppy1, FRM_CollegeCorridorAnnouncementBoardFloppy2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorAnnouncementBoard);
	_roomDataOffsets = PART_25_FLOPPY_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	partMainSetExecuteAction(1, &IgorEngine::part25FloppyExecuteAction);
	part25FloppyApplyObjectState(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 250) {
			part25FloppyEnterFromLeft();
		} else if (_currentPart == 251) {
			part25FloppyEnterFromRight();
		} else if (_currentPart == 252) {
			part25FloppyEnterFromMensToilet();
		}
	}
	enterPartLoop();
	while (_currentPart >= 250 && _currentPart <= 252 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
