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

void IgorEngine::part50ExecuteAction(int action) {
	switch (action) {
	case 101:
		part50EnterMaze();
		break;
	case 102: // look at maze door
		igorSay(201, 1, 1083);
		break;
	case 103:
		_objectsState[kCemeteryEntrySideStateIndex] = 0;
		_currentPart = 700;
		break;
	default:
		error("part50ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part50EnterMaze() {
	_roomObjectAreasTable[_screenLayer2[kPart50DoorY * 320 + kPart50DoorX]].area = kPart50DoorArea;
	--_walkDataLastIndex;
	buildWalkPath(222, 102, kPart50DoorX, kPart50DoorY);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 663;
	_objectsState[kMazeEntryDirectionStateIndex] = 1;
}

void IgorEngine::part50EnterFromDoor() {
	if (!isCDVersion())
		playMusic(7, true);
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
	if (_game.version == kIdSpaFloppy)
		playSound(kSpaFloppySoundPart50Door, 1);
}

void IgorEngine::part50EnterFromHill() {
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
void IgorEngine::part50UpdateBackground() {
	if (compareGameTick(0x3D) && getRandomNumber(49) == 0) {
		playRoomSound(kSoundPart50Ambient, kSpaFloppySoundPart50Door);
	}
}

void IgorEngine::part50() {
	if (isCDVersion())
		playMusic(4);
	_gameState.enableLight = 2;
	loadRoomData(PAL_OutsideMaze, IMG_OutsideMaze, BOX_OutsideMaze, MSK_OutsideMaze, TXT_OutsideMaze);
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	setVerbPanelColors();
	loadActionData(DAT_OutsideMaze);
	_roomDataOffsets = PART_50_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(49, 22, 224, 119);
	partMainSetExecuteAction(1, &IgorEngine::part50ExecuteAction);
	_updateRoomBackground = &IgorEngine::part50UpdateBackground;
	copyLayerToScreen(_screenLayer1Surface);

	// the whole room is darker than its palette
	darkenPalette(_paletteBuffer, 1, 207, kPart50PaletteDarkness);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 500) {
			part50EnterFromDoor();
		} else {
			part50EnterFromHill();
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
