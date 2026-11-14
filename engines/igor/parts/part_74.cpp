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

static const int kPart74FramesBase = 0xC318;
static const int kPart74FramesTable = 0x15BE;

// Frames of the sailor: the last is the standing one, four more are used while speaking.
static const int kPart74SailorStanding = 13;
static const int kPart74SailorTalking = 13;
// Frames of the captain: 3..7 speaking with the first expression and 8..12 with the second.
static const int kPart74CaptainStandingA = 3;
static const int kPart74CaptainStandingB = 8;

// Where the lines are shown: x, y and color of the first (blue) and of the second (violet) character.
static const int kPart74SailorX = 141;
static const int kPart74SailorY = 55;
static const int kPart74CaptainX = 247;
static const int kPart74CaptainY = 38;

// Length in timer ticks of the part of the scene that only turns the colors.
static const int kPart74ColorsTicks = 750;

void IgorEngine::part74CycleColors() {
	if (compareGameTick(3, 32)) {
		scrollPalette(192, 207);
		setPaletteRange(192, 207);
	}
}

void IgorEngine::part74DrawCharacterFrame(int frame) {
	decodeAnimFrame(getAnimFrame(kPart74FramesBase, kPart74FramesTable, frame), _screenVGA, true);
}

void IgorEngine::part74UpdateSailorDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		part74DrawCharacterFrame(kPart74SailorStanding);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part74DrawCharacterFrame(kPart74SailorTalking + getRandomNumber(3));
		break;
	}
}

void IgorEngine::part74UpdateCaptainADialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		part74DrawCharacterFrame(kPart74CaptainStandingA);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part74DrawCharacterFrame(kPart74CaptainStandingA + getRandomNumber(4));
		break;
	}
}

void IgorEngine::part74UpdateCaptainBDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		part74DrawCharacterFrame(kPart74CaptainStandingB);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part74DrawCharacterFrame(kPart74CaptainStandingB + getRandomNumber(4));
		break;
	}
}

/**
 * One line of the scene, said by the first character (sailor) or by the second (captain).
 */
void IgorEngine::part74Say(int who, const Common::Array<DialogueText> &lines) {
	switch (who) {
	case 0:
		cutsceneSayWithCallback(kPart74SailorX, kPart74SailorY, 0, 50, 63, lines, &IgorEngine::part74UpdateSailorDialogue);
		break;
	case 1:
		cutsceneSayWithCallback(kPart74CaptainX, kPart74CaptainY, 63, 32, 63, lines, &IgorEngine::part74UpdateCaptainBDialogue);
		break;
	default:
		cutsceneSayWithCallback(kPart74CaptainX, kPart74CaptainY, 63, 32, 63, lines, &IgorEngine::part74UpdateCaptainADialogue);
		break;
	}
}

/**
 * Boat scene
 */
void IgorEngine::part74ShipCutscene() {
	const bool savingWasAllowed = _canSaveGame;
	_canSaveGame = false;
	UpdateRoomBackgroundProc previousBackground = _updateRoomBackground;
	if (isCDVersion())
		playMusic(4);
	_gameState.enableLight = 1;
	fadeOut(624);
	uint8 savedPalette[624];
	memcpy(savedPalette, _paletteBuffer, 624);

	loadData(PAL_Part74, _paletteBuffer);
	loadData(IMG_Part74, _screenVGA);
	{
		uint8 *text = loadData(TXT_Part74);
		decodeRoomText(text);
		free(text);
	}
	static const int anim[] = { ANM_Part74, 0 };
	loadAnimData(anim, kPart74FramesBase);
	fadeIn(624);
	_gameState.igorMoving = false;
	_updateRoomBackground = &IgorEngine::part74CycleColors;

	// water sound
	playSound(43, 1);
	do {
		part74CycleColors();
		waitForTimer();
	} while (_mixer->isSoundHandleActive(_sfxHandle) || isDialogueSpeechPlaying());

	part74Say(0, { { 201, 1, 935 }, { 202, 1, 936 } });
	part74DrawCharacterFrame(8);
	part74Say(1, { { 203, 1, 937 } });
	part74DrawCharacterFrame(3);
	part74Say(2, { { 204, 1, 938 } });
	part74DrawCharacterFrame(1);

	for (int ticks = 0; ticks < kPart74ColorsTicks; ticks += kTimerTicksCount) {
		part74CycleColors();
		waitForTimer();
	}

	part74DrawCharacterFrame(2);
	part74Say(0, { { 205, 1, 939 } });
	part74DrawCharacterFrame(8);
	part74Say(1, { { 206, 1, 940 } });
	part74DrawCharacterFrame(3);
	part74Say(2, { { 207, 1, 941 } });
	part74Say(0, { { 208, 1, 942 } });
	part74DrawCharacterFrame(8);
	part74Say(1, { { 209, 1, 943 } });
	part74DrawCharacterFrame(3);
	part74Say(2, { { 210, 1, 944 } });

	// the colors of the room come back and the picture of the room is shown from its top
	memcpy(_paletteBuffer, _currentPalette, 624);
	fadeOut(624);
	memcpy(_paletteBuffer, savedPalette, 624);
	memset(_screenTextLayer, 0, 46080);
	memset(_screenVGA, 0, 46080);
	for (int row = 0; row <= 0x8F; ++row) {
		memcpy(_screenVGA + row * 320 + 0x6A, _animFramesBuffer + row * 0x82, 0x82);
	}
	fadeIn(624);
	_gameState.enableLight = 2;
	_updateRoomBackground = previousBackground;
	_canSaveGame = savingWasAllowed;
}

} // End of namespace Igor
