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


static const int kPart81HeadOffset = 28 * 320 + 140;
static const int kPart81HeadWidth = 21;
static const int kPart81HeadHeight = 53;
static const int kPart81HeadFramesBase = 0x1AE5;
static const int kPart81HeadFrameSize = kPart81HeadWidth * kPart81HeadHeight;
static const int kPart81HeadStanding = 4;

// Another picture of the head, drawn into the room picture while it is there.
static const int kPart81HeadRestFrame = 1;

static const int kPart81IgorOffset = 32 * 320 + 116;
static const int kPart81IgorWidth = 43;
static const int kPart81IgorHeight = 62;
static const int kPart81IgorFrameSize = kPart81IgorWidth * kPart81IgorHeight;
static const int kPart81IgorHanding = 0xC4DF;
static const int kPart81IgorStanding = 0xBA75;

static const int kPart81StoryFramesBase = 0x4206;

// Where the lines of the lighthouse keeper are shown.
static const int kPart81TextX = 148;
static const int kPart81TextY = 31;
static const int kPart81TextR = 63;
static const int kPart81TextG = 32;
static const int kPart81TextB = 0;

static const int kPart81TallWidth = 0x82;
static const int kPart81TallX = 0x6A;
static const int kPart81StripWidth = 0x11D;
static const int kPart81StripRows = 0x48;

static const int kPart81EndFramesBase = 0xBAE0;
static const int kPart81EndFramesTable = 0x828;

static const int kPart81InventoryIndex = 69;

static const uint8 kPart81FigureFrames[4] = { 1, 2, 3, 1 };

static const uint8 kPart81EndFrames[10] = { 1, 2, 1, 2, 1, 2, 1, 2, 1, 2 };
static const uint8 kPart81EndHold[10] = { 45, 35, 25, 40, 45, 30, 45, 35, 20, 30 };
static const uint8 kPart81EndFinalHold[10] = { 25, 30, 20, 40, 25, 35, 40, 42, 20, 25 };

static const int kPart81StepTicks = 16;

void IgorEngine::part81LoadRoom() {
	loadRoomData(PAL_Part81, IMG_Part81, BOX_Part81, MSK_Part81, TXT_Part81);
	// the colors of Igor
	memcpy(_paletteBuffer + 192 * 3, _igorPalette, 48);
	setVerbPanelColors();
}

void IgorEngine::part81LoadAnimation() {
	static const int anim[] = { ANM_Part81, 0 };
	loadAnimData(anim);
}

void IgorEngine::part81DrawLighthouseKeeperHead(int frame) {
	drawAnimRect(kPart81HeadOffset, kPart81HeadFramesBase + frame * kPart81HeadFrameSize, kPart81HeadWidth, kPart81HeadHeight,
				 true, kBlendBehindIgorAndText);
	if (_gameState.dialogueTextRunning) {
		memcpy(_screenLayer1 + _dialogueDirtyRectY, _screenTextLayer + 23040, _dialogueDirtyRectSize);
	}
}

void IgorEngine::part81UpdateBackground() {
	// if user has seen church bell message, show lighthouse keeper
	if (compareGameTick(0x3D) && getRandomNumber(1) == 0 && _objectsState[kChurchBellMessageSeenStateIndex] == 1) {
		part81DrawLighthouseKeeperHead(getRandomNumber(2) + 1);
	}
}

void IgorEngine::part81UpdateLighthouseKeeperDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		part81DrawLighthouseKeeperHead(kPart81HeadStanding);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part81DrawLighthouseKeeperHead(getRandomNumber(4) + 5);
		break;
	}
}

void IgorEngine::part81SayLighthouseKeeper(const Common::Array<DialogueText> &lines) {
	UpdateRoomBackgroundProc previous = _updateRoomBackground;
	_updateRoomBackground = 0;
	cutsceneSayWithCallback(kPart81TextX, kPart81TextY, kPart81TextR, kPart81TextG, kPart81TextB, lines,
							&IgorEngine::part81UpdateLighthouseKeeperDialogue);
	_updateRoomBackground = previous;
}

void IgorEngine::part81SayIgor(int num, int count, int sound) {
	UpdateRoomBackgroundProc previous = _updateRoomBackground;
	_updateRoomBackground = 0;
	igorSayAndWait(num, count, sound);
	_updateRoomBackground = previous;
}

void IgorEngine::part81ApplyObjectState(int num) {
	if (num != 1 && num != 255) {
		return;
	}
	// change door hotspot to lighthouse keeper and vice-versa
	uint8 objectNames[16];
	loadData(TXT_Part81ObjectNames, objectNames);
	const uint8 *name = objectNames + (_objectsState[kChurchBellMessageSeenStateIndex] == 1 ? 0 : objectNames[0] + 1);
	memcpy(_roomObjectNames[2], name + 1, name[0]);
	_roomObjectNames[2][name[0]] = '\0';
	// show lighthouse keeper if the player has seen the church bell message
	if (_objectsState[kChurchBellMessageSeenStateIndex] == 1) {
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
 * The room picture comes in from the left, over the actual scene (a 160 columns picture).
 */
void IgorEngine::part81ScrollIn() {
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
 * Transition: for the message cutscene, the scene scrolls up to the top of lighthouse
 */
void IgorEngine::part81TransitionScrollUp() {
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
 * The opposite transition: from the top of the lighthouse back to the normal scene.
 */
void IgorEngine::part81TransitionScrollDown() {
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

void IgorEngine::part81DrawLighthouseFrame(int frame) {
	decodeAnimFrame(getAnimFrame(kPart81EndFramesBase, kPart81EndFramesTable, frame), _screenVGA, false);
}

void IgorEngine::part81LighthouseSendMessage() {
	part81TransitionScrollUp();
	// The Spanish floppy goes straight to the flashes, without the long pauses
	const bool pauses = _game.version != kIdSpaFloppy;
	for (int i = 0; pauses && i < 3; ++i) {
		waitForTimer(251);
	}
	// lighthouse flashes message
	for (int i = 0; i < 10; ++i) {
		part81DrawLighthouseFrame(kPart81EndFrames[i]);
		waitForTimer(kPart81EndHold[i]);
	}
	if (pauses)
		waitForTimer(251);
	part74ShipCutscene();
	// ship receives message
	for (int i = 0; i < 10; ++i) {
		for (int frame = 3; frame <= 8; ++frame) {
			part81DrawLighthouseFrame(frame);
			waitForTimer(10);
		}
		waitForTimer(kPart81EndFinalHold[i]);
	}
	part81TransitionScrollDown();
}

void IgorEngine::part81DrawLighthouseKeeper(int frame) {
	copyArea(_screenVGA, kPart81HeadOffset, 320, _animFramesBuffer + kPart81StoryFramesBase + frame * kPart81HeadFrameSize,
			 kPart81HeadWidth, kPart81HeadWidth, kPart81HeadHeight);
}

void IgorEngine::part81DrawIgor(int offset) {
	copyArea(_screenVGA, kPart81IgorOffset, 320, _animFramesBuffer + offset, kPart81IgorWidth, kPart81IgorWidth,
			 kPart81IgorHeight);
}

void IgorEngine::part81RejectItem() {
	for (int frame = 3; frame <= 4; ++frame) {
		part81DrawLighthouseKeeperHead(frame);
		waitForTimer(31);
	}
	part81SayIgor(208, 1, 952);
	part81DrawIgor(kPart81IgorHanding);
	part81SayLighthouseKeeper({ { 209, 1, 953 } });
	part81DrawIgor(kPart81IgorStanding);
}

void IgorEngine::part81GiveLampToLighthouseKeeper() {
	if (_objectsState[kVaseStateIndex] <= 1) {
		part81RejectItem();
		return;
	}
	fillScreenRows(144, 56, 0);
	for (int frame = 3; frame <= 4; ++frame) {
		part81DrawLighthouseKeeperHead(frame);
		waitForTimer(31);
	}
	part81SayIgor(208, 1, 952);
	for (int step = 0; step < 4; ++step) {
		part81DrawIgor((kPart81FigureFrames[step] - 1) * kPart81IgorFrameSize);
		waitForTimer(61);
	}

	// giving the lamp
	if (_inventoryInfo[kPart81InventoryIndex] != 0) {
		removeInventoryEntry(kPart81InventoryIndex);
	}
	part81SayLighthouseKeeper({ { 210, 1, 954 }, { 211, 1, 955 } });
	part81SayIgor(212, 1, 956);
	part81SayLighthouseKeeper({ { 213, 1, 957 }, { 214, 1, 958 } });
	part81SayIgor(215, 1, 959);
	part81SayLighthouseKeeper({ { 216, 1, 960 }, { 217, 1, 961 } });
	part81SayIgor(218, 1, 962);
	part81SayLighthouseKeeper({ { 219, 2, 963 }, { 221, 2, 964 }, { 223, 1, 965 } });
	playSound(14, 1);
	for (int frame = 1; frame <= 3; ++frame) {
		part81DrawLighthouseKeeper(frame);
		waitForTimer(31);
	}

	// the scene of the end, with the pictures of its own
	static const int cutsceneAnim[] = { ANM_Part81Cutscene, 0 };
	loadAnimData(cutsceneAnim);
	loadData(IMG_Part81Strip, _screenLayer2);
	memcpy(_screenLayer1, _screenVGA, 46080);
	part81LighthouseSendMessage();

	// the room again
	loadActionData(DAT_Part81);
	part81LoadRoom();
	part81LoadAnimation();
	part81ApplyObjectState(255);
	for (int frame = 4; frame <= 6; ++frame) {
		part81DrawLighthouseKeeper(frame);
		if (frame == 6) {
			playSound(13, 1);
		}
		waitForTimer(31);
	}
	while (_mixer->isSoundHandleActive(_sfxHandle) && !_eventQuitGame) {
		waitForTimer(1);
	}
	part81SayLighthouseKeeper({ { 224, 1, 966 }, { 225, 1, 967 }, { 226, 3, 968 } });
	part81SayIgor(229, 1, 969);
	part81SayLighthouseKeeper({ { 230, 1, 970 } });
	_objectsState[kLighthouseMessageSentStateIndex] = 1;
	drawVerbsPanel();
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	part81ApplyObjectState(255);
}

void IgorEngine::part81DialogueCode(int code) {
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

void IgorEngine::part81HandleDialogue() {
	UpdateRoomBackgroundProc previousBackground = _updateRoomBackground;
	_updateRoomBackground = 0;
	_updateDialogue = &IgorEngine::part81UpdateLighthouseKeeperDialogue;
	loadDialogueData(DLG_Part81);
	const bool resuming = _dialogueResuming;
	_dialogueResuming = false;
	_gameState.dialogueStarted = 1;
	if (!resuming) { // a loaded savegame keeps the menu position it was saved at
		_gameState.dialogueChoiceStart = 1;
		_gameState.dialogueChoiceCount = 1;
	}

	bool done = false;
	do {
		drawDialogueChoices();
		part81DrawLighthouseKeeperHead(kPart81HeadStanding);
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
			part81DialogueCode(code);
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

	fillScreenRows(144, 56, 0);
	drawVerbsPanel();
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	_currentAction.verb = kVerbWalk;
	_gameState.dialogueStarted = 0;
	_updateDialogue = 0;
	_updateRoomBackground = previousBackground;
}

void IgorEngine::part81Talk() {
	if (_objectsState[kChurchBellMessageSeenStateIndex] == 1 && _objectsState[kLighthouseMessageSentStateIndex] == 1) {
		igorSay(234, 1, 974);
		return;
	}
	part81SayIgor(231, 1, 971);
	part81SayLighthouseKeeper({ { 232, 1, 972 } });
	part81HandleDialogue();
	part81ApplyObjectState(255);
}

void IgorEngine::part81WalkIn() {
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

void IgorEngine::part81ExecuteAction(int action) {
	switch (action) {
	case 101:
		_objectsState[kCemeteryEntrySideStateIndex] = 1;
		_currentPart = 700;
		break;
	case 102: // talk to lighthouse keeper
		part81Talk();
		break;
	case 103: // look at lighthouse keeper
		if (_objectsState[kChurchBellMessageSeenStateIndex] == 1) {
			igorSay(202, 1, 946);
		} else {
			igorSay(201, 1, 945);
		}
		break;
	case 104: // open door
		igorSay(233, 1, 973);
		break;
	case 106: // look at tree
		igorSay(203, 1, 947);
		break;
	case 107: // look at lighthouse
		igorSay(204, 1, 948);
		break;
	case 108: // look at sea
		if (_objectsState[kLighthouseMessageSentStateIndex] == 0) {
			igorSay({ { 206, 1, 950 }, { 207, 1, 951 } });
		} else {
			igorSay(205, 1, 949);
		}
		break;
	case 109:
		part81GiveLampToLighthouseKeeper();
		break;
	case 110:
		part81RejectItem();
		break;
	default:
		error("part81ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part81() {
	if (isCDVersion())
		playMusic(4);
	_gameState.enableLight = 2;
	loadActionData(DAT_Part81);
	part81LoadRoom();
	part81LoadAnimation();
	_roomDataOffsets = PART_81_ROOM_DATA_OFFSETS;
	partMainSetExecuteAction(1, &IgorEngine::part81ExecuteAction);
	_updateRoomBackground = &IgorEngine::part81UpdateBackground;
	part81ApplyObjectState(255);

	// the colors that change are darker than their palette
	darkenPalette(_paletteBuffer, 192, 207, 20);
	copyLayerToScreen(_screenLayer1Surface);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		part81ScrollIn();
		part81WalkIn();
	}
	enterPartLoop();
	if (takeResumeDialogue(1)) { // loaded while talking to the lighthouse keeper
		part81HandleDialogue();
		part81ApplyObjectState(255);
		finishResumedDialogue();
	}
	while (_currentPart == 810 && !_gameStateLoaded) {
		runPartLoop();
	}
	if (!_gameStateLoaded && _objectsState[kChurchBellMessageSeenStateIndex] == 1 && _objectsState[kLighthouseMessageSentStateIndex] == 1) {
		_objectsState[kChurchBellMessageSeenStateIndex] = 0;
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
