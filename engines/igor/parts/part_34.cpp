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
// ids of the dialogues of this room stored in a savegame while a choice menu is open
static const uint8 kOldLadyDialogueId = 1;
static const uint8 kLauraDialogueId = 2;

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
void IgorEngine::part34And35DrawOldLadyFrame(uint8 *dst, int frame) {
	decodeAnimFrame(getAnimFrame(kParkFrames, kParkFrameTable, frame), dst, true);
}

/**
 * Draws a frame of FRM_ParkLaura1 on the screen preserving the dialogue text colors.
 */
void IgorEngine::part34DrawLauraFrame(int frame) {
	decodeAnimFrame(getAnimFrame(kParkLauraFrames, kParkLauraFrameTable, frame), _screenVGA, true);
}

/**
 * Plays the pick up animation, one raw 27x49 frame at a time. The first row of every frame is
 * not drawn.
 */
void IgorEngine::part34And35PickUpAnimation(int screenOffset, int framesOffset) {
	for (uint i = 0; i < ARRAYSIZE(kPickUpFrames); ++i) {
		drawAnimRect(screenOffset + 320, framesOffset + kPickUpFrames[i] * 1323 + 27, 27, 48);
		waitForTimer(30);
	}
}

/**
 * Birds chirp.
 */
void IgorEngine::part34And35UpdateAmbientSound() {
	if (compareGameTick(1) && getRandomNumber(14) == 0) {
		if (_game.version == kIdSpaFloppy) {
			switch (getRandomNumber(2)) {
			case 0:
				playSound(21, 1);
				break;
			case 1:
				playSound(kSpaFloppySoundParkAmbient, 1);
				break;
			case 2:
				playSound(23, 1);
				break;
			}
		} else {
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
}

void IgorEngine::part34UpdateLadyDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		part34And35DrawOldLadyFrame(_screenVGA, 1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part34And35DrawOldLadyFrame(_screenVGA, getRandomNumber(4) + 1);
		break;
	}
}

void IgorEngine::part34UpdateLauraDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		part34DrawLauraFrame(20);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part34DrawLauraFrame(20 + getRandomNumber(5));
		break;
	}
}

/**
 * Copies lines of the loaded conversation replies to the dialogue texts pool and queues them
 * as one dialogue text.
 */
void IgorEngine::part34QueueReplyText(int reply, int count, int sound, int &textIndex) {
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
void IgorEngine::part34WaitForIgorDialogue() {
	UpdateRoomBackgroundProc roomBackground = _updateRoomBackground;
	_updateRoomBackground = 0;
	waitForEndOfIgorDialogue();
	_updateRoomBackground = roomBackground;
}

void IgorEngine::part34WaitForLadyDialogue() {
	UpdateRoomBackgroundProc roomBackground = _updateRoomBackground;
	_updateRoomBackground = &IgorEngine::part34And35UpdateAmbientSound;
	_updateDialogue = &IgorEngine::part34UpdateLadyDialogue;
	waitForEndOfCutsceneDialogue(121, 68, 63, 23, 0);
	_updateDialogue = 0;
	_updateRoomBackground = roomBackground;
}

void IgorEngine::part34WaitForLauraDialogue() {
	UpdateRoomBackgroundProc roomBackground = _updateRoomBackground;
	_updateRoomBackground = &IgorEngine::part34And35UpdateAmbientSound;
	_updateDialogue = &IgorEngine::part34UpdateLauraDialogue;
	waitForEndOfCutsceneDialogue(185, 69, 63, 0, 38);
	_updateDialogue = 0;
	_updateRoomBackground = roomBackground;
}

void IgorEngine::part34ExecuteAction(int action) {
	debugC(9, kDebugGame, "part34ExecuteAction %d", action);
	// Only the birds keep chirping while an action takes over the room
	UpdateRoomBackgroundProc roomBackground = _updateRoomBackground;
	_updateRoomBackground = &IgorEngine::part34And35UpdateAmbientSound;
	switch (action) {
	case 101: // look at statue
		igorSay(201, 2, 562);
		break;
	case 102: // look at tree
		igorSay(203, 2, 563);
		_objectsState[kTreeResinDiscoveredStateIndex] = 1;
		part34ApplyObjectState(3);
		break;
	case 103:
		part34TakeResin();
		break;
	case 104: // look at roses
		igorSay(205, 1, 564);
		break;
	case 105:
		part34TalkToOldLady();
		break;
	case 106: // look at old lady
		igorSay(206, 2, 565);
		break;
	case 107:
		igorSay(208, 2, 566);
		break;
	case 108:
		part34GiveLizardToOldLady();
		break;
	case 109:
		part34ScrollRight();
		break;
	default:
		warning("part34ExecuteAction unhandled action %d", action);
		break;
	}
	_updateRoomBackground = roomBackground;
}

void IgorEngine::part34TakeResin() {
	if (_inventoryInfo[62] != 0) {
		igorSay(210, 1, 567);
		return;
	}
	part34And35PickUpAnimation(16499, kParkPickUpFrames);
	addObjectToInventory(kInventoryObjectResin, 62);
	part34ApplyObjectState(255);
}

void IgorEngine::part34TalkToOldLady() {
	part34OldLadyConversation();
	if (_gameStateLoaded || _eventQuitGame)
		return;
	part34ApplyObjectState(255);
	_parkLadyIdleStep = 4;
	part34OldLadyIdleAnimationFrame(4);
}

void IgorEngine::part34GiveLizardToOldLady() {
	part34And35DrawOldLadyFrame(_screenVGA, 1);
	igorSay(211, 2, 568);
	part34WaitForIgorDialogue();
	for (int frame = 12; frame <= 13; ++frame) {
		part34And35DrawOldLadyFrame(_screenVGA, frame);
		waitForTimer(45);
	}
	if (_objectsState[kLizardFedStateIndex] == 0) {
		cutsceneSayStart(121, 58, 63, 23, 0, { { 213, 1, 569 }, { 216, 1, 570 } });
		part34WaitForLadyDialogue();
		for (int frame = 14; frame >= 12; --frame) {
			part34And35DrawOldLadyFrame(_screenVGA, frame);
			waitForTimer(45);
		}
		part34And35DrawOldLadyFrame(_screenVGA, 17);
		_parkLadyIdleStep = 1;
		return;
	}

	cutsceneSayStart(121, 58, 63, 23, 0, { { 217, 1, 571 }, { 218, 1, 572 } });
	part34WaitForLadyDialogue();
	_parkLadyIdleStep = 1;
	for (uint i = 0; i < ARRAYSIZE(kOldLadyPetFrames); ++i) {
		part34And35DrawOldLadyFrame(_screenVGA, kOldLadyPetFrames[i]);
		waitForTimer(45);
	}
	removeObjectFromInventory(59);
	cutsceneSayStart(121, 58, 63, 23, 0, { { 219, 2, 573 }, { 221, 2, 574 }, { 223, 1, 575 }, { 224, 2, 576 } });
	part34WaitForLadyDialogue();
	for (uint i = 0; i < ARRAYSIZE(kOldLadyBagFrames); ++i) {
		part34And35DrawOldLadyFrame(_screenVGA, kOldLadyBagFrames[i]);
		waitForTimer(30);
	}
	while (isDialogueSpeechPlaying() && !_eventQuitGame) {
		waitForTimer();
	}
	addObjectToInventory(kInventoryObjectRocket, 61);
	igorSay(226, 1, 577);
	part34WaitForIgorDialogue();
	cutsceneSayStart(121, 58, 63, 23, 0, { { 227, 2, 578 }, { 229, 1, 579 } });
	part34WaitForLadyDialogue();
	igorSay(230, 1, 580);
	part34WaitForIgorDialogue();
	for (int frame = 18; frame <= 31; ++frame) {
		part34And35DrawOldLadyFrame(_screenVGA, frame);
		waitForTimer(30);
	}
	_objectsState[kLizardGivenStateIndex] = 1;

	part34LauraConversation();
	if (_gameStateLoaded || _eventQuitGame)
		return;
	part34ReloadParkAfterLaura();
}

void IgorEngine::part34ReloadParkAfterLaura() {
	loadActionData(DAT_ParkLeft);
	loadRoomData(PAL_Park, IMG_Park, BOX_Park, MSK_Park, TXT_Park);
	SET_PAL_240_48_1();
	static const int frames[] = { FRM_Park1, FRM_Park2, FRM_Park3, FRM_Park4, 0 };
	loadAnimData(frames, kParkFrames);
	part34ApplyObjectState(255);
}

void IgorEngine::part34OldLadyConversation() {
	_roomDataOffsets.dlg = kOldLadyDialogueData;
	loadDialogueData(DLG_ParkLady);
	const bool resuming = _dialogueResuming;
	if (_objectsState[kOldLadyConversationSeenStateIndex] == 0) {
		memset(_screenVGA + 46080, 0, 17920);
	}
	int textIndex = 250;
	if (!resuming) {
		part34QueueReplyText(1, 1, 584, textIndex);
		SET_DIALOGUE_TEXT(1, 1);
		startCutsceneDialogue(160, 58, 63, 63, 63);
		part34WaitForIgorDialogue();
	}
	if (_objectsState[kOldLadyConversationSeenStateIndex] == 0) {
		if (!resuming) {
			textIndex = 250;
			part34QueueReplyText(2, 1, 585, textIndex);
			part34QueueReplyText(3, 1, 586, textIndex);
			SET_DIALOGUE_TEXT(1, 2);
			startCutsceneDialogue(121, 68, 63, 23, 0);
			part34WaitForLadyDialogue();
		}
		_updateDialogue = &IgorEngine::part34UpdateLadyDialogue;
		handleDialogue(121, 68, 63, 23, 0, false, kOldLadyDialogueId);
		_updateDialogue = 0;
		if (_gameStateLoaded || _eventQuitGame) {
			_roomDataOffsets.dlg = kNoDialogueData;
			return;
		}
	}
	textIndex = 250;
	part34QueueReplyText(4, 2, 587, textIndex);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(121, 68, 63, 23, 0);
	part34WaitForLadyDialogue();
	if (_objectsState[kOldLadyConversationSeenStateIndex] == 0) {
		memset(_screenVGA + 46080, 0, 17920);
		drawVerbsPanel();
		drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
		_currentAction.verb = kVerbWalk;
		_objectsState[kOldLadyConversationSeenStateIndex] = 1;
	}
	_roomDataOffsets.dlg = kNoDialogueData;
}

/**
 * Laura walks into the park, talks with Igor and leaves.
 */
void IgorEngine::part34LauraConversation() {
	static const int frames[] = { FRM_ParkLaura1, FRM_ParkLaura2, FRM_ParkLaura3, 0 };
	loadAnimData(frames, kParkLauraFrames);
	memcpy(_paletteBuffer + 176 * 3, _animFramesBuffer + kParkLauraPalette, 48);
	memcpy(_currentPalette + 176 * 3, _animFramesBuffer + kParkLauraPalette, 48);
	setPaletteRange(176, 191);

	_roomDataOffsets.dlg = kLauraDialogueData;
	loadDialogueData(DLG_ParkLaura);
	// a loaded savegame continues at the choice menu
	const bool resuming = _dialogueResuming;
	memset(_screenVGA + 46080, 0, 17920);
	int textIndex = 250;
	// the frames of Laura walking in are also drawn when resuming
	for (int frame = 1; frame <= 19; ++frame) {
		part34DrawLauraFrame(frame);
		if (!resuming)
			waitForTimer(30);
	}
	if (!resuming) {
		part34QueueReplyText(1, 1, 600, textIndex);
		SET_DIALOGUE_TEXT(1, 1);
		startCutsceneDialogue(185, 69, 63, 0, 38);
		part34WaitForLauraDialogue();
		for (int frame = 26; frame <= 29; ++frame) {
			part34DrawLauraFrame(frame);
			waitForTimer(5);
		}
		textIndex = 250;
		part34QueueReplyText(2, 1, 601, textIndex);
		SET_DIALOGUE_TEXT(1, 1);
		startCutsceneDialogue(162, 64, 63, 63, 63);
		WalkData *wd = &_walkData[_walkDataLastIndex - 1];
		wd->x = 162;
		wd->y = 117;
		wd->posNum = kFacingPositionRight;
		part34WaitForIgorDialogue();
		textIndex = 250;
		part34QueueReplyText(3, 1, 602, textIndex);
		SET_DIALOGUE_TEXT(1, 1);
		startCutsceneDialogue(185, 69, 63, 0, 38);
		part34WaitForLauraDialogue();
	}
	_updateDialogue = &IgorEngine::part34UpdateLauraDialogue;
	handleDialogue(185, 69, 63, 0, 38, false, kLauraDialogueId);
	_updateDialogue = 0;
	if (_gameStateLoaded || _eventQuitGame) {
		_roomDataOffsets.dlg = kNoDialogueData;
		return;
	}
	part34DrawLauraFrame(29);
	textIndex = 250;
	part34QueueReplyText(4, 1, 603, textIndex);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(162, 64, 63, 63, 63);
	part34WaitForIgorDialogue();
	for (int frame = 31; frame <= 57; ++frame) {
		part34DrawLauraFrame(frame);
		waitForTimer(30);
	}
	textIndex = 250;
	part34QueueReplyText(5, 1, 604, textIndex);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(162, 64, 63, 63, 63);
	part34WaitForIgorDialogue();
	memset(_screenVGA + 46080, 0, 17920);
	drawVerbsPanel();
	drawInventory(_inventoryInfo[kInventoryFirstVisible], 0);
	_currentAction.verb = kVerbWalk;
	_roomDataOffsets.dlg = kNoDialogueData;
}

void IgorEngine::part34ScrollRight() {
	int xPos = 240;
	const int yPos = 124;
	int step = 1;
	_gameTicks = 8; // original counter 15 normalized to 8-tick engine units
	do {
		if (compareGameTick(1, 16)) {
			copyArea(_screenLayer2, 0, 320, _screenLayer1 + step * 8, 320, 320 - step * 8, 144);
			copyArea(_screenLayer2, 320 - step * 8, 320, _animFramesBuffer, 160, step * 8, 144);
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
void IgorEngine::part34ApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[kLizardGivenStateIndex] == 0 && _objectsState[kLauraMetAtCollegeStateIndex] == 1) {
			part34And35DrawOldLadyFrame(_screenLayer1, 1);
		} else {
			_roomObjectAreasTable[5].object = 0;
		}
	}
	if (num == 3 || num == 255) {
		_roomObjectAreasTable[4].object = (_objectsState[kTreeResinDiscoveredStateIndex] == 0) ? 2 : 3;
	}
}

void IgorEngine::part34OldLadyIdleAnimationFrame(int step) {
	drawAnimRect(73 * 320 + 112, kParkOldLadyIdleFrames + kOldLadyIdleFrames[step - 1] * 630, 18, 35, true, kBlendBehindIgorAndText);
}

void IgorEngine::part34UpdateBackground() {
	part34And35UpdateAmbientSound();
	if (compareGameTick(61) && _objectsState[kLizardGivenStateIndex] == 0 && _objectsState[kLauraMetAtCollegeStateIndex] == 1 && getRandomNumber(4) == 0) {
		part34OldLadyIdleAnimationFrame(_parkLadyIdleStep);
		_parkLadyIdleStep = (_parkLadyIdleStep == 6) ? 1 : _parkLadyIdleStep + 1;
	}
}

void IgorEngine::part34() {
	if (isCDVersion())
		playMusic(1);
	_gameState.enableLight = 1;
	loadActionData(DAT_ParkLeft);
	loadRoomData(PAL_ParkRight, IMG_ParkRight, BOX_ParkRight, MSK_ParkRight, TXT_ParkRight);
	copyArea(_animFramesBuffer, 0, 160, _screenLayer1 + 160, 320, 160, 144);
	loadRoomData(PAL_Park, IMG_Park, BOX_Park, MSK_Park, TXT_Park); // active left panel;
	static const int frames[] = { FRM_Park1, FRM_Park2, FRM_Park3, FRM_Park4, 0 };
	loadAnimData(frames, kParkFrames);
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	partMainSetExecuteAction(1, &IgorEngine::part34ExecuteAction);
	_updateRoomBackground = &IgorEngine::part34UpdateBackground;
	_parkLadyIdleStep = 1;
	_roomDataOffsets = PART_34_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	part34ApplyObjectState(255);

	if (!restoreRoomAfterLoad()) {
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
	}
	enterPartLoop();
	if (takeResumeDialogue(kOldLadyDialogueId)) { // loaded while talking to the old lady
		part34OldLadyConversation();
		if (!_gameStateLoaded && !_eventQuitGame)
			part34ApplyObjectState(255);
		finishResumedDialogue();
	} else if (takeResumeDialogue(kLauraDialogueId)) { // loaded while talking to laura
		part34LauraConversation();
		if (!_gameStateLoaded && !_eventQuitGame)
			part34ReloadParkAfterLaura();
		finishResumedDialogue();
	}
	while (_currentPart == 340 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (_currentPart == kInvalidPart && !_gameStateLoaded) {
		fadeOut(768);
	}
}

} // End of namespace Igor
