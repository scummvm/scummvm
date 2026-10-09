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

// A character stands on the right of the room. His head is a 33x43 picture drawn at (263,88); the frames follow
// the first part of the animation resource.
static const int kPart68NpcOffset = 88 * 320 + 263;
static const int kPart68NpcWidth = 33;
static const int kPart68NpcHeight = 43;
static const int kPart68NpcFramesOffset = 2516;
static const int kPart68NpcFrameSize = kPart68NpcWidth * kPart68NpcHeight;
static const int kPart68NpcStanding = 4;
static const int kPart68NpcFacing = 3;

// A small picture (39x14, four frames) at (39,31) blinks from time to time.
static const int kPart68AmbientOffset = 31 * 320 + 39;
static const int kPart68AmbientWidth = 39;
static const int kPart68AmbientHeight = 14;
static const int kPart68AmbientFramesOffset = 0x3BB7;

// The object in the room (9x12, two frames) at (179,92).
static const int kPart68ObjectOffset = 92 * 320 + 179;
static const int kPart68ObjectWidth = 9;
static const int kPart68ObjectHeight = 12;

// The picture of the exchange (23x50, two frames) at (170,83).
static const int kPart68PoseOffset = 83 * 320 + 170;
static const int kPart68PoseWidth = 23;
static const int kPart68PoseHeight = 50;
static const int kPart68PoseFramesOffset = 0xD8;

// The picture of the object that is given (52x30, four frames) at (244,86).
static const int kPart68GiveOffset = 86 * 320 + 244;
static const int kPart68GiveWidth = 52;
static const int kPart68GiveHeight = 30;
static const int kPart68GiveFramesOffset = 0x443F;
static const int kPart68GiveFrameSize = kPart68GiveWidth * kPart68GiveHeight;

// Sprites played when the character leaves: frame table at offset 0x140E of the block starting at 0x5C9F.
static const int kPart68LeaveBase = 0x5C9F;
static const int kPart68LeaveTable = 0x140E;

// Where the character's lines are shown.
static const int kPart68TextX = 282;
static const int kPart68TextY = 85;
static const int kPart68TextR = 63;
static const int kPart68TextG = 50;
static const int kPart68TextB = 0;

// Flags of the story kept in the objects state.
static const int kPart68SideState = 90;
static const int kPart68ObjectTakenState = 91;
static const int kPart68TalkState = 92;
static const int kPart68GivenState = 93;
static const int kPart68IntroState = 94;
static const int kPart68FlagState = 104;

// Inventory objects.
static const int kPart68TakenObject = 34;
static const int kPart68TakenObjectIndex = 69;
static const int kPart68GiveInventoryIndex = 40;

static const int kPart68TalkCodeEnd = 100;

void IgorEngine::PART_68_LOAD_ROOM() {
	loadRoomData(PAL_Part68, IMG_Part68, BOX_Part68, MSK_Part68, TXT_Part68);
	SET_PAL_240_48_1();
}

void IgorEngine::PART_68_DRAW_NPC(int frame) {
	drawAnimRect(kPart68NpcOffset, kPart68NpcFramesOffset + frame * kPart68NpcFrameSize, kPart68NpcWidth, kPart68NpcHeight,
				 true, kBlendBehindIgorAndText);
}

void IgorEngine::PART_68_DRAW_AMBIENT(int frame) {
	drawAnimRect(kPart68AmbientOffset, kPart68AmbientFramesOffset + frame * kPart68AmbientWidth * kPart68AmbientHeight,
				 kPart68AmbientWidth, kPart68AmbientHeight);
}

void IgorEngine::PART_68_DRAW_OBJECT(int frame) {
	copyArea(_screenLayer1, kPart68ObjectOffset, 320, _animFramesBuffer + frame * kPart68ObjectWidth * kPart68ObjectHeight,
			 kPart68ObjectWidth, kPart68ObjectWidth, kPart68ObjectHeight);
}

void IgorEngine::PART_68_DRAW_POSE(int frame) {
	drawAnimRect(kPart68PoseOffset, kPart68PoseFramesOffset + frame * kPart68PoseWidth * kPart68PoseHeight,
				 kPart68PoseWidth, kPart68PoseHeight);
}

/**
 * The character is in the room until the conversation with him is over; after that the room is loaded again
 * without him. The object is drawn while it is there.
 */
void IgorEngine::PART_68_APPLY_OBJECT_STATE(int num) {
	if (num == 3 || num == 255) {
		if (_objectsState[kPart68TalkState] == 0) {
			PART_68_DRAW_NPC(0);
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
			PART_68_LOAD_ROOM();
			WRITE_LE_UINT16(_roomActionsTable + 156, 0x9081);
			_roomActionsTable[246] = 3;
			for (int i = 0; i < MAX_ROOM_OBJECT_AREAS; ++i) {
				if (_roomObjectAreasTable[i].object == 3) {
					_roomObjectAreasTable[i].object = 0;
				}
			}
			_part68NpcPresent = false;
		}
	}
	if (num == 2 || num == 255) {
		if (_objectsState[kPart68ObjectTakenState] == 0) {
			PART_68_DRAW_OBJECT(0);
		} else {
			PART_68_DRAW_OBJECT(1);
			for (int i = 0; i < MAX_ROOM_OBJECT_AREAS; ++i) {
				if (_roomObjectAreasTable[i].object == 2) {
					_roomObjectAreasTable[i].object = 0;
				}
			}
		}
	}
}

void IgorEngine::PART_68_UPDATE_AMBIENT() {
	if (compareGameTick(0x3D)) {
		if (_roomAmbientIndex == 3) {
			_roomAmbientIndex = 0;
			PART_68_DRAW_AMBIENT(0);
		} else if (getRandomNumber(9) == 0) {
			_roomAmbientIndex = getRandomNumber(3);
			PART_68_DRAW_AMBIENT(_roomAmbientIndex);
		}
	}
}

/**
 * Used while the character is talking: no idle frames of him.
 */
void IgorEngine::PART_68_UPDATE_ROOM_BACKGROUND_TALK() {
	if (compareGameTick(1) && getRandomNumber(49) == 0) {
		playSound(41, 1);
	}
	PART_68_UPDATE_AMBIENT();
}

void IgorEngine::PART_68_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(1) && getRandomNumber(49) == 0) {
		playSound(41, 1);
	}
	if (compareGameTick(0x3D) && _part68NpcPresent) {
		PART_68_DRAW_NPC(getRandomNumber(2));
	}
	PART_68_UPDATE_AMBIENT();
}

void IgorEngine::PART_68_UPDATE_DIALOGUE_NPC(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		PART_68_DRAW_NPC(kPart68NpcStanding);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_68_DRAW_NPC(kPart68NpcStanding + getRandomNumber(4));
		break;
	}
}

/**
 * The character says lines of the room texts.
 */
void IgorEngine::PART_68_NPC_SAY(const Common::Array<DialogueText> &lines) {
	UpdateRoomBackgroundProc previous = _updateRoomBackground;
	_updateRoomBackground = &IgorEngine::PART_68_UPDATE_ROOM_BACKGROUND_TALK;
	cutsceneSayWithCallback(kPart68TextX, kPart68TextY, kPart68TextR, kPart68TextG, kPart68TextB, lines,
							&IgorEngine::PART_68_UPDATE_DIALOGUE_NPC);
	_updateRoomBackground = previous;
}

/**
 * The character says lines taken from the replies of the conversation, one line per page.
 */
void IgorEngine::PART_68_NPC_SAY_REPLIES(const int *replies, const int *sounds, int count) {
	Common::Array<DialogueText> lines;
	for (int i = 0; i < count; ++i) {
		Common::strlcpy(_globalDialogueTexts[250 + i], _dialogueReplies[replies[i] - 1], sizeof(_globalDialogueTexts[250 + i]));
		DialogueText line = { 250 + i, 1, sounds[i] };
		lines.push_back(line);
	}
	PART_68_NPC_SAY(lines);
}

/**
 * Igor comes from the right edge and walks to the left.
 */
void IgorEngine::PART_68_ENTER_FROM_RIGHT() {
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
	waitForIgorMove(&IgorEngine::PART_68_UPDATE_ROOM_BACKGROUND);
}

/**
 * Igor comes from the left edge and walks to the right.
 */
void IgorEngine::PART_68_ENTER_FROM_LEFT() {
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
	waitForIgorMove(&IgorEngine::PART_68_UPDATE_ROOM_BACKGROUND);
}

/**
 * Igor appears next to the character, smaller, and walks down.
 */
void IgorEngine::PART_68_ENTER_INSIDE() {
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
	waitForIgorMove(&IgorEngine::PART_68_UPDATE_ROOM_BACKGROUND);
}

/**
 * The character gives Igor the object; it goes to the inventory.
 */
void IgorEngine::PART_68_ACTION_102_takeObject() {
	if (_objectsState[kPart68GivenState] != 0) {
		return;
	}
	PART_68_DRAW_POSE(0);
	PART_68_DRAW_NPC(kPart68NpcFacing);
	waitForTimer(30);
	PART_68_NPC_SAY({ { 202, 1, 1008 }, { 203, 1, 1009 }, { 204, 1, 1010 }, { 205, 1, 1011 } });
	PART_68_DRAW_POSE(1);
	waitForTimer(30);

	// Igor turns to the right
	_walkData[_walkDataLastIndex - 1].posNum = kFacingPositionRight;
	_walkDataCurrentIndex = _walkDataLastIndex - 1;
	moveIgor(kFacingPositionRight, 0);
	_walkDataCurrentIndex = _walkDataLastIndex;
	igorSayAndWait({ { 206, 1, 1012 }, { 207, 1, 1013 } });
	PART_68_NPC_SAY({ { 208, 1, 1014 }, { 209, 1, 1015 }, { 210, 1, 1016 }, { 211, 1, 1017 } });
	PART_68_DRAW_NPC(kPart68NpcFacing);
	waitForTimer(30);
	for (int frame = 0; frame <= 1; ++frame) {
		PART_68_DRAW_POSE(frame);
		waitForTimer(120);
	}
	addObjectToInventory(kPart68TakenObject, kPart68TakenObjectIndex);
	_objectsState[kPart68ObjectTakenState] = 1;
	PART_68_APPLY_OBJECT_STATE(255);
}

/**
 * Igor gives the object to the character.
 */
void IgorEngine::PART_68_ACTION_108_giveObject() {
	PART_68_DRAW_NPC(kPart68NpcFacing);
	waitForTimer(30);
	drawAnimRect(kPart68GiveOffset, kPart68GiveFramesOffset, kPart68GiveWidth, kPart68GiveHeight);
	if (_inventoryInfo[kPart68GiveInventoryIndex] != 0) {
		removeInventoryEntry(kPart68GiveInventoryIndex);
		drawInventory(_inventoryInfo[72], 0);
	}
	playSound(63, 1);
	igorSayAndWait(214, 1, 1019);
	PART_68_NPC_SAY({ { 215, 1, 1020 }, { 216, 1, 1021 }, { 217, 1, 1022 }, { 218, 1, 1023 } });
	for (int frame = 1; frame <= 3; ++frame) {
		drawAnimRect(kPart68GiveOffset, kPart68GiveFramesOffset + frame * kPart68GiveFrameSize, kPart68GiveWidth, kPart68GiveHeight);
		waitForTimer(40);
	}
	PART_68_NPC_SAY({ { 219, 1, 1024 }, { 220, 1, 1025 }, { 221, 1, 1026 }, { 222, 1, 1027 } });
	PART_68_DRAW_NPC(kPart68NpcFacing);
	waitForTimer(30);
	_objectsState[kPart68GivenState] = 1;
	PART_68_APPLY_OBJECT_STATE(255);
}

/**
 * The changes of the conversation for some of the answers.
 */
void IgorEngine::PART_68_DIALOGUE_CODE(int code) {
	uint8 *data = _gameState.dialogueData;
	if (code == 10) {
		const int entries = _objectsState[kPart68FlagState] == 1 ? 4 : 5;
		for (int i = 1; i <= entries; ++i) {
			data[27 + 6 * i] = _part68QuestionCounter;
			++_part68QuestionCounter;
			if (_part68QuestionCounter == 16) {
				_part68QuestionCounter = 1;
			}
		}
		if (_objectsState[kPart68FlagState] == 1) {
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
void IgorEngine::PART_68_NPC_LEAVES() {
	for (int frame = 1; frame <= 6; ++frame) {
		decodeAnimFrame(getAnimFrame(kPart68LeaveBase, kPart68LeaveTable, frame), _screenVGA, true);
		waitForTimer(30);
	}
}

void IgorEngine::PART_68_CONVERSATION() {
	UpdateRoomBackgroundProc previousBackground = _updateRoomBackground;
	_updateRoomBackground = &IgorEngine::PART_68_UPDATE_ROOM_BACKGROUND_TALK;
	_updateDialogue = &IgorEngine::PART_68_UPDATE_DIALOGUE_NPC;
	loadDialogueData(DLG_Part68);
	_part68QuestionCounter = 1;
	PART_68_DRAW_NPC(kPart68NpcStanding);

	memset(_screenVGA + 46080, 0, 17920);
	_gameState.dialogueStarted = true;
	_gameState.dialogueChoiceStart = 1;
	_gameState.dialogueChoiceCount = 1;
	if (_objectsState[kPart68IntroState] == 0) {
		igorSayAndWait(225, 1, 1029);
		static const int replies[4] = { 1, 2, 3, 4 };
		static const int sounds[4] = { 1051, 1052, 1053, 1054 };
		PART_68_NPC_SAY_REPLIES(replies, sounds, 4);
		_objectsState[kPart68IntroState] = 1;
	} else {
		igorSayAndWait(226, 1, 1030);
		static const int replies[4] = { 5, 6, 7, 8 };
		static const int sounds[4] = { 1055, 1056, 1057, 1058 };
		PART_68_NPC_SAY_REPLIES(replies, sounds, 4);
	}

	bool done = false;
	do {
		drawDialogueChoices();
		PART_68_DRAW_NPC(kPart68NpcStanding);
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
			PART_68_DIALOGUE_CODE(_part68LastDialogueCode);
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
		PART_68_NPC_SAY_REPLIES(replies, sounds, 4);
		PART_68_NPC_LEAVES();
		_objectsState[kPart68TalkState] = 2;
	}
	memset(_screenVGA + 46080, 0, 17920);
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	_currentAction.verb = kVerbWalk;
	_gameState.dialogueStarted = false;
	_updateDialogue = 0;
	_updateRoomBackground = previousBackground;
}

void IgorEngine::PART_68_ACTION_104_talk() {
	PART_68_CONVERSATION();
	if (_objectsState[kPart68TalkState] == 2) {
		_objectsState[kPart68TalkState] = 1;
		PART_68_APPLY_OBJECT_STATE(255);
	}
}

void IgorEngine::PART_68_ACTION_106() {
	if (_objectsState[kPart68TalkState] == 0) {
		igorSayAndWait(223, 2, 1028);
		PART_68_ACTION_104_talk();
	} else {
		_currentPart = 710;
	}
}

void IgorEngine::PART_68_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		_objectsState[kPart68SideState] = 1;
		_currentPart = 700;
		break;
	case 102:
		PART_68_ACTION_102_takeObject();
		break;
	case 103:
		igorSay(201, 1, 1007);
		break;
	case 104:
		PART_68_ACTION_104_talk();
		break;
	case 105:
		igorSay(212, 2, 1018);
		break;
	case 106:
		PART_68_ACTION_106();
		break;
	case 108:
		PART_68_ACTION_108_giveObject();
		break;
	case 109:
		_objectsState[kPart68SideState] = 0;
		_currentPart = 700;
		break;
	default:
		error("PART_68_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_68() {
	playMusic(4);
	_gameState.enableLight = 2;
	loadActionData(DAT_Part68);
	PART_68_LOAD_ROOM();
	static const int anim[] = { ANM_Part68, 0 };
	loadAnimData(anim);
	_roomDataOffsets = PART_68_ROOM_DATA_OFFSETS;
	setRoomClickFix(143, -1, -1, false);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_68_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_68_UPDATE_ROOM_BACKGROUND;
	PART_68_APPLY_OBJECT_STATE(255);
	memcpy(_screenVGA, _screenLayer1, 46080);

	// the colors that change are darker than their palette
	for (int i = 192 * 3; i < 208 * 3; ++i) {
		const int value = _paletteBuffer[i] - 10;
		_paletteBuffer[i] = value < 1 ? 0 : value;
	}

	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 680) {
			if (_objectsState[kPart68SideState] == 0) {
				PART_68_ENTER_FROM_RIGHT();
			} else {
				PART_68_ENTER_FROM_LEFT();
			}
		} else {
			PART_68_ENTER_INSIDE();
		}
	}
	enterPartLoop();
	while ((_currentPart == 680 || _currentPart == 681) && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
