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

void IgorEngine::part20FloppyExecuteAction(int action) {
	switch (action) {
	case 101: // exit right
		_currentPart = 313;
		break;
	case 102: // look at the corridor
	case 103: // look at the door
		igorSay(201, 1, 0);
		break;
	case 104: // look at the plaque
		igorSay({ { 202, 1, 0 }, { 203, 1, 0 } });
		break;
	case 105: // look at the notice board
		igorSay({ { 204, 1, 0 }, { 205, 2, 0 }, { 207, 2, 0 } });
		break;
	case 106: // exit left
		_currentPart = 240;
		break;
	default:
		error("part20FloppyExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part20FloppyEnterFromLeft() {
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

void IgorEngine::part20FloppyEnterFromRight() {
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

void IgorEngine::part20Floppy() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeCorridorMathClass, IMG_CollegeCorridorMathClass, BOX_CollegeCorridorMathClass, MSK_CollegeCorridorMathClass, TXT_CollegeCorridorMathClass);
	loadActionData(DAT_CollegeCorridorMathClass);
	_roomDataOffsets = PART_20_FLOPPY_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	partMainSetExecuteAction(1, &IgorEngine::part20FloppyExecuteAction);
	memcpy(_screenVGA, _screenLayer1, 46080);
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 200) {
			part20FloppyEnterFromLeft();
		} else {
			part20FloppyEnterFromRight();
		}
	}
	enterPartLoop();
	while ((_currentPart == 200 || _currentPart == 201) && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
