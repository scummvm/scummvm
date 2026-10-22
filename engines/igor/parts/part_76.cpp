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

static const int kPart76ParachuteWidth = 7;
static const int kPart76ParachuteHeight = 6;
static const int kPart76ParachuteSize = kPart76ParachuteWidth * kPart76ParachuteHeight;
static const int kPart76ParachuteOffset = 83 * 320 + 300;

static const int kPart76PlaneDoorWidth = 68;
static const int kPart76PlaneDoorHeight = 54;
static const int kPart76PlaneDoorSize = kPart76PlaneDoorWidth * kPart76PlaneDoorHeight;
static const int kPart76PlaneDoorFirst = 0x54;
static const int kPart76PlaneDoorOffset = 27 * 320 + 188;

static const int kPart76OpenDoorAnimBase = 0x2902;
static const int kPart76OpenDoorAnimTable = 0x1753;

static const int kPart76ParachuteAnimBase = 0x4067;
static const int kPart76ParachuteAnimTable = 0x3ABB;

static const int kPart76TakeParachuteOffset = 81 * 320 + 289;
static const int kPart76TakeParachuteWidth = 17;
static const int kPart76TakeParachuteHeight = 16;
static const int kPart76TakeParachuteFrames = 0x1BF4;
static const int kPart76TakeParachuteFrameSize = kPart76TakeParachuteWidth * kPart76TakeParachuteHeight;

static const int kPart76SpriteOffset = 71 * 320 + 285;
static const int kPart76SpriteWidth = 23;
static const int kPart76SpriteHeight = 49;
static const int kPart76SpriteFrames = 0x1BCD;
static const int kPart76SpriteFrameSize = kPart76SpriteWidth * kPart76SpriteHeight;

static const uint8 kPart76DoorFrames[5] = { 1, 2, 3, 2, 3 };

static const uint8 kPart76SpriteOrder[3] = { 1, 2, 1 };

static const uint8 kPart76ParachuteFrames[20] = { 18, 19, 20, 21, 22, 23, 22, 23, 22, 23, 22, 23, 22, 23, 22, 23, 24, 25, 26, 27 };

// Point where Igor stops at the end of the walk of the scene and the one he reaches later.
static const int kPart76WalkEndX = 295;
static const int kPart76WalkEndY = 119;

static const int kPart76ParachuteInventoryIndex = 38;

void IgorEngine::part76ExecuteAction(int action) {
	if (isCDVersion() && action != 103) {
		// the music of the room is started again by every action but the third one
		playMusic(12);
	}
	switch (action) {
	case 101: // look at door
		igorSay(201, 1, 700);
		break;
	case 102: // look at lever
		igorSay(202, 1, 701);
		break;
	case 103:
		part76OpenDoor();
		break;
	case 104:
		part76TakeParachute();
		break;
	case 105:  // look at parachute
		igorSay(207, 2, 705);
		break;
	case 106:
		part76UseParachute();
		break;
	case 107: // look at crate
		igorSay(224, 1, 719);
		break;
	case 108: // look at crates
		igorSay(225, 1, 720);
		break;
	default:
		error("part76ExecuteAction unhandled action %d", action);
		break;
	}
}

/**
 * Pieces of the room picture that follow the state of the objects (they are drawn into the picture, not on the
 * screen) and the actions and objects that go with them.
 */
void IgorEngine::part76ApplyObjectState() {
	if (_objectsState[kPlaneDoorOpenStateIndex] == 0) {
		copyArea(_screenLayer1, kPart76PlaneDoorOffset, 320, _animFramesBuffer + kPart76PlaneDoorFirst, kPart76PlaneDoorWidth,
				 kPart76PlaneDoorWidth, kPart76PlaneDoorHeight);
		_roomActionsTable[79] = 9;
		_roomActionsTable[81] = 14;
	} else {
		copyArea(_screenLayer1, kPart76PlaneDoorOffset, 320,
				 _animFramesBuffer + kPart76PlaneDoorFirst + kPart76PlaneDoorSize, kPart76PlaneDoorWidth, kPart76PlaneDoorWidth,
				 kPart76PlaneDoorHeight);
		_roomObjectAreasTable[10].object = 1;
		_roomActionsTable[79] = 11;
		_roomActionsTable[81] = 12;
	}
	if (_objectsState[kParachuteTakenStateIndex] == 0) {
		copyArea(_screenLayer1, kPart76ParachuteOffset, 320, _animFramesBuffer, kPart76ParachuteWidth,
				 kPart76ParachuteWidth, kPart76ParachuteHeight);
	} else {
		copyArea(_screenLayer1, kPart76ParachuteOffset, 320, _animFramesBuffer + kPart76ParachuteSize,
				 kPart76ParachuteWidth, kPart76ParachuteWidth, kPart76ParachuteHeight);
		// once the object was taken its areas stop being selectable
		_roomObjectAreasTable[11].object = 0;
		_roomObjectAreasTable[12].object = 0;
	}
}

void IgorEngine::part76DrawParachuteFrame(int frame) {
	decodeAnimFrame(getAnimFrame(kPart76ParachuteAnimBase, kPart76ParachuteAnimTable, frame), _screenVGA);
}

void IgorEngine::part76DrawOpenDoorFrame(int frame) {
	decodeAnimFrame(getAnimFrame(kPart76OpenDoorAnimBase, kPart76OpenDoorAnimTable, frame), _screenVGA);
}

void IgorEngine::part76OpenDoor() {
	for (int i = 0; i < 5; ++i) {
		part76DrawOpenDoorFrame(kPart76DoorFrames[i]);
		waitForTimer(51);
	}
	for (int frame = 4; frame <= 9; ++frame) {
		part76DrawOpenDoorFrame(frame);
		waitForTimer(31);
	}
	igorSay(206, 1, 704);
	_objectsState[kPlaneDoorOpenStateIndex] = 1;
	part76ApplyObjectState();
}

/**
 * Three frames of a sprite are drawn on the screen, then object 3 goes into the inventory.
 */
void IgorEngine::part76TakeParachute() {
	for (int frame = 1; frame <= 3; ++frame) {
		drawAnimRect(kPart76TakeParachuteOffset, kPart76TakeParachuteFrames + frame * kPart76TakeParachuteFrameSize, kPart76TakeParachuteWidth, kPart76TakeParachuteHeight);
		waitForTimer(91);
	}
	addObjectToInventory(kInventoryObjectParachute, kPart76ParachuteInventoryIndex);
	_objectsState[kParachuteTakenStateIndex] = 1;
	part76ApplyObjectState();
	igorSay({ { 203, 1, 702 }, { 204, 2, 703 } });
}

/**
 * Scripted sequence that is only played once the third action was done (before that, Igor only says a line). Igor stops
 * where he is, talks, walks to the right of the picture, the sprite is drawn, he talks again and walks to (190, 85);
 * the full screen frames follow and the story goes on in state 790.
 */
void IgorEngine::part76UseParachute() {
	if (_objectsState[kPlaneDoorOpenStateIndex] == 0) {
		igorSay(209, 1, 706);
		return;
	}
	// the panel and the colors 240 to 255 are cleared
	memset(_screenVGA + 46080, 0, 17920);
	memset(_paletteBuffer + 240 * 3, 0, 48);
	memset(_currentPalette + 240 * 3, 0, 48);
	if (_gameState.igorMoving) {
		_walkDataLastIndex = _walkDataCurrentIndex + 1;
		_walkData[_walkDataCurrentIndex].frameNum = 0;
		moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
		_gameState.igorMoving = false;
	}
	igorSay({ { 210, 1, 707 }, { 211, 1, 708 } });
	waitForEndOfIgorDialogue();

	--_walkDataLastIndex;
	buildWalkPath(_walkData[_walkDataLastIndex].x, _walkData[_walkDataLastIndex].y, kPart76WalkEndX, kPart76WalkEndY);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();

	for (int i = 0; i < 3; ++i) {
		drawAnimRect(kPart76SpriteOffset, kPart76SpriteFrames + kPart76SpriteOrder[i] * kPart76SpriteFrameSize,
					 kPart76SpriteWidth, kPart76SpriteHeight, false, kBlendLitSpriteNoShade);
		waitForTimer(46);
	}
	igorSay({ { 212, 2, 709 }, { 214, 2, 710 } });
	waitForEndOfIgorDialogue();

	--_walkDataLastIndex;
	buildWalkPath(kPart76WalkEndX, kPart76WalkEndY, 190, 85);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkData[_walkDataLastIndex].posNum = 2;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();

	igorSay({ { 216, 1, 711 }, { 217, 1, 712 }, { 218, 1, 713 }, { 219, 1, 714 } });
	waitForEndOfIgorDialogue();
	part76DrawParachuteFrame(11);
	waitForTimer(255);
	for (int frame = 12; frame <= 17; ++frame) {
		part76DrawParachuteFrame(frame);
		if (frame < 17) {
			waitForTimer(3);
		}
	}
	igorSay({ { 220, 1, 715 }, { 221, 1, 716 }, { 222, 1, 717 }, { 223, 1, 718 } });
	waitForEndOfIgorDialogue();
	for (int i = 0; i < 20; ++i) {
		part76DrawParachuteFrame(kPart76ParachuteFrames[i]);
		waitForTimer(31);
	}
	// the colors 240 to 255 are black on the screen
	for (int i = 240; i <= 255; ++i) {
		setPaletteColor(i, 0, 0, 0);
	}
	_currentPart = 790;
}

/**
 * Igor says something before he is in the picture, then he walks in from the left to (159, 94). The text that goes
 * with the arrival is shown while the room is already running.
 */
void IgorEngine::part76Entry() {
	WalkData *wd = &_walkData[0];
	wd->x = 0;
	wd->y = 71;
	wd->scaleWidth = 49;
	wd->scaleHeight = 49;
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
	igorSay({ { 226, 2, 721 }, { 228, 1, 722 } });
	waitForEndOfIgorDialogue(false);
	waitForTimer(255);
	waitForTimer(255);

	wd->setPos(10, 71, 2, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 30;
	wd->scaleWidth = 50;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 50;
	_walkDataLastIndex = 0;
	// the point at the left edge is part of the walkable area while the path is built
	const int startArea = _screenLayer2[71 * 320 + 10];
	_roomObjectAreasTable[startArea].area = 1;
	buildWalkPath(10, 71, 159, 94);
	_roomObjectAreasTable[startArea].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();

	igorSay({ { 229, 1, 723 }, { 230, 2, 724 }, { 232, 1, 725 } });
}

void IgorEngine::part76() {
	if (isCDVersion())
		playMusic(12);
	if (!_gameStateLoaded) {
		// the card that opens the part
		partMainMeanwhile(IMG_Part76Card);
	}
	_gameState.enableLight = 2;
	loadIgorBeachFrames();
	loadActionData(DAT_Part76);
	loadRoomData(PAL_Part76, IMG_Part76, BOX_Part76, MSK_Part76, TXT_Part76);
	SET_PAL_240_48_1();
	static const int anim[] = { ANM_Part76, 0 };
	loadAnimData(anim);
	SET_PAL_208_96_1();
	drawVerbsPanel();
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	_roomDataOffsets = PART_76_ROOM_DATA_OFFSETS;
	setRoomClickFix(119, 153, 295, true, 119);
	partMainSetExecuteAction(1, &IgorEngine::part76ExecuteAction);
	part76ApplyObjectState();
	if (_game.version == kIdSpaFloppy)
		playSound(kSpaFloppySoundPlaneEngine, 1);
	memcpy(_screenVGA, _screenLayer1, 46080);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		part76Entry();
	}
	enterPartLoop();
	while (_currentPart == 760 && !_gameStateLoaded) {
		runPartLoop();
	}
	stopDialogueSpeech();
	if (!_gameStateLoaded) {
		// the panel is cleared and the colors 208 to 254 go black in both palettes
		memset(_screenVGA + 46080, 0, 17920);
		memset(_paletteBuffer + 208 * 3, 0, 141);
		memset(_currentPalette + 208 * 3, 0, 141);
	} else {
		loadIgorFrames();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
	// the display starts at the top of the picture again
	_screenVGAVOffset = 0;
}

} // End of namespace Igor
