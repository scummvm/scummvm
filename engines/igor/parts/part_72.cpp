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

static const int kPart72FlowerOffset = 84 * 320 + 133;
static const int kPart72FlowerWidth = 9;
static const int kPart72FlowerHeight = 5;
static const int kPart72TakeFramesOffset = 2 * kPart72FlowerWidth * kPart72FlowerHeight;
static const int kPart72TakeWidth = 35;
static const int kPart72TakeHeight = 49;
static const int kPart72TakeFrameSize = kPart72TakeWidth * kPart72TakeHeight;
static const int kPart72TakeOffset = 42 * 320 + 133;

// The areas that show the object while it is there.
static const int kPart72ObjectAreas[5] = { 42, 44, 50, 56, 61 };

// Screen positions of the five blinking eyes, each one drawn as two pixels two columns apart.
static const int kPart72EyeOffsets[5] = { 0x2C06, 0x135B, 0x0FC5, 0x23E7, 0x254D };

static const int kPart72FlowerObjectNumber = 33;
static const int kPart72FlowerObjectIndex = 68;

static bool PART_72_shouldBlink(uint8 color) {
	return (color >= 0xC0 && color <= 0xCF) || (color >= 0xF0 && color <= 0xF1);
}

void IgorEngine::PART_72_EXEC_ACTION(int action) {
	switch (action) {
	case 101: // go to swamp
		_currentPart = 690;
		break;
	case 102:
		PART_72_ACTION_102_takeFlower();
		break;
	case 103: // look at flower
		igorSay(202, 1, 1094);
		break;
	case 104: // look at birds
		igorSay(203, 1, 1095);
		break;
	case 105: // look at eyes
		igorSay(204, 1, 1096);
		break;
	case 106:
		_objectsState[kCemeteryEntrySideStateIndex] = 0;
		_currentPart = 700;
		break;
	default:
		error("PART_72_EXEC_ACTION unhandled action %d", action);
		break;
	}
}


void IgorEngine::PART_72_APPLY_OBJECT_STATE() {
	const int taken = _objectsState[kFlowerTakenStateIndex] == 0 ? 0 : 1;
	copyArea(_screenLayer1, kPart72FlowerOffset, 320, _animFramesBuffer + taken * kPart72FlowerWidth * kPart72FlowerHeight,
			 kPart72FlowerWidth, kPart72FlowerWidth, kPart72FlowerHeight);
	if (taken) {
		for (int i = 0; i < 5; ++i) {
			_roomObjectAreasTable[kPart72ObjectAreas[i]].object = 0;
		}
	}
}

void IgorEngine::PART_72_ACTION_102_takeFlower() {
	for (int i = 0; i < 3; ++i) {
		const uint8 *frame = _animFramesBuffer + kPart72TakeFramesOffset + i * kPart72TakeFrameSize;
		copyArea(_screenVGA, kPart72TakeOffset, 320, frame, kPart72TakeWidth, kPart72TakeWidth, kPart72TakeHeight);
		waitForTimer(90);
	}
	addObjectToInventory(kPart72FlowerObjectNumber, kPart72FlowerObjectIndex);
	_objectsState[kFlowerTakenStateIndex] = 1;
	PART_72_APPLY_OBJECT_STATE();
	igorSay(201, 1, 1093);
}

void IgorEngine::PART_72_enterFromMap() {
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

void IgorEngine::PART_72_enterFromSwamp() {
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
 * Eyes in the background blink from time to time
 */
void IgorEngine::PART_72_updateBlinkingEyes() {
	int pos = kPart72EyeOffsets[_roomAmbientIndex - 1];
	if (!PART_72_shouldBlink(_screenVGA[pos])) {
		_screenVGA[pos] = 0xB3;
	}
	if (!PART_72_shouldBlink(_screenVGA[pos])) {
		_screenVGA[pos + 2] = 0xB3;
	}
	if (getRandomNumber(4) != 0) {
		return;
	}
	_roomAmbientIndex = getRandomNumber(4) + 1;
	pos = kPart72EyeOffsets[_roomAmbientIndex - 1];
	if (!PART_72_shouldBlink(_screenVGA[pos])) {
		_screenVGA[pos] = _screenLayer1[pos];
	}
	if (!PART_72_shouldBlink(_screenVGA[pos])) {
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
		PART_72_updateBlinkingEyes();
	}
}

void IgorEngine::PART_72() {
	playMusic(4);
	_gameState.enableLight = 2;
	_roomAmbientIndex = 1;
	loadActionData(DAT_Part72);
	loadRoomData(PAL_Part72, IMG_Part72, BOX_Part72, MSK_Part72, TXT_Part72);
	// the colors of Igor
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	SET_PAL_240_48_1();
	static const int anim[] = { ANM_Part72, 0 };
	loadAnimData(anim);
	_roomDataOffsets = PART_72_ROOM_DATA_OFFSETS;
	setRoomClickFix(143, -1, -1, true);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_72_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_72_UPDATE_ROOM_BACKGROUND;
	PART_72_APPLY_OBJECT_STATE();
	memcpy(_screenVGA, _screenLayer1, 46080);

	// Darken the room colors by 10 and Igor's colors by 15.
	darkenPalette(_paletteBuffer, 1, 175, 10);
	darkenPalette(_paletteBuffer, 192, 207, 15);

	PART_72_paintBlinkingEyes();

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 720) {
			PART_72_enterFromMap();
		} else {
			PART_72_enterFromSwamp();
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

void IgorEngine::PART_72_paintBlinkingEyes() {
	for (int i = 0; i < 5; ++i) {
		const int pos = kPart72EyeOffsets[i];
		_screenVGA[pos] = 0xB3;
		_screenVGA[pos + 2] = 0xB3;
	}
}

} // End of namespace Igor
