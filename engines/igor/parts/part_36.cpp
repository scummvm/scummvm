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


void IgorEngine::part36ExecuteAction(int action) {
	switch (action) {
	case 101: // look at desk
		igorSay(201, 1, 655);
		break;
	case 102:
		part36TakePen();
		break;
	case 103: // look at pen
		igorSay(202, 1, 656);
		break;
	case 104: // look at portrait
		igorSay(203, 1, 657);
		break;
	case 105: // look at blackboard
		igorSay(204, 2, 658);
		break;
	case 106: // use blackboard
		igorSay(209, 1, 661);
		break;
	case 107: // look at closet
		igorSay(206, 2, 659);
		break;
	case 108: // open closet
		igorSay(208, 1, 660);
		break;
	case 109:
		_currentPart = 242;
		break;
	default:
		error("part36ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part36TakePen() {
	for (int i = 1; i >= 0; --i) {
		drawAnimRect(23750, 14 + i * 1666, 34, 49, false, kBlendLitSprite);
		waitForTimer(45);
	}
	addObjectToInventory(kInventoryObjectPen, 65);
	_objectsState[kPenTakenStateIndex] = 1;
	part36ApplyObjectState(255);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
}

void IgorEngine::part36ApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[kPenTakenStateIndex] == 0) {
			part36DrawPen(1);
		} else {
			part36DrawPen(0);
			_roomObjectAreasTable[6].object = 1;
			_roomObjectAreasTable[7].object = 1;
		}
	}
}

void IgorEngine::part36WalkIn() {
	_walkData[0].setPos(319, 143, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkCurrentFrame = 1;
	buildWalkPath(319, 143, 259, 143);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part36DrawPen(int frame) {
	if (frame == 0) {
		memcpy(_screenLayer1 + 31430, _animFramesBuffer, 7);
	} else if (frame == 1) {
		memcpy(_screenLayer1 + 31430, _animFramesBuffer + 7, 7);
	}
}

void IgorEngine::part36() {
	if (isCDVersion())
		playMusic(2);
	_gameState.enableLight = 1;
	loadRoomData(PAL_ChemistryClassroom, IMG_ChemistryClassroom, BOX_ChemistryClassroom, MSK_ChemistryClassroom, TXT_ChemistryClassroom);
	static const int anm[] = { FRM_ChemistryClassroom1, FRM_ChemistryClassroom2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_ChemistryClassroom);
	_roomDataOffsets = PART_36_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(90, 0, 319, 143);
	partMainSetExecuteAction(1, &IgorEngine::part36ExecuteAction);
	part36ApplyObjectState(255);
	if (!restoreRoomAfterLoad()) {
		copyLayerToScreen(_screenLayer1Surface);
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		part36WalkIn();
		enterPartLoop();
	}
	while (_currentPart == 360 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}


} // End of namespace Igor
