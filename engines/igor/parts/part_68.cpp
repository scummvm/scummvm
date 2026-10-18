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

static const int kPart68NBorisOffset = 88 * 320 + 263;
static const int kPart68BorisWidth = 33;
static const int kPart68BorisHeight = 43;
static const int kPart68BorisFramesOffset = 2516;
static const int kPart68BorisFrameSize = kPart68BorisWidth * kPart68BorisHeight;
static const int kPart68BorisStanding = 4;
static const int kPart68BorisFacing = 3;


static const int kPart68OwlsEyesOffset = 31 * 320 + 39;
static const int kPart68OwlsEyesWidth = 39;
static const int kPart68OwlsEyesHeight = 14;
static const int kPart68OwlsEyesFramesOffset = 0x3BB7;

static const int kPart68VaseOffset = 92 * 320 + 179;
static const int kPart68VaseWidth = 9;
static const int kPart68VaseHeight = 12;

// The picture of the exchange
static const int kPart68PoseOffset = 83 * 320 + 170;
static const int kPart68PoseWidth = 23;
static const int kPart68PoseHeight = 50;
static const int kPart68PoseFramesOffset = 0xD8;

static const int kPart68GiveAnimOffset = 86 * 320 + 244;
static const int kPart68GiveWidth = 52;
static const int kPart68GiveHeight = 30;
static const int kPart68GiveFramesOffset = 0x443F;
static const int kPart68GiveFrameSize = kPart68GiveWidth * kPart68GiveHeight;

static const int kPart68LeaveBase = 0x5C9F;
static const int kPart68LeaveTable = 0x140E;

// Where Boris' lines are shown.
static const int kPart68TextX = 282;
static const int kPart68TextY = 85;
static const int kPart68TextR = 63;
static const int kPart68TextG = 50;
static const int kPart68TextB = 0;

// Inventory objects.
static const int kPart68TakenObjectIndex = 69;
static const int kPart68GiveInventoryIndex = 40;

static const int kPart68TalkCodeEnd = 100;

void IgorEngine::part68LoadRoom() {
	loadRoomData(PAL_Part68, IMG_Part68, BOX_Part68, MSK_Part68, TXT_Part68);
	SET_PAL_240_48_1();
}

void IgorEngine::part68DrawUndertaker(int frame) {
	drawAnimRect(kPart68NBorisOffset, kPart68BorisFramesOffset + frame * kPart68BorisFrameSize, kPart68BorisWidth, kPart68BorisHeight,
				 true, kBlendBehindIgorAndText);
}

void IgorEngine::part68DrawAmbient(int frame) {
	drawAnimRect(kPart68OwlsEyesOffset, kPart68OwlsEyesFramesOffset + frame * kPart68OwlsEyesWidth * kPart68OwlsEyesHeight,
				 kPart68OwlsEyesWidth, kPart68OwlsEyesHeight);
}

void IgorEngine::part68DrawVase(int frame) {
	copyArea(_screenLayer1, kPart68VaseOffset, 320, _animFramesBuffer + frame * kPart68VaseWidth * kPart68VaseHeight,
			 kPart68VaseWidth, kPart68VaseWidth, kPart68VaseHeight);
}

void IgorEngine::part68DrawPose(int frame) {
	drawAnimRect(kPart68PoseOffset, kPart68PoseFramesOffset + frame * kPart68PoseWidth * kPart68PoseHeight,
				 kPart68PoseWidth, kPart68PoseHeight);
}

void IgorEngine::part68ApplyObjectState(int num) {
	if (num == 3 || num == 255) {
		// undertaker is still in the room
		if (_objectsState[kUndertakerLeftStateIndex] == 0) {
			part68DrawUndertaker(0);
			for (int i = 0; i < MAX_ROOM_OBJECT_AREAS; ++i) {
				if (_roomObjectAreasTable[i].area >= 3 && _roomObjectAreasTable[i].area <= 4) {
					_roomObjectAreasTable[i].area = 0;
				}
			}
			WRITE_LE_UINT16(_roomActionsTable + 156, 0xA87D);
			_roomActionsTable[246] = 1;
			_part68NpcPresent = true;
		} else {
			loadActionData(DAT_Part68);
			part68LoadRoom();
			WRITE_LE_UINT16(_roomActionsTable + 156, 0x9081);
			_roomActionsTable[246] = 3;
			// disable undertaker hotspot
			for (int i = 0; i < MAX_ROOM_OBJECT_AREAS; ++i) {
				if (_roomObjectAreasTable[i].object == 3) {
					_roomObjectAreasTable[i].object = 0;
				}
			}
			_part68NpcPresent = false;
		}
	}
	if (num == 2 || num == 255) {
		if (_objectsState[kVaseTakenStateIndex] == 0) {
			part68DrawVase(0);
		} else {
			part68DrawVase(1);
			for (int i = 0; i < MAX_ROOM_OBJECT_AREAS; ++i) {
				if (_roomObjectAreasTable[i].object == 2) {
					_roomObjectAreasTable[i].object = 0;
				}
			}
		}
	}
}

void IgorEngine::part68UpdateOwlsEyes() {
	if (compareGameTick(0x3D)) {
		if (_roomAmbientIndex == 3) {
			_roomAmbientIndex = 0;
			part68DrawAmbient(0);
		} else if (getRandomNumber(9) == 0) {
			_roomAmbientIndex = getRandomNumber(3);
			part68DrawAmbient(_roomAmbientIndex);
		}
	}
}

void IgorEngine::part68PlayAmbientSound() {
	if (_game.version == kIdSpaFloppy)
		playSound(getRandomNumber(1) == 0 ? kSpaFloppySoundPart68AmbientA : kSpaFloppySoundPart68AmbientB, 1);
	else
		playSound(41, 1);
}

void IgorEngine::part68UpdateBackgroundTalk() {
	if (compareGameTick(1) && getRandomNumber(49) == 0) {
		part68PlayAmbientSound();
	}
	part68UpdateOwlsEyes();
}

void IgorEngine::part68UpdateBackground() {
	if (compareGameTick(1) && getRandomNumber(49) == 0) {
		part68PlayAmbientSound();
	}
	if (compareGameTick(0x3D) && _part68NpcPresent) {
		part68DrawUndertaker(getRandomNumber(2));
	}
	part68UpdateOwlsEyes();
}

void IgorEngine::part68SayBorisReplies(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		part68DrawUndertaker(kPart68BorisStanding);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part68DrawUndertaker(kPart68BorisStanding + getRandomNumber(4));
		break;
	}
}

void IgorEngine::part68SayBoris(const Common::Array<DialogueText> &lines) {
	UpdateRoomBackgroundProc previous = _updateRoomBackground;
	_updateRoomBackground = &IgorEngine::part68UpdateBackgroundTalk;
	cutsceneSayWithCallback(kPart68TextX, kPart68TextY, kPart68TextR, kPart68TextG, kPart68TextB, lines,
							&IgorEngine::part68SayBorisReplies);
	_updateRoomBackground = previous;
}

/**
 * Since Boris speaks in four separate pages, utility function to help on that
 */
void IgorEngine::part68SayBorisReplies(const int *replies, const int *sounds, int count) {
	Common::Array<DialogueText> lines;
	for (int i = 0; i < count; ++i) {
		Common::strlcpy(_globalDialogueTexts[250 + i], _dialogueReplies[replies[i] - 1], sizeof(_globalDialogueTexts[250 + i]));
		DialogueText line = { 250 + i, 1, sounds[i] };
		lines.push_back(line);
	}
	part68SayBoris(lines);
}

void IgorEngine::part68EnterFromRight() {
	WalkData *wd = &_walkData[0];
	wd->setPos(319, 143, kFacingPositionLeft, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 15;
	wd->scaleWidth = 50;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 50;
	_walkDataLastIndex = 0;
	buildWalkPath(319, 143, 259, 143);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(&IgorEngine::part68UpdateBackground);
}

void IgorEngine::part68EnterFromLeft() {
	WalkData *wd = &_walkData[0];
	wd->setPos(0, 143, kFacingPositionRight, 0);
	wd->clipSkipX = 16;
	wd->clipWidth = 15;
	wd->scaleWidth = 50;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 50;
	_walkDataLastIndex = 0;
	buildWalkPath(0, 143, 60, 143);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(&IgorEngine::part68UpdateBackground);
}

void IgorEngine::part68EnterFromCemetery() {
	WalkData *wd = &_walkData[0];
	wd->setPos(193, 115, kFacingPositionRight, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 25;
	wd->scaleWidth = 42;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 42;
	_walkDataLastIndex = 0;
	buildWalkPath(193, 115, 253, 138);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove(&IgorEngine::part68UpdateBackground);
}


void IgorEngine::part68TakeVase() {
	// will only let us take the vase if the records have been given to him
	if (_objectsState[kRecordGivenToUndertakerStateIndex] == 0) {
		part68DrawPose(0);
		part68DrawUndertaker(kPart68BorisFacing);
		waitForTimer(30);
		part68SayBoris({ { 202, 1, 1008 }, { 203, 1, 1009 }, { 204, 1, 1010 }, { 205, 1, 1011 } });
		part68DrawPose(1);
		waitForTimer(30);

		// Igor turns to the right
		_walkData[_walkDataLastIndex - 1].posNum = kFacingPositionRight;
		_walkDataCurrentIndex = _walkDataLastIndex - 1;
		moveIgor(kFacingPositionRight, 0);
		_walkDataCurrentIndex = _walkDataLastIndex;
		igorSayAndWait({ { 206, 1, 1012 }, { 207, 1, 1013 } });
		part68SayBoris({ { 208, 1, 1014 }, { 209, 1, 1015 }, { 210, 1, 1016 }, { 211, 1, 1017 } });
		part68DrawUndertaker(kPart68BorisFacing);
		waitForTimer(30);
		return;
	}
	for (int frame = 0; frame <= 1; ++frame) {
		part68DrawPose(frame);
		waitForTimer(120);
	}
	addObjectToInventory(kInventoryObjectVase, kPart68TakenObjectIndex);
	_objectsState[kVaseTakenStateIndex] = 1;
	part68ApplyObjectState(255);
}

/**
 * Igor gives the object to the character.
 */
void IgorEngine::part68GiveRecordsToBoris() {
	part68DrawUndertaker(kPart68BorisFacing);
	waitForTimer(30);
	drawAnimRect(kPart68GiveAnimOffset, kPart68GiveFramesOffset, kPart68GiveWidth, kPart68GiveHeight);
	if (_inventoryInfo[kPart68GiveInventoryIndex] != 0) {
		removeInventoryEntry(kPart68GiveInventoryIndex);
		drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	}
	playSound(63, 1);
	igorSayAndWait(214, 1, 1019);
	part68SayBoris({ { 215, 1, 1020 }, { 216, 1, 1021 }, { 217, 1, 1022 }, { 218, 1, 1023 } });
	for (int frame = 1; frame <= 3; ++frame) {
		drawAnimRect(kPart68GiveAnimOffset, kPart68GiveFramesOffset + frame * kPart68GiveFrameSize, kPart68GiveWidth, kPart68GiveHeight);
		waitForTimer(_game.version == kIdSpaFloppy ? 60 : 40);
	}
	part68SayBoris({ { 219, 1, 1024 }, { 220, 1, 1025 }, { 221, 1, 1026 }, { 222, 1, 1027 } });
	part68DrawUndertaker(kPart68BorisFacing);
	waitForTimer(30);
	_objectsState[kRecordGivenToUndertakerStateIndex] = 1;
	part68ApplyObjectState(255);
}

/**
 * Chooses names at random for the replies
 */
void IgorEngine::part68ChooseRandomNames(int code) {
	uint8 *data = _gameState.dialogueData;
	if (code == 10) {
		const int entries = _objectsState[kLighthouseMessageSentStateIndex] == 1 ? 4 : 5;
		for (int i = 1; i <= entries; ++i) {
			data[27 + 6 * i] = _part68QuestionCounter;
			++_part68QuestionCounter;
			if (_part68QuestionCounter == 16) {
				_part68QuestionCounter = 1;
			}
		}
		if (_objectsState[kLighthouseMessageSentStateIndex] == 1) {
			data[57] = 20;
			data[58] = 55;
			data[56] = 0;
			data[59] = kPart68TalkCodeEnd;
		}
	}
	if (code == 20) {
		data[6] = 0;
	}
	if (code == 30) {
		data[12] = 0;
	}
}

/**
 * The sprites of the character going away.
 */
void IgorEngine::part68BorisLeaves() {
	for (int frame = 1; frame <= 6; ++frame) {
		decodeAnimFrame(getAnimFrame(kPart68LeaveBase, kPart68LeaveTable, frame), _screenVGA, true);
		waitForTimer(30);
	}
}

void IgorEngine::part68Conversation() {
	UpdateRoomBackgroundProc previousBackground = _updateRoomBackground;
	_updateRoomBackground = &IgorEngine::part68UpdateBackgroundTalk;
	_updateDialogue = &IgorEngine::part68SayBorisReplies;
	loadDialogueData(DLG_Part68);
	const bool resuming = _dialogueResuming;
	_dialogueResuming = false;
	_part68QuestionCounter = 1;
	part68DrawUndertaker(kPart68BorisStanding);

	memset(_screenVGA + 46080, 0, 17920);
	_gameState.dialogueStarted = 1;
	if (!resuming) {
		_gameState.dialogueChoiceStart = 1;
		_gameState.dialogueChoiceCount = 1;
		// boris says different greeting depending if its the first time or not
		if (_objectsState[kBorisIntroducedStateIndex] == 0) {
			igorSayAndWait(225, 1, 1029);
			static const int replies[4] = { 1, 2, 3, 4 };
			static const int sounds[4] = { 1051, 1052, 1053, 1054 };
			part68SayBorisReplies(replies, sounds, 4);
			_objectsState[kBorisIntroducedStateIndex] = 1;
		} else {
			igorSayAndWait(226, 1, 1030);
			static const int replies[4] = { 5, 6, 7, 8 };
			static const int sounds[4] = { 1055, 1056, 1057, 1058 };
			part68SayBorisReplies(replies, sounds, 4);
		}
	}

	bool done = false;
	do {
		drawDialogueChoices();
		part68DrawUndertaker(kPart68BorisStanding);
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
		dialogueReplyToQuestion(kPart68TextX, kPart68TextY, kPart68TextR, kPart68TextG, kPart68TextB);
		const int offset = (_dialogueInfo[choice] - 1) * 6 + (_gameState.dialogueChoiceCount - 1) * 30 +
						   (_gameState.dialogueChoiceStart - 1) * 30;
		_part68LastDialogueCode = _gameState.dialogueData[offset + 5];
		if (_part68LastDialogueCode != 0) {
			part68ChooseRandomNames(_part68LastDialogueCode);
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

	if (_part68LastDialogueCode == kPart68TalkCodeEnd) {
		static const int replies[4] = { 29, 30, 31, 32 };
		static const int sounds[4] = { 1079, 1080, 1081, 1082 };
		part68SayBorisReplies(replies, sounds, 4);
		part68BorisLeaves();
		_objectsState[kUndertakerLeftStateIndex] = 2;
	}
	memset(_screenVGA + 46080, 0, 17920);
	drawVerbsPanel();
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	_currentAction.verb = kVerbWalk;
	_gameState.dialogueStarted = 0;
	_updateDialogue = 0;
	_updateRoomBackground = previousBackground;
}

void IgorEngine::part68TalkToBoris() {
	part68Conversation();
	if (_gameStateLoaded || _eventQuitGame)
		return;
	if (_objectsState[kUndertakerLeftStateIndex] == 2) {
		_objectsState[kUndertakerLeftStateIndex] = 1;
		part68ApplyObjectState(255);
	}
}

void IgorEngine::part68EnterCemetery() {
	if (_objectsState[kUndertakerLeftStateIndex] == 0) {
		igorSayAndWait(223, 2, 1028);
		part68TalkToBoris();
	} else {
		_currentPart = 710;
	}
}

void IgorEngine::part68ExecuteAction(int action) {
	switch (action) {
	case 101: // exit through either side will cause next entry to be that side
		_objectsState[kCemeteryEntrySideStateIndex] = 1;
		_currentPart = 700;
		break;
	case 102: // take vase
		part68TakeVase();
		break;
	case 103: // look at vase
		igorSay(201, 1, 1007);
		break;
	case 104: // talk to Boris
		part68TalkToBoris();
		break;
	case 105: // look at Boris
		igorSay(212, 2, 1018);
		break;
	case 106:
		part68EnterCemetery();
		break;
	case 108:
		part68GiveRecordsToBoris();
		break;
	case 109:
		_objectsState[kCemeteryEntrySideStateIndex] = 0;
		_currentPart = 700;
		break;
	default:
		error("part68ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part68() {
	if (isCDVersion())
		playMusic(4);
	_gameState.enableLight = 2;
	loadActionData(DAT_Part68);
	part68LoadRoom();
	static const int anim[] = { ANM_Part68, 0 };
	loadAnimData(anim);
	_roomDataOffsets = PART_68_ROOM_DATA_OFFSETS;
	setRoomClickFix(143, -1, -1, false);
	partMainSetExecuteAction(1, &IgorEngine::part68ExecuteAction);
	_updateRoomBackground = &IgorEngine::part68UpdateBackground;
	SET_PAL_208_96_1();
	part68ApplyObjectState(255);
	memcpy(_screenVGA, _screenLayer1, 46080);

	// the colors that change are darker than their palette
	darkenPalette(_paletteBuffer, 192, 207, 10);

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 680) {
			if (_objectsState[kCemeteryEntrySideStateIndex] == 0) {
				part68EnterFromRight();
			} else {
				part68EnterFromLeft();
			}
		} else {
			part68EnterFromCemetery();
		}
	}
	enterPartLoop();
	if (takeResumeDialogue(1)) { // loaded while talking to boris
		part68TalkToBoris();
		finishResumedDialogue();
	}
	while ((_currentPart == 680 || _currentPart == 681) && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
