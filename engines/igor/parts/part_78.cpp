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

// The animation resource ends with four tables of offsets, one word per frame (frame 1 first): the frames drawn
// over the whole picture, the pieces of the picture kept in the second layer, the frames of a band of the picture
// that are kept in a resource of their own and, last, the tiles of a small patch of the picture.
static const int kPart78FigureTable = 0xF832;
static const int kPart78BackgroundTable = 0xF8B2;
static const int kPart78BandTable = 0xF8D0;
static const int kPart78PatchTiles = 0xF8EE;

// The patch is a 12x12 area of the picture cut from 13x13 tiles.
static const int kPart78PatchTileSize = 13 * 13;
static const int kPart78PatchOffset = 57 * 320 + 72;
static const int kPart78PatchSize = 12;

// The scene draws its text with this color index instead of the usual one.
static const int kPart78TalkColor = 242;

// The figure that is drawn first is also the first of the frames of the loop of the first part of the scene. The
// loop goes through the steps 1 to 44, one step every 32 ticks, and each step shows the frame of this table.
static const int kPart78FirstFigure = 0x16;
static const int kPart78StepFirstRepeat = 0x16;
static const int kPart78StepLast = 0x2C;
static const uint8 kPart78StepFrames[45] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
	24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45
};

// Who talks: the three figures drawn over the whole picture, the two frame sets of the band and the patch.
enum {
	kPart78FigureA,
	kPart78FigureB,
	kPart78FigureC,
	kPart78BandA,
	kPart78BandB,
	kPart78Patch
};

void IgorEngine::PART_78_DRAW_FIGURE(int frame) {
	decodeAnimFrame(getAnimFrame(0, kPart78FigureTable, frame), _screenVGA, false);
}

void IgorEngine::PART_78_DRAW_BACKGROUND_FRAME(int frame) {
	const int offset = READ_LE_UINT16(_animFramesBuffer + kPart78BackgroundTable + (frame - 1) * 2);
	decodeAnimFrame(_screenLayer2 + offset, _screenVGA, false);
}

void IgorEngine::PART_78_DRAW_BAND(int frame) {
	const int offset = READ_LE_UINT16(_animFramesBuffer + kPart78BandTable + (frame - 1) * 2);
	decodeAnimFrame(_part78BandFrames + offset - 1, _screenVGA, false);
}

void IgorEngine::PART_78_DRAW_PATCH(int frame) {
	copyArea(_screenVGA, kPart78PatchOffset, 320, _animFramesBuffer + kPart78PatchTiles + (frame - 1) * kPart78PatchTileSize,
			 13, kPart78PatchSize, kPart78PatchSize);
}

void IgorEngine::PART_78_UPDATE_DIALOGUE_FIGURE_A(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_78_DRAW_FIGURE(0x3B);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_78_DRAW_FIGURE(0x3B + getRandomNumber(4));
		break;
	}
}

void IgorEngine::PART_78_UPDATE_DIALOGUE_FIGURE_B(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_78_DRAW_FIGURE(0x16);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_78_DRAW_FIGURE(0x16 + getRandomNumber(1));
		break;
	}
}

void IgorEngine::PART_78_UPDATE_DIALOGUE_FIGURE_C(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_78_DRAW_FIGURE(0x2D);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_78_DRAW_FIGURE(0x2D + getRandomNumber(1));
		break;
	}
}

void IgorEngine::PART_78_UPDATE_DIALOGUE_BAND_A(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_78_DRAW_BAND(8);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_78_DRAW_BAND(8 + getRandomNumber(3));
		break;
	}
}

void IgorEngine::PART_78_UPDATE_DIALOGUE_BAND_B(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_78_DRAW_BAND(12);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_78_DRAW_BAND(12 + getRandomNumber(3));
		break;
	}
}

void IgorEngine::PART_78_UPDATE_DIALOGUE_PATCH(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_78_DRAW_PATCH(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_78_DRAW_PATCH(getRandomNumber(3) + 1);
		break;
	}
}

/**
 * Starts the lines of one of the speakers. The caller waits for them with PART_78_WAIT_FOR_LINES.
 */
void IgorEngine::PART_78_SAY_START(int speaker, const Common::Array<DialogueText> &lines) {
	switch (speaker) {
	case kPart78FigureA:
		cutsceneSayStartWithCallback(173, 65, 0, 43, 43, lines, &IgorEngine::PART_78_UPDATE_DIALOGUE_FIGURE_A);
		break;
	case kPart78FigureB:
		cutsceneSayStartWithCallback(260, 91, 0, 43, 43, lines, &IgorEngine::PART_78_UPDATE_DIALOGUE_FIGURE_B);
		break;
	case kPart78FigureC:
		cutsceneSayStartWithCallback(98, 90, 0, 43, 43, lines, &IgorEngine::PART_78_UPDATE_DIALOGUE_FIGURE_C);
		break;
	case kPart78BandA:
		cutsceneSayStartWithCallback(142, 58, 63, 63, 63, lines, &IgorEngine::PART_78_UPDATE_DIALOGUE_BAND_A);
		break;
	case kPart78BandB:
		cutsceneSayStartWithCallback(142, 58, 63, 63, 63, lines, &IgorEngine::PART_78_UPDATE_DIALOGUE_BAND_B);
		break;
	default:
		cutsceneSayStartWithCallback(78, 54, 63, 63, 0, lines, &IgorEngine::PART_78_UPDATE_DIALOGUE_PATCH);
		break;
	}
}

void IgorEngine::PART_78_WAIT_FOR_LINES(int speaker) {
	switch (speaker) {
	case kPart78FigureA:
		waitForEndOfCutsceneDialogue(173, 65, 0, 43, 43);
		break;
	case kPart78FigureB:
		waitForEndOfCutsceneDialogue(260, 91, 0, 43, 43);
		break;
	case kPart78FigureC:
		waitForEndOfCutsceneDialogue(98, 90, 0, 43, 43);
		break;
	case kPart78BandA:
	case kPart78BandB:
		waitForEndOfCutsceneDialogue(142, 58, 63, 63, 63);
		break;
	default:
		waitForEndOfCutsceneDialogue(78, 54, 63, 63, 0);
		break;
	}
	_updateDialogue = nullptr;
}

void IgorEngine::PART_78_SAY(int speaker, const Common::Array<DialogueText> &lines) {
	PART_78_SAY_START(speaker, lines);
	PART_78_WAIT_FOR_LINES(speaker);
}

/**
 * One of the three short lines said at random while the first part of the scene goes on.
 */
void IgorEngine::PART_78_INTERJECTION(int which) {
	static const DialogueText kLines[3] = { { 211, 1, 676 }, { 212, 1, 677 }, { 213, 1, 678 } };
	Common::Array<DialogueText> lines;
	lines.push_back(kLines[which - 1]);
	PART_78_SAY(_roomAmbientIndex == 0x16 ? kPart78FigureB : kPart78FigureC, lines);
}

void IgorEngine::PART_78() {
	playMusic(11);
	// the card with the time that has passed
	PART_MEANWHILE(IMG_Part78MinutesLater);
	memset(_currentPalette, 0, 768);
	setPaletteRange(0, 255);
	memset(_screenVGA + 46080, 0, 17920);
	_gameState.enableLight = 1;
	loadRoomData(PAL_Part78, IMG_Part78, BOX_Part78, 0, TXT_Part78);
	static const int anim[] = { ANM_Part78, 0 };
	loadAnimData(anim);
	// the second layer holds the pieces of the picture, starting at its second byte
	loadData(IMG_Part78Layer2, _screenLayer2 + 1);
	_part78BandFrames = loadData(ANM_Part78Frames);
	memcpy(_screenVGA, _screenLayer1, 46080);
	PART_78_DRAW_FIGURE(kPart78FirstFigure);
	PART_78_DRAW_BAND(1);
	memset(_currentPalette, 0, 768);
	setPaletteRange(208, 255);
	fadeIn(768);
	_gameState.igorMoving = false;
	_talkColorIndex = kPart78TalkColor;

	PART_78_SAY(kPart78FigureB, { { 204, 1, 671 }, { 205, 2, 672 }, { 207, 1, 673 }, { 208, 2, 674 }, { 210, 1, 675 } });

	// the figures go through their frames while one of them talks now and then, until five turns are done
	_roomAmbientIndex = kPart78StepFirstRepeat;
	int turns = 1;
	while (turns <= 5) {
		if (compareGameTick(1, 32)) {
			if (_roomAmbientIndex == kPart78StepLast) {
				_roomAmbientIndex = 1;
			} else {
				++_roomAmbientIndex;
			}
			if (_roomAmbientIndex == kPart78StepFirstRepeat) {
				++turns;
			}
			PART_78_DRAW_FIGURE(kPart78StepFrames[_roomAmbientIndex]);
			if ((_roomAmbientIndex == kPart78StepFirstRepeat || _roomAmbientIndex == kPart78StepLast) && turns <= 5) {
				if (getRandomNumber(2) == 0) {
					PART_78_INTERJECTION(getRandomNumber(2) + 1);
				}
			}
		}
		if (compareGameTick(0x3D)) {
			PART_78_DRAW_BACKGROUND_FRAME(getRandomNumber(13) + 2);
			PART_78_DRAW_BAND(getRandomNumber(4) + 1);
		}
		waitForTimer();
	}

	for (int i = 6; i <= 7; ++i) {
		PART_78_DRAW_BAND(i);
		waitForTimer(90);
	}
	PART_78_DRAW_BAND(3);
	PART_78_SAY_START(kPart78BandA, { { 201, 1, 668 } });
	PART_78_DRAW_FIGURE(0x2F);
	PART_78_WAIT_FOR_LINES(kPart78BandA);
	PART_78_DRAW_BACKGROUND_FRAME(1);
	PART_78_DRAW_BAND(2);

	PART_78_SAY(kPart78FigureB, { { 214, 1, 679 } });
	for (int i = 0x30; i <= 0x3A; ++i) {
		PART_78_DRAW_FIGURE(i);
		waitForTimer(40);
	}
	PART_78_SAY(kPart78FigureA, { { 215, 1, 680 } });
	PART_78_SAY(kPart78BandB, { { 216, 1, 681 } });
	PART_78_SAY(kPart78FigureA, { { 217, 1, 682 }, { 218, 1, 683 } });
	PART_78_SAY(kPart78BandB, { { 219, 1, 684 } });
	PART_78_SAY(kPart78FigureA, { { 235, 1, 698 }, { 236, 2, 699 } });
	PART_78_SAY(kPart78BandB, { { 222, 1, 685 }, { 223, 1, 686 } });
	PART_78_DRAW_BAND(3);
	PART_78_SAY(kPart78Patch, { { 224, 1, 687 }, { 225, 1, 688 }, { 226, 1, 689 } });
	PART_78_DRAW_BAND(2);
	PART_78_SAY(kPart78FigureA, { { 227, 1, 690 }, { 228, 1, 691 } });
	PART_78_DRAW_BAND(3);
	PART_78_SAY(kPart78Patch, { { 229, 1, 692 }, { 230, 1, 693 } });
	PART_78_DRAW_BAND(2);
	PART_78_SAY(kPart78FigureA, { { 231, 1, 694 }, { 232, 1, 695 }, { 233, 1, 696 }, { 234, 1, 697 } });

	PART_78_DRAW_FIGURE(0x40);
	waitForTimer(255);
	waitForTimer(255);
	waitForTimer(255);

	_talkColorIndex = kTalkColor;
	free(_part78BandFrames);
	_part78BandFrames = nullptr;
	memcpy(_paletteBuffer, _currentPalette, 624);
	fadeOut(768);
	_objectsState[111] = 1;
	playMusic(2);
	// the card with the time that has passed
	PART_MEANWHILE(IMG_Part78NextDay);
	_currentPart = 281;
}

} // End of namespace Igor
