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

// FRM_BellChurch1 is a full 128 rows picture (the scene Igor comes from / goes back to) followed
// by FRM_BellChurch2, the two 35x30 frames of the pick up animation.
static const int kPart22PictureRows = 128;
static const int kPart22PickUpFrames = 40960;
static const int kPart22PickUpFrameSize = 35 * 30;

static const uint8 PART_22_PICK_UP_FRAMES[3] = { 1, 2, 1 };

void IgorEngine::PART_22_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		PART_22_ACTION_101();
		break;
	case 102:
		PART_22_ACTION_102();
		break;
	default:
		error("PART_22_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

/**
 * Takes the object (inventory object 29) from the bell tower.
 */
void IgorEngine::PART_22_ACTION_101() {
	if (_objectsState[78] == 1 && _inventoryInfo[64] == 0) {
		ADD_DIALOGUE_TEXT(203, 2, 855);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		for (int i = 0; i < 3; ++i) {
			// each frame is drawn as is, over Igor too
			const uint8 *frame = _animFramesBuffer + kPart22PickUpFrames +
					(PART_22_PICK_UP_FRAMES[i] - 1) * kPart22PickUpFrameSize;
			for (int y = 0; y <= 29; ++y) {
				memcpy(_screenVGA + y * 320 + 24141, frame + y * 35, 35);
			}
			waitForTimer(60);
		}
		addObjectToInventory(29, 64);
		PART_22_APPLY_OBJECT_STATE(255);
		ADD_DIALOGUE_TEXT(205, 1, 856);
		ADD_DIALOGUE_TEXT(206, 1, 857);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		_objectsState[105] = 1;
	} else {
		ADD_DIALOGUE_TEXT(201, 2, 854);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
	}
}

/**
 * Igor leaves the tower: he walks away from the viewer, shrinking, and the picture scrolls up to
 * the church.
 */
void IgorEngine::PART_22_ACTION_102() {
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	for (int i = 9; i >= 0; --i) {
		PART_22_IGOR_STEP(kFacingPositionBack, i);
		waitForTimer(15);
	}
	for (int i = 16; i >= 0;) {
		if (compareGameTick(1, 16)) {
			PART_22_SCROLL_STEP(i);
			--i;
		}
		waitForTimer();
	}
	_currentPart = 141;
}

/**
 * Nothing to redraw for this room: the object state has no visual effect.
 */
void IgorEngine::PART_22_APPLY_OBJECT_STATE(int num) {
}

/**
 * One step of Igor walking in place, scaled by the number of steps left to the ground.
 */
void IgorEngine::PART_22_IGOR_STEP(int facing, int step) {
	WalkData *wd = &_walkData[0];
	wd->setPos(138, 123, facing, _walkCurrentFrame);
	WalkData::setNextFrame(facing, _walkCurrentFrame);
	wd->clipSkipX = 1;
	wd->clipWidth = 30;
	wd->scaleWidth = 23 + step * 3;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 3;
	wd->scaleHeight = 50;
	moveIgor(wd->posNum, wd->frameNum);
}

/**
 * Shows the bottom (step * 8 + 16) rows of the room picture on top of the top
 * (128 - step * 8) rows of the neighbouring picture.
 */
void IgorEngine::PART_22_SCROLL_STEP(int step) {
	const int roomRows = step * 8 + 16;
	const int pictureRows = kPart22PictureRows - step * 8;
	memcpy(_screenTextLayer, _screenLayer1 + pictureRows * 320, roomRows * 320);
	memcpy(_screenTextLayer + roomRows * 320, _animFramesBuffer, pictureRows * 320);
	memcpy(_screenVGA, _screenTextLayer, 46080);
}

/**
 * The first screen shows the neighbouring picture; the room scrolls in from the top and Igor
 * walks into it.
 */
void IgorEngine::PART_22_ENTER() {
	memcpy(_screenVGA, _screenLayer1 + kPart22PictureRows * 320, 16 * 320);
	memcpy(_screenVGA + 16 * 320, _animFramesBuffer, kPart22PictureRows * 320);
	_currentAction.verb = kVerbWalk;
	fadeIn(768);

	for (int i = 0; i <= 16;) {
		if (compareGameTick(1, 16)) {
			PART_22_SCROLL_STEP(i);
			++i;
		}
		waitForTimer();
	}

	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	for (int i = 0; i <= 9; ++i) {
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		PART_22_IGOR_STEP(kFacingPositionFront, i);
		waitForTimer(15);
	}
	_walkDataLastIndex = 0;
	buildWalkPathSimple(138, 123, 150, 123);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_22() {
	playMusic(3);
	_gameState.enableLight = 1;
	loadRoomData(PAL_BellChurch, IMG_BellChurch, BOX_BellChurch, MSK_BellChurch, TXT_BellChurch);
	SET_PAL_240_48_1();
	static const int anm[] = { FRM_BellChurch1, FRM_BellChurch2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_BellChurch);
	_roomDataOffsets = PART_22_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(126, 123, 193, 123);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_22_EXEC_ACTION);
	PART_22_APPLY_OBJECT_STATE(255);
	if (!restoreRoomAfterLoad()) {
		PART_22_ENTER();
	}
	enterPartLoop();
	while (_currentPart == 220 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
