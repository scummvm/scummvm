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

// The animation resource starts with the three frames of Igor taking the object (34x50 each, drawn on the screen) and
// ends with the two versions of the object as it is drawn in the room picture (5x6 each).
static const int kPart71TakeFramesCount = 3;
static const int kPart71TakeWidth = 34;
static const int kPart71TakeHeight = 50;
static const int kPart71TakeFrameSize = kPart71TakeWidth * kPart71TakeHeight;
static const int kPart71TakeOffset = 23 * 320 + 101;
static const int kPart71TakeFrameDelay = 91;

static const int kPart71ObjectOffset = 55 * 320 + 101;
static const int kPart71ObjectWidth = 5;
static const int kPart71ObjectHeight = 6;
static const int kPart71ObjectPicturesOffset = kPart71TakeFramesCount * kPart71TakeFrameSize;

static const int kPart71TakenStateIndex = 100;
static const int kPart71ExaminedStateIndex = 101;
static const int kPart71TakeDoneStateIndex = 106;
static const int kPart71ObjectNameIndex = 2;

static const int kPart71InventoryObject = 25;
static const int kPart71InventoryIndex = 60;

// What PART_71_APPLY_OBJECT_STATE refreshes.
static const int kPart71ApplyPicture = 1;
static const int kPart71ApplyName = 2;
static const int kPart71ApplyAll = 0xFF;

// Thunder comes a number of lightning checks after the lightning.
static const int kPart71ThunderDelay = 10;
static const int kPart71ThunderSound = 42;

void IgorEngine::PART_71_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		igorSay({ { 201, 1, 993 }, { 202, 1, 994 }, { 203, 1, 995 } });
		break;
	case 102:
		PART_71_ACTION_102_takeObject();
		break;
	case 103:
		PART_71_ACTION_103_look();
		break;
	case 104:
		igorSay({ { 208, 1, 997 }, { 209, 1, 998 }, { 210, 1, 999 } });
		break;
	case 105:
		_currentPart = 681;
		break;
	case 106:
		igorSay({ { 212, 1, 1000 }, { 213, 1, 1001 }, { 214, 1, 1002 } });
		break;
	case 107:
		igorSay({ { 216, 1, 1003 }, { 217, 2, 1004 } });
		break;
	default:
		error("PART_71_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

/**
 * Draws one of the two versions of the object into the room picture.
 */
void IgorEngine::PART_71_DRAW_OBJECT(int frame) {
	const int frameSize = kPart71ObjectWidth * kPart71ObjectHeight;
	copyArea(_screenLayer1, kPart71ObjectOffset, 320, _animFramesBuffer + kPart71ObjectPicturesOffset + frame * frameSize,
			 kPart71ObjectWidth, kPart71ObjectWidth, kPart71ObjectHeight);
}

/**
 * Refreshes what depends on the story: the picture of the object (it is gone once Igor has taken it) and its name
 * (it changes once Igor has examined it).
 */
void IgorEngine::PART_71_APPLY_OBJECT_STATE(int what) {
	if (what == kPart71ApplyPicture || what == kPart71ApplyAll) {
		PART_71_DRAW_OBJECT(_objectsState[kPart71TakenStateIndex] == 0 ? 0 : 1);
	}
	if (what == kPart71ApplyName || what == kPart71ApplyAll) {
		const char *name = getString(_objectsState[kPart71ExaminedStateIndex] == 0 ? STR_ShinyThing : STR_Statuette);
		// TODO: the name of the object before it is examined is only known in Spanish; the other languages keep the
		// name that comes with the room.
		if (name) {
			Common::strlcpy(_roomObjectNames[kPart71ObjectNameIndex], name, sizeof(_roomObjectNames[kPart71ObjectNameIndex]));
		}
	}
}

/**
 * Igor examines the object: the first time he describes it and it gets its real name.
 */
void IgorEngine::PART_71_ACTION_103_look() {
	if (_objectsState[kPart71ExaminedStateIndex] == 0) {
		igorSay(220, 2, 1005);
		_objectsState[kPart71ExaminedStateIndex] = 1;
		PART_71_APPLY_OBJECT_STATE(kPart71ApplyName);
	} else {
		igorSay(222, 2, 1006);
	}
}

/**
 * Igor takes the object: three frames of the animation are drawn over the screen, then the object goes to the
 * inventory.
 */
void IgorEngine::PART_71_ACTION_102_takeObject() {
	if (_objectsState[kPart71ExaminedStateIndex] == 0) {
		igorSayAndWait(220, 2, 1005);
		_objectsState[kPart71ExaminedStateIndex] = 1;
	}
	for (int i = 0; i < kPart71TakeFramesCount; ++i) {
		drawAnimRect(kPart71TakeOffset, i * kPart71TakeFrameSize, kPart71TakeWidth, kPart71TakeHeight, false, kBlendLitSpriteNoShade);
		waitForTimer(kPart71TakeFrameDelay);
	}
	addObjectToInventory(kPart71InventoryObject, kPart71InventoryIndex);
	_objectsState[kPart71TakenStateIndex] = 1;
	PART_71_APPLY_OBJECT_STATE(kPart71ApplyAll);
	igorSay(204, 3, 996);
	_objectsState[kPart71TakeDoneStateIndex] = 1;
}

/**
 * Igor comes from the right edge, walking to the left.
 */
void IgorEngine::PART_71_ENTER_FROM_RIGHT() {
	WalkData *wd = &_walkData[0];
	wd->setPos(319, 138, kFacingPositionLeft, 0);
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
	buildWalkPath(319, 138, 276, 123);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

/**
 * The picture and the colors of the room are inverted twice, as when lightning strikes outside. The thunder follows
 * later.
 */
void IgorEngine::PART_71_LIGHTNING() {
	uint8 normalPalette[624];
	uint8 invertedPalette[624];

	memcpy(normalPalette, _currentPalette, sizeof(normalPalette));
	for (int i = 3; i < (int)sizeof(invertedPalette); ++i) {
		_currentPalette[i] = 62 - _currentPalette[i];
	}
	memcpy(invertedPalette, _currentPalette, sizeof(invertedPalette));

	for (int flash = 0; flash < 2; ++flash) {
		memcpy(_currentPalette, invertedPalette, sizeof(invertedPalette));
		setPaletteRange(1, 207);
		waitForTimer(15);
		memcpy(_currentPalette, normalPalette, sizeof(normalPalette));
		setPaletteRange(1, 207);
		if (flash == 0) {
			waitForTimer(5);
		}
	}
	_roomAmbientIndex = 1;
}

void IgorEngine::PART_71_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(1)) {
		if (getRandomNumber(99) == 0) {
			PART_71_LIGHTNING();
		}
		if (_roomAmbientIndex > 0) {
			++_roomAmbientIndex;
		}
	}
	if (_roomAmbientIndex == kPart71ThunderDelay) {
		playSound(kPart71ThunderSound, 1);
		_roomAmbientIndex = 0;
	}
}

void IgorEngine::PART_71() {
	playMusic(4);
	_gameState.enableLight = 1;
	loadActionData(DAT_Part71);
	loadRoomData(PAL_Part71, IMG_Part71, BOX_Part71, MSK_Part71, TXT_Part71);
	// the colors of Igor
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	SET_PAL_240_48_1();
	static const int anim[] = { ANM_Part71, 0 };
	loadAnimData(anim);
	_roomDataOffsets = PART_71_ROOM_DATA_OFFSETS;
	setRoomClickFix(138, 127, -1, true, 138);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_71_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_71_UPDATE_ROOM_BACKGROUND;
	PART_71_APPLY_OBJECT_STATE(kPart71ApplyAll);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_roomAmbientIndex = 0;

	// the colors of Igor are darker than their palette
	darkenPalette(_paletteBuffer, 192, 207, 10);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		PART_71_ENTER_FROM_RIGHT();
	}
	enterPartLoop();
	while (_currentPart == 710 && !_gameStateLoaded) {
		runPartLoop();
	}
	// the colors that changed are kept
	memcpy(_paletteBuffer, _currentPalette, 624);
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
