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

const int kDoorFramesBase = 0xFE5E;

const int kDoorAnimOffset = 16754;
const int kOpenDoorOffset = 16758;

const uint8 kOpenDoorFrames[7] = { 2, 3, 2, 3, 2, 3, 1 };
const uint8 kKeysDoorFrames[10] = { 2, 3, 2, 3, 2, 3, 2, 3, 4, 5 };

const int kAstronomyLabCell = 75 * 320 + 135;
const int kDownstairsCell = 107 * 320 + 190;

void IgorEngine::part31FloppyExecuteAction(int action) {
	switch (action) {
	case 101:
		_currentPart = 251;
		break;
	case 102:
		part31FloppyEnterAstronomyLab();
		break;
	case 103:
		part31FloppyOpenDoor();
		break;
	case 104: // close door
		if (_objectsState[kAstronomyLabDoorStateIndex] == 0) {
			partMainExecuteAction(14);
		} else {
			igorSay(205, 1, 0);
		}
		break;
	case 105: // look at door
		igorSay(201, 1, 0);
		break;
	case 106:
		part31FloppyGoDownstairs();
		break;
	case 107: // look at downstairs
		igorSay(203, 1, 0);
		break;
	case 108:
		_currentPart = 200;
		break;
	case 109: // look at plaque
		igorSay(204, 1, 0);
		break;
	case 110:
		part31FloppyUseKeysWithDoor();
		break;
	default:
		error("part31FloppyExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part31FloppyEnterAstronomyLab() {
	if (_objectsState[kAstronomyLabDoorStateIndex] == 0) {
		return;
	}
	_roomObjectAreasTable[_screenLayer2[kAstronomyLabCell]].area = 2;
	--_walkDataLastIndex;
	buildWalkPath(127, 113, 135, 75);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 320;
}

void IgorEngine::part31FloppyDrawDoorFrame(int frame) {
	drawAnimRect(kDoorAnimOffset, (frame * 0x7C0 + kDoorFramesBase) & 0xFFFF, 32, 62);
}

void IgorEngine::part31FloppyOpenDoor() {
	if (_objectsState[kAstronomyLabDoorStateIndex] == 1) {
		partMainExecuteAction(11);
		return;
	}
	for (int i = 1; i <= 7; ++i) {
		part31FloppyDrawDoorFrame(kOpenDoorFrames[i - 1]);
		if (i < 7) {
			waitForTimer(31);
		}
	}
	igorSay(202, 1, 0);
}

void IgorEngine::part31FloppyGoDownstairs() {
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[kDownstairsCell]].area = 2;
	buildWalkPath(190, 113, 190, 107);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	for (int i = 9; i >= 0; --i) {
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		WalkData *wd = &_walkData[0];
		wd->setPos(190, 107, 1, _walkCurrentFrame);
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

void IgorEngine::part31FloppyUseKeysWithDoor() {
	if (_objectsState[kAstronomyLabDoorStateIndex] == 1) {
		partMainExecuteAction(11);
		return;
	}
	for (int i = 1; i <= 10; ++i) {
		part31FloppyDrawDoorFrame(kKeysDoorFrames[i - 1]);
		if (i == 10) {
			playSound(13, 1);
		} else {
			waitForTimer(31);
		}
	}
	_objectsState[kAstronomyLabDoorStateIndex] = 1;
	part31FloppyApplyObjectState(1);
}

void IgorEngine::part31FloppyApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[kAstronomyLabDoorStateIndex] == 0) {
			_roomActionsTable[30] = 6;
		} else {
			copyArea(_screenLayer1, kOpenDoorOffset, 320, _animFramesBuffer, 27, 27, 58);
			_roomActionsTable[30] = 7;
		}
	}
}

void IgorEngine::part31FloppyEnterFromLeft() {
	_walkData[0].setPos(0, 134, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 15;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPath(0, 134, 30, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part31FloppyEnterFromAstronomyLab() {
	_walkData[0].setPos(135, 75, 3, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[kAstronomyLabCell]].area = 2;
	buildWalkPath(135, 75, 127, 118);
	_roomObjectAreasTable[_screenLayer2[kAstronomyLabCell]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part31FloppyEnterFromDownstairs() {
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	_walkCurrentPos = 3;
	for (int i = 0; i <= 9; ++i) {
		WalkData *wd = &_walkData[0];
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		wd->setPos(190, 107, 3, _walkCurrentFrame);
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
	_roomObjectAreasTable[_screenLayer2[kDownstairsCell]].area = 2;
	--_walkDataLastIndex;
	buildWalkPath(190, 107, 190, 125);
	_roomObjectAreasTable[_screenLayer2[kDownstairsCell]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part31FloppyEnterFromRight() {
	_walkData[0].setPos(319, 134, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPath(319, 134, 289, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part31Floppy() {
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeStairsSecondFloor, IMG_CollegeStairsSecondFloor, BOX_CollegeStairsSecondFloor, MSK_CollegeStairsSecondFloor, TXT_CollegeStairsSecondFloor);
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	static const int anm[] = { FRM_CollegeStairsSecondFloorFloppy1, FRM_CollegeStairsSecondFloorFloppy2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeStairsSecondFloor);
	_roomDataOffsets = PART_31_FLOPPY_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 248, 143);
	partMainSetExecuteAction(1, &IgorEngine::part31FloppyExecuteAction);
	part31FloppyApplyObjectState(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		_gameState.unk10 = 1;
		fadeIn(768);
		if (_currentPart == 310) {
			part31FloppyEnterFromLeft();
		} else if (_currentPart == 311) {
			part31FloppyEnterFromAstronomyLab();
		} else if (_currentPart == 312) {
			part31FloppyEnterFromDownstairs();
		} else if (_currentPart == 313) {
			part31FloppyEnterFromRight();
		}
		enterPartLoop();
	}
	while (_currentPart >= 310 && _currentPart <= 313 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
