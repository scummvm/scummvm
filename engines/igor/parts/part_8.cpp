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


enum Part08PendingDeanDrink {
	kPart08PendingDeanDrinkNone = 0,
	kPart08PendingDeanDrinkWhisky = 1,
	kPart08PendingDeanDrinkWater = 2
};

void IgorEngine::part8ApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		const uint32 srcOffset = _objectsState[kDeanDoorOpenStateIndex] == 0 ? 0 : 0x4C8;
		copyArea(_screenLayer1, 0x572C, 320, _animFramesBuffer + srcOffset, 24, 24, 51);
		_roomActionsTable[150] = _objectsState[kDeanDoorOpenStateIndex] == 0 ? 6 : 7;
	}
	if (num == 2 || num == 255) {
		if (_objectsState[kExamCompletedStateIndex] == 1) {
			_roomObjectAreasTable[7].object = 0;
			_roomObjectAreasTable[8].object = 0;
			_roomActionsTable[146] = 4;
			return;
		}
		uint32 srcOffset;
		int dstOffset;
		int rows;
		int width;
		if (_objectsState[kDeanPassedOutStateIndex] == 0) {
			srcOffset = 0x208A;
			dstOffset = 0x6BC3;
			rows = 26;
			width = 34;
			_roomObjectAreasTable[8].object = 0;
		} else {
			srcOffset = _objectsState[kNewspaperTakenStateIndex] == 0 ? 0x9992 : 0x985C;
			dstOffset = 0x7FC8;
			rows = 10;
			width = 31;
			_roomObjectAreasTable[8].object = _objectsState[kNewspaperTakenStateIndex] == 0 ? 3 : 0;
		}
		drawAnimRect(dstOffset, srcOffset, width, rows, true);
		_roomActionsTable[146] = 4;
	}
}

void IgorEngine::part8DrawDoor(bool open) {
	if ((_objectsState[kDeanDoorOpenStateIndex] != 0) == open) {
		const int text = open ? 19 : 23;
		igorSayAndWait(text, 1, text);
		return;
	}

	const int lastFrame = open ? 2 : 0;
	const int step = open ? 1 : -1;
	for (int frame = 1;; frame += step) {
		drawAnimRect(0x5729, 0x3DA6 + frame * 0x597, 27, 53);
		if ((open && frame == 2) || (!open && frame == 1))
			playSound(open ? 13 : 14, 1);
		if (frame == lastFrame)
			break;
		waitForTimer(127);
	}
	_objectsState[kDeanDoorOpenStateIndex] = open ? 1 : 0;
	part8ApplyObjectState(1);
}

void IgorEngine::part8DrawDean() {
	drawAnimRect(0x6BC3, 0x23FE, 34, 26, true);
}

void IgorEngine::part8DrawDeanTalkingFrame(int frame) {
	drawAnimRect(0x6BC6, 0x4E6B + frame * 0x226, 25, 22, true);
}

void IgorEngine::part8UpdateDeanDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		part8DrawDeanTalkingFrame(0);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part8DrawDeanTalkingFrame(getRandomNumber(6));
		break;
	}
}

void IgorEngine::part8TakeNewspaper() {
	for (int frame = 0; frame <= 1; ++frame) {
		drawAnimRect(0x66E3, 0x125E + frame * 0x55C, 28, 49);
		if (frame == 0)
			waitForTimer(127);
	}
	addObjectToInventory(kInventoryObjectNewspaper, 50);
	_objectsState[kNewspaperTakenStateIndex] = 1;
	part8ApplyObjectState(255);
}

void IgorEngine::part8TakeBook() {
	for (int frame = 0; frame <= 1; ++frame) {
		drawAnimRect(0x4F77, 2448 + frame * 1127, 23, 49, false, kBlendLitSprite);
		if (frame == 0)
			waitForTimer(127);
	}

	addObjectToInventory(kInventoryObjectDeansBook, 48);
	part8ApplyObjectState(255);
	_objectsState[kDeanBookTakenStateIndex] = 1;
}

void IgorEngine::part8DrawSecretaryTalkingFrame(int frame) {
	const uint16 frameOffset = READ_LE_UINT16(_animFramesBuffer + 0xE240 + frame * 2);
	decodeAnimFrame(_animFramesBuffer + 0x9AC8 + frameOffset - 1, _screenVGA, true);
}

void IgorEngine::part8UpdateSecretaryDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		part8DrawSecretaryTalkingFrame(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part8DrawSecretaryTalkingFrame(getRandomNumber(2) + 2);
		break;
	}
}

void IgorEngine::part8DeanCallsSecretary() {
	const int currentPart = _currentPart;
	const int actionFrameOffset = _objectsState[kNewspaperTakenStateIndex] == 0 ? 0x2773 : 0x328D;
	playSound(60, 1);

	for (int frame = 0; frame <= 1; ++frame) {
		drawAnimRect(0x6D1E, actionFrameOffset - 1 + frame * 0x58D, 29, 49);
		if (frame == 0)
			waitForTimer(127);
	}

	part8HandleIntercomDialogue();

	_currentPart = 91;
	fadeOut(768);
	// the room is saved again as it was left, Igor included
	uint8 *roomScreen = (uint8 *)malloc(46080);
	memcpy(roomScreen, _screenVGA, 46080);
	part9();

	// back to room 8
	loadActionData(DAT_DeanPepperOffice);
	loadRoomData(PAL_DeanPepperOffice, IMG_DeanPepperOffice, BOX_DeanPepperOffice, MSK_DeanPepperOffice, TXT_DeanPepperOffice);
	_roomDataOffsets = PART_08_ROOM_DATA_OFFSETS;
	setRoomClickFix(143, -1, 281, false);
	part8ApplyObjectState(255);
	memcpy(_screenVGA, roomScreen, 46080);
	free(roomScreen);
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	drawVerbsPanel();
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	fadeIn(768);
	_currentPart = currentPart;
	_objectsState[kIntercomUsedStateIndex] = 1;
}

void IgorEngine::part8DeanDrinksBottle() {
	static const uint8 frames[] = {
		3, 1, 2, 3, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5,
		4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 6, 7, 8, 9
	};
	part8DrawDean();
	for (uint i = 0; i < ARRAYSIZE(frames); ++i) {
		decodeAnimFrame(getAnimFrame(0x76DD, 0xD98, frames[i]), _screenVGA, true);
		if (_game.version == kIdSpaFloppy) {
			// The Spanish floppy shows the gulping while Dean drinks
			if (i == 3)
				cutsceneSayStart(78, 75, 26, 58, 0, 228, 1, 0);
			if (i == 22) {
				memcpy(_screenVGA + _dialogueDirtyRectY,
						_screenTextLayer + 320 * 72, _dialogueDirtyRectSize);
				_gameState.dialogueTextRunning = false;
			}
		}
		waitForTimer(31);
	}
}

void IgorEngine::part8DeanPassesOut() {
	const uint8 savedTalkMode = _gameState.talkMode;
	_gameState.talkMode = kTalkModeTextOnly;
	waitForTimer(255);

	cutsceneSayStart(78, 75, 26, 58, 0, 229, 1, 0);
	playSound(61, 1);

	for (int step = 1; step <= 20; ++step) {
		const int frame = getRandomNumber(5);
		drawAnimRect(0x6806, 0x8489 + frame * 0x2D5, 25, 29);
		waitForTimer(61);

		if (step == 10) {
			memcpy(_screenVGA + _dialogueDirtyRectY,
					_screenTextLayer + 320 * 72, _dialogueDirtyRectSize);
			cutsceneSayStart(78, 75, 26, 58, 0, 230, 1, 0);
		}
	}

	memcpy(_screenVGA + _dialogueDirtyRectY,
			_screenTextLayer + 320 * 72, _dialogueDirtyRectSize);
	_gameState.dialogueTextRunning = false;
	playSound(15, 1);
	drawAnimRect(0x6806, 0x9587, 25, 29);
	waitForTimer(255);
	_gameState.talkMode = savedTalkMode;
}

void IgorEngine::part8HandleDeanDialogue() {
	loadDialogueData(DLG_DeanPepperOffice);
	_updateDialogue = &IgorEngine::part8UpdateDeanDialogue;
	handleDialogue(78, 75, 26, 58, 0);
	_updateDialogue = 0;
}

void IgorEngine::part8HandleIntercomDialogue() {
	const RoomDataOffsets savedOffsets = _roomDataOffsets;
	_roomDataOffsets.dlg = { -174, 2, 257, 0, 30, 0, 459, 459, 0 };
	loadDialogueData(DLG_Part8Intercom);
	_updateDialogue = 0;
	handleDialogue(0, 0, 0, 0, 0, false, 2);
	_roomDataOffsets = savedOffsets;
}

void IgorEngine::part8TalkToDean() {
	if (_objectsState[kDeanPassedOutStateIndex] == 1) {
		igorSayAndWait(223, 1, 169);
		return;
	}

	if (_objectsState[kDeanDoorOpenStateIndex] == 1) { // door open
		part8DrawDean();
		cutsceneSayWithCallback(78, 75, 26, 58, 0, { { 220, 1, 166 }, { 221, 1, 167 } }, &IgorEngine::part8UpdateDeanDialogue);
		part8ApplyObjectState(255);
		return;
	}

	part8HandleDeanDialogue();
	part8ApplyObjectState(255);
}

void IgorEngine::part8GiveBottleToDean() {

	if (_objectsState[kDeanPassedOutStateIndex] == 1) {
		igorSayAndWait(227, 1, 172);
		return;
	}
	if (_objectsState[kBottleStateIndex] == 1) {
		igorSayAndWait(226, 1, 171);
		return;
	}

	part8DrawDean();
	igorSayAndWait(210, 2, 158);

	static const uint8 deanFrames[] = { 1, 2, 3, 0 };
	static const uint8 igorFrames[] = { 0, 1, 2, 3 };

	for (int step = 0; step < 4; ++step) {
		if (deanFrames[step] != 0) {
			drawAnimRect(0x6463, 0x5755 + deanFrames[step] * 0x620, 32, 49);
		}
		if (igorFrames[step] != 0) {
			drawAnimRect(0x6D06, 0x6D7D + igorFrames[step] * 0x258, 24, 25);
		}
		waitForTimer(41);
	}


	if (_inventoryInfo[58] != 0) {
		removeInventoryEntry(58);
		drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	}

	cutsceneSayWithCallback(78, 75, 26, 58, 0, { { 216, 1, 162 }, { 217, 1, 163 }, { 218, 1, 164 }, { 219, 1, 165 } }, &IgorEngine::part8UpdateDeanDialogue);
	const Part08PendingDeanDrink pendingDeanDrink =
		_objectsState[kBottleStateIndex] < 2 ? kPart08PendingDeanDrinkWhisky : kPart08PendingDeanDrinkWater;
	_objectsState[kPendingDeanDrinkStateIndex] = pendingDeanDrink;
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	playSound(63, 1);
	part8ApplyObjectState(255);
}

void IgorEngine::part8ExecuteAction(int action) {
	switch (action) {
	case 101: // exit office
		// With the door closed nothing happens: Igor has to open it first
		if (_objectsState[kDeanDoorOpenStateIndex] == 0)
			break;
		if (_objectsState[kDeanDoorOpenStateIndex] != 0) {
			for (int area = 10; area <= 11; ++area)
				_roomObjectAreasTable[area].area = 4;
			for (int area = 13; area <= 14; ++area)
				_roomObjectAreasTable[area].area = 4;
			--_walkDataLastIndex;
			buildWalkPath(242, 121, 270, 108);
			_walkDataCurrentIndex = 1;
			_gameState.igorMoving = true;
			waitForIgorMove();
			uint8 &pendingDeanDrink = _objectsState[kPendingDeanDrinkStateIndex];
			if (pendingDeanDrink != kPart08PendingDeanDrinkNone) {
				if (_game.version != kIdSpaFloppy)
					waitForTimer(251);
				part8DeanDrinksBottle();
				const bool bottleCaught = _objectsState[kButterflyNetStateIndex] == 1;
				if (bottleCaught)
					_objectsState[kButterflyNetStateIndex] = 2;
				else
					_objectsState[kGlassShardsPresentStateIndex] = 1;
				if (_game.version == kIdSpaFloppy) {
					// Only the shards fall with a pause and a sound
					if (!bottleCaught) {
						waitForTimer(100);
						playRoomSound(kSoundPart8Shards, kSpaFloppySoundPart8Shards);
					}
				} else {
					waitForTimer(101);
					if (!bottleCaught)
						playRoomSound(kSoundPart8Shards, kSpaFloppySoundPart8Shards);
				}
				if (pendingDeanDrink == kPart08PendingDeanDrinkWater) {
					part8DeanPassesOut();
					_objectsState[kDeanPassedOutStateIndex] = 1;
				}
				pendingDeanDrink = kPart08PendingDeanDrinkNone;
			}
			_currentPart = 71;
		}
		break;
	case 102: // Look at window
		igorSayAndWait(202, 1, 153);
		break;
	case 104: // Look at dean
		if (_objectsState[kDeanPassedOutStateIndex] == 1)
			ADD_DIALOGUE_TEXT(222, 1, 168);
		else
			ADD_DIALOGUE_TEXT(203, 2, 154);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue();
		break;
	case 107: // look at intercom
		igorSayAndWait(205, 2, 155);
		break;
	case 110: // look at bookcase
		igorSayAndWait(207, 2, 156);
		break;
	case 111: // look at door
		igorSayAndWait(209, 1, 157);
		break;
	case 105:
		part8TakeNewspaper();
		break;
	case 103:
		part8TalkToDean();
		break;
	case 108:
		if (_objectsState[kDeanPassedOutStateIndex] == 0) {
			igorSayAndWait(212, 1, 159);
		} else if (_objectsState[kIntercomUsedStateIndex] == 1) {
			igorSayAndWait(224, 2, 170);
		} else {
			part8DeanCallsSecretary();
		}
		break;
	case 109:
		if (_objectsState[kDeanPassedOutStateIndex] == 0) {
			igorSayAndWait(213, 2, 160);
		} else if (_objectsState[kDeanBookTakenStateIndex] == 1) {
			igorSayAndWait(215, 1, 161);
		} else {
			part8TakeBook();
		}
		break;
	case 114:
		part8GiveBottleToDean();
		break;
	case 112:
		part8DrawDoor(true);
		break;
	case 113:
		part8DrawDoor(false);
		break;
	default:
		warning("part8ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part8() {
	if (isCDVersion())
		playMusic(2);
	_gameState.enableLight = 1;
	loadActionData(DAT_DeanPepperOffice);
	loadRoomData(PAL_DeanPepperOffice, IMG_DeanPepperOffice, BOX_DeanPepperOffice, MSK_DeanPepperOffice, TXT_DeanPepperOffice);
	static const int anim[] = {
		FRM_DeanPepperOffice1, FRM_DeanPepperOffice2,
		FRM_DeanPepperOffice3, FRM_DeanPepperOffice4,
		FRM_DeanPepperOffice5, FRM_DeanPepperOffice6,
		FRM_DeanPepperOffice7, FRM_DeanPepperOffice8,
		FRM_DeanPepperOffice9, FRM_DeanPepperOffice10,
		FRM_DeanPepperOffice11, FRM_DeanPepperOffice12,
		FRM_DeanPepperOffice13, FRM_DeanPepperOffice14,
		FRM_DeanPepperOffice15, FRM_DeanPepperOffice16, 0
	};
	loadAnimData(anim);
	partMainSetExecuteAction(1, &IgorEngine::part8ExecuteAction);
	_roomDataOffsets = PART_08_ROOM_DATA_OFFSETS;
	setRoomClickFix(143, -1, 281, false);
	part8ApplyObjectState(255);

	memcpy(_screenVGA, _screenLayer1, 46080);
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		_walkData[0].setPos(241, 121, kFacingPositionLeft, 0);
		_walkData[0].setDefaultScale();
		_walkDataCurrentIndex = 0;
		moveIgor(kFacingPositionLeft, 0);
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
		fadeIn(768);
	}

	enterPartLoop();
	if (takeResumeDialogue(1)) { // loaded while talking to the dean
		part8HandleDeanDialogue();
		part8ApplyObjectState(255);
		finishResumedDialogue();
	}
	while (_currentPart == 80 && !_gameStateLoaded)
		runPartLoop();
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
