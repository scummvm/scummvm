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

static const int kLadiesRoomDoorCell = 114 * 320 + 181;
static const int kLadiesRoomDoorPictureOffset = 18730;
static const int kLadiesRoomDoorAnimOffset = 18727;
static const int kLadiesRoomDoorFramesBase = 0x5B8;
static const int kLadiesRoomDoorFrameSize = 0x7E0;
static const int kLadiesRoomDoorClosedPictureSize = 29 * 60;

void IgorEngine::part23FloppyExecuteAction(int action) {
	switch (action) {
	case 101: // exit left
		_currentPart = 280;
		break;
	case 102: // look at the portrait
		igorSay(201, 2, 0);
		break;
	case 103:
		igorSay(203, 1, 0);
		break;
	case 104:
		igorSay(204, 1, 0);
		break;
	case 105:
		part23FloppyEnterLadiesRoom();
		break;
	case 106: // look at the door
		igorSay(205, 1, 0);
		break;
	case 107:
		part23FloppyOpenDoor();
		break;
	case 108:
		part23FloppyCloseDoor();
		break;
	case 109: // exit right
		_currentPart = 300;
		break;
	default:
		error("part23FloppyExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part23FloppyEnterLadiesRoom() {
	if (_objectsState[kLadiesRoomDoorStateIndex] == 0) {
		return;
	}
	_roomObjectAreasTable[_screenLayer2[kLadiesRoomDoorCell]].area = 2;
	--_walkDataLastIndex;
	buildWalkPathSimple(179, 120, 181, 114);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 190;
}

void IgorEngine::part23FloppyOpenDoor() {
	if (_objectsState[kLadiesRoomDoorStateIndex] == 1) {
		partMainExecuteAction(11);
		return;
	}
	for (int i = 2; i <= 3; ++i) {
		drawAnimRect(kLadiesRoomDoorAnimOffset, kLadiesRoomDoorFramesBase + i * kLadiesRoomDoorFrameSize, 32, 63);
		if (i == 3) {
			playSound(13, 1);
		}
		waitForTimer(127);
	}
	_objectsState[kLadiesRoomDoorStateIndex] = 1;
	part23FloppyApplyObjectState(1);
}

void IgorEngine::part23FloppyCloseDoor() {
	if (_objectsState[kLadiesRoomDoorStateIndex] == 0) {
		partMainExecuteAction(14);
		return;
	}
	for (int i = 2; i >= 1; --i) {
		drawAnimRect(kLadiesRoomDoorAnimOffset, kLadiesRoomDoorFramesBase + i * kLadiesRoomDoorFrameSize, 32, 63);
		if (i == 2) {
			playSound(14, 1);
		}
		waitForTimer(127);
	}
	_objectsState[kLadiesRoomDoorStateIndex] = 0;
	part23FloppyApplyObjectState(1);
}

void IgorEngine::part23FloppyApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[kLadiesRoomDoorStateIndex] == 0) {
			copyArea(_screenLayer1, kLadiesRoomDoorPictureOffset, 320, _animFramesBuffer, 29, 29, 60);
			_roomActionsTable[31] = 6;
		} else {
			copyArea(_screenLayer1, kLadiesRoomDoorPictureOffset, 320, _animFramesBuffer + kLadiesRoomDoorClosedPictureSize, 29, 29, 60);
			_roomActionsTable[31] = 7;
		}
	}
}

void IgorEngine::part23FloppyEnterFromLeft() {
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

void IgorEngine::part23FloppyEnterFromRight() {
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

void IgorEngine::part23FloppyEnterFromLadiesRoom() {
	_walkData[0].setPos(181, 114, 3, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[kLadiesRoomDoorCell]].area = 2;
	buildWalkPathSimple(181, 114, 181, 125);
	_roomObjectAreasTable[_screenLayer2[kLadiesRoomDoorCell]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	if (_objectsState[kToiletGirlsHaveLeftStateIndex] == 0) {
		// The door is closed behind Igor while he is still in front of it
		drawAnimRect(kLadiesRoomDoorPictureOffset, 0, 29, 60, false, kBlendBehindIgor);
		playSound(14, 1);
		_objectsState[kLadiesRoomDoorStateIndex] = 0;
		part23FloppyApplyObjectState(1);
		igorSay(206, 2, 0);
	}
}

void IgorEngine::part23Floppy() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeCorridorLadiesRoom, IMG_CollegeCorridorLadiesRoom, BOX_CollegeCorridorLadiesRoom, MSK_CollegeCorridorLadiesRoom, TXT_CollegeCorridorLadiesRoom);
	static const int anm[] = { FRM_CollegeCorridorLadiesRoomFloppy1, FRM_CollegeCorridorLadiesRoomFloppy2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorLadiesRoom);
	_roomDataOffsets = PART_23_FLOPPY_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	partMainSetExecuteAction(1, &IgorEngine::part23FloppyExecuteAction);
	part23FloppyApplyObjectState(255);
	copyLayerToScreen(_screenLayer1Surface);
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 230) {
			part23FloppyEnterFromLeft();
		} else if (_currentPart == 231) {
			part23FloppyEnterFromRight();
		} else if (_currentPart == 232) {
			part23FloppyEnterFromLadiesRoom();
		}
	}
	enterPartLoop();
	while (_currentPart >= 230 && _currentPart <= 232 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
