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

// The frames of the two characters follow the animation of the room, from this offset on; the table of
// frame offsets is after the frames.
static const int kPart74FramesBase = 0xC318;
static const int kPart74FramesTable = 0x15BE;

// Frames of the first character (left): the last is the standing one, four more are used while speaking.
static const int kPart74FirstStanding = 13;
static const int kPart74FirstTalking = 13;
// Frames of the second character (right): 3..7 speaking with the first expression and 8..12 with the second.
static const int kPart74SecondStandingA = 3;
static const int kPart74SecondStandingB = 8;

// Where the lines are shown: x, y and color of the first (blue) and of the second (violet) character.
static const int kPart74FirstX = 141;
static const int kPart74FirstY = 55;
static const int kPart74SecondX = 247;
static const int kPart74SecondY = 38;

// Length in timer ticks of the part of the scene that only turns the colors.
static const int kPart74ColorsTicks = 750;

/**
 * The colors 192 to 207 rotate while the scene is on.
 */
void IgorEngine::PART_74_CYCLE_COLORS() {
	if (compareGameTick(3, 32)) {
		scrollPalette(192, 207);
		setPaletteRange(192, 207);
	}
}

void IgorEngine::PART_74_DRAW_FRAME(int frame) {
	decodeAnimFrame(getAnimFrame(kPart74FramesBase, kPart74FramesTable, frame), _screenVGA, true);
}

void IgorEngine::PART_74_UPDATE_DIALOGUE_FIRST(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		PART_74_DRAW_FRAME(kPart74FirstStanding);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_74_DRAW_FRAME(kPart74FirstTalking + getRandomNumber(3));
		break;
	}
}

void IgorEngine::PART_74_UPDATE_DIALOGUE_SECOND_A(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		PART_74_DRAW_FRAME(kPart74SecondStandingA);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_74_DRAW_FRAME(kPart74SecondStandingA + getRandomNumber(4));
		break;
	}
}

void IgorEngine::PART_74_UPDATE_DIALOGUE_SECOND_B(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
	case kUpdateDialogueAnimStanding:
		PART_74_DRAW_FRAME(kPart74SecondStandingB);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_74_DRAW_FRAME(kPart74SecondStandingB + getRandomNumber(4));
		break;
	}
}

/**
 * One line of the scene, said by the first character (blue) or by the second (violet).
 */
void IgorEngine::PART_74_SAY(int who, const Common::Array<DialogueText> &lines) {
	switch (who) {
	case 0:
		cutsceneSayWithCallback(kPart74FirstX, kPart74FirstY, 0, 50, 63, lines, &IgorEngine::PART_74_UPDATE_DIALOGUE_FIRST);
		break;
	case 1:
		cutsceneSayWithCallback(kPart74SecondX, kPart74SecondY, 63, 32, 63, lines, &IgorEngine::PART_74_UPDATE_DIALOGUE_SECOND_B);
		break;
	default:
		cutsceneSayWithCallback(kPart74SecondX, kPart74SecondY, 63, 32, 63, lines, &IgorEngine::PART_74_UPDATE_DIALOGUE_SECOND_A);
		break;
	}
}

/**
 * Scene with two characters over a picture whose colors rotate. It is part of the story of state 810.
 */
void IgorEngine::PART_74_CUTSCENE() {
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

	// the sound plays while the colors turn
	playSound(43, 1);
	do {
		PART_74_CYCLE_COLORS();
		waitForTimer();
	} while (_mixer->isSoundHandleActive(_sfxHandle) || isDialogueSpeechPlaying());

	PART_74_SAY(0, { { 201, 1, 935 }, { 202, 1, 936 } });
	PART_74_DRAW_FRAME(8);
	PART_74_SAY(1, { { 203, 1, 937 } });
	PART_74_DRAW_FRAME(3);
	PART_74_SAY(2, { { 204, 1, 938 } });
	PART_74_DRAW_FRAME(1);

	for (int ticks = 0; ticks < kPart74ColorsTicks; ticks += kTimerTicksCount) {
		PART_74_CYCLE_COLORS();
		waitForTimer();
	}

	PART_74_DRAW_FRAME(2);
	PART_74_SAY(0, { { 205, 1, 939 } });
	PART_74_DRAW_FRAME(8);
	PART_74_SAY(1, { { 206, 1, 940 } });
	PART_74_DRAW_FRAME(3);
	PART_74_SAY(2, { { 207, 1, 941 } });
	PART_74_SAY(0, { { 208, 1, 942 } });
	PART_74_DRAW_FRAME(8);
	PART_74_SAY(1, { { 209, 1, 943 } });
	PART_74_DRAW_FRAME(3);
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
