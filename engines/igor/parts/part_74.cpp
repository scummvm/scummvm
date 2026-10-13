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

void IgorEngine::PART_74_CYCLE_COLORS() {
	if (compareGameTick(3, 32)) {
		scrollPalette(192, 207);
		setPaletteRange(192, 207);
	}
}

void IgorEngine::PART_74_drawCharacterFrame(int frame) {
	decodeAnimFrame(getAnimFrame(kPart74FramesBase, kPart74FramesTable, frame), _screenVGA, true);
}

void IgorEngine::PART_74_UPDATE_DIALOGUE_Sailor(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		PART_74_drawCharacterFrame(kPart74SailorStanding);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_74_drawCharacterFrame(kPart74SailorTalking + getRandomNumber(3));
		break;
	}
}

void IgorEngine::PART_74_UPDATE_DIALOGUE_CaptainA(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		PART_74_drawCharacterFrame(kPart74CaptainStandingA);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_74_drawCharacterFrame(kPart74CaptainStandingA + getRandomNumber(4));
		break;
	}
}

void IgorEngine::PART_74_UPDATE_DIALOGUE_CaptainB(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		PART_74_drawCharacterFrame(kPart74CaptainStandingB);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_74_drawCharacterFrame(kPart74CaptainStandingB + getRandomNumber(4));
		break;
	}
}

/**
 * One line of the scene, said by the first character (sailor) or by the second (captain).
 */
void IgorEngine::PART_74_SAY(int who, const Common::Array<DialogueText> &lines) {
	switch (who) {
	case 0:
		cutsceneSayWithCallback(kPart74SailorX, kPart74SailorY, 0, 50, 63, lines, &IgorEngine::PART_74_UPDATE_DIALOGUE_Sailor);
		break;
	case 1:
		cutsceneSayWithCallback(kPart74CaptainX, kPart74CaptainY, 63, 32, 63, lines, &IgorEngine::PART_74_UPDATE_DIALOGUE_CaptainB);
		break;
	default:
		cutsceneSayWithCallback(kPart74CaptainX, kPart74CaptainY, 63, 32, 63, lines, &IgorEngine::PART_74_UPDATE_DIALOGUE_CaptainA);
		break;
	}
}

/**
 * Boat scene
 */
void IgorEngine::PART_74_shipCutscene() {
	UpdateRoomBackgroundProc previousBackground = _updateRoomBackground;
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
	_updateRoomBackground = &IgorEngine::PART_74_CYCLE_COLORS;

	// water sound
	playSound(43, 1);
	do {
		PART_74_CYCLE_COLORS();
		waitForTimer();
	} while (_mixer->isSoundHandleActive(_sfxHandle) || isDialogueSpeechPlaying());

	PART_74_SAY(0, { { 201, 1, 935 }, { 202, 1, 936 } });
	PART_74_drawCharacterFrame(8);
	PART_74_SAY(1, { { 203, 1, 937 } });
	PART_74_drawCharacterFrame(3);
	PART_74_SAY(2, { { 204, 1, 938 } });
	PART_74_drawCharacterFrame(1);

	for (int ticks = 0; ticks < kPart74ColorsTicks; ticks += kTimerTicksCount) {
		PART_74_CYCLE_COLORS();
		waitForTimer();
	}

	PART_74_drawCharacterFrame(2);
	PART_74_SAY(0, { { 205, 1, 939 } });
	PART_74_drawCharacterFrame(8);
	PART_74_SAY(1, { { 206, 1, 940 } });
	PART_74_drawCharacterFrame(3);
	PART_74_SAY(2, { { 207, 1, 941 } });
	PART_74_SAY(0, { { 208, 1, 942 } });
	PART_74_drawCharacterFrame(8);
	PART_74_SAY(1, { { 209, 1, 943 } });
	PART_74_drawCharacterFrame(3);
	PART_74_SAY(2, { { 210, 1, 944 } });

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
}

} // End of namespace Igor
