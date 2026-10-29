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


void IgorEngine::part27ExecuteAction(int action) {
	switch (action) {
	case 101: // go to right
		_currentPart = 211;
		break;
	case 102: // look at lockers
		if (_objectsState[kLockersExaminedStateIndex] == 0) {
			igorSay(216, 2, 1173);
			_objectsState[kLockersExaminedStateIndex] = 1;
			part27ApplyObjectState(255);
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
	case 106: // open Philip's locker
		part27OpenPhilipLocker();
		break;
	case 107:
		part27CloseLocker();
		break;
	case 108:
		part27OpenLocker();
		break;
	case 109: // look at bottles
		igorSay(215, 1, 1172);
		break;
	case 110:
		part27UseFlaskOnLocker();
		break;
	case 111: // look at messageboard
		igorSay(218, 2, 1174);
		break;
	case 112: // look at plaque
		igorSay(221, 1, 1176);
		break;
	case 113: // go to library
		_currentPart = 150;
		break;
	default:
		error("part27ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part27OpenPhilipLocker() {
	if (_objectsState[kPhillipLockerStateIndex] == 1) {
		partMainExecuteAction(11);
		return;
	}
	if (_objectsState[kPhillipLockerCombinationLearnedStateIndex] == 0) {
		igorSay(206, 1, 1166);
		return;
	}
	if (_objectsState[kPhillipLockerCombinationLearnedStateIndex] == 1) {
		igorSayAndWait(209, 3, 1169);
		_objectsState[kPhillipLockerCombinationLearnedStateIndex] = 2;
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
	_objectsState[kPhillipLockerStateIndex] = 1;
	part27ApplyObjectState(1);
}

void IgorEngine::part27CloseLocker() {
	if (_objectsState[kPhillipLockerStateIndex] == 0) {
		partMainExecuteAction(14);
		return;
	}
	const int offset = 21810;
	for (int i = 2; i >= 1; --i) {
		drawAnimRect(offset, 0x3A0 + i * 1568, 32, 49);
		if (i == 2) {
			playSound(4, 1);
			waitForTimer(100);
		}
	}
	_objectsState[kPhillipLockerStateIndex] = 0;
	part27ApplyObjectState(1);
}

void IgorEngine::part27OpenLocker() {
	if (_inventoryInfo[58] > 0 || _objectsState[kButterflyNetStateIndex] == 2) {
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
	addObjectToInventory(kInventoryObjectBottle, 58);
	part27ApplyObjectState(1);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
}

void IgorEngine::part27UseFlaskOnLocker() {
	igorSayAndWait({ { 212, 1, 1170 }, { 213, 2, 1171 } });
	const int offset = 25012;
	for (int i = 1; i <= 2; ++i) {
		drawAnimRect(offset, 0x19AA + i * 630, 21, 30);
		if (i == 1) {
			waitForTimer(100);
		}
	}
	removeObjectFromInventory(55);
	part27ApplyObjectState(255);
	_objectsState[kFlaskLeftInPhillipLockerStateIndex] = 1;
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
}

void IgorEngine::part27ApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[kPhillipLockerStateIndex] == 0) {
			part27DrawClosedLocker();
			_roomActionsTable[3] = 6;
			_roomObjectAreasTable[6].object = 3;
		} else {
			part27DrawOpenLocker();
			_roomActionsTable[3] = 7;
			_roomObjectAreasTable[6].object = 4;
		}
	}
	if (num == 2 || num == 255) {
		if (_objectsState[kLockersExaminedStateIndex] == 0) {
			_roomObjectAreasTable[5].object = 2;
			_roomObjectAreasTable[6].object = 2;
		} else {
			_roomObjectAreasTable[5].object = 3;
		}
	}
}

void IgorEngine::part27EnterFromLeft() {
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

void IgorEngine::part27DrawClosedLocker() {
	const int offset = 21816;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer, 26, 26, 48);
}

void IgorEngine::part27DrawOpenLocker() {
	const int offset = 21816;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + 0x4E0, 26, 26, 48);
}

void IgorEngine::part27EnterFromLibrary() {
	_walkData[0].setPos(270, 134, 4, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(270, 134, 140, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part27() {
	if (isCDVersion())
		playMusic(2);
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeLockers, IMG_CollegeLockers, BOX_CollegeLockers, MSK_CollegeLockers, TXT_CollegeLockers);
	static const int anm[] = { FRM_CollegeLockers1, FRM_CollegeLockers2, FRM_CollegeLockers3, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeLockers);
	_roomDataOffsets = PART_27_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	partMainSetExecuteAction(1, &IgorEngine::part27ExecuteAction);
	part27ApplyObjectState(255);
	if (!restoreRoomAfterLoad()) {
		copyLayerToScreen(_screenLayer1Surface);
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 270) {
			part27EnterFromLeft();
		} else {
			part27EnterFromLibrary();
		}
		enterPartLoop();
	}
	while ((_currentPart == 270 || _currentPart == 271) && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
