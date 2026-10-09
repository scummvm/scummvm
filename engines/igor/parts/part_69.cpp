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

// The animation resource is made of several blocks, each one a table of frame offsets and the frames it points to:
// the first and the second firefly of the room, the transformation and the ending.
static const int kPart69FireflyAFramesBase = 0;
static const int kPart69FireflyAFramesTable = 0xB138;
static const int kPart69FireflyBFramesBase = 0xB200;
static const int kPart69FireflyBFramesTable = 0xBBA;
static const int kPart69FramesCBase = 0xBE3A;
static const int kPart69FramesCTable = 0x3FF;
static const int kPart69FramesDBase = 0xC265;
static const int kPart69FramesDTable = 0x179C;
static const int kPart69PictureAnimOffset = 0xDA1F;
static const int kPart69PictureOffset = 69 * 320 + 148;
static const int kPart69PictureWidth = 27;
static const int kPart69PictureHeight = 49;

// The walk-in: Igor comes from the right edge.
static const int kPart69EntryY = 130;
static const int kPart69EntryDstX = 259;

// objectsState
static const int kPart69SceneDoneState = 96;
static const int kPart69FlagState = 97;
static const int kPart69SequenceState = 98;
static const int kPart69SecondFireflyState = 99;
static const int kPart69RewardState = 6;

// The animation of the two fireflies is a sequence of steps (1 to 6). Each step plays the frames of the first firefly from
// the first to the last frame of the step; every 5 steps of the first firefly the second firefly plays too. The values come
// from the data segment of the executable (index 0 is not used).
static const uint8 kPart69StepFirstFireflyFrameA[7] = { 37, 71, 1, 41, 0, 80, 20 };
static const uint8 kPart69StepLastFireflyFrameA[7] = { 0, 80, 20, 50, 100, 40, 70 };
static const uint8 kPart69StepNext[7] = { 70, 1, 2, 3, 2, 3, 1 };
static const uint8 kPart69StepFirstFireflyFrameB[4] = { 1, 41, 1, 21 };
static const uint8 kPart69StepLastFireflyFrameB[4] = { 0, 64, 20, 40 };
// The areas that answer to the step, and the areas of the second firefly.
static const uint8 kPart69StepAreas[4] = { 0, 1, 52, 16 };
static const uint8 kPart69SecondFireflyAreas[4] = { 0, 2, 72, 45 };

static const int kPart69FirstFireflyFrameA = 71;
static const int kPart69FirstFireflyFrameB = 40;
static const int kPart69TalkFrameFirst = 0x29;
static const int kPart69TalkFrameLast = 0x32;
static const int kPart69TransformFrameLast = 0x16;
static const int kPart69EndingFrameFirst = 1;
static const int kPart69EndingFrameLast = 0xE;

void IgorEngine::PART_69_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_currentPart = 721;
		break;
	case 102:
		igorSay(201, 2, 1084);
		break;
	case 103:
		igorSay({ { 203, 2, 1085 }, { 205, 1, 1086 } });
		_objectsState[kPart69FlagState] = 1;
		break;
	case 104:
		igorSay(206, 2, 1087);
		break;
	case 105:
		igorSay(208, 2, 1088);
		break;
	case 106:
		igorSay(210, 1, 1089);
		break;
	case 107:
		PART_69_ACTION_107_watch();
		break;
	case 108:
		igorSay(215, 2, 1092);
		break;
	default:
		error("PART_69_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

/**
 * Once the scene is over the places of the fireflies are not objects anymore.
 */
void IgorEngine::PART_69_APPLY_OBJECT_STATE() {
	if (_objectsState[kPart69SceneDoneState] != 1) {
		return;
	}
	for (int i = 1; i <= 3; ++i) {
		_roomObjectAreasTable[kPart69StepAreas[i]].object = 0;
		_roomObjectAreasTable[kPart69SecondFireflyAreas[i]].object = 5;
	}
	for (int i = 0; i <= 5; ++i) {
		if (_roomActionsTable[72 + i] == 1) {
			_roomActionsTable[72 + i] = 0;
		}
	}
}

void IgorEngine::PART_69_DRAW_FIREFLY_A(int frame) {
	decodeAnimFrame(getAnimFrame(kPart69FireflyAFramesBase, kPart69FireflyAFramesTable, frame), _screenVGA, true);
}

void IgorEngine::PART_69_DRAW_FIREFLY_B(int frame) {
	decodeAnimFrame(getAnimFrame(kPart69FireflyBFramesBase, kPart69FireflyBFramesTable, frame), _screenVGA, true);
}

/**
 * Runs the sequence of the two fireflies, one frame of each every 16 ticks, and sets which places answer to the
 * fireflies.
 */
void IgorEngine::PART_69_ANIMATE_FIREFLIES() {
	if (!compareGameTick(13, 16)) {
		return;
	}
	if (_objectsState[kPart69SceneDoneState] != 0) {
		return;
	}
	uint8 &sequence = _objectsState[kPart69SequenceState];
	uint8 &secondFirefly = _objectsState[kPart69SecondFireflyState];
	if (_part69FireflyStepCount == 5) {
		secondFirefly = sequence;
		_part69FireflyFrameB = kPart69StepFirstFireflyFrameB[secondFirefly];
		_part69FireflyStepCount = 0;
	}
	if (secondFirefly > 0) {
		PART_69_DRAW_FIREFLY_B(_part69FireflyFrameB);
		if (kPart69StepLastFireflyFrameB[secondFirefly] == _part69FireflyFrameB) {
			secondFirefly = 0;
			sequence += 3;
		} else {
			++_part69FireflyFrameB;
		}
	}
	PART_69_DRAW_FIREFLY_A(_part69FireflyFrameA);
	if (kPart69StepLastFireflyFrameA[sequence] == _part69FireflyFrameA) {
		if (sequence > 3) {
			sequence = kPart69StepNext[sequence];
		}
		_part69FireflyFrameA = kPart69StepFirstFireflyFrameA[sequence];
		++_part69FireflyStepCount;
	} else {
		++_part69FireflyFrameA;
	}

	if (sequence > 3) {
		for (int i = 1; i <= 3; ++i) {
			_roomObjectAreasTable[kPart69StepAreas[i]].object = 0;
		}
	} else {
		for (int i = 1; i <= 3; ++i) {
			_roomObjectAreasTable[kPart69StepAreas[i]].object = sequence == i ? 3 : 0;
		}
	}
	if (secondFirefly > 0) {
		for (int i = 1; i <= 3; ++i) {
			_roomObjectAreasTable[kPart69SecondFireflyAreas[i]].object = 5;
		}
	} else {
		for (int i = 1; i <= 3; ++i) {
			const bool answers = kPart69StepNext[sequence] == i && _objectsState[kPart69FlagState] == 1;
			_roomObjectAreasTable[kPart69SecondFireflyAreas[i]].object = answers ? 4 : 5;
		}
	}
}

/**
 * The end of the scene: the last frames of the ending and what Igor says.
 */
void IgorEngine::PART_69_FINALE(int num, int sound) {
	for (int frame = kPart69EndingFrameLast; frame <= kPart69EndingFrameLast + 1; ++frame) {
		decodeAnimFrame(getAnimFrame(kPart69FramesDBase, kPart69FramesDTable, frame), _screenVGA, true);
		waitForTimer(46);
	}
	igorSay(num, 2, sound);
}

/**
 * Igor watches the fireflies. If the reward has not been given yet nothing happens for a while; otherwise the first
 * firefly transforms and the scene ends.
 */
void IgorEngine::PART_69_ACTION_107_watch() {
	drawAnimRect(kPart69PictureOffset, kPart69PictureAnimOffset, kPart69PictureWidth, kPart69PictureHeight);
	if (_objectsState[kPart69RewardState] == 0) {
		for (int i = 0; i < 1000 / kTimerTicksCount && !_eventQuitGame; ++i) {
			PART_69_ANIMATE_FIREFLIES();
			waitForTimer();
		}
		PART_69_FINALE(211, 1090);
		return;
	}

	do {
		PART_69_ANIMATE_FIREFLIES();
		waitForTimer();
	} while (!(_objectsState[kPart69SequenceState] == 3 && _objectsState[kPart69SecondFireflyState] == 0) && !_eventQuitGame);

	uint8 frameC = 1;
	do {
		if (compareGameTick(13, 16)) {
			PART_69_DRAW_FIREFLY_A(_part69FireflyFrameA);
			_part69FireflyFrameA = _part69FireflyFrameA == kPart69TalkFrameLast ? kPart69TalkFrameFirst : _part69FireflyFrameA + 1;
			decodeAnimFrame(getAnimFrame(kPart69FramesCBase, kPart69FramesCTable, frameC), _screenVGA, true);
			if (frameC < kPart69TransformFrameLast) {
				++frameC;
			}
		}
		waitForTimer();
	} while (!(frameC == kPart69TransformFrameLast && _part69FireflyFrameA == kPart69TalkFrameFirst) && !_eventQuitGame);

	_part69FireflyFrameA = kPart69EndingFrameFirst;
	do {
		if (compareGameTick(13, 16)) {
			decodeAnimFrame(getAnimFrame(kPart69FramesDBase, kPart69FramesDTable, _part69FireflyFrameA), _screenVGA, true);
			++_part69FireflyFrameA;
		}
		waitForTimer();
	} while (_part69FireflyFrameA != kPart69EndingFrameLast && !_eventQuitGame);

	waitForTimer(255);
	_objectsState[kPart69SceneDoneState] = 1;
	_objectsState[kPart69RewardState] = 2;
	UPDATE_OBJECT_STATE(7);
	drawInventory(_inventoryInfo[72], 0);
	playSound(63, 1);
	while (_mixer->isSoundHandleActive(_sfxHandle) && !_eventQuitGame) {
		waitForTimer(1);
	}
	playSound(51, 1);
	PART_69_APPLY_OBJECT_STATE();
	PART_69_FINALE(213, 1091);
}

/**
 * Igor comes from the right edge, walking to the left, while the fireflies move.
 */
void IgorEngine::PART_69_ENTER_FROM_RIGHT() {
	WalkData *wd = &_walkData[0];
	wd->setPos(319, kPart69EntryY, kFacingPositionLeft, 0);
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
	buildWalkPath(319, kPart69EntryY, kPart69EntryDstX, kPart69EntryY);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove(&IgorEngine::PART_69_ANIMATE_FIREFLIES);
}

void IgorEngine::PART_69_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(1)) {
		if (getRandomNumber(14) == 0 && !isDialogueSpeechPlaying()) {
			switch (getRandomNumber(2)) {
			case 0:
				playSound(36, 1);
				break;
			case 1:
				playSound(37, 1);
				break;
			case 2:
				playSound(38, 1);
				break;
			}
		}
	}
	PART_69_ANIMATE_FIREFLIES();
}

void IgorEngine::PART_69() {
	playMusic(4);
	_gameState.enableLight = 2;
	loadActionData(DAT_Part69);
	loadRoomData(PAL_Part69, IMG_Part69, BOX_Part69, MSK_Part69, TXT_Part69);
	// the colors of Igor
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	SET_PAL_240_48_1();
	static const int anim[] = { ANM_Part69, 0 };
	loadAnimData(anim);
	_roomDataOffsets = PART_69_ROOM_DATA_OFFSETS;
	setRoomClickFix(130, 166, -1, false, 130);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_69_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_69_UPDATE_ROOM_BACKGROUND;
	PART_69_APPLY_OBJECT_STATE();
	memcpy(_screenVGA, _screenLayer1, 46080);
	if (_objectsState[kPart69SceneDoneState] == 0) {
		PART_69_DRAW_FIREFLY_A(kPart69FirstFireflyFrameA);
		PART_69_DRAW_FIREFLY_B(kPart69FirstFireflyFrameB);
	}
	_part69FireflyStepCount = 0;
	_part69FireflyFrameB = 0;
	_objectsState[kPart69SequenceState] = 1;
	_objectsState[kPart69SecondFireflyState] = 0;
	_part69FireflyFrameA = kPart69FirstFireflyFrameA;

	// the whole picture is darker than its palette
	darkenPalette(_paletteBuffer, 1, 175, 10);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		PART_69_ENTER_FROM_RIGHT();
	}
	enterPartLoop();
	while (_currentPart == 690 && !_gameStateLoaded) {
		runPartLoop();
	}
	// the colors that changed are kept
	memcpy(_paletteBuffer, _currentPalette, 624);
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
