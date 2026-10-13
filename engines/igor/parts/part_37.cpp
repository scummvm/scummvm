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


void IgorEngine::part37ExecuteAction(int action) {
	switch (action) {
	case 101: // exit
		_currentPart = 212;
		break;
	case 102:
		part37LookAtPaperBinTakePaper();
		break;
	case 103: // look at blackboard
		igorSay(203, 2, 664);
		break;
	case 104: // look at desk
		igorSay(205, 1, 665);
		break;
	case 105: // use blackboard
		igorSay(207, 1, 667);
		break;
	case 106: // use paper bin
		igorSay(206, 1, 666);
		break;
	default:
		error("part37ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part37LookAtPaperBinTakePaper() {
	if (_objectsState[kPhysicsClassPaperTakenStateIndex] == 1) {
		igorSay(202, 1, 663);
		return;
	}
	igorSayAndWait(201, 1, 662);
	for (int i = 1; i >= 0; --i) {
		int offset = 16423;
		drawAnimRect(offset, i * 1550, 31, 50);
		waitForTimer(60);
	}
	addObjectToInventory(kInventoryObjectLabNote, 67);
	_objectsState[kPhysicsClassPaperTakenStateIndex] = 1;
	part37ApplyObjectState(255);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
}

void IgorEngine::part37ApplyObjectState(int num) {
}

void IgorEngine::part37WalkIn() {
	_walkData[0].setPos(0, 143, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 16;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkCurrentFrame = 1;
	buildWalkPath(0, 143, 60, 143);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part37() {
	playMusic(2);
	_gameState.enableLight = 1;
	loadRoomData(PAL_PhysicsClassroom, IMG_PhysicsClassroom, BOX_PhysicsClassroom, MSK_PhysicsClassroom, TXT_PhysicsClassroom);
	static const int anm[] = { FRM_PhysicsClassroom1, 0 };
	loadAnimData(anm);
	loadActionData(DAT_PhysicsClassroom);
	_roomDataOffsets = PART_37_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 228, 143);
	partMainSetExecuteAction(1, &IgorEngine::part37ExecuteAction);
	part37ApplyObjectState(255);
	if (!restoreRoomAfterLoad()) {
		memcpy(_screenVGA, _screenLayer1, 46080);
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		part37WalkIn();
		enterPartLoop();
	}
	while (_currentPart == 370 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
