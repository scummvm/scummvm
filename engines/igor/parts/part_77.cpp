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

static const int kPart77MonkeyFramesTable = 0x67D2;

static const int kPart77MonkeyStandingFrame = 40;

static const int kPart77MonkeyTextX = 120;
static const int kPart77MonkeyTextY = 40;

static const uint8 kPart77CutsceneFrames[6] = { 17, 18, 17, 18, 17, 18 };

void IgorEngine::PART_77_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		PART_17_ACTION_101_walkIn(true);
		break;
	case 102: // look at college door
		igorSay(209, 1, 1125);
		break;
	case 103: // look at cage
		igorSay(203, 3, 1122);
		break;
	case 104: // use cage
		igorSay(201, 2, 1121);
		break;
	case 105:
		PART_17_ACTION_105_leaveThroughPath(true);
		break;
	case 106:
		PART_77_ACTION_106_monkeyCageCutscene();
		break;
	default:
		error("PART_77_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_77_APPLY_OBJECT_STATE() {
	_roomObjectAreasTable[15].object = 2;
}

void IgorEngine::PART_77_drawMonkeyFrame(int frame) {
	decodeAnimFrame(getAnimFrame(0, kPart77MonkeyFramesTable, frame), _screenVGA, true);
}

void IgorEngine::PART_77_UPDATE_DIALOGUE(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_77_drawMonkeyFrame(kPart77MonkeyStandingFrame);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_77_drawMonkeyFrame(kPart77MonkeyStandingFrame + getRandomNumber(4));
		break;
	}
}

void IgorEngine::PART_77_ACTION_106_monkeyCageCutscene() {
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
		PART_77_drawMonkeyFrame(i);
		waitForTimer(31);
	}
	igorSayAndWait(206, 2, 1123);
	for (int i = 6; i <= 16; ++i) {
		PART_77_drawMonkeyFrame(i);
		waitForTimer(31);
	}
	for (int i = 0; i < 6; ++i) {
		PART_77_drawMonkeyFrame(kPart77CutsceneFrames[i]);
		waitForTimer(46);
	}
	for (int i = 19; i <= 21; ++i) {
		PART_77_drawMonkeyFrame(i);
		waitForTimer(46);
	}
	_walkData[_walkDataLastIndex - 1].x = 155;
	_walkData[_walkDataLastIndex - 1].y = 75;
	igorSay(208, 1, 1124);
	waitForEndOfIgorDialogue(false);
	waitForTimer(255);
	for (int i = 22; i <= 39; ++i) {
		PART_77_drawMonkeyFrame(i);
		waitForTimer(31);
	}
	cutsceneSayWithCallback(kPart77MonkeyTextX, kPart77MonkeyTextY, 63, 57, 0,
							{ { 210, 2, 1126 }, { 212, 2, 1127 }, { 214, 1, 1128 }, { 215, 2, 1129 }, { 217, 2, 1130 } },
							&IgorEngine::PART_77_UPDATE_DIALOGUE);
	waitForTimer(255);
	// the colors 240 to 255 are black on the screen
	for (int i = 240; i <= 255; ++i) {
		setPaletteColor(i, 0, 0, 0);
	}
	_currentPart = 760;
}

void IgorEngine::PART_77_enterFromCollege() {
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
	PART_17_HELPER_5_changeZindexOfPath(0);
	PART_17_HELPER_2_clearEntryAreas();
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
	PART_17_HELPER_3_disableArea(0);

	if (_gameStateLoaded) {
		PART_17_HELPER_5_changeZindexOfPath(0);
	}
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		if (_currentPart == 770) {
			fadeIn(768);
			PART_17_HELPER_2_walkFromMap(true);
		} else {
			PART_77_enterFromCollege();
		}
	}
	enterPartLoop();
	while ((_currentPart == 770 || _currentPart == 771) && !_gameStateLoaded) {
		runPartLoop();
	}
	if (!_gameStateLoaded) {
		_objectsState[kPhillipAndJimmyPresentStateIndex] = 1;
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
