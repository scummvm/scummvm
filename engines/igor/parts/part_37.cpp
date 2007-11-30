/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "igor/igor.h"

namespace Igor {

void IgorEngine::PART_37_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_currentPart = 212;
		break;
	case 102:
		PART_37_ACTION_102();
		break;
	case 103:
		ADD_DIALOGUE_TEXT(203, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(205, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		ADD_DIALOGUE_TEXT(207, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106:
		ADD_DIALOGUE_TEXT(206, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	default:
		error("PART_37_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_37_ACTION_102() {
	if (_objectsState[89] == 1) {
		ADD_DIALOGUE_TEXT(202, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		return;
	}
	ADD_DIALOGUE_TEXT(201, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	for (int i = 1; i >= 0; --i) {
		int offset = 16423;
		for (int j = 0; j <= 49; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + i * 1550 + j * 31, 31);
		}
		waitForTimer(60);
	}
	addObjectToInventory(32, 67);
	_objectsState[89] = 1;
	PART_37_HELPER_1(255);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
}

void IgorEngine::PART_37_HELPER_1(int num) {
}

void IgorEngine::PART_37_HELPER_2() {
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

void IgorEngine::PART_37() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_PhysicsClassroom, IMG_PhysicsClassroom, BOX_PhysicsClassroom, MSK_PhysicsClassroom, TXT_PhysicsClassroom);
	static const int anm[] = { FRM_PhysicsClassroom1, 0 };
	loadAnimData(anm);
	loadActionData(DAT_PhysicsClassroom);
	_roomDataOffsets = PART_37_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 228, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_37_EXEC_ACTION);
	PART_37_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeInPalette(768);
	PART_37_HELPER_2();
	enterPartLoop();
	while (_currentPart == 370) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

} // namespace Igor
