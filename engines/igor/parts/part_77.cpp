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

// The animation resource starts with the frames and ends with the table of their offsets.
static const int kPart77FramesTable = 0x67D2;

// Frame shown when a line ends; while a line goes on one of this and the next four frames is picked at random.
static const int kPart77StandingFrame = 40;

// Where the lines of the cutscene are shown.
static const int kPart77TextX = 120;
static const int kPart77TextY = 40;

// Areas that get another value for the walk that leaves the room and go back to 0 after the walks that enter it.
static const int kPart77ScriptedAreas[8] = { 5, 10, 11, 13, 16, 17, 26, 28 };
static const int kPart77ScriptedAreaValues[8] = { 1, 2, 2, 2, 3, 4, 4, 4 };

// Areas whose first light limit is changed while Igor leaves the room.
static const int kPart77LightAreas[4] = { 23, 26, 27, 32 };

// Positions of the picture whose area is set to 4 while a scripted walk from or to them is built.
static const int kPart77TopPointOffset = 115 * 320 + 65;
static const int kPart77BottomPointOffset = 140 * 320 + 36;

// Frames shown for 46 ticks each, between the frames 6 to 16 and the frames 19 to 21.
static const uint8 kPart77CutsceneFrames[6] = { 17, 18, 17, 18, 17, 18 };

void IgorEngine::PART_77_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		PART_77_ACTION_101_exitToStairs();
		break;
	case 102:
		igorSay(209, 1, 1125);
		break;
	case 103:
		igorSay(203, 3, 1122);
		break;
	case 104:
		igorSay(201, 2, 1121);
		break;
	case 105:
		PART_77_ACTION_105_leave();
		break;
	case 106:
		PART_77_ACTION_106_cutscene();
		break;
	default:
		error("PART_77_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

/**
 * The object of area 15.
 */
void IgorEngine::PART_77_APPLY_OBJECT_STATE() {
	_roomObjectAreasTable[15].object = 2;
}

/**
 * First light limit of the areas of the room that change while Igor leaves it.
 */
void IgorEngine::PART_77_SET_AREAS_LIGHT(int lum) {
	for (int i = 0; i < 4; ++i) {
		_roomObjectAreasTable[kPart77LightAreas[i]].y1Lum = lum;
	}
}

void IgorEngine::PART_77_CLEAR_ENTRY_AREAS() {
	for (int i = 0; i < 8; ++i) {
		_roomObjectAreasTable[kPart77ScriptedAreas[i]].area = 0;
	}
}

/**
 * Size of Igor by row: it grows by one every four rows from row 17 to row 85 and stays the same below.
 */
void IgorEngine::PART_77_SET_Y_SCALE_RAMP() {
	for (int i = 0x11; i <= 0x55; ++i) {
		_walkYScaleRoom[i] = ((i - 13) >> 2) + 5;
	}
	for (int i = 0x56; i <= 0x87; ++i) {
		_walkYScaleRoom[i] = 0x17;
	}
}

void IgorEngine::PART_77_SET_Y_SCALE_UNIFORM() {
	for (int i = 0x11; i <= 0x87; ++i) {
		_walkYScaleRoom[i] = 0x32;
	}
}

void IgorEngine::PART_77_DRAW_FRAME(int frame) {
	decodeAnimFrame(getAnimFrame(0, kPart77FramesTable, frame), _screenVGA, true);
}

void IgorEngine::PART_77_UPDATE_DIALOGUE(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_77_DRAW_FRAME(kPart77StandingFrame);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_77_DRAW_FRAME(kPart77StandingFrame + getRandomNumber(4));
		break;
	}
}

/**
 * Igor walks to the exit at the right of the picture and the story goes on in state 302.
 */
void IgorEngine::PART_77_ACTION_101_exitToStairs() {
	_roomObjectAreasTable[_screenLayer2[68 * 320 + 274]].area = 8;
	_roomObjectAreasTable[_screenLayer2[74 * 320 + 243]].area = 8;
	--_walkDataLastIndex;
	buildWalkPath(243, 77, 243, 74);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	--_walkDataLastIndex;
	buildWalkPath(243, 74, 274, 68);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 302;
}

/**
 * Igor walks away into the distance in three walks and the story goes on in state 40.
 */
void IgorEngine::PART_77_ACTION_105_leave() {
	for (int i = 0; i < 8; ++i) {
		_roomObjectAreasTable[kPart77ScriptedAreas[i]].area = kPart77ScriptedAreaValues[i];
	}
	--_walkDataLastIndex;
	buildWalkPath(45, 95, 41, 86);
	_walkDataCurrentIndex = 1;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	bool skipped = waitForIgorMove(0, true, false);

	PART_77_SET_AREAS_LIGHT(0x8F);
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[kPart77BottomPointOffset]].area = 4;
	buildWalkPath(41, 86, 36, 143);
	_roomObjectAreasTable[_screenLayer2[kPart77BottomPointOffset]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	skipped |= waitForIgorMove(0, true, skipped, 1);

	PART_77_SET_Y_SCALE_RAMP();
	WalkData *wd = &_walkData[0];
	wd->setPos(65, 115, 1, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 14;
	wd->scaleWidth = 23;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 23;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[kPart77TopPointOffset]].area = 4;
	buildWalkPath(65, 115, 21, 17);
	_roomObjectAreasTable[_screenLayer2[kPart77TopPointOffset]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(0, true, skipped);
	_currentPart = 40;
}

/**
 * Igor walks to (124, 88), the scene plays and the story goes on in state 760.
 */
void IgorEngine::PART_77_ACTION_106_cutscene() {
	memset(_screenVGA + 46080, 0, 17920);
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[88 * 320 + 124]].area = 5;
	buildWalkPath(_walkData[_walkDataLastIndex].x, _walkData[_walkDataLastIndex].y, 124, 88);
	_walkDataCurrentIndex = 1;
	_walkData[_walkDataLastIndex].posNum = 2;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();

	for (int i = 1; i <= 5; ++i) {
		PART_77_DRAW_FRAME(i);
		waitForTimer(31);
	}
	igorSayAndWait(206, 2, 1123);
	for (int i = 6; i <= 16; ++i) {
		PART_77_DRAW_FRAME(i);
		waitForTimer(31);
	}
	for (int i = 0; i < 6; ++i) {
		PART_77_DRAW_FRAME(kPart77CutsceneFrames[i]);
		waitForTimer(46);
	}
	for (int i = 19; i <= 21; ++i) {
		PART_77_DRAW_FRAME(i);
		waitForTimer(46);
	}
	_walkData[_walkDataLastIndex - 1].x = 155;
	_walkData[_walkDataLastIndex - 1].y = 75;
	igorSay(208, 1, 1124);
	waitForEndOfIgorDialogue(false);
	waitForTimer(255);
	for (int i = 22; i <= 39; ++i) {
		PART_77_DRAW_FRAME(i);
		waitForTimer(31);
	}
	cutsceneSayWithCallback(kPart77TextX, kPart77TextY, 63, 57, 0,
							{ { 210, 2, 1126 }, { 212, 2, 1127 }, { 214, 1, 1128 }, { 215, 2, 1129 }, { 217, 2, 1130 } },
							&IgorEngine::PART_77_UPDATE_DIALOGUE);
	waitForTimer(255);
	// the colors 240 to 255 are black on the screen
	for (int i = 240; i <= 255; ++i) {
		setPaletteColor(i, 0, 0, 0);
	}
	_currentPart = 760;
}

/**
 * State 770: Igor appears small at the top of the picture and walks in three walks to (45, 95).
 */
void IgorEngine::PART_77_ENTER_FROM_MAP() {
	PART_77_SET_Y_SCALE_RAMP();
	WalkData *wd = &_walkData[0];
	wd->setPos(21, 17, 3, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 4;
	wd->scaleWidth = 6;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 6;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[kPart77TopPointOffset]].area = 4;
	buildWalkPath(21, 17, 65, 115);
	_roomObjectAreasTable[_screenLayer2[kPart77TopPointOffset]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	// the escape key draws what is left of the walks at once
	bool skipped = waitForIgorMove(0, true, false);

	PART_77_SET_Y_SCALE_UNIFORM();
	wd->setPos(36, 140, 3, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 30;
	wd->scaleWidth = 50;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 50;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[kPart77BottomPointOffset]].area = 4;
	buildWalkPath(36, 143, 41, 86);
	_roomObjectAreasTable[_screenLayer2[kPart77BottomPointOffset]].area = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	skipped |= waitForIgorMove(0, true, skipped, 3);

	PART_77_SET_AREAS_LIGHT(0);
	--_walkDataLastIndex;
	buildWalkPath(41, 86, 45, 95);
	_walkDataCurrentIndex = 2;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove(0, true, skipped);
	PART_77_CLEAR_ENTRY_AREAS();
}

/**
 * State 771: Igor appears at (225, 100), the first step is drawn before the picture fades in, and walks to (205, 124).
 */
void IgorEngine::PART_77_ENTER_FROM_STAIRS() {
	PART_77_APPLY_OBJECT_STATE();
	WalkData *wd = &_walkData[0];
	wd->setPos(225, 100, 3, 1);
	wd->clipSkipX = 1;
	wd->clipWidth = 30;
	wd->scaleWidth = 50;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 50;
	_walkCurrentFrame = 1;
	_walkDataCurrentIndex = 0;
	_walkDataLastIndex = 0;
	buildWalkPath(225, 100, 205, 124);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	moveIgor(3, _walkData[1].frameNum);
	++_walkDataCurrentIndex;
	fadeIn(768);
	waitForIgorMove();
	PART_77_SET_AREAS_LIGHT(0);
	PART_77_CLEAR_ENTRY_AREAS();
}

void IgorEngine::PART_77() {
	playMusic(1);
	_gameState.enableLight = 1;
	loadActionData(DAT_Part77);
	loadRoomData(PAL_Part77, IMG_Part77, BOX_Part77, MSK_Part77, TXT_Part77);
	SET_PAL_240_48_1();
	static const int anim[] = { ANM_Part77, 0 };
	loadAnimData(anim);
	_roomDataOffsets = PART_77_ROOM_DATA_OFFSETS;
	setRoomClickFix(143, -1, 286, true);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_77_EXEC_ACTION);
	PART_77_APPLY_OBJECT_STATE();
	memcpy(_screenVGA, _screenLayer1, 46080);
	for (int i = 18; i <= 20; ++i) {
		_roomObjectAreasTable[i].area = 0;
	}

	if (_gameStateLoaded) {
		PART_77_SET_AREAS_LIGHT(0);
	}
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		if (_currentPart == 770) {
			fadeIn(768);
			PART_77_ENTER_FROM_MAP();
		} else {
			PART_77_ENTER_FROM_STAIRS();
		}
	}
	enterPartLoop();
	while ((_currentPart == 770 || _currentPart == 771) && !_gameStateLoaded) {
		runPartLoop();
	}
	if (!_gameStateLoaded) {
		_objectsState[54] = 1;
		if (_currentPart == 760) {
			// the colors 208 to 254 are cleared in both palettes
			memset(_paletteBuffer + 208 * 3, 0, 141);
			memset(_currentPalette + 208 * 3, 0, 141);
		}
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
