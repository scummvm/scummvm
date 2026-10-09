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

// A character stands in the middle of the room. His head is a 21x53 picture drawn at (140,28); the animation
// resource has the frames from 1 on, 1113 bytes each, after a base offset.
static const int kPart81HeadOffset = 28 * 320 + 140;
static const int kPart81HeadWidth = 21;
static const int kPart81HeadHeight = 53;
static const int kPart81HeadFramesBase = 0x1AE5;
static const int kPart81HeadFrameSize = kPart81HeadWidth * kPart81HeadHeight;
static const int kPart81HeadStanding = 4;

// Another picture of the head, drawn into the room picture while it is there.
static const int kPart81HeadRestFrame = 1;

// A picture (43x62, frames of 2666 bytes) at (116,32).
static const int kPart81FigureOffset = 32 * 320 + 116;
static const int kPart81FigureWidth = 43;
static const int kPart81FigureHeight = 62;
static const int kPart81FigureFrameSize = kPart81FigureWidth * kPart81FigureHeight;
static const int kPart81FigureFirst = 0xC4DF;
static const int kPart81FigureSecond = 0xBA75;

// The frames of the head that follow the story: 6 frames of the third block of the animation.
static const int kPart81StoryFramesBase = 0x4206;

// Where the lines of the character are shown.
static const int kPart81TextX = 148;
static const int kPart81TextY = 31;
static const int kPart81TextR = 63;
static const int kPart81TextG = 32;
static const int kPart81TextB = 0;

// The scenes use a tall picture (130x369) at the start of the second animation and a smaller one (285x72) in the
// walk mask buffer.
static const int kPart81TallWidth = 0x82;
static const int kPart81TallX = 0x6A;
static const int kPart81StripWidth = 0x11D;
static const int kPart81StripRows = 0x48;

// The last frames of the animation of the end of the story.
static const int kPart81EndFramesBase = 0xBAE0;
static const int kPart81EndFramesTable = 0x808;

// Flags of the story kept in the objects state.
static const int kPart81StoryState = 6;
static const int kPart81SideState = 90;
static const int kPart81InventoryState = 69;
static const int kPart81EventDoneState = 104;
static const int kPart81BellState = 105;

// First picture of each of the steps of the animation of the story.
static const uint8 kPart81FigureFrames[4] = { 1, 2, 3, 1 };
// Frames and times of the end scene (timer ticks).
static const uint8 kPart81EndFrames[10] = { 1, 2, 1, 2, 1, 2, 1, 2, 1, 2 };
static const uint8 kPart81EndHold[10] = { 45, 35, 25, 40, 45, 30, 45, 35, 20, 30 };
static const uint8 kPart81EndFinalHold[10] = { 25, 30, 20, 40, 25, 35, 40, 42, 20, 25 };

static const int kPart81StepTicks = 16;

void IgorEngine::PART_81_LOAD_ROOM() {
	loadRoomData(PAL_Part81, IMG_Part81, BOX_Part81, MSK_Part81, TXT_Part81);
	SET_PAL_240_48_1();
}

void IgorEngine::PART_81_LOAD_ANIMATION() {
	static const int anim[] = { ANM_Part81, 0 };
	loadAnimData(anim);
}

void IgorEngine::PART_81_DRAW_HEAD(int frame) {
	drawAnimRect(kPart81HeadOffset, kPart81HeadFramesBase + frame * kPart81HeadFrameSize, kPart81HeadWidth, kPart81HeadHeight,
				 true, kBlendBehindIgorAndText);
	if (_gameState.dialogueTextRunning) {
		memcpy(_screenLayer1 + _dialogueDirtyRectY, _screenTextLayer + 23040, _dialogueDirtyRectSize);
	}
}

void IgorEngine::PART_81_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(0x3D) && getRandomNumber(1) == 0 && _objectsState[kPart81BellState] == 1) {
		PART_81_DRAW_HEAD(getRandomNumber(2) + 1);
	}
}

void IgorEngine::PART_81_UPDATE_DIALOGUE_NPC(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		PART_81_DRAW_HEAD(kPart81HeadStanding);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_81_DRAW_HEAD(getRandomNumber(4) + 5);
		break;
	}
}

/**
 * The character says lines of the room texts. The lines of the room itself do not run meanwhile.
 */
void IgorEngine::PART_81_NPC_SAY(const Common::Array<DialogueText> &lines) {
	UpdateRoomBackgroundProc previous = _updateRoomBackground;
	_updateRoomBackground = 0;
	cutsceneSayWithCallback(kPart81TextX, kPart81TextY, kPart81TextR, kPart81TextG, kPart81TextB, lines,
							&IgorEngine::PART_81_UPDATE_DIALOGUE_NPC);
	_updateRoomBackground = previous;
}

void IgorEngine::PART_81_IGOR_SAY(int num, int count, int sound) {
	UpdateRoomBackgroundProc previous = _updateRoomBackground;
	_updateRoomBackground = 0;
	igorSayAndWait(num, count, sound);
	_updateRoomBackground = previous;
}

/**
 * The picture of the character at rest is part of the room picture while the bell is on; the dialogue and the
 * objects of the room depend on the story.
 */
void IgorEngine::PART_81_APPLY_OBJECT_STATE(int num) {
	if (num != 1 && num != 255) {
		return;
	}
	if (_objectsState[kPart81BellState] == 1) {
		copyArea(_screenLayer1, kPart81HeadOffset, 320, _animFramesBuffer + kPart81HeadRestFrame * kPart81HeadFrameSize + 0x1AE5,
				 kPart81HeadWidth, kPart81HeadWidth, kPart81HeadHeight);
		_roomActionsTable[30] = 2;
		_roomActionsTable[99] = 0;
		_roomActionsTable[100] = 0;
		_roomActionsTable[101] = 0;
		_roomActionsTable[102] = 0;
	} else {
		_roomActionsTable[30] = 4;
		_roomActionsTable[91] = 0;
		_roomActionsTable[92] = 0;
		_roomActionsTable[99] = 0x68;
		_roomActionsTable[100] = 1;
		_roomActionsTable[101] = 0xE;
		_roomActionsTable[102] = 1;
		for (int i = 1; i <= 0x23; ++i) {
			for (int j = 1; j <= 2; ++j) {
				_roomActionsTable[82 * i + j + 3258] = 0;
			}
		}
		_roomObjectAreasTable[7].y2Lum = 0xEC;
	}
}

/**
 * The room picture comes in from the left, over the one that was on the screen (a 160 columns picture).
 */
void IgorEngine::PART_81_SCROLL_IN() {
	uint8 *buffer = _screenTextLayer;
	for (int step = 20; step >= 0; --step) {
		const int shift = step * 8;
		for (int row = 0; row <= 0x8F; ++row) {
			memcpy(buffer + row * 320, _screenLayer1 + row * 320 + shift, 320 - shift);
			memcpy(buffer + row * 320 + (320 - shift), _animFramesBuffer + row * 160 + 0x6075, shift);
		}
		memcpy(_screenVGA, buffer, 46080);
		if (step == 20) {
			fadeIn(768);
		} else {
			waitForTimer(step == 0 ? 1 : kPart81StepTicks);
		}
	}
}

/**
 * Transition: the screen moves down while the small picture of the walk mask buffer, then the tall picture of
 * the animation, come in from above.
 */
void IgorEngine::PART_81_TRANSITION_IN() {
	uint8 *buffer = _screenTextLayer;
	memset(buffer, 0, 46080);
	for (int i = 1; i <= 9; ++i) {
		memcpy(buffer + 8 * i * 320, _screenVGA + (8 * i - 8) * 320, (0x90 - 8 * i) * 320);
		int dstRow = 0;
		for (int row = kPart81StripRows - 8 * i; row <= kPart81StripRows - 1; ++row, ++dstRow) {
			memcpy(buffer + dstRow * 320, _screenLayer2 + row * kPart81StripWidth, kPart81StripWidth);
		}
		memcpy(_screenVGA, buffer, 46080);
		waitForTimer(i == 9 ? 1 : kPart81StepTicks);
	}

	memset(buffer, 0, 46080);
	for (int i = 1; i <= 18; ++i) {
		memcpy(buffer + 8 * i * 320, _screenVGA + (8 * i - 8) * 320, (0x90 - 8 * i) * 320);
		int dstRow = 0;
		for (int row = 0x170 - 8 * i; row <= 0x16F; ++row, ++dstRow) {
			memset(buffer + dstRow * 320, 0, kPart81TallX);
			memcpy(buffer + dstRow * 320 + kPart81TallX, _animFramesBuffer + row * kPart81TallWidth, kPart81TallWidth);
		}
		memcpy(_screenVGA, buffer, 46080);
		waitForTimer(i == 18 ? 1 : kPart81StepTicks);
	}

	for (int i = 1; i <= 28; ++i) {
		int dstRow = 0;
		for (int row = 0xE0 - 8 * i; row <= 0xE0 - 8 * i + 0x8F; ++row, ++dstRow) {
			memcpy(buffer + dstRow * 320 + kPart81TallX, _animFramesBuffer + row * kPart81TallWidth, kPart81TallWidth);
		}
		memcpy(_screenVGA, buffer, 46080);
		waitForTimer(i == 28 ? 1 : kPart81StepTicks);
	}
}

/**
 * The opposite transition: the tall picture goes down, then the small picture and finally the room picture come in.
 */
void IgorEngine::PART_81_TRANSITION_OUT() {
	uint8 *buffer = _screenTextLayer;
	for (int n = 0x2E; n >= 0x12; --n) {
		int dstRow = 0;
		for (int row = 0x170 - 8 * n; row <= 0x170 - 8 * n + 0x8F; ++row, ++dstRow) {
			memcpy(buffer + dstRow * 320 + kPart81TallX, _animFramesBuffer + row * kPart81TallWidth, kPart81TallWidth);
		}
		memcpy(_screenVGA, buffer, 46080);
		waitForTimer(n == 0x12 ? 1 : kPart81StepTicks);
	}

	for (int n = 8; n >= 0; --n) {
		int dstRow = 8 * n + kPart81StripRows;
		for (int row = 0; row <= kPart81StripRows - 1 - 8 * n; ++row, ++dstRow) {
			memcpy(buffer + dstRow * 320, _screenLayer2 + row * kPart81StripWidth, kPart81StripWidth);
		}
		dstRow = 0;
		for (int row = 0x128 - 8 * n; row <= 0x16F; ++row, ++dstRow) {
			memcpy(buffer + dstRow * 320 + kPart81TallX, _animFramesBuffer + row * kPart81TallWidth, kPart81TallWidth);
		}
		memcpy(_screenVGA, buffer, 46080);
		waitForTimer(n == 0 ? 1 : kPart81StepTicks);
	}

	for (int n = 17; n >= 0; --n) {
		memcpy(buffer + 8 * n * 320, _screenLayer1, (0x90 - 8 * n) * 320);
		memcpy(buffer, _screenVGA + 0xA00, 8 * n * 320);
		memcpy(_screenVGA, buffer, 46080);
		waitForTimer(n == 0 ? 1 : kPart81StepTicks);
	}
}

void IgorEngine::PART_81_DRAW_END_FRAME(int frame) {
	decodeAnimFrame(getAnimFrame(kPart81EndFramesBase, kPart81EndFramesTable, frame), _screenVGA, false);
}

/**
 * The end of the story: the transition, the long picture, the scene of the two characters and back.
 */
void IgorEngine::PART_81_END_SCENE() {
	PART_81_TRANSITION_IN();
	for (int i = 0; i < 3; ++i) {
		waitForTimer(251);
	}
	for (int i = 0; i < 10; ++i) {
		PART_81_DRAW_END_FRAME(kPart81EndFrames[i]);
		waitForTimer(kPart81EndHold[i]);
	}
	waitForTimer(251);
	PART_74_CUTSCENE();
	for (int i = 0; i < 10; ++i) {
		for (int frame = 3; frame <= 8; ++frame) {
			PART_81_DRAW_END_FRAME(frame);
			waitForTimer(10);
		}
		waitForTimer(kPart81EndFinalHold[i]);
	}
	PART_81_TRANSITION_OUT();
}

void IgorEngine::PART_81_DRAW_STORY_HEAD(int frame) {
	copyArea(_screenVGA, kPart81HeadOffset, 320, _animFramesBuffer + kPart81StoryFramesBase + frame * kPart81HeadFrameSize,
			 kPart81HeadWidth, kPart81HeadWidth, kPart81HeadHeight);
}

void IgorEngine::PART_81_DRAW_FIGURE(int offset) {
	copyArea(_screenVGA, kPart81FigureOffset, 320, _animFramesBuffer + offset, kPart81FigureWidth, kPart81FigureWidth,
			 kPart81FigureHeight);
}

/**
 * Looking at the picture on the wall before the story is complete.
 */
void IgorEngine::PART_81_ACTION_110() {
	for (int frame = 3; frame <= 4; ++frame) {
		PART_81_DRAW_HEAD(frame);
		waitForTimer(31);
	}
	PART_81_IGOR_SAY(208, 1, 952);
	PART_81_DRAW_FIGURE(kPart81FigureFirst);
	PART_81_NPC_SAY({ { 209, 1, 953 } });
	PART_81_DRAW_FIGURE(kPart81FigureSecond);
}

/**
 * The story of the room: Igor shows what he has, the two characters talk and the end scene plays.
 */
void IgorEngine::PART_81_ACTION_109() {
	if (_objectsState[kPart81StoryState] <= 1) {
		PART_81_ACTION_110();
		return;
	}
	memset(_screenVGA + 46080, 0, 17920);
	for (int frame = 3; frame <= 4; ++frame) {
		PART_81_DRAW_HEAD(frame);
		waitForTimer(31);
	}
	PART_81_IGOR_SAY(208, 1, 952);
	for (int step = 0; step < 4; ++step) {
		PART_81_DRAW_FIGURE((kPart81FigureFrames[step] - 1) * kPart81FigureFrameSize);
		waitForTimer(61);
	}

	// the object that Igor brought is given
	if (_inventoryInfo[kPart81InventoryState] != 0) {
		_inventoryInfo[_inventoryInfo[kPart81InventoryState] - 1] = 0;
		_inventoryInfo[kPart81InventoryState] = 0;
		packInventory();
		if (_inventoryInfo[72] > _inventoryInfo[73])
			_inventoryInfo[72] = _inventoryOffsetTable[(_inventoryInfo[73] - 1) / 7];
	}
	PART_81_NPC_SAY({ { 210, 1, 954 }, { 211, 1, 955 } });
	PART_81_IGOR_SAY(212, 1, 956);
	PART_81_NPC_SAY({ { 213, 1, 957 }, { 214, 1, 958 } });
	PART_81_IGOR_SAY(215, 1, 959);
	PART_81_NPC_SAY({ { 216, 1, 960 }, { 217, 1, 961 } });
	PART_81_IGOR_SAY(218, 1, 962);
	PART_81_NPC_SAY({ { 219, 2, 963 }, { 221, 2, 964 }, { 223, 1, 965 } });
	playSound(14, 1);
	for (int frame = 1; frame <= 3; ++frame) {
		PART_81_DRAW_STORY_HEAD(frame);
		waitForTimer(31);
	}

	// the scene of the end, with the pictures of its own
	static const int cutsceneAnim[] = { ANM_Part81Cutscene, 0 };
	loadAnimData(cutsceneAnim);
	loadData(IMG_Part81Strip, _screenLayer2);
	memcpy(_screenLayer1, _screenVGA, 46080);
	PART_81_END_SCENE();

	// the room again
	loadActionData(DAT_Part81);
	PART_81_LOAD_ROOM();
	PART_81_LOAD_ANIMATION();
	PART_81_APPLY_OBJECT_STATE(255);
	for (int frame = 4; frame <= 6; ++frame) {
		PART_81_DRAW_STORY_HEAD(frame);
		if (frame == 6) {
			playSound(13, 1);
		}
		waitForTimer(31);
	}
	while (_mixer->isSoundHandleActive(_sfxHandle) && !_eventQuitGame) {
		waitForTimer(1);
	}
	PART_81_NPC_SAY({ { 224, 1, 966 }, { 225, 1, 967 }, { 226, 3, 968 } });
	PART_81_IGOR_SAY(229, 1, 969);
	PART_81_NPC_SAY({ { 230, 1, 970 } });
	_objectsState[kPart81EventDoneState] = 1;
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	PART_81_APPLY_OBJECT_STATE(255);
}

void IgorEngine::PART_81_DIALOGUE_CODE(int code) {
	uint8 *data = _gameState.dialogueData;
	if (code == 10) {
		data[0] = 0;
	}
	if (code == 20) {
		data[6] = 0;
	}
	if (code == 30) {
		data[12] = 0;
	}
}

void IgorEngine::PART_81_CONVERSATION() {
	UpdateRoomBackgroundProc previousBackground = _updateRoomBackground;
	_updateRoomBackground = 0;
	_updateDialogue = &IgorEngine::PART_81_UPDATE_DIALOGUE_NPC;
	loadDialogueData(DLG_Part81);
	_gameState.dialogueStarted = true;
	_gameState.dialogueChoiceStart = 1;
	_gameState.dialogueChoiceCount = 1;

	bool done = false;
	do {
		drawDialogueChoices();
		PART_81_DRAW_HEAD(kPart81HeadStanding);
		const int choice = selectDialogue();
		if (choice == 0) {
			break;
		}
		if (choice == -1) {
			_updateDialogue = 0;
			_updateRoomBackground = previousBackground;
			return;
		}
		_dialogueChoiceSelected = choice;
		dialogueAskQuestion();
		dialogueReplyToQuestion(kPart81TextX, kPart81TextY, kPart81TextR, kPart81TextG, kPart81TextB);
		const int offset = (_dialogueInfo[choice] - 1) * 6 + (_gameState.dialogueChoiceCount - 1) * 30 +
						   (_gameState.dialogueChoiceStart - 1) * 30;
		const int code = _gameState.dialogueData[offset + 5];
		if (code >= 1 && code <= 99) {
			PART_81_DIALOGUE_CODE(code);
		}
		switch (_gameState.dialogueData[offset + 2]) {
		case 1:
			_gameState.dialogueChoiceCount = _gameState.dialogueData[offset + 1];
			++_gameState.dialogueChoiceStart;
			break;
		case 2:
			_gameState.dialogueChoiceCount = _gameState.dialogueData[offset + 1];
			--_gameState.dialogueChoiceStart;
			break;
		case 4:
			_gameState.dialogueChoiceCount = _gameState.dialogueData[offset + 1];
			_gameState.dialogueChoiceStart -= 2;
			break;
		case 0:
			done = true;
			break;
		}
	} while (!done);

	memset(_screenVGA + 46080, 0, 17920);
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	_currentAction.verb = kVerbWalk;
	_gameState.dialogueStarted = false;
	_updateDialogue = 0;
	_updateRoomBackground = previousBackground;
}

void IgorEngine::PART_81_ACTION_102_talk() {
	if (_objectsState[kPart81BellState] == 1 && _objectsState[kPart81EventDoneState] == 1) {
		igorSay(234, 1, 974);
		return;
	}
	PART_81_IGOR_SAY(231, 1, 971);
	PART_81_NPC_SAY({ { 232, 1, 972 } });
	PART_81_CONVERSATION();
	PART_81_APPLY_OBJECT_STATE(255);
}

void IgorEngine::PART_81_ENTER() {
	WalkData *wd = &_walkData[0];
	wd->setPos(0, 138, kFacingPositionRight, 0);
	wd->clipSkipX = 15;
	wd->clipWidth = 15;
	wd->scaleWidth = 50;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 50;
	_walkDataLastIndex = 0;
	buildWalkPath(0, 138, 30, 138);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_81_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_objectsState[kPart81SideState] = 1;
		_currentPart = 700;
		break;
	case 102:
		PART_81_ACTION_102_talk();
		break;
	case 103:
		if (_objectsState[kPart81BellState] == 1) {
			igorSay(202, 1, 946);
		} else {
			igorSay(201, 1, 945);
		}
		break;
	case 104:
		igorSay(233, 1, 973);
		break;
	case 106:
		igorSay(203, 1, 947);
		break;
	case 107:
		igorSay(204, 1, 948);
		break;
	case 108:
		if (_objectsState[kPart81EventDoneState] == 0) {
			igorSay({ { 206, 1, 950 }, { 207, 1, 951 } });
		} else {
			igorSay(205, 1, 949);
		}
		break;
	case 109:
		PART_81_ACTION_109();
		break;
	case 110:
		PART_81_ACTION_110();
		break;
	default:
		error("PART_81_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_81() {
	playMusic(4);
	_gameState.enableLight = 2;
	loadActionData(DAT_Part81);
	PART_81_LOAD_ROOM();
	PART_81_LOAD_ANIMATION();
	_roomDataOffsets = PART_81_ROOM_DATA_OFFSETS;
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_81_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_81_UPDATE_ROOM_BACKGROUND;
	PART_81_APPLY_OBJECT_STATE(255);

	// the colors that change are darker than their palette
	for (int i = 192 * 3; i < 208 * 3; ++i) {
		const int value = _paletteBuffer[i] - 20;
		_paletteBuffer[i] = value < 1 ? 0 : value;
	}
	memcpy(_screenVGA, _screenLayer1, 46080);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		PART_81_SCROLL_IN();
		PART_81_ENTER();
	}
	enterPartLoop();
	while (_currentPart == 810 && !_gameStateLoaded) {
		runPartLoop();
	}
	if (!_gameStateLoaded && _objectsState[kPart81BellState] == 1 && _objectsState[kPart81EventDoneState] == 1) {
		_objectsState[kPart81BellState] = 0;
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
