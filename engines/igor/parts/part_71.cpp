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

static const int kPart71TakeFramesCount = 3;
static const int kPart71TakeWidth = 34;
static const int kPart71TakeHeight = 50;
static const int kPart71TakeFrameSize = kPart71TakeWidth * kPart71TakeHeight;
static const int kPart71TakeOffset = 23 * 320 + 101;
static const int kPart71TakeFrameDelay = 91;

static const int kPart71StatuetteFrameOffset = 55 * 320 + 101;
static const int kPart71StatuetteFrameWidth = 5;
static const int kPart71StatuetteFrameHeight = 6;
static const int kPart71StatuetteFramesOffset = kPart71TakeFramesCount * kPart71TakeFrameSize;

static const int kPart71StatuetteNameIndex = 2;
static const int kPart71StatuetteInventoryIndex = 60;

// What part71ApplyObjectState refreshes.
static const int kPart71ApplyPicture = 1;
static const int kPart71ApplyName = 2;
static const int kPart71ApplyAll = 0xFF;

// Thunder comes a number of lightning checks after the lightning.
static const int kPart71ThunderDelay = 10;
static const int kPart71ThunderSound = 42;

void IgorEngine::part71ExecuteAction(int action) {
	switch (action) {
	case 101: // look at John O'Callahan's grave
		igorSay({ { 201, 1, 993 }, { 202, 1, 994 }, { 203, 1, 995 } });
		break;
	case 102: // take statuette
		part71TakeObject();
		break;
	case 103: // look at statuette
		part71Look();
		break;
	case 104: // look at Mary O'hara's grave
		igorSay({ { 208, 1, 997 }, { 209, 1, 998 }, { 210, 1, 999 } });
		break;
	case 105: // leave
		_currentPart = 681;
		break;
	case 106: // look at chris mcpherson's grave
		igorSay({ { 212, 1, 1000 }, { 213, 1, 1001 }, { 214, 1, 1002 } });
		break;
	case 107: // look at pat's grave
		igorSay({ { 216, 1, 1003 }, { 217, 2, 1004 } });
		break;
	default:
		error("part71ExecuteAction unhandled action %d", action);
		break;
	}
}

/**
 * Draws one of the two versions of the object into the room picture.
 */
void IgorEngine::part71DrawObject(int frame) {
	const int frameSize = kPart71StatuetteFrameWidth * kPart71StatuetteFrameHeight;
	copyArea(_screenLayer1, kPart71StatuetteFrameOffset, 320, _animFramesBuffer + kPart71StatuetteFramesOffset + frame * frameSize,
			 kPart71StatuetteFrameWidth, kPart71StatuetteFrameWidth, kPart71StatuetteFrameHeight);
}


void IgorEngine::part71ApplyObjectState(int what) {
	if (what == kPart71ApplyPicture || what == kPart71ApplyAll) {
		part71DrawObject(_objectsState[kStatuetteTakenStateIndex] == 0 ? 0 : 1);
	}
	if (what == kPart71ApplyName || what == kPart71ApplyAll) {
		const char *name = getString(_objectsState[kStatuetteExaminedStateIndex] == 0 ? STR_ShinyThing : STR_Statuette);
		if (name) {
			Common::strlcpy(_roomObjectNames[kPart71StatuetteNameIndex], name, sizeof(_roomObjectNames[kPart71StatuetteNameIndex]));
		}
	}
}


void IgorEngine::part71Look() {
	if (_objectsState[kStatuetteExaminedStateIndex] == 0) {
		igorSay(220, 2, 1005);
		_objectsState[kStatuetteExaminedStateIndex] = 1;
		part71ApplyObjectState(kPart71ApplyName);
	} else {
		igorSay(222, 2, 1006);
	}
}


void IgorEngine::part71TakeObject() {
	if (_objectsState[kStatuetteExaminedStateIndex] == 0) {
		igorSayAndWait(220, 2, 1005);
		_objectsState[kStatuetteExaminedStateIndex] = 1;
	}
	for (int i = 0; i < kPart71TakeFramesCount; ++i) {
		drawAnimRect(kPart71TakeOffset, i * kPart71TakeFrameSize, kPart71TakeWidth, kPart71TakeHeight, false, kBlendLitSpriteNoShade);
		waitForTimer(kPart71TakeFrameDelay);
	}
	addObjectToInventory(kInventoryObjectLizard, kPart71StatuetteInventoryIndex);
	_objectsState[kStatuetteTakenStateIndex] = 1;
	part71ApplyObjectState(kPart71ApplyAll);
	igorSay(204, 3, 996);
	_objectsState[kStatuetteTaken2StateIndex] = 1;
}

void IgorEngine::part71EnterFromRight() {
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
void IgorEngine::part71Lightning() {
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

void IgorEngine::part71UpdateBackground() {
	if (compareGameTick(1)) {
		if (getRandomNumber(99) == 0) {
			part71Lightning();
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

void IgorEngine::part71() {
	if (isCDVersion())
		playMusic(4);
	_gameState.enableLight = 1;
	loadActionData(DAT_Part71);
	loadRoomData(PAL_Part71, IMG_Part71, BOX_Part71, MSK_Part71, TXT_Part71);
	// the colors of Igor
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	setVerbPanelColors();
	static const int anim[] = { ANM_Part71, 0 };
	loadAnimData(anim);
	_roomDataOffsets = PART_71_ROOM_DATA_OFFSETS;
	setRoomClickFix(138, 127, -1, true, 138);
	partMainSetExecuteAction(1, &IgorEngine::part71ExecuteAction);
	_updateRoomBackground = &IgorEngine::part71UpdateBackground;
	part71ApplyObjectState(kPart71ApplyAll);
	copyLayerToScreen(_screenLayer1Surface);
	_roomAmbientIndex = 0;

	// the colors of Igor are darker than their palette
	darkenPalette(_paletteBuffer, 192, 207, 10);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		part71EnterFromRight();
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
