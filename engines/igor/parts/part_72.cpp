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

// The object that can be taken is drawn in the room picture as a 9x5 patch; the animation resource starts with
// its two versions and then holds the three frames of the taking animation.
static const int kPart72ObjectOffset = 84 * 320 + 133;
static const int kPart72ObjectWidth = 9;
static const int kPart72ObjectHeight = 5;
static const int kPart72ObjectStateIndex = 102;
static const int kPart72TakeFramesOffset = 2 * kPart72ObjectWidth * kPart72ObjectHeight;
static const int kPart72TakeWidth = 35;
static const int kPart72TakeHeight = 49;
static const int kPart72TakeFrameSize = kPart72TakeWidth * kPart72TakeHeight;
static const int kPart72TakeOffset = 42 * 320 + 133;

// The areas that show the object while it is there.
static const int kPart72ObjectAreas[5] = { 42, 44, 50, 56, 61 };

// Screen positions of the five stars, each one drawn as two pixels two columns apart.
static const int kPart72StarOffsets[5] = { 0x2C06, 0x135B, 0x0FC5, 0x23E7, 0x254D };

static const int kPart72InventoryObject = 33;
static const int kPart72InventoryIndex = 68;

static bool PART_72_IS_PROTECTED_COLOR(uint8 color) {
	return (color >= 0xC0 && color <= 0xCF) || (color >= 0xF0 && color <= 0xF1);
}

static void PART_72_DARKEN_PALETTE(uint8 *palette, int firstColor, int lastColor, int amount) {
	for (int i = firstColor * 3; i < (lastColor + 1) * 3; ++i) {
		const int value = palette[i] - amount;
		palette[i] = value < 1 ? 0 : value;
	}
}

void IgorEngine::PART_72_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_currentPart = 690;
		break;
	case 102:
		PART_72_ACTION_102_takeObject();
		break;
	case 103:
		igorSay(202, 1, 1094);
		break;
	case 104:
		igorSay(203, 1, 1095);
		break;
	case 105:
		igorSay(204, 1, 1096);
		break;
	case 106:
		_objectsState[90] = 0;
		_currentPart = 700;
		break;
	default:
		error("PART_72_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

/**
 * Draws the object into the room picture while it is there. Once it is taken its places are not objects anymore.
 */
void IgorEngine::PART_72_APPLY_OBJECT_STATE() {
	const int taken = _objectsState[kPart72ObjectStateIndex] == 0 ? 0 : 1;
	copyArea(_screenLayer1, kPart72ObjectOffset, 320, _animFramesBuffer + taken * kPart72ObjectWidth * kPart72ObjectHeight,
			 kPart72ObjectWidth, kPart72ObjectWidth, kPart72ObjectHeight);
	if (taken) {
		for (int i = 0; i < 5; ++i) {
			_roomObjectAreasTable[kPart72ObjectAreas[i]].object = 0;
		}
	}
}

/**
 * Igor takes the object: three frames of the animation are drawn over the screen, then the object goes to the
 * inventory.
 */
void IgorEngine::PART_72_ACTION_102_takeObject() {
	for (int i = 0; i < 3; ++i) {
		const uint8 *frame = _animFramesBuffer + kPart72TakeFramesOffset + i * kPart72TakeFrameSize;
		copyArea(_screenVGA, kPart72TakeOffset, 320, frame, kPart72TakeWidth, kPart72TakeWidth, kPart72TakeHeight);
		waitForTimer(90);
	}
	addObjectToInventory(kPart72InventoryObject, kPart72InventoryIndex);
	_objectsState[kPart72ObjectStateIndex] = 1;
	PART_72_APPLY_OBJECT_STATE();
	igorSay(201, 1, 1093);
}

/**
 * Igor comes from the right edge, walking to the left.
 */
void IgorEngine::PART_72_ENTER_FROM_RIGHT() {
	WalkData *wd = &_walkData[0];
	wd->setPos(319, 68, kFacingPositionLeft, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 15;
	wd->scaleWidth = 50;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 50;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(319, 68, 248, 108);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

/**
 * Igor comes from the left edge, walking to the right.
 */
void IgorEngine::PART_72_ENTER_FROM_LEFT() {
	WalkData *wd = &_walkData[0];
	wd->setPos(0, 119, kFacingPositionRight, 0);
	wd->clipSkipX = 15;
	wd->clipWidth = 15;
	wd->scaleWidth = 50;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 50;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(0, 119, 41, 119);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

/**
 * The current star is lit and, from time to time, another one is chosen and put out.
 */
void IgorEngine::PART_72_UPDATE_STAR() {
	int pos = kPart72StarOffsets[_part72StarIndex - 1];
	if (!PART_72_IS_PROTECTED_COLOR(_screenVGA[pos])) {
		_screenVGA[pos] = 0xB3;
	}
	if (!PART_72_IS_PROTECTED_COLOR(_screenVGA[pos])) {
		_screenVGA[pos + 2] = 0xB3;
	}
	if (getRandomNumber(4) != 0) {
		return;
	}
	_part72StarIndex = getRandomNumber(4) + 1;
	pos = kPart72StarOffsets[_part72StarIndex - 1];
	if (!PART_72_IS_PROTECTED_COLOR(_screenVGA[pos])) {
		_screenVGA[pos] = _screenLayer1[pos];
	}
	if (!PART_72_IS_PROTECTED_COLOR(_screenVGA[pos])) {
		_screenVGA[pos + 2] = _screenLayer1[pos + 2];
	}
}

void IgorEngine::PART_72_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(1)) {
		if (getRandomNumber(14) == 0 && !isDialogueSpeechPlaying()) {
			switch (getRandomNumber(1)) {
			case 0:
				playSound(39, 1);
				break;
			case 1:
				playSound(40, 1);
				break;
			}
		}
	}
	if (compareGameTick(0x3D)) {
		PART_72_UPDATE_STAR();
	}
}

void IgorEngine::PART_72() {
	playMusic(4);
	_gameState.enableLight = 2;
	_part72StarIndex = 1;
	loadActionData(DAT_Part72);
	loadRoomData(PAL_Part72, IMG_Part72, BOX_Part72, MSK_Part72, TXT_Part72);
	SET_PAL_240_48_1();
	static const int anim[] = { ANM_Part72, 0 };
	loadAnimData(anim);
	_roomDataOffsets = PART_72_ROOM_DATA_OFFSETS;
	setRoomClickFix(143, -1, -1, true);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_72_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_72_UPDATE_ROOM_BACKGROUND;
	PART_72_APPLY_OBJECT_STATE();
	memcpy(_screenVGA, _screenLayer1, 46080);

	// the whole picture is darker than its palette
	PART_72_DARKEN_PALETTE(_paletteBuffer, 1, 175, 10);
	PART_72_DARKEN_PALETTE(_paletteBuffer, 192, 207, 15);

	for (int i = 0; i < 5; ++i) {
		const int pos = kPart72StarOffsets[i];
		_screenVGA[pos] = 0xB3;
		_screenVGA[pos + 2] = 0xB3;
	}

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 720) {
			PART_72_ENTER_FROM_RIGHT();
		} else {
			PART_72_ENTER_FROM_LEFT();
		}
	}
	enterPartLoop();
	while ((_currentPart == 720 || _currentPart == 721) && !_gameStateLoaded) {
		runPartLoop();
	}
	// the colors that changed are kept
	memcpy(_paletteBuffer, _currentPalette, 624);
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
