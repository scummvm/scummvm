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

static const int kLockerAnimOffset = 19715;
static const int kLockerFrameSize = 34 * 63;
static const int kLockerPictureOffset = 19397;
static const int kLockerClosedPictureOffset = 8568;
static const int kLockerOpenPictureOffset = 8568 + 32 * 56;

void IgorEngine::part27FloppyExecuteAction(int action) {
	switch (action) {
	case 101: // exit left
		_currentPart = 211;
		break;
	case 102: // look at the lockers
		if (_objectsState[kLockersExaminedStateIndex] == 0) {
			igorSay(216, 2, 0);
			_objectsState[kLockersExaminedStateIndex] = 1;
			part27FloppyApplyObjectState(255);
		} else {
			igorSay(201, 1, 0);
		}
		break;
	case 103: // open lockers
		igorSay(203, 1, 0);
		break;
	case 104: // close lockers
		igorSay(204, 1, 0);
		break;
	case 105: // look at Philip's locker
		igorSay(205, 1, 0);
		break;
	case 106:
		part27FloppyOpenPhilipLocker();
		break;
	case 107:
		part27FloppyClosePhilipLocker();
		break;
	case 108:
		part27FloppyTakeBottle();
		break;
	case 109: // look at the bottles
		igorSay(215, 1, 0);
		break;
	case 110:
		part27FloppyUseFlaskOnLocker();
		break;
	default:
		error("part27FloppyExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part27FloppyDrawLockerFrame(int frame) {
	drawAnimRect(kLockerAnimOffset, frame * kLockerFrameSize, 34, 63);
}

void IgorEngine::part27FloppyOpenPhilipLocker() {
	if (_objectsState[kPhillipLockerStateIndex] == 1) {
		partMainExecuteAction(11);
		return;
	}
	if (_objectsState[kPhillipLockerCombinationLearnedStateIndex] == 0) {
		igorSay(206, 1, 0);
		return;
	}
	igorSayAndWait(209, 3, 0);
	for (int i = 1; i <= 3; ++i) {
		part27FloppyDrawLockerFrame(i - 1);
		if (i == 3) {
			playSound(13, 1);
		}
		if (i < 3) {
			waitForTimer(91);
		}
	}
	_objectsState[kPhillipLockerStateIndex] = 1;
	part27FloppyApplyObjectState(1);
}

void IgorEngine::part27FloppyClosePhilipLocker() {
	if (_objectsState[kPhillipLockerStateIndex] == 0) {
		partMainExecuteAction(14);
		return;
	}
	for (int i = 3; i >= 1; --i) {
		part27FloppyDrawLockerFrame(i - 1);
		if (i == 3) {
			playSound(14, 1);
		}
		if (i < 3) {
			waitForTimer(91);
		}
	}
	_objectsState[kPhillipLockerStateIndex] = 0;
	part27FloppyApplyObjectState(1);
}

void IgorEngine::part27FloppyTakeBottle() {
	if (_inventoryInfo[58] > 0 || _objectsState[kButterflyNetStateIndex] == 2) {
		igorSay(208, 1, 0);
		return;
	}
	igorSayAndWait(207, 1, 0);
	for (int i = 4; i >= 1; --i) {
		part27FloppyDrawLockerFrame(i - 1);
		if (i > 3) {
			waitForTimer(91);
		}
	}
	addObjectToInventory(kInventoryObjectBottle, 58);
	part27FloppyApplyObjectState(255);
}

void IgorEngine::part27FloppyUseFlaskOnLocker() {
	igorSayAndWait({ { 212, 1, 0 }, { 213, 2, 0 } });
	for (int i = 4; i >= 1; --i) {
		part27FloppyDrawLockerFrame(i - 1);
		if (i > 3) {
			waitForTimer(91);
		}
	}
	removeObjectFromInventory(55);
	part27FloppyApplyObjectState(255);
	_objectsState[kFlaskLeftInPhillipLockerStateIndex] = 1;
}

void IgorEngine::part27FloppyApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[kPhillipLockerStateIndex] == 0) {
			copyArea(_screenLayer1, kLockerPictureOffset, 320, _animFramesBuffer + kLockerClosedPictureOffset, 32, 32, 56);
			_roomActionsTable[3] = 6;
			_roomObjectAreasTable[4].object = 3;
		} else {
			copyArea(_screenLayer1, kLockerPictureOffset, 320, _animFramesBuffer + kLockerOpenPictureOffset, 32, 32, 56);
			_roomActionsTable[3] = 7;
			_roomObjectAreasTable[4].object = 4;
		}
	}
	if (num == 2 || num == 255) {
		if (_objectsState[kLockersExaminedStateIndex] == 0) {
			_roomObjectAreasTable[3].object = 2;
			_roomObjectAreasTable[4].object = 2;
		} else {
			_roomObjectAreasTable[3].object = 3;
		}
	}
}

void IgorEngine::part27FloppyEnterFromLeft() {
	_walkData[0].setPos(0, 134, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPathSimple(0, 134, 30, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part27Floppy() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeLockers, IMG_CollegeLockers, BOX_CollegeLockers, MSK_CollegeLockers, TXT_CollegeLockers);
	static const int anm[] = { FRM_CollegeLockersFloppy1, FRM_CollegeLockersFloppy2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeLockers);
	_roomDataOffsets = PART_27_FLOPPY_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 296, 143);
	partMainSetExecuteAction(1, &IgorEngine::part27FloppyExecuteAction);
	part27FloppyApplyObjectState(255);
	copyLayerToScreen(_screenLayer1Surface);
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		part27FloppyEnterFromLeft();
	}
	enterPartLoop();
	while (_currentPart == 270 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
