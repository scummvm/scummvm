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
#include "common/system.h"
#include "igor/igor.h"

namespace Igor {

// The picture of both states is shown below a black band of this many rows.
static const int kPart79BandRows = 24;

// ---------------------------------------------------------------------------------------------------------------
// State 790
// ---------------------------------------------------------------------------------------------------------------

// The animation resource ends with a table of offsets (one word per frame, frame 1 first). The frames drawn over the
// whole picture are 1 to 88.
static const int kPart79FigureTable = 0xF858;

// Small pieces of 17x9 that are drawn at (86,135), five of them, 153 bytes each, after the table.
static const int kPart79PatchOffset = 135 * 320 + 86;
static const int kPart79PatchWidth = 17;
static const int kPart79PatchHeight = 9;
static const int kPart79PatchFrames = 0xF908;
static const int kPart79PatchFrameSize = kPart79PatchWidth * kPart79PatchHeight;
static const uint8 kPart79PatchSteps[8] = { 0, 1, 2, 3, 4, 3, 2, 1 };
static const int kPart79PatchClearColor = 0x68;

// Where and in which color the text of the scene is shown.
static const int kPart79TextX = 94;
static const int kPart79TextY = 108;
static const int kPart79TextR = 63;
static const int kPart79TextG = 0;
static const int kPart79TextB = 38;

// Frame the talking figure goes back to at the end of a line, and the number of frames it picks from while it talks.
static const int kPart79TalkFrame = 2;

// Frames shown before the picture slides to the next one, the first one of the standing loop and the last.
static const int kPart79StandingFirst = 7;
static const int kPart79StandingEnd = 0x31;
static const int kPart79FinalFirst = 0x39;
static const int kPart79FinalLast = 0x4C;

// Slide to the second picture: it goes 8 pixels at a time, 20 steps, one step every 16 ticks. The first 12 steps draw
// the frames 77 to 88 over the picture, the last 8 frames 1 to 8 of the group kept in a resource of their own.
static const int kPart79SlideSteps = 20;
static const int kPart79SlideFirstFrame = 77;
static const int kPart79SlideLastFrame = 88;
static const int kPart79SlideAnimSteps = 12;
static const int kPart79SlideLayer2Offset = 0xA0;
static const int kPart79BankTable = 0x6DA6;

void IgorEngine::PART_79_DRAW_FIGURE(int frame) {
	decodeAnimFrame(getAnimFrame(0, kPart79FigureTable, frame), _screenVGA, false);
}

void IgorEngine::PART_79_DRAW_PATCH(int step) {
	copyArea(_screenVGA, kPart79PatchOffset, 320, _animFramesBuffer + kPart79PatchFrames + step * kPart79PatchFrameSize,
			 kPart79PatchWidth, kPart79PatchWidth, kPart79PatchHeight);
}

void IgorEngine::PART_79_CLEAR_PATCH() {
	for (int y = 0; y < kPart79PatchHeight; ++y) {
		memset(_screenVGA + kPart79PatchOffset + y * 320, kPart79PatchClearColor, kPart79PatchWidth);
	}
}

/**
 * Every 64 ticks the small piece goes on to the next of its steps.
 */
void IgorEngine::PART_79_UPDATE_AMBIENT_790() {
	if (compareGameTick(0x3D)) {
		PART_79_DRAW_PATCH(kPart79PatchSteps[_roomAmbientIndex]);
		_roomAmbientIndex = (_roomAmbientIndex == 7) ? 0 : _roomAmbientIndex + 1;
	}
}

void IgorEngine::PART_79_UPDATE_DIALOGUE_790(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_79_DRAW_FIGURE(kPart79TalkFrame);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_79_DRAW_FIGURE(kPart79TalkFrame + getRandomNumber(4));
		break;
	}
}

void IgorEngine::PART_79_SAY_790(const Common::Array<DialogueText> &lines) {
	cutsceneSayStartWithCallback(kPart79TextX, kPart79TextY, kPart79TextR, kPart79TextG, kPart79TextB, lines,
								 &IgorEngine::PART_79_UPDATE_DIALOGUE_790);
	_updateRoomBackground = &IgorEngine::PART_79_UPDATE_AMBIENT_790;
	waitForEndOfCutsceneDialogue(kPart79TextX, kPart79TextY, kPart79TextR, kPart79TextG, kPart79TextB);
	_updateDialogue = nullptr;
	_updateRoomBackground = nullptr;
}

/**
 * The picture slides to the left 8 pixels per step, the second picture comes in from the right and frames are
 * drawn over the result.
 */
void IgorEngine::PART_79_SLIDE() {
	uint8 *bank = loadData(ANM_Part79Frames);
	uint8 *buffer = _screenTextLayer;
	int frame = kPart79SlideFirstFrame;
	int step = 1;
	_gameTicks = 0;
	do {
		if (compareGameTick(1, 16)) {
			const int shift = step * 8;
			copyArea(buffer, 0, 320, _screenLayer1 + shift, 320, 320 - shift, 144);
			copyArea(buffer, 320 - shift, 320, _screenLayer2 + kPart79SlideLayer2Offset, 320, shift, 144);
			if (step <= kPart79SlideAnimSteps) {
				decodeAnimFrame(getAnimFrame(0, kPart79FigureTable, frame), buffer, false);
				frame = (frame == kPart79SlideLastFrame) ? 1 : frame + 1;
			} else {
				const int offset = READ_LE_UINT16(bank + kPart79BankTable + (frame - 1) * 2);
				decodeAnimFrame(bank + offset - 1, buffer, false);
				++frame;
			}
			memcpy(_screenVGA, buffer, 46080);
			++step;
		}
		waitForTimer();
	} while (step != kPart79SlideSteps + 1);
	free(bank);
}

void IgorEngine::PART_79_790() {
	playMusic(13);
	memset(_currentPalette, 0, 768);
	setPaletteRange(0, 255);
	loadRoomData(PAL_Part79, IMG_Part79, 0, 0, 0);
	uint8 *text = loadData(TXT_Part79);
	decodeRoomStrings(text, true);
	free(text);
	loadData(IMG_Part79Layer2, _screenLayer2);
	loadData(ANM_Part79, _animFramesBuffer);
	memcpy(_screenVGA, _screenLayer1, 46080);
	PART_79_DRAW_FIGURE(1);
	fadeIn(768);
	_roomAmbientIndex = 0;

	PART_79_SAY_790({ { 201, 2, 742 }, { 203, 1, 743 }, { 204, 2, 744 }, { 206, 1, 745 }, { 207, 1, 746 } });
	PART_79_SAY_790({ { 208, 1, 747 }, { 209, 2, 748 } });

	// the figure goes through its frames while the small piece keeps changing
	int frame = kPart79StandingFirst;
	do {
		if (compareGameTick(1, 16)) {
			PART_79_DRAW_FIGURE(frame);
			++frame;
		}
		if (compareGameTick(0x3D)) {
			PART_79_DRAW_PATCH(kPart79PatchSteps[_roomAmbientIndex]);
			_roomAmbientIndex = (_roomAmbientIndex == 7) ? 0 : _roomAmbientIndex + 1;
			PART_79_DRAW_PATCH(getRandomNumber(4));
		}
		waitForTimer();
	} while (frame != kPart79StandingEnd);

	PART_79_SAY_790({ { 211, 1, 749 }, { 212, 1, 750 }, { 213, 1, 751 }, { 214, 2, 752 } });
	PART_79_CLEAR_PATCH();
	for (frame = kPart79FinalFirst; frame <= kPart79FinalLast; ++frame) {
		PART_79_DRAW_FIGURE(frame);
		waitForTimer(31);
	}
	PART_79_SLIDE();
	_currentPart = 791;
}

// ---------------------------------------------------------------------------------------------------------------
// State 791
// ---------------------------------------------------------------------------------------------------------------

// Groups of frames and where their table of offsets (one word per frame, frame 1 first) is. The offset of a frame is
// relative to the base of its group. The groups are in the animation resource or in the second layer.
struct Part79FrameGroup {
	int table;
	int base;
};

// frames 1.. of the animation resource that is loaded first (the table is the last 200 bytes of it)
static const Part79FrameGroup kPart79Scenes = { 0xF6BA, 0 };
// frames kept in the second layer
static const Part79FrameGroup kPart79Layer2Frames = { 0x594C, 0 };
static const Part79FrameGroup kPart79Layer2Frames2 = { 0x7FF0, 0x5989 };
// frames of the second animation resource
static const Part79FrameGroup kPart79Anim2A = { 0xC093, 0x88C8 };
static const Part79FrameGroup kPart79Anim2B = { 0x3CC9, 0 };
static const Part79FrameGroup kPart79Anim2C = { 0x7ACD, 0x3CEF };
static const Part79FrameGroup kPart79Anim2D = { 0x88B8, 0x7AF1 };

// Frame shown when a line of a speaker ends / the first frame of the group the speaker picks from while talking.
static const int kPart79FirstSpeakerEndFrame = 1;
static const int kPart79FirstSpeakerBase = 7;
static const int kPart79ThirdSpeakerEndFrame = 22;
static const int kPart79ThirdSpeakerBase = 23;

// Frames that alternate in the background while Igor talks.
static const int kPart79AmbientFirst = 0x57;
static const int kPart79AmbientSecond = 0x58;

// The three groups of 90x23, 13x11 and 44x23 pieces that are drawn at the end.
static const int kPart79Block1Offset = 53 * 320 + 115;
static const int kPart79Block1Width = 90;
static const int kPart79Block1Height = 23;
static const int kPart79Block1Frames = 0xC0A3;
static const int kPart79Block2Offset = 108 * 320 + 201;
static const int kPart79Block2Width = 13;
static const int kPart79Block2Height = 11;
static const int kPart79Block2Color = 0x68;
static const int kPart79Block3Offset = 53 * 320 + 138;
static const int kPart79Block3Width = 44;
static const int kPart79Block3Height = 23;
static const int kPart79Block3Frames = 0xC8B9;

// Colors faded in and out by pairs, in the colors the Igor texts go through the sky in.
static const int kPart79FadeFirstColor = 0xF5;
static const int kPart79FadeLastColor = 0xFE;
static const uint8 kPart79FadeColors[6] = { 0x3F, 0x20, 0x00, 0x2C, 0x0C, 0x00 };

void IgorEngine::PART_79_DRAW_GROUP_FRAME(const uint8 *buffer, int table, int base, int frame) {
	decodeAnimFrame(buffer + base + READ_LE_UINT16(buffer + table + frame * 2) - 1, _screenVGA, false);
}

void IgorEngine::PART_79_DRAW_SCENE(int frame) {
	PART_79_DRAW_GROUP_FRAME(_animFramesBuffer, kPart79Scenes.table, kPart79Scenes.base, frame);
}

void IgorEngine::PART_79_DRAW_LAYER2_FRAME(int frame) {
	PART_79_DRAW_GROUP_FRAME(_screenLayer2, kPart79Layer2Frames.table, kPart79Layer2Frames.base, frame);
}

// The frames of this group are addressed by the offset from the base without the usual correction of one byte.
void IgorEngine::PART_79_DRAW_LAYER2_FRAME_2(int frame) {
	const int offset = READ_LE_UINT16(_screenLayer2 + kPart79Layer2Frames2.table + frame * 2);
	decodeAnimFrame(_screenLayer2 + kPart79Layer2Frames2.base + offset, _screenVGA, false);
}

void IgorEngine::PART_79_DRAW_ANIM2_FRAME(const void *group, int frame) {
	const Part79FrameGroup *g = (const Part79FrameGroup *)group;
	PART_79_DRAW_GROUP_FRAME(_animFramesBuffer, g->table, g->base, frame);
}

/**
 * Talk animation of Igor in these states: 14x11 piece of his head over the picture (the colors of the head are
 * moved into the range of the picture) or the picture itself where the head is transparent.
 */
void IgorEngine::PART_79_ANIMATE_IGOR_HEAD(int frame) {
	WalkData *wd = &_walkData[_walkDataLastIndex - 1];
	int dst = (wd->y - wd->scaleWidth + 1) * 320;
	const int delta = wd->x - _walkWidthScaleTable[wd->scaleHeight - 1] / 2;
	if (delta > 0) {
		dst += delta;
	}
	for (int row = 0; row <= 10; ++row, dst += 320) {
		for (int col = 8; col <= 21; ++col) {
			const int offset = (wd->posNum - 1) * 924 + frame * 154 + row * 14 + (col - 8);
			uint8 color = _igorHeadFrames[offset];
			if (color > 0) {
				_screenVGA[dst + col] = color + 0x20;
			} else {
				_screenVGA[dst + col] = _screenLayer1[dst + col];
			}
		}
	}
}

void IgorEngine::PART_79_UPDATE_DIALOGUE_SPEAKER(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_79_DRAW_SCENE(_part79EndFrame);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_79_DRAW_SCENE(_part79BaseFrame + getRandomNumber(3));
		break;
	}
}

void IgorEngine::PART_79_UPDATE_DIALOGUE_FIRST_SPEAKER(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_79_DRAW_SCENE(kPart79FirstSpeakerEndFrame);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_79_DRAW_SCENE(kPart79FirstSpeakerBase + getRandomNumber(3));
		break;
	}
}

void IgorEngine::PART_79_UPDATE_DIALOGUE_THIRD_SPEAKER(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_79_DRAW_LAYER2_FRAME(kPart79ThirdSpeakerEndFrame);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_79_DRAW_LAYER2_FRAME(kPart79ThirdSpeakerBase + getRandomNumber(3));
		break;
	}
}

void IgorEngine::PART_79_UPDATE_AMBIENT_791() {
	if (compareGameTick(0x3D)) {
		PART_79_DRAW_SCENE(_roomAmbientIndex);
		_roomAmbientIndex = (_roomAmbientIndex == kPart79AmbientFirst) ? kPart79AmbientSecond : kPart79AmbientFirst;
	}
}

/**
 * Lines of a person of the scene: the frames of the speaker go with the text, which is shown at (x, y).
 */
void IgorEngine::PART_79_SAY_SPEAKER(int startX, int startY, int x, int y, int r, int g, int b, int endFrame, int baseFrame,
									 const Common::Array<DialogueText> &lines) {
	_part79EndFrame = endFrame;
	_part79BaseFrame = baseFrame;
	cutsceneSayStartWithCallback(startX, startY, r, g, b, lines, &IgorEngine::PART_79_UPDATE_DIALOGUE_SPEAKER);
	waitForEndOfCutsceneDialogue(x, y, r, g, b);
	_updateDialogue = nullptr;
}

void IgorEngine::PART_79_SAY_FIRST_SPEAKER(int startX, int startY, const Common::Array<DialogueText> &lines) {
	cutsceneSayStartWithCallback(startX, startY, 63, 0, 38, lines, &IgorEngine::PART_79_UPDATE_DIALOGUE_FIRST_SPEAKER);
	waitForEndOfCutsceneDialogue(145, 86, 63, 0, 38);
	_updateDialogue = nullptr;
}

void IgorEngine::PART_79_SAY_THIRD_SPEAKER(const Common::Array<DialogueText> &lines) {
	cutsceneSayStartWithCallback(220, 91, 63, 0, 38, lines, &IgorEngine::PART_79_UPDATE_DIALOGUE_THIRD_SPEAKER);
	waitForEndOfCutsceneDialogue(220, 91, 63, 0, 38);
	_updateDialogue = nullptr;
}

/**
 * Igor talks from the position given. The head is drawn with his talk animation.
 */
void IgorEngine::PART_79_IGOR_SAY(const Common::Array<DialogueText> &lines) {
	igorSay(lines);
	waitForEndOfIgorDialogue();
}

void IgorEngine::PART_79_SET_IGOR_POS(int x, int y) {
	_walkData[0].x = x;
	_walkData[0].y = y;
}

/**
 * Colors first to last go from black to the colors of the palette buffer (in) or back to black (out) one level at a
 * time, the palette is set after each level.
 */
void IgorEngine::PART_79_FADE_COLORS(int first, int last, bool fadeInColors) {
	_system->copyRectToScreen(_screenVGA, 320, 0, _screenVGAVOffset, 320, 200 - _screenVGAVOffset);
	for (int step = 1; step <= 63; ++step) {
		const int level = fadeInColors ? 64 - step : step;
		for (int i = first * 3; i < (last + 1) * 3; ++i) {
			if (_paletteBuffer[i] >= level) {
				if (fadeInColors) {
					++_currentPalette[i];
				} else {
					--_currentPalette[i];
				}
			}
		}
		setPaletteRange(first, last);
		_system->updateScreen();
		_system->delayMillis(1000 / 60);
	}
}

/**
 * Two colors of the palette buffer are set to the orange of the texts and faded in.
 */
void IgorEngine::PART_79_FADE_IN_PAIR(int firstColor) {
	memcpy(_paletteBuffer + firstColor * 3, kPart79FadeColors, 6);
	PART_79_FADE_COLORS(firstColor, firstColor + 1, true);
}

void IgorEngine::PART_79_791() {
	playMusic(13);
	_gameState.enableLight = 2;
	loadData(IMG_Part79b, _screenLayer1);
	uint8 *text = loadData(TXT_Part79b);
	decodeRoomStrings(text, true);
	free(text);
	loadData(ANM_Part79bFrames, _animFramesBuffer);
	loadData(ANM_Part79bLayer2, _screenLayer2);

	PART_79_SAY_SPEAKER(178, 82, 178, 82, 63, 63, 0, 2, 2, { { 201, 2, 753 }, { 203, 1, 754 }, { 204, 1, 755 } });
	PART_79_SAY_FIRST_SPEAKER(145, 86, { { 205, 2, 756 }, { 207, 2, 757 } });
	PART_79_SAY_SPEAKER(172, 82, 178, 82, 63, 63, 0, 2, 2, { { 209, 2, 758 } });
	for (int frame = 0x10; frame <= 0x1C; ++frame) {
		PART_79_DRAW_SCENE(frame);
		waitForTimer(31);
	}
	PART_79_SAY_FIRST_SPEAKER(145, 86, { { 211, 1, 759 } });
	for (int frame = 0xB; frame <= 0xF; ++frame) {
		PART_79_DRAW_SCENE(frame);
		waitForTimer(11);
	}
	PART_79_DRAW_SCENE(29);
	PART_79_SAY_SPEAKER(178, 82, 178, 82, 63, 63, 0, 0x1D, 0x1E, { { 212, 1, 760 }, { 213, 1, 761 } });

	PART_79_SET_IGOR_POS(229, 137);
	_walkData[0].posNum = 4;
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkData[0].scaleWidth = 50;
	_walkData[0].scaleHeight = 50;
	_walkDataLastIndex = 1;
	PART_79_IGOR_SAY({ { 214, 2, 762 }, { 216, 2, 763 } });
	PART_79_SAY_SPEAKER(178, 82, 178, 82, 63, 63, 0, 0x1D, 0x1E, { { 218, 2, 764 } });
	for (int frame = 0x21; frame <= 0x26; ++frame) {
		PART_79_DRAW_SCENE(frame);
		waitForTimer(31);
	}
	PART_79_DRAW_SCENE(46);

	PART_79_SET_IGOR_POS(236, 138);
	PART_79_IGOR_SAY({ { 220, 2, 765 } });
	PART_79_SAY_SPEAKER(195, 83, 195, 83, 63, 63, 0, 40, 41, { { 222, 2, 766 } });
	PART_79_DRAW_SCENE(39);
	waitForTimer(31);
	PART_79_DRAW_SCENE(46);
	PART_79_IGOR_SAY({ { 224, 2, 767 } });
	for (int frame = 0x2F; frame <= 0x32; ++frame) {
		PART_79_DRAW_SCENE(frame);
		waitForTimer(31);
	}
	PART_79_SAY_SPEAKER(206, 85, 206, 85, 63, 63, 0, 0x35, 0x36, { { 226, 2, 768 } });
	PART_79_DRAW_SCENE(51);
	waitForTimer(31);

	PART_79_SET_IGOR_POS(246, 138);
	PART_79_IGOR_SAY({ { 228, 2, 769 }, { 230, 2, 770 } });
	PART_79_SAY_SPEAKER(206, 85, 206, 85, 63, 63, 0, 0x35, 0x36, { { 232, 1, 771 } });
	playSound(28, 1);
	for (int frame = 0x3A; frame <= 0x56; ++frame) {
		PART_79_DRAW_SCENE(frame);
		if (frame == 0x4F) {
			playSound(29, 1);
		}
		waitForTimer(17);
	}
	PART_79_DRAW_SCENE(52);
	waitForTimer(33);
	// the sound goes on until it ends
	while (_mixer->isSoundHandleActive(_sfxHandle) || isDialogueSpeechPlaying()) {
		waitForTimer();
	}
	_roomAmbientIndex = kPart79AmbientFirst;
	igorSay(233, 2, 772);
	_updateRoomBackground = &IgorEngine::PART_79_UPDATE_AMBIENT_791;
	waitForEndOfIgorDialogue();
	_updateRoomBackground = nullptr;
	for (int frame = 0x58; frame <= 0x64; ++frame) {
		PART_79_DRAW_SCENE(frame);
		if (frame < 0x64) {
			waitForTimer(31);
		}
		if (frame == 0x59) {
			playSound(48, 1);
		}
	}

	for (int frame = 1; frame <= 3; ++frame) {
		PART_79_DRAW_LAYER2_FRAME(frame);
		waitForTimer(31);
	}
	PART_79_SET_IGOR_POS(178, 142);
	_walkData[0].posNum = 2;
	PART_79_IGOR_SAY({ { 235, 1, 773 }, { 236, 2, 774 } });
	for (int frame = 4; frame <= 0x12; ++frame) {
		PART_79_DRAW_LAYER2_FRAME(frame);
		waitForTimer(31);
	}
	PART_79_IGOR_SAY({ { 238, 1, 775 } });
	// the texts 201 to 215 are replaced by the ones of the main resource
	text = loadData(TXT_Part79bMain);
	decodeRoomStrings(text, true);
	free(text);
	for (int frame = 0x13; frame <= 0x15; ++frame) {
		PART_79_DRAW_LAYER2_FRAME(frame);
		waitForTimer(31);
	}

	PART_79_SET_IGOR_POS(196, 141);
	PART_79_IGOR_SAY({ { 201, 2, 776 }, { 203, 2, 777 } });
	PART_79_SAY_THIRD_SPEAKER({ { 205, 1, 778 }, { 206, 1, 779 }, { 207, 1, 780 } });
	PART_79_IGOR_SAY({ { 208, 1, 781 }, { 209, 1, 782 } });
	PART_79_SAY_THIRD_SPEAKER({ { 210, 1, 783 } });
	PART_79_IGOR_SAY({ { 211, 1, 784 } });
	PART_79_SAY_THIRD_SPEAKER({ { 212, 1, 785 } });
	PART_79_IGOR_SAY({ { 213, 1, 786 } });
	PART_79_SAY_THIRD_SPEAKER({ { 214, 1, 787 } });
	for (int frame = 0x1B; frame <= 0x1D; ++frame) {
		PART_79_DRAW_LAYER2_FRAME(frame);
		waitForTimer(31);
	}
	for (int frame = 1; frame <= 0xE; ++frame) {
		PART_79_DRAW_LAYER2_FRAME_2(frame);
		waitForTimer(31);
	}
	waitForTimer(255);
	PART_79_DRAW_LAYER2_FRAME(30);
	waitForTimer(255);
	for (int frame = 0xF; frame <= 0x11; ++frame) {
		PART_79_DRAW_LAYER2_FRAME_2(frame);
		waitForTimer(31);
	}

	// the second animation resource replaces the first one
	loadData(ANM_Part79bScenes, _animFramesBuffer);
	copyArea(_screenVGA, kPart79Block1Offset, 320, _animFramesBuffer + kPart79Block1Frames, kPart79Block1Width,
			 kPart79Block1Width, kPart79Block1Height);

	PART_79_FADE_IN_PAIR(0xF5);
	for (int i = 0; i < 5; ++i) {
		waitForTimer(255);
	}
	PART_79_FADE_IN_PAIR(0xF7);
	waitForTimer(255);
	PART_79_FADE_IN_PAIR(0xF9);
	waitForTimer(255);
	PART_79_FADE_IN_PAIR(0xFB);
	waitForTimer(255);
	PART_79_FADE_IN_PAIR(0xFD);
	for (int i = 0; i < 3; ++i) {
		waitForTimer(255);
	}
	PART_79_FADE_COLORS(kPart79FadeFirstColor, kPart79FadeLastColor, false);

	for (int frame = 1; frame <= 7; ++frame) {
		PART_79_DRAW_ANIM2_FRAME(&kPart79Anim2A, frame);
		waitForTimer(31);
	}
	for (int frame = 2; frame <= 0x12; ++frame) {
		PART_79_DRAW_ANIM2_FRAME(&kPart79Anim2B, frame);
		waitForTimer(31);
	}
	for (int frame = 1; frame <= 0x12; ++frame) {
		PART_79_DRAW_ANIM2_FRAME(&kPart79Anim2B, frame);
		waitForTimer(31);
	}
	PART_79_DRAW_ANIM2_FRAME(&kPart79Anim2C, 1);
	waitForTimer(31);
	for (int y = 0; y < kPart79Block2Height; ++y) {
		memset(_screenVGA + kPart79Block2Offset + y * 320, kPart79Block2Color, kPart79Block2Width);
	}
	for (int round = 1; round <= 5; ++round) {
		for (int frame = 0x10; frame <= 0x11; ++frame) {
			PART_79_DRAW_ANIM2_FRAME(&kPart79Anim2C, frame);
			waitForTimer(61);
		}
	}
	for (int frame = 2; frame <= 0xF; ++frame) {
		PART_79_DRAW_ANIM2_FRAME(&kPart79Anim2C, frame);
		waitForTimer(31);
	}
	cutsceneSayStart(206, 89, 63, 32, 0, { { 215, 1, 788 } });
	waitForEndOfCutsceneDialogue(206, 89, 63, 32, 0);
	waitForTimer(255);
	for (int frame = 1; frame <= 7; ++frame) {
		PART_79_DRAW_ANIM2_FRAME(&kPart79Anim2D, frame);
		waitForTimer(31);
	}
	copyArea(_screenVGA, kPart79Block3Offset, 320, _animFramesBuffer + kPart79Block3Frames, kPart79Block3Width,
			 kPart79Block3Width, kPart79Block3Height);
	PART_79_FADE_COLORS(kPart79FadeFirstColor, kPart79FadeLastColor, true);
	for (int i = 0; i < 8; ++i) {
		waitForTimer(255);
	}
	PART_79_FADE_COLORS(kPart79FadeFirstColor, kPart79FadeLastColor, false);

	loadIgorFrames();
	// TODO: a byte variable that the next state reads is cleared here (not identified yet)
	_currentPart = 910;
}

void IgorEngine::PART_79() {
	// the picture is shown below a black band
	if (_currentPart == 790) {
		memset(_screenVGA, 0, 64000);
	}
	uint8 band[320 * kPart79BandRows];
	memset(band, 0, sizeof(band));
	_system->copyRectToScreen(band, 320, 0, 0, 320, kPart79BandRows);
	_screenVGAVOffset = kPart79BandRows;
	if (_currentPart == 790) {
		PART_79_790();
	} else {
		PART_79_791();
	}
	_screenVGAVOffset = 0;
}

} // End of namespace Igor
