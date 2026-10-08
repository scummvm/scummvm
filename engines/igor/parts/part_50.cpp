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

// The palette of the room is darker by this amount.
static const int kPart50PaletteDarkness = 5;

// Area given temporarily to the place where Igor appears next to the door, so that the path to or from it can be built.
static const int kPart50DoorArea = 7;
static const int kPart50DoorX = 264;
static const int kPart50DoorY = 89;

void IgorEngine::PART_50_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		PART_50_ACTION_101_enterMaze();
		break;
	case 102:
		ADD_DIALOGUE_TEXT(201, 1, 1083);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 103:
		_objectsState[90] = 0;
		_currentPart = 700;
		break;
	default:
		error("PART_50_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

/**
 * Igor walks to the door and goes into the maze, through its last room.
 */
void IgorEngine::PART_50_ACTION_101_enterMaze() {
	_roomObjectAreasTable[_screenLayer2[kPart50DoorY * 320 + kPart50DoorX]].area = kPart50DoorArea;
	--_walkDataLastIndex;
	buildWalkPath(222, 102, kPart50DoorX, kPart50DoorY);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 663;
	_objectsState[71] = 1;
}

/**
 * Igor comes out of the door, on the right, and walks to the left.
 */
void IgorEngine::PART_50_ENTER_FROM_DOOR() {
	WalkData *wd = &_walkData[0];
	wd->setPos(kPart50DoorX, kPart50DoorY, kFacingPositionLeft, 0);
	wd->setDefaultScale();
	_walkDataLastIndex = 0;
	const int area = _screenLayer2[kPart50DoorY * 320 + kPart50DoorX];
	_roomObjectAreasTable[area].area = kPart50DoorArea;
	buildWalkPath(kPart50DoorX, kPart50DoorY, 222, 102);
	_roomObjectAreasTable[area].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

/**
 * Igor comes from the far top left, tiny, and walks down the hill to the right.
 */
void IgorEngine::PART_50_ENTER_FROM_HILL() {
	WalkData *wd = &_walkData[0];
	wd->setPos(45, 22, kFacingPositionFront, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 5;
	wd->scaleWidth = 9;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 9;
	_walkDataLastIndex = 0;
	buildWalkPath(45, 22, 84, 111);
	_walkData[_walkDataLastIndex].posNum = kFacingPositionRight;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

/**
 * Something is heard from time to time.
 */
void IgorEngine::PART_50_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(0x3D) && getRandomNumber(49) == 0) {
		playSound(44, 1);
	}
}

void IgorEngine::PART_50() {
	playMusic(4);
	_gameState.enableLight = 2;
	loadRoomData(PAL_OutsideMaze, IMG_OutsideMaze, BOX_OutsideMaze, MSK_OutsideMaze, TXT_OutsideMaze);
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	SET_PAL_240_48_1();
	loadActionData(DAT_OutsideMaze);
	_roomDataOffsets = PART_50_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(49, 22, 224, 119);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_50_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_50_UPDATE_ROOM_BACKGROUND;
	memcpy(_screenVGA, _screenLayer1, 46080);

	// the whole room is darker than its palette
	for (int i = 3; i <= 207 * 3 + 2; ++i) {
		const int value = _paletteBuffer[i] - kPart50PaletteDarkness;
		_paletteBuffer[i] = value < 1 ? 0 : value;
	}

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 500) {
			PART_50_ENTER_FROM_DOOR();
		} else {
			PART_50_ENTER_FROM_HILL();
		}
	}
	enterPartLoop();
	while ((_currentPart == 500 || _currentPart == 501) && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
