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
#include "igor/statics.h"

namespace Igor {

static const DialogueDataOffsets kNoDialogueData = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// Conversation with the old lady
static const DialogueDataOffsets kOldLadyDialogueData = {
	-162, 3,   // questions
	433, 10,   // replies
	30, 30,    // choices matrix size, replies data
	1553,      // question sounds
	1559, 10   // reply sounds
};

// Conversation with Laura
static const DialogueDataOffsets kLauraDialogueData = {
	-131, 7,   // questions
	1120, 16,  // replies
	30, 60,    // choices matrix size, replies data
	2852,      // question sounds
	2866, 16   // reply sounds
};

// Frames of the old lady's idle animation, indexed by step 1..6
static const uint8 kOldLadyIdleFrames[] = { 0, 1, 2, 3, 4, 2 };

// Frames of the pick up animation
static const uint8 kPickUpFrames[] = { 0, 1, 2, 1, 2, 1, 2, 1, 2, 0 };

// Frames shown while the old lady looks at the animal Igor found and recognizes it
static const uint8 kOldLadyPetFrames[] = { 6, 15, 7, 17, 9, 10, 9, 10, 9, 10, 1 };

// Frames shown while the old lady looks for the rocket in her bag
static const uint8 kOldLadyBagFrames[] = { 11, 15, 16, 6, 17, 1 };

/**
 * Draws a frame of FRM_Park1 (old lady and Igor) preserving the dialogue text colors.
 */
void IgorEngine::PARK_DRAW_LADY_FRAME(uint8 *dst, int frame) {
	decodeAnimFrame(getAnimFrame(kParkFrames, kParkFrameTable, frame), dst, true);
}

/**
 * Draws a frame of FRM_ParkLaura1 on the screen preserving the dialogue text colors.
 */
void IgorEngine::PARK_DRAW_LAURA_FRAME(int frame) {
	decodeAnimFrame(getAnimFrame(kParkLauraFrames, kParkLauraFrameTable, frame), _screenVGA, true);
}

/**
 * Plays the pick up animation, one raw 27x49 frame at a time. The first row of every frame is
 * not drawn.
 */
void IgorEngine::PARK_PICK_UP_ANIMATION(int screenOffset, int framesOffset) {
	for (uint i = 0; i < ARRAYSIZE(kPickUpFrames); ++i) {
		for (int row = 1; row <= 48; ++row) {
			memcpy(_screenVGA + screenOffset + row * 320,
					_animFramesBuffer + framesOffset + kPickUpFrames[i] * 1323 + row * 27, 27);
		}
		waitForTimer(30);
	}
}

/**
 * Birds chirp from time to time while the park is shown.
 */
void IgorEngine::PARK_UPDATE_AMBIENT_SOUND() {
	if (compareGameTick(1) && getRandomNumber(14) == 0) {
		switch (getRandomNumber(3)) {
		case 0:
			playSound(21, 1);
			break;
		case 1:
			playSound(22, 1);
			break;
		case 2:
			playSound(23, 1);
			break;
		case 3:
			playSound(18, 1);
			break;
		}
	}
}

void IgorEngine::PARK_UPDATE_DIALOGUE_LADY(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		PARK_DRAW_LADY_FRAME(_screenVGA, 1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PARK_DRAW_LADY_FRAME(_screenVGA, getRandomNumber(4) + 1);
		break;
	}
}

void IgorEngine::PARK_UPDATE_DIALOGUE_LAURA(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		PARK_DRAW_LAURA_FRAME(20);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PARK_DRAW_LAURA_FRAME(20 + getRandomNumber(5));
		break;
	}
}

/**
 * Copies lines of the loaded conversation replies to the dialogue texts pool and queues them
 * as one dialogue text.
 */
void IgorEngine::PARK_QUEUE_REPLY_TEXT(int reply, int count, int sound, int &textIndex) {
	for (int i = 0; i < count; ++i) {
		Common::strlcpy(_globalDialogueTexts[textIndex + i], _dialogueReplies[reply - 1 + i],
				sizeof(_globalDialogueTexts[0]));
	}
	ADD_DIALOGUE_TEXT(textIndex, count, sound);
	textIndex += count;
}

/**
 * Igor's blocking dialogues do not run the birds' sound.
 */
void IgorEngine::PARK_WAIT_FOR_IGOR_DIALOGUE() {
	UpdateRoomBackgroundProc roomBackground = _updateRoomBackground;
	_updateRoomBackground = 0;
	waitForEndOfIgorDialogue();
	_updateRoomBackground = roomBackground;
}

void IgorEngine::PARK_WAIT_FOR_LADY_DIALOGUE() {
	UpdateRoomBackgroundProc roomBackground = _updateRoomBackground;
	_updateRoomBackground = &IgorEngine::PARK_UPDATE_AMBIENT_SOUND;
	_updateDialogue = &IgorEngine::PARK_UPDATE_DIALOGUE_LADY;
	waitForEndOfCutsceneDialogue(121, 68, 63, 23, 0);
	_updateDialogue = 0;
	_updateRoomBackground = roomBackground;
}

void IgorEngine::PARK_WAIT_FOR_LAURA_DIALOGUE() {
	UpdateRoomBackgroundProc roomBackground = _updateRoomBackground;
	_updateRoomBackground = &IgorEngine::PARK_UPDATE_AMBIENT_SOUND;
	_updateDialogue = &IgorEngine::PARK_UPDATE_DIALOGUE_LAURA;
	waitForEndOfCutsceneDialogue(185, 69, 63, 0, 38);
	_updateDialogue = 0;
	_updateRoomBackground = roomBackground;
}

void IgorEngine::PART_34_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_34_EXEC_ACTION %d", action);
	// Only the birds keep chirping while an action takes over the room
	UpdateRoomBackgroundProc roomBackground = _updateRoomBackground;
	_updateRoomBackground = &IgorEngine::PARK_UPDATE_AMBIENT_SOUND;
	switch (action) {
	case 101:
		ADD_DIALOGUE_TEXT(201, 2, 562);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 102:
		ADD_DIALOGUE_TEXT(203, 2, 563);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		_objectsState[82] = 1;
		PART_34_APPLY_OBJECT_STATE(3);
		break;
	case 103:
		PART_34_ACTION_103_TAKE();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(205, 1, 564);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		PART_34_ACTION_105_TALK();
		break;
	case 106:
		ADD_DIALOGUE_TEXT(206, 2, 565);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107:
		ADD_DIALOGUE_TEXT(208, 2, 566);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 108:
		PART_34_ACTION_108_OLD_LADY();
		break;
	case 109:
		PART_34_ACTION_109_SCROLL_RIGHT();
		break;
	default:
		warning("PART_34_EXEC_ACTION unhandled action %d", action);
		break;
	}
	_updateRoomBackground = roomBackground;
}

void IgorEngine::PART_34_ACTION_103_TAKE() {
	if (_inventoryInfo[62] != 0) {
		ADD_DIALOGUE_TEXT(210, 1, 567);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		return;
	}
	PARK_PICK_UP_ANIMATION(16499, kParkPickUpFrames);
	addObjectToInventory(27, 62);
	PART_34_APPLY_OBJECT_STATE(255);
}

void IgorEngine::PART_34_ACTION_105_TALK() {
	PART_34_OLD_LADY_CONVERSATION();
	PART_34_APPLY_OBJECT_STATE(255);
	_parkLadyIdleStep = 4;
	PART_34_LADY_IDLE(4);
}

/**
 * Igor shows the old lady the animal he found. Depending on the animal she either rejects it
 * or recognizes her pet, gives Igor a rocket and leaves, and Laura shows up.
 */
void IgorEngine::PART_34_ACTION_108_OLD_LADY() {
	PARK_DRAW_LADY_FRAME(_screenVGA, 1);
	ADD_DIALOGUE_TEXT(211, 2, 568);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	PARK_WAIT_FOR_IGOR_DIALOGUE();
	for (int frame = 12; frame <= 13; ++frame) {
		PARK_DRAW_LADY_FRAME(_screenVGA, frame);
		waitForTimer(45);
	}
	if (_objectsState[1] == 0) {
		ADD_DIALOGUE_TEXT(213, 1, 569);
		ADD_DIALOGUE_TEXT(216, 1, 570);
		SET_DIALOGUE_TEXT(1, 2);
		startCutsceneDialogue(121, 58, 63, 23, 0);
		PARK_WAIT_FOR_LADY_DIALOGUE();
		for (int frame = 14; frame >= 12; --frame) {
			PARK_DRAW_LADY_FRAME(_screenVGA, frame);
			waitForTimer(45);
		}
		PARK_DRAW_LADY_FRAME(_screenVGA, 17);
		_parkLadyIdleStep = 1;
		return;
	}

	ADD_DIALOGUE_TEXT(217, 1, 571);
	ADD_DIALOGUE_TEXT(218, 1, 572);
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(121, 58, 63, 23, 0);
	PARK_WAIT_FOR_LADY_DIALOGUE();
	_parkLadyIdleStep = 1;
	for (uint i = 0; i < ARRAYSIZE(kOldLadyPetFrames); ++i) {
		PARK_DRAW_LADY_FRAME(_screenVGA, kOldLadyPetFrames[i]);
		waitForTimer(45);
	}
	removeObjectFromInventory(59);
	ADD_DIALOGUE_TEXT(219, 2, 573);
	ADD_DIALOGUE_TEXT(221, 2, 574);
	ADD_DIALOGUE_TEXT(223, 1, 575);
	ADD_DIALOGUE_TEXT(224, 2, 576);
	SET_DIALOGUE_TEXT(1, 4);
	startCutsceneDialogue(121, 58, 63, 23, 0);
	PARK_WAIT_FOR_LADY_DIALOGUE();
	for (uint i = 0; i < ARRAYSIZE(kOldLadyBagFrames); ++i) {
		PARK_DRAW_LADY_FRAME(_screenVGA, kOldLadyBagFrames[i]);
		waitForTimer(30);
	}
	while (isDialogueSpeechPlaying() && !_eventQuitGame) {
		waitForTimer();
	}
	addObjectToInventory(26, 61);
	ADD_DIALOGUE_TEXT(226, 1, 577);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	PARK_WAIT_FOR_IGOR_DIALOGUE();
	ADD_DIALOGUE_TEXT(227, 2, 578);
	ADD_DIALOGUE_TEXT(229, 1, 579);
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(121, 58, 63, 23, 0);
	PARK_WAIT_FOR_LADY_DIALOGUE();
	ADD_DIALOGUE_TEXT(230, 1, 580);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	PARK_WAIT_FOR_IGOR_DIALOGUE();
	for (int frame = 18; frame <= 31; ++frame) {
		PARK_DRAW_LADY_FRAME(_screenVGA, frame);
		waitForTimer(30);
	}
	_objectsState[80] = 1;

	PART_34_LAURA_CONVERSATION();

	loadActionData(DAT_ParkLeft);
	loadRoomData(PAL_Park, IMG_Park, BOX_Park, MSK_Park, TXT_Park);
	SET_PAL_240_48_1();
	static const int frames[] = { FRM_Park1, FRM_Park2, FRM_Park3, FRM_Park4, 0 };
	loadAnimData(frames, kParkFrames);
	PART_34_APPLY_OBJECT_STATE(255);
}

/**
 * Conversation with the old lady. The first time the lady greets Igor and Igor chooses what to ask,
 * the following times only the greetings are exchanged.
 */
void IgorEngine::PART_34_OLD_LADY_CONVERSATION() {
	_roomDataOffsets.dlg = kOldLadyDialogueData;
	loadDialogueData(DLG_ParkLady);
	if (_objectsState[81] == 0) {
		memset(_screenVGA + 46080, 0, 17920);
	}
	int textIndex = 250;
	PARK_QUEUE_REPLY_TEXT(1, 1, 584, textIndex);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(160, 58, 63, 63, 63);
	PARK_WAIT_FOR_IGOR_DIALOGUE();
	if (_objectsState[81] == 0) {
		textIndex = 250;
		PARK_QUEUE_REPLY_TEXT(2, 1, 585, textIndex);
		PARK_QUEUE_REPLY_TEXT(3, 1, 586, textIndex);
		SET_DIALOGUE_TEXT(1, 2);
		startCutsceneDialogue(121, 68, 63, 23, 0);
		PARK_WAIT_FOR_LADY_DIALOGUE();
		_updateDialogue = &IgorEngine::PARK_UPDATE_DIALOGUE_LADY;
		handleDialogue(121, 68, 63, 23, 0, false);
		_updateDialogue = 0;
	}
	textIndex = 250;
	PARK_QUEUE_REPLY_TEXT(4, 2, 587, textIndex);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(121, 68, 63, 23, 0);
	PARK_WAIT_FOR_LADY_DIALOGUE();
	if (_objectsState[81] == 0) {
		memset(_screenVGA + 46080, 0, 17920);
		drawVerbsPanel();
		drawInventory(_inventoryInfo[72], 0);
		_currentAction.verb = kVerbWalk;
		_objectsState[81] = 1;
	}
	_roomDataOffsets.dlg = kNoDialogueData;
}

/**
 * Laura walks into the park, talks with Igor and leaves.
 */
void IgorEngine::PART_34_LAURA_CONVERSATION() {
	static const int frames[] = { FRM_ParkLaura1, FRM_ParkLaura2, FRM_ParkLaura3, 0 };
	loadAnimData(frames, kParkLauraFrames);
	memcpy(_paletteBuffer + 176 * 3, _animFramesBuffer + kParkLauraPalette, 48);
	memcpy(_currentPalette + 176 * 3, _animFramesBuffer + kParkLauraPalette, 48);
	setPaletteRange(176, 191);

	_roomDataOffsets.dlg = kLauraDialogueData;
	loadDialogueData(DLG_ParkLaura);
	memset(_screenVGA + 46080, 0, 17920);
	for (int frame = 1; frame <= 19; ++frame) {
		PARK_DRAW_LAURA_FRAME(frame);
		waitForTimer(30);
	}
	int textIndex = 250;
	PARK_QUEUE_REPLY_TEXT(1, 1, 600, textIndex);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(185, 69, 63, 0, 38);
	PARK_WAIT_FOR_LAURA_DIALOGUE();
	for (int frame = 26; frame <= 29; ++frame) {
		PARK_DRAW_LAURA_FRAME(frame);
		waitForTimer(5);
	}
	textIndex = 250;
	PARK_QUEUE_REPLY_TEXT(2, 1, 601, textIndex);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(162, 64, 63, 63, 63);
	WalkData *wd = &_walkData[_walkDataLastIndex - 1];
	wd->x = 162;
	wd->y = 117;
	wd->posNum = kFacingPositionRight;
	PARK_WAIT_FOR_IGOR_DIALOGUE();
	textIndex = 250;
	PARK_QUEUE_REPLY_TEXT(3, 1, 602, textIndex);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(185, 69, 63, 0, 38);
	PARK_WAIT_FOR_LAURA_DIALOGUE();
	_updateDialogue = &IgorEngine::PARK_UPDATE_DIALOGUE_LAURA;
	handleDialogue(185, 69, 63, 0, 38, false);
	_updateDialogue = 0;
	PARK_DRAW_LAURA_FRAME(29);
	textIndex = 250;
	PARK_QUEUE_REPLY_TEXT(4, 1, 603, textIndex);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(162, 64, 63, 63, 63);
	PARK_WAIT_FOR_IGOR_DIALOGUE();
	for (int frame = 31; frame <= 57; ++frame) {
		PARK_DRAW_LAURA_FRAME(frame);
		waitForTimer(30);
	}
	textIndex = 250;
	PARK_QUEUE_REPLY_TEXT(5, 1, 604, textIndex);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(162, 64, 63, 63, 63);
	PARK_WAIT_FOR_IGOR_DIALOGUE();
	memset(_screenVGA + 46080, 0, 17920);
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	_currentAction.verb = kVerbWalk;
	_roomDataOffsets.dlg = kNoDialogueData;
}

void IgorEngine::PART_34_ACTION_109_SCROLL_RIGHT() {
	int xPos = 240;
	const int yPos = 124;
	int step = 1;
	_gameTicks = 8; // original counter 15 normalized to 8-tick engine units
	do {
		if (compareGameTick(1, 16)) {
			for (int y = 0; y <= 143; ++y) {
				memcpy(_screenLayer2 + y * 320, _screenLayer1 + y * 320 + step * 8, 320 - step * 8);
				memcpy(_screenLayer2 + y * 320 + 320 - step * 8, _animFramesBuffer + y * 160, step * 8);
			}
			if (step < 15) {
				xPos += _walkScaleTable[0x8F9 + _walkCurrentFrame];
				WalkData::setNextFrame(kFacingPositionRight, _walkCurrentFrame);
			} else {
				_walkCurrentFrame = 0;
			}
			int dstOffset = (yPos - 50) * 320 + xPos - 15 - step * 8;
			for (int row = 0; row <= 49; ++row) {
				dstOffset += 320;
				for (int col = 0; col <= 29; ++col) {
					const uint8 color = _facingIgorFrames[kFacingPositionRight - 1]
						[_walkCurrentFrame * 1500 + row * 30 + col];
					if (color != 0) {
						_screenLayer2[dstOffset + col] = color;
					}
				}
			}
			memcpy(_screenVGA, _screenLayer2, 46080);
			++step;
		}
		waitForTimer();
	} while (step != 21);

	_walkData[0].setPos(xPos - 160, yPos, kFacingPositionRight, 0);
	_walkData[0].setDefaultScale();
	_currentPart = 351;
}

/**
 * Redraws the old lady if she is in the park and updates the state dependent objects.
 */
void IgorEngine::PART_34_APPLY_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[80] == 0 && _objectsState[73] == 1) {
			PARK_DRAW_LADY_FRAME(_screenLayer1, 1);
		} else {
			_roomObjectAreasTable[5].object = 0;
		}
	}
	if (num == 3 || num == 255) {
		_roomObjectAreasTable[4].object = (_objectsState[82] == 0) ? 2 : 3;
	}
}

/**
 * The old lady idles in her place by shifting between the frames of FRM_Park4. Pixels of Igor and
 * of the dialogue text are kept on the screen.
 */
void IgorEngine::PART_34_LADY_IDLE(int step) {
	const uint8 *frame = _animFramesBuffer + kParkIdleFrames + kOldLadyIdleFrames[step - 1] * 630;
	uint8 buffer[35 * 18];
	for (int y = 0; y < 35; ++y) {
		for (int x = 0; x < 18; ++x) {
			const uint8 color = _screenVGA[(73 + y) * 320 + 112 + x];
			if ((color >= 0xC0 && color <= 0xCF) || color == 0xF0 || color == 0xF1) {
				buffer[y * 18 + x] = color;
			} else {
				buffer[y * 18 + x] = frame[y * 18 + x];
			}
		}
	}
	for (int y = 0; y < 35; ++y) {
		memcpy(_screenVGA + (73 + y) * 320 + 112, buffer + y * 18, 18);
		memcpy(_screenLayer1 + (73 + y) * 320 + 112, frame + y * 18, 18);
	}
}

void IgorEngine::PART_34_UPDATE_ROOM_BACKGROUND() {
	PARK_UPDATE_AMBIENT_SOUND();
	if (compareGameTick(61) && _objectsState[80] == 0 && _objectsState[73] == 1 && getRandomNumber(4) == 0) {
		PART_34_LADY_IDLE(_parkLadyIdleStep);
		_parkLadyIdleStep = (_parkLadyIdleStep == 6) ? 1 : _parkLadyIdleStep + 1;
	}
}

void IgorEngine::PART_34() {
	_gameState.enableLight = 1;
	loadActionData(DAT_ParkLeft);
	loadRoomData(PAL_ParkRight, IMG_ParkRight, BOX_ParkRight, MSK_ParkRight, TXT_ParkRight);
	for (int y = 0; y <= 143; ++y) {
		memcpy(_animFramesBuffer + y * 160, _screenLayer1 + y * 320 + 160, 160);
	}
	loadRoomData(PAL_Park, IMG_Park, BOX_Park, MSK_Park, TXT_Park); // active left panel;
	static const int frames[] = { FRM_Park1, FRM_Park2, FRM_Park3, FRM_Park4, 0 };
	loadAnimData(frames, kParkFrames);
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_34_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_34_UPDATE_ROOM_BACKGROUND;
	_parkLadyIdleStep = 1;
	_roomDataOffsets = PART_34_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	PART_34_APPLY_OBJECT_STATE(255);

	if (!restoreRoomAfterLoad()) {
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
	}
	enterPartLoop();
	while (_currentPart == 340 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (_currentPart == kInvalidPart && !_gameStateLoaded) {
		fadeOut(768);
	}
}

} // End of namespace Igor
