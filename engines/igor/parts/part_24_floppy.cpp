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

static const int kChemistryDoorCell = 114 * 320 + 181;
static const int kChemistryDoorPictureOffset = 18729;
static const int kChemistryDoorAnimOffset = 18727;
static const int kChemistryDoorFramesBase = 0x5BD;
static const int kChemistryDoorFrameSize = 0x7A1;
static const int kChemistryDoorClosedPictureSize = 29 * 59;

static const uint8 kCloseDoorFrames[3] = { 4, 2, 1 };

void IgorEngine::part24FloppyExecuteAction(int action) {
	switch (action) {
	case 101: // exit left
		_currentPart = 201;
		break;
	case 102:
		part24FloppyEnterChemistryClass();
		break;
	case 103: // look at the chemistry door
		igorSay(201, 1, 0);
		break;
	case 104:
		part24FloppyOpenDoor();
		break;
	case 105:
		part24FloppyCloseDoor();
		break;
	case 106: // look at the plaque
		igorSay(202, 1, 0);
		break;
	case 107: // exit right
		_currentPart = 330;
		break;
	default:
		error("part24FloppyExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part24FloppyEnterChemistryClass() {
	if (_objectsState[kChemistryDoorStateIndex] == 0) {
		return;
	}
	_roomObjectAreasTable[_screenLayer2[kChemistryDoorCell]].area = 2;
	--_walkDataLastIndex;
	buildWalkPathSimple(179, 120, 181, 114);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 360;
}

void IgorEngine::part24FloppyOpenDoor() {
	if (_objectsState[kChemistryDoorStateIndex] == 1) {
		partMainExecuteAction(11);
		return;
	}
	for (int i = 1; i <= 3; ++i) {
		drawAnimRect(kChemistryDoorAnimOffset, kChemistryDoorFramesBase + i * kChemistryDoorFrameSize, 31, 63);
		if (i == 3) {
			playSound(13, 1);
		}
		waitForTimer(127);
	}
	_objectsState[kChemistryDoorStateIndex] = 1;
	part24FloppyApplyObjectState(1);
}

void IgorEngine::part24FloppyCloseDoor() {
	if (_objectsState[kChemistryDoorStateIndex] == 0) {
		partMainExecuteAction(14);
		return;
	}
	for (int i = 1; i <= 3; ++i) {
		drawAnimRect(kChemistryDoorAnimOffset, kChemistryDoorFramesBase + kCloseDoorFrames[i - 1] * kChemistryDoorFrameSize, 31, 63);
		if (i == 2) {
			playSound(14, 1);
		}
		waitForTimer(127);
	}
	_objectsState[kChemistryDoorStateIndex] = 0;
	part24FloppyApplyObjectState(1);
}

void IgorEngine::part24FloppyApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[kChemistryDoorStateIndex] == 0) {
			copyArea(_screenLayer1, kChemistryDoorPictureOffset, 320, _animFramesBuffer, 29, 29, 59);
			_roomActionsTable[30] = 6;
		} else {
			copyArea(_screenLayer1, kChemistryDoorPictureOffset, 320, _animFramesBuffer + kChemistryDoorClosedPictureSize, 29, 29, 59);
			_roomActionsTable[30] = 7;
		}
	}
}

void IgorEngine::part24FloppyEnterFromLeft() {
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

void IgorEngine::part24FloppyEnterFromRight() {
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

void IgorEngine::part24FloppyEnterFromChemistryClass() {
	_walkData[0].setPos(181, 114, 3, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[kChemistryDoorCell]].area = 2;
	buildWalkPathSimple(181, 114, 181, 125);
	_roomObjectAreasTable[_screenLayer2[kChemistryDoorCell]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part24Floppy() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeCorridorChemistryClass, IMG_CollegeCorridorChemistryClass, BOX_CollegeCorridorChemistryClass, MSK_CollegeCorridorChemistryClass, TXT_CollegeCorridorChemistryClass);
	static const int anm[] = { FRM_CollegeCorridorChemistryClassFloppy1, FRM_CollegeCorridorChemistryClassFloppy2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorChemistryClass);
	_roomDataOffsets = PART_24_FLOPPY_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	partMainSetExecuteAction(1, &IgorEngine::part24FloppyExecuteAction);
	part24FloppyApplyObjectState(255);
	copyLayerToScreen(_screenLayer1Surface);
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 240) {
			part24FloppyEnterFromLeft();
		} else if (_currentPart == 241) {
			part24FloppyEnterFromRight();
		} else if (_currentPart == 242) {
			part24FloppyEnterFromChemistryClass();
		}
	}
	enterPartLoop();
	while (_currentPart >= 240 && _currentPart <= 242 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
