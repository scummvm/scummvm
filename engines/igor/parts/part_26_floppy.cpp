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

static const int kLabDoorCell = 128 * 320;
static const int kLabDoorPictureOffset = 21458;
static const int kLabDoorAnimOffset = 21459;
static const int kLabDoorPictureSize = 13 * 67;
static const int kLabDoorFrameSize = 27 * 67;

void IgorEngine::part26FloppyExecuteAction(int action) {
	switch (action) {
	case 101: // look at plaque
		igorSay(202, 1, 0);
		break;
	case 102: // look at door
		igorSay(203, 1, 0);
		break;
	case 103:
		part26FloppyOpenDoor();
		break;
	case 104:
		part26FloppyCloseDoor();
		break;
	case 105: // look at message board
		switch (getRandomNumber(2) + 1) {
		case 1:
			igorSay({ { 204, 2, 0 }, { 206, 1, 0 } });
			break;
		case 2:
			igorSay({ { 207, 1, 0 }, { 208, 1, 0 }, { 209, 1, 0 }, { 206, 1, 0 } });
			break;
		case 3:
			igorSay({ { 210, 1, 0 }, { 211, 1, 0 }, { 206, 1, 0 } });
			break;
		}
		break;
	case 106: // exit right
		_currentPart = 250;
		break;
	case 107:
		part26FloppyEnterLab();
		break;
	default:
		error("part26FloppyExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part26FloppyOpenDoor() {
	if (_objectsState[kLaboratoryDoorOpenStateIndex] == 1) {
		partMainExecuteAction(11);
		return;
	}
	for (int i = 2; i <= 3; ++i) {
		drawAnimRect(kLabDoorAnimOffset, i * kLabDoorFrameSize - 67, 27, 67);
		if (i == 3) {
			playSound(13, 1);
		}
		waitForTimer(127);
	}
	_objectsState[kLaboratoryDoorOpenStateIndex] = 1;
	part26FloppyApplyObjectState(1);
}

void IgorEngine::part26FloppyCloseDoor() {
	if (_objectsState[kLaboratoryDoorOpenStateIndex] == 0) {
		partMainExecuteAction(14);
		return;
	}
	for (int i = 2; i >= 1; --i) {
		drawAnimRect(kLabDoorAnimOffset, i * kLabDoorFrameSize - 67, 27, 67);
		if (i == 2) {
			playSound(14, 1);
		}
		waitForTimer(127);
	}
	_objectsState[kLaboratoryDoorOpenStateIndex] = 0;
	part26FloppyApplyObjectState(1);
}

void IgorEngine::part26FloppyEnterLab() {
	if (_objectsState[kLaboratoryDoorOpenStateIndex] == 0) {
		return;
	}
	_roomObjectAreasTable[_screenLayer2[kLabDoorCell]].area = 1;
	--_walkDataLastIndex;
	buildWalkPathSimple(37, 131, 0, 128);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 160;
}

void IgorEngine::part26FloppyApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[kLaboratoryDoorOpenStateIndex] == 0) {
			copyArea(_screenLayer1, kLabDoorPictureOffset, 320, _animFramesBuffer, 13, 13, 67);
			_roomActionsTable[2] = 6;
		} else {
			copyArea(_screenLayer1, kLabDoorPictureOffset, 320, _animFramesBuffer + kLabDoorPictureSize, 13, 13, 67);
			_roomActionsTable[2] = 7;
		}
	}
}

void IgorEngine::part26FloppyEnterFromRight() {
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

void IgorEngine::part26FloppyEnterFromLaboratory() {
	_walkData[0].setPos(0, 128, 3, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[kLabDoorCell]].area = 1;
	buildWalkPathSimple(0, 128, 45, 132);
	_roomObjectAreasTable[_screenLayer2[kLabDoorCell]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	if (_objectsState[kMargaretStateIndex] == kMargaretStatePhotoTaken) {
		igorSay({ { 212, 1, 0 }, { 213, 2, 0 }, { 215, 1, 0 } });
		_objectsState[kMargaretStateIndex] = kMargaretStatePhotoSequenceComplete;
		drawVerbsPanel();
		redrawVerb(kVerbWalk, true);
		_currentAction.verb = kVerbWalk;
		drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
		part26FloppyApplyObjectState(255);
	}
}

void IgorEngine::part26Floppy() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeCorridorMissBarrymore, IMG_CollegeCorridorMissBarrymore, BOX_CollegeCorridorMissBarrymore, MSK_CollegeCorridorMissBarrymore, TXT_CollegeCorridorMissBarrymore);
	static const int anm[] = { FRM_CollegeCorridorMissBarrymoreFloppy1, FRM_CollegeCorridorMissBarrymoreFloppy2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorMissBarrymore);
	_roomDataOffsets = PART_26_FLOPPY_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(24, 0, 319, 143);
	partMainSetExecuteAction(1, &IgorEngine::part26FloppyExecuteAction);
	part26FloppyApplyObjectState(255);
	copyLayerToScreen(_screenLayer1Surface);
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 260) {
			part26FloppyEnterFromRight();
		} else {
			part26FloppyEnterFromLaboratory();
		}
	}
	enterPartLoop();
	while (_currentPart >= 260 && _currentPart <= 261 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
