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

// The animation resource: two 7x6 pieces and two 68x54 pieces of the room picture first, then the sprite strips and
// the full screen frames, each group followed by the table of its frame offsets.
static const int kPart76SmallPieceWidth = 7;
static const int kPart76SmallPieceHeight = 6;
static const int kPart76SmallPieceSize = kPart76SmallPieceWidth * kPart76SmallPieceHeight;
static const int kPart76SmallPieceOffset = 83 * 320 + 300;

static const int kPart76BigPieceWidth = 68;
static const int kPart76BigPieceHeight = 54;
static const int kPart76BigPieceSize = kPart76BigPieceWidth * kPart76BigPieceHeight;
static const int kPart76BigPieceFirst = 0x54;
static const int kPart76BigPieceOffset = 27 * 320 + 188;

// Frames of the first sequence: the group starts at 0x2902 and its table of offsets follows the frames.
static const int kPart76SequenceBase = 0x2902;
static const int kPart76SequenceTable = 0x1753;

// Frames of the full screen scenes: the group starts at 0x4067 and its table of offsets follows the frames.
static const int kPart76SceneBase = 0x4067;
static const int kPart76SceneTable = 0x3ABB;

// A 17x16 sprite drawn at (289,81), three frames of 272 bytes.
static const int kPart76LampOffset = 81 * 320 + 289;
static const int kPart76LampWidth = 17;
static const int kPart76LampHeight = 16;
static const int kPart76LampFrames = 0x1BF4;
static const int kPart76LampFrameSize = kPart76LampWidth * kPart76LampHeight;

// A 23x49 sprite drawn at (285,71), frames of 1127 bytes.
static const int kPart76SpriteOffset = 71 * 320 + 285;
static const int kPart76SpriteWidth = 23;
static const int kPart76SpriteHeight = 49;
static const int kPart76SpriteFrames = 0x1BCD;
static const int kPart76SpriteFrameSize = kPart76SpriteWidth * kPart76SpriteHeight;

// Order in which the frames of the first sequence are shown before the closing frames 4 to 9.
static const uint8 kPart76SequenceFrames[5] = { 1, 2, 3, 2, 3 };

// Order of the sprite frames of the third action.
static const uint8 kPart76SpriteOrder[3] = { 1, 2, 1 };

// Order of the frames of the last part of the third action.
static const uint8 kPart76EndFrames[20] = { 18, 19, 20, 21, 22, 23, 22, 23, 22, 23, 22, 23, 22, 23, 22, 23, 24, 25, 26, 27 };

// Point where Igor stops at the end of the walk of the scene and the one he reaches later.
static const int kPart76WalkEndX = 295;
static const int kPart76WalkEndY = 119;

// Object of the inventory that action 104 puts into it, and the position where the inventory keeps its index.
static const int kPart76InventoryObject = 3;
static const int kPart76InventoryObjectIndex = 38;

void IgorEngine::PART_76_EXEC_ACTION(int action) {
	if (action != 103) {
		// the music of the room is started again by every action but the third one
		playMusic(12);
	}
	switch (action) {
	case 101:
		igorSay(201, 1, 700);
		break;
	case 102:
		igorSay(202, 1, 701);
		break;
	case 103:
		PART_76_ACTION_103_sequence();
		break;
	case 104:
		PART_76_ACTION_104_takeObject();
		break;
	case 105:
		igorSay(207, 2, 705);
		break;
	case 106:
		PART_76_ACTION_106_finale();
		break;
	case 107:
		igorSay(224, 1, 719);
		break;
	case 108:
		igorSay(225, 1, 720);
		break;
	default:
		error("PART_76_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

/**
 * Pieces of the room picture that follow the state of the objects (they are drawn into the picture, not on the
 * screen) and the actions and objects that go with them.
 */
void IgorEngine::PART_76_APPLY_OBJECT_STATE() {
	if (_objectsState[108] == 0) {
		copyArea(_screenLayer1, kPart76BigPieceOffset, 320, _animFramesBuffer + kPart76BigPieceFirst, kPart76BigPieceWidth,
				 kPart76BigPieceWidth, kPart76BigPieceHeight);
		_roomActionsTable[79] = 9;
		_roomActionsTable[81] = 14;
	} else {
		copyArea(_screenLayer1, kPart76BigPieceOffset, 320,
				 _animFramesBuffer + kPart76BigPieceFirst + kPart76BigPieceSize, kPart76BigPieceWidth, kPart76BigPieceWidth,
				 kPart76BigPieceHeight);
		_roomObjectAreasTable[10].object = 1;
		_roomActionsTable[79] = 11;
		_roomActionsTable[81] = 12;
	}
	if (_objectsState[109] == 0) {
		copyArea(_screenLayer1, kPart76SmallPieceOffset, 320, _animFramesBuffer, kPart76SmallPieceWidth,
				 kPart76SmallPieceWidth, kPart76SmallPieceHeight);
	} else {
		copyArea(_screenLayer1, kPart76SmallPieceOffset, 320, _animFramesBuffer + kPart76SmallPieceSize,
				 kPart76SmallPieceWidth, kPart76SmallPieceWidth, kPart76SmallPieceHeight);
	}
	_roomObjectAreasTable[11].object = 0;
	_roomObjectAreasTable[12].object = 0;
}

/**
 * Frame n of the full screen scenes, drawn on the screen.
 */
void IgorEngine::PART_76_DRAW_SCENE_FRAME(int frame) {
	decodeAnimFrame(getAnimFrame(kPart76SceneBase, kPart76SceneTable, frame), _screenVGA);
}

/**
 * Frame n of the first sequence, drawn on the screen.
 */
void IgorEngine::PART_76_DRAW_SEQUENCE_FRAME(int frame) {
	decodeAnimFrame(getAnimFrame(kPart76SequenceBase, kPart76SequenceTable, frame), _screenVGA);
}

/**
 * Frames 1 to 5 and then 4 to 9 are drawn on the screen, then the object changes its state.
 */
void IgorEngine::PART_76_ACTION_103_sequence() {
	for (int i = 0; i < 5; ++i) {
		PART_76_DRAW_SEQUENCE_FRAME(kPart76SequenceFrames[i]);
		waitForTimer(51);
	}
	for (int frame = 4; frame <= 9; ++frame) {
		PART_76_DRAW_SEQUENCE_FRAME(frame);
		waitForTimer(31);
	}
	igorSay(206, 1, 704);
	_objectsState[108] = 1;
	PART_76_APPLY_OBJECT_STATE();
}

/**
 * Three frames of a sprite are drawn on the screen, then object 3 goes into the inventory.
 */
void IgorEngine::PART_76_ACTION_104_takeObject() {
	for (int frame = 1; frame <= 3; ++frame) {
		drawAnimRect(kPart76LampOffset, kPart76LampFrames + frame * kPart76LampFrameSize, kPart76LampWidth, kPart76LampHeight);
		waitForTimer(91);
	}
	addObjectToInventory(kPart76InventoryObject, kPart76InventoryObjectIndex);
	_objectsState[109] = 1;
	PART_76_APPLY_OBJECT_STATE();
	igorSay({ { 203, 1, 702 }, { 204, 2, 703 } });
}

/**
 * Scripted sequence that is only played once the third action was done (before that, Igor only says a line). Igor stops
 * where he is, talks, walks to the right of the picture, the sprite is drawn, he talks again and walks to (190, 85);
 * the full screen frames follow and the story goes on in state 790.
 */
void IgorEngine::PART_76_ACTION_106_finale() {
	if (_objectsState[108] == 0) {
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
	PART_76_DRAW_SCENE_FRAME(11);
	waitForTimer(255);
	for (int frame = 12; frame <= 17; ++frame) {
		PART_76_DRAW_SCENE_FRAME(frame);
		if (frame < 17) {
			waitForTimer(3);
		}
	}
	igorSay({ { 220, 1, 715 }, { 221, 1, 716 }, { 222, 1, 717 }, { 223, 1, 718 } });
	waitForEndOfIgorDialogue();
	for (int i = 0; i < 20; ++i) {
		PART_76_DRAW_SCENE_FRAME(kPart76EndFrames[i]);
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
void IgorEngine::PART_76_ENTRY() {
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

void IgorEngine::PART_76() {
	playMusic(12);
	if (!_gameStateLoaded) {
		// the card that opens the part
		PART_MEANWHILE(IMG_Part76Card);
	}
	_gameState.enableLight = 2;
	loadIgorFrames();
	loadActionData(DAT_Part76);
	loadRoomData(PAL_Part76, IMG_Part76, BOX_Part76, MSK_Part76, TXT_Part76);
	SET_PAL_240_48_1();
	static const int anim[] = { ANM_Part76, 0 };
	loadAnimData(anim);
	SET_PAL_208_96_1();
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	_roomDataOffsets = PART_76_ROOM_DATA_OFFSETS;
	setRoomClickFix(119, 153, 295, true, 119);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_76_EXEC_ACTION);
	PART_76_APPLY_OBJECT_STATE();
	memcpy(_screenVGA, _screenLayer1, 46080);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		PART_76_ENTRY();
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
