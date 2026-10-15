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
// the swarm, the main firefly, the transformation and the ending.
static const int kPart69SwarmFramesBase = 0;
static const int kPart69SwarmFramesTable = 0xB138;

static const int kPart69MainFireflyFramesBase = 0xB200;
static const int kPart69MainFireflyFramesTable = 0xBBA;

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

// The swarm animation is a sequence of steps (1 to 6). Each step plays the swarm frames from
// the first to the last frame of the step; every 5 swarm steps the main firefly plays too.
static const uint8 kPart69SwarmStepFirstFrame[7] = { 37, 71, 1, 41, 0, 80, 20 };
static const uint8 kPart69SwarmStepLastFrame[7] = { 0, 80, 20, 50, 100, 40, 70 };
static const uint8 kPart69SwarmStepNext[7] = { 70, 1, 2, 3, 2, 3, 1 };
static const uint8 kPart69MainFireflyStepFirstFrame[4] = { 1, 41, 1, 21 };
static const uint8 kPart69MainFireflyStepLastFrame[4] = { 0, 64, 20, 40 };

// The areas that answer to the swarm step, and the areas of the main firefly.
static const uint8 kPart69SwarmStepAreas[4] = { 0, 1, 52, 16 };
static const uint8 kPart69MainFireflyAreas[4] = { 0, 2, 72, 45 };

static const int kPart69SwarmInitialFrame = 71;
static const int kPart69MainFireflyInitialFrame = 40;
static const int kPart69TalkFrameFirst = 0x29;
static const int kPart69TalkFrameLast = 0x32;
static const int kPart69TransformFrameLast = 0x16;
static const int kPart69EndingFrameFirst = 1;
static const int kPart69EndingFrameLast = 0xE;

void IgorEngine::part69ExecuteAction(int action) {
	switch (action) {
	case 101:
		_currentPart = 721;
		break;
	case 102:
		igorSay(201, 2, 1084);
		break;
	case 103:
		igorSay({ { 203, 2, 1085 }, { 205, 1, 1086 } });
		_objectsState[kPart69LeaderFireflyIdentifiedStateIndex] = 1;
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
		part69UseVaseWithFireflies();
		break;
	case 108:
		igorSay(215, 2, 1092);
		break;
	default:
		error("part69ExecuteAction unhandled action %d", action);
		break;
	}
}

/**
 * Once the scene is over the places of the fireflies are not objects anymore.
 */
void IgorEngine::part69ApplyObjectState() {
	if (_objectsState[kPart69FireflyCapturedStateIndex] != 1) {
		return;
	}
	for (int i = 1; i <= 3; ++i) {
		_roomObjectAreasTable[kPart69SwarmStepAreas[i]].object = 0;
		_roomObjectAreasTable[kPart69MainFireflyAreas[i]].object = 5;
	}
	for (int i = 0; i <= 5; ++i) {
		if (_roomActionsTable[72 + i] == 1) {
			_roomActionsTable[72 + i] = 0;
		}
	}
}

void IgorEngine::part69DrawSwarm(int frame) {
	decodeAnimFrame(getAnimFrame(kPart69SwarmFramesBase, kPart69SwarmFramesTable, frame), _screenVGA, true);
}

void IgorEngine::part69DrawMainFirefly(int frame) {
	decodeAnimFrame(getAnimFrame(kPart69MainFireflyFramesBase, kPart69MainFireflyFramesTable, frame), _screenVGA, true);
}

void IgorEngine::part69AnimateFireflies() {
	if (!compareGameTick(13, 16)) {
		return;
	}
	if (_objectsState[kPart69FireflyCapturedStateIndex] != 0) {
		return;
	}
	uint8 &sequence = _objectsState[kPart69SwarmAnimationPhaseStateIndex];
	uint8 &secondFirefly = _objectsState[kPart69LeaderFireflyPathStateIndex];
	if (_part69FireflyStepCount == 5) {
		secondFirefly = sequence;
		_part69FireflyFrameB = kPart69MainFireflyStepFirstFrame[secondFirefly];
		_part69FireflyStepCount = 0;
	}
	if (secondFirefly > 0) {
		part69DrawMainFirefly(_part69FireflyFrameB);
		if (kPart69MainFireflyStepLastFrame[secondFirefly] == _part69FireflyFrameB) {
			secondFirefly = 0;
			sequence += 3;
		} else {
			++_part69FireflyFrameB;
		}
	}
	part69DrawSwarm(_part69FireflyFrameA);
	if (kPart69SwarmStepLastFrame[sequence] == _part69FireflyFrameA) {
		if (sequence > 3) {
			sequence = kPart69SwarmStepNext[sequence];
		}
		_part69FireflyFrameA = kPart69SwarmStepFirstFrame[sequence];
		++_part69FireflyStepCount;
	} else {
		++_part69FireflyFrameA;
	}

	if (sequence > 3) {
		for (int i = 1; i <= 3; ++i) {
			_roomObjectAreasTable[kPart69SwarmStepAreas[i]].object = 0;
		}
	} else {
		for (int i = 1; i <= 3; ++i) {
			_roomObjectAreasTable[kPart69SwarmStepAreas[i]].object = sequence == i ? 3 : 0;
		}
	}
	if (secondFirefly > 0) {
		for (int i = 1; i <= 3; ++i) {
			_roomObjectAreasTable[kPart69MainFireflyAreas[i]].object = 5;
		}
	} else {
		for (int i = 1; i <= 3; ++i) {
			const bool answers = kPart69SwarmStepNext[sequence] == i && _objectsState[kPart69LeaderFireflyIdentifiedStateIndex] == 1;
			_roomObjectAreasTable[kPart69MainFireflyAreas[i]].object = answers ? 4 : 5;
		}
	}
}


void IgorEngine::part69TakeVaseBack(int num, int sound) {
	for (int frame = kPart69EndingFrameLast; frame <= kPart69EndingFrameLast + 1; ++frame) {
		decodeAnimFrame(getAnimFrame(kPart69FramesDBase, kPart69FramesDTable, frame), _screenVGA, true);
		waitForTimer(46);
	}
	igorSay(num, 2, sound);
}


void IgorEngine::part69UseVaseWithFireflies() {
	drawAnimRect(kPart69PictureOffset, kPart69PictureAnimOffset, kPart69PictureWidth, kPart69PictureHeight);
	if (_objectsState[kVaseStateIndex] == 0) {
		for (int i = 0; i < 1000 / kTimerTicksCount && !_eventQuitGame; ++i) {
			part69AnimateFireflies();
			waitForTimer();
		}
		part69TakeVaseBack(211, 1090);
		return;
	}

	do {
		part69AnimateFireflies();
		waitForTimer();
	} while (!(_objectsState[kPart69SwarmAnimationPhaseStateIndex] == 3 && _objectsState[kPart69LeaderFireflyPathStateIndex] == 0) && !_eventQuitGame);

	uint8 frameC = 1;
	do {
		if (compareGameTick(13, 16)) {
			part69DrawSwarm(_part69FireflyFrameA);
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
	_objectsState[kPart69FireflyCapturedStateIndex] = 1;
	_objectsState[kVaseStateIndex] = 2;
	partMainUpdateObjectState(7);
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	playSound(63, 1);
	while (_mixer->isSoundHandleActive(_sfxHandle) && !_eventQuitGame) {
		waitForTimer(1);
	}
	playSound(51, 1);
	part69ApplyObjectState();
	part69TakeVaseBack(213, 1091);
}

/**
 * Igor comes from the right edge, walking to the left, while the fireflies move.
 */
void IgorEngine::part69EnterFromRight() {
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
	waitForIgorMove(&IgorEngine::part69AnimateFireflies);
}

void IgorEngine::part69UpdateBackground() {
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
	part69AnimateFireflies();
}

void IgorEngine::part69() {
	if (isCDVersion())
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
	partMainSetExecuteAction(1, &IgorEngine::part69ExecuteAction);
	_updateRoomBackground = &IgorEngine::part69UpdateBackground;
	part69ApplyObjectState();
	memcpy(_screenVGA, _screenLayer1, 46080);
	if (_objectsState[kPart69FireflyCapturedStateIndex] == 0) {
		part69DrawSwarm(kPart69SwarmInitialFrame);
		part69DrawMainFirefly(kPart69MainFireflyInitialFrame);
	}
	_part69FireflyStepCount = 0;
	_part69FireflyFrameB = 0;
	_objectsState[kPart69SwarmAnimationPhaseStateIndex] = 1;
	_objectsState[kPart69LeaderFireflyPathStateIndex] = 0;
	_part69FireflyFrameA = kPart69SwarmInitialFrame;

	// the whole picture is darker than its palette
	darkenPalette(_paletteBuffer, 1, 175, 10);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		part69EnterFromRight();
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
