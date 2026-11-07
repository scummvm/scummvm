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

static const int kPart79BandRows = 24;

static const int kPart79LauraTable = 0xF858;

static const int kPart79ReflectionOffset = 135 * 320 + 86;
static const int kPart79ReflectionWidth = 17;
static const int kPart79PatchHeight = 9;
static const int kPart79ReflectionFrames = 0xF908;
static const int kPart79ReflectionFrameSize = kPart79ReflectionWidth * kPart79PatchHeight;
static const uint8 kPart79ReflectionSteps[8] = { 0, 1, 2, 3, 4, 3, 2, 1 };
static const int kPart79ReflectionClearColor = 0x68;

// Where and in which color the text of the scene is shown.
static const int kPart79TextX = 94;
static const int kPart79TextY = 108;
static const int kPart79TextR = 63;
static const int kPart79TextG = 0;
static const int kPart79TextB = 38;

static const int kPart79TalkFrame = 2;
static const int kPart79StandingFirst = 7;
static const int kPart79StandingEnd = 0x31;
static const int kPart79WalkFirst = 0x39;
static const int kPart79WalkLast = 0x4C;

static const int kPart79SlideSteps = 20;
static const int kPart79SlideFirstFrame = 77;
static const int kPart79SlideLastFrame = 88;
static const int kPart79SlideAnimSteps = 12;
static const int kPart79SlideLayer2Offset = 0xA0;
static const int kPart79BankTable = 0x6DA6;

void IgorEngine::part79DrawLauraInWater(int frame) {
	decodeAnimFrame(getAnimFrame(0, kPart79LauraTable, frame), _screenVGA, false);
}

void IgorEngine::part79DrawReflection(int step) {
	copyArea(_screenVGA, kPart79ReflectionOffset, 320, _animFramesBuffer + kPart79ReflectionFrames + step * kPart79ReflectionFrameSize,
			 kPart79ReflectionWidth, kPart79ReflectionWidth, kPart79PatchHeight);
}

void IgorEngine::part79ClearReflection() {
	for (int y = 0; y < kPart79PatchHeight; ++y) {
		memset(_screenVGA + kPart79ReflectionOffset + y * 320, kPart79ReflectionClearColor, kPart79ReflectionWidth);
	}
}

/**
 * Every 64 ticks the small piece goes on to the next of its steps.
 */
void IgorEngine::part79UpdateReflection() {
	if (compareGameTick(0x3D)) {
		part79DrawReflection(kPart79ReflectionSteps[_roomAmbientIndex]);
		_roomAmbientIndex = (_roomAmbientIndex == 7) ? 0 : _roomAmbientIndex + 1;
	}
}

void IgorEngine::part79UpdateLauraInWaterDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part79DrawLauraInWater(kPart79TalkFrame);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part79DrawLauraInWater(kPart79TalkFrame + getRandomNumber(4));
		break;
	}
}

void IgorEngine::part79SayLaura(const Common::Array<DialogueText> &lines) {
	cutsceneSayStartWithCallback(kPart79TextX, kPart79TextY, kPart79TextR, kPart79TextG, kPart79TextB, lines,
								 &IgorEngine::part79UpdateLauraInWaterDialogue);
	_updateRoomBackground = &IgorEngine::part79UpdateReflection;
	waitForEndOfCutsceneDialogue(kPart79TextX, kPart79TextY, kPart79TextR, kPart79TextG, kPart79TextB);
	_updateDialogue = nullptr;
	_updateRoomBackground = nullptr;
}

void IgorEngine::part79Scroll() {
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
				decodeAnimFrame(getAnimFrame(0, kPart79LauraTable, frame), buffer, false);
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

// Laura in the water
void IgorEngine::part79LauraInWater() {
	if (isCDVersion())
		playMusic(13);
	else
		playMusic(11, true);
	memset(_currentPalette, 0, 768);
	setPaletteRange(0, 255);
	loadRoomData(PAL_Part79, IMG_Part79, 0, 0, 0);
	uint8 *text = loadData(TXT_Part79);
	decodeRoomStrings(text, true);
	free(text);
	loadData(IMG_Part79Layer2, _screenLayer2);
	loadData(ANM_Part79, _animFramesBuffer);
	if (!isCDVersion())
		fillScreenRows(0, 200, 0);
	copyLayerToScreen(_screenLayer1Surface);
	part79DrawLauraInWater(1);
	fadeIn(768);
	_roomAmbientIndex = 0;

	part79SayLaura({ { 201, 2, 742 }, { 203, 1, 743 }, { 204, 2, 744 }, { 206, 1, 745 }, { 207, 1, 746 } });
	part79SayLaura({ { 208, 1, 747 }, { 209, 2, 748 } });

	// the figure goes through its frames while the small piece keeps changing
	int frame = kPart79StandingFirst;
	do {
		if (compareGameTick(1, 16)) {
			part79DrawLauraInWater(frame);
			++frame;
		}
		if (compareGameTick(0x3D)) {
			part79DrawReflection(kPart79ReflectionSteps[_roomAmbientIndex]);
			_roomAmbientIndex = (_roomAmbientIndex == 7) ? 0 : _roomAmbientIndex + 1;
			part79DrawReflection(getRandomNumber(4));
		}
		waitForTimer();
	} while (frame != kPart79StandingEnd);

	part79SayLaura({ { 211, 1, 749 }, { 212, 1, 750 }, { 213, 1, 751 }, { 214, 2, 752 } });
	part79ClearReflection();
	// laura walks
	for (frame = kPart79WalkFirst; frame <= kPart79WalkLast; ++frame) {
		part79DrawLauraInWater(frame);
		waitForTimer(31);
	}
	part79Scroll();
	_currentPart = 791;
}


struct Part79FrameGroup {
	int table;
	int base;
};

static const Part79FrameGroup kPart79Scenes = { 0xF6BA, 0 };

static const Part79FrameGroup kPart79Layer2Frames = { 0x594C, 0 };
static const Part79FrameGroup kPart79Layer2Frames2 = { 0x7FF0, 0x5989 };

static const Part79FrameGroup kPart79Anim2A = { 0xC093, 0x88C8 };
static const Part79FrameGroup kPart79Anim2B = { 0x3CC9, 0 };
static const Part79FrameGroup kPart79Anim2C = { 0x7ACD, 0x3CEF };
static const Part79FrameGroup kPart79Anim2D = { 0x88B8, 0x7AF1 };

static const int kPart79FirstSpeakerEndFrame = 1;
static const int kPart79FirstSpeakerBase = 7;
static const int kPart79ThirdSpeakerEndFrame = 22;
static const int kPart79ThirdSpeakerBase = 23;

static const int kPart79AmbientFirst = 0x57;
static const int kPart79AmbientSecond = 0x58;

static const int kPart79Block1Offset = 53 * 320 + 115;
static const int kPart79Block1Width = 90;
static const int kPart79Block1Height = 23;
static const int kPart79Block1Frames = 0xC0A3;
static const int kPart79Block1FloppyOffset = 53 * 320 + 84;
static const int kPart79Block1FloppyWidth = 151;
static const int kPart79Block2Offset = 108 * 320 + 201;
static const int kPart79Block2Width = 13;
static const int kPart79Block2Height = 11;
static const int kPart79Block2Color = 0x68;
static const int kPart79Block3Offset = 53 * 320 + 138;
static const int kPart79Block3Width = 44;
static const int kPart79Block3Height = 23;
static const int kPart79Block3Frames = 0xC8B9;
static const int kPart79Block3FloppyOffset = 53 * 320 + 93;
static const int kPart79Block3FloppyWidth = 133;
static const int kPart79Block3FloppyFrames = 0xCE34;
// The Spanish floppy title blocks are smaller, lower on the screen and in a different color
static const int kPart79Block1SpaFloppyOffset = 89 * 320 + 121;
static const int kPart79Block1SpaFloppyWidth = 79;
static const int kPart79Block1SpaFloppyHeight = 22;
static const int kPart79Block1SpaFloppyMark1 = 0x73C0;
static const int kPart79Block1SpaFloppyMark2 = 0x74FF;
static const int kPart79Block3SpaFloppyOffset = 89 * 320 + 141;
static const int kPart79Block3SpaFloppyWidth = 38;
static const int kPart79Block3SpaFloppyHeight = 22;
static const int kPart79Block3SpaFloppyFrames = 0xC76D;

static const int kPart79FadeFirstColor = 0xF5;
static const int kPart79FadeLastColor = 0xFE;
static const uint8 kPart79FadeColors[6] = { 0x3F, 0x20, 0x00, 0x2C, 0x0C, 0x00 };
static const uint8 kPart79SpaFloppyFadeColors[6] = { 0x00, 0x27, 0x19, 0x00, 0x12, 0x00 };

void IgorEngine::part79DrawGroupFrame(const uint8 *buffer, int table, int base, int frame) {
	decodeAnimFrame(buffer + base + READ_LE_UINT16(buffer + table + frame * 2) - 1, _screenVGA, false);
}

void IgorEngine::part79DrawBoomerangScene(int frame) {
	part79DrawGroupFrame(_animFramesBuffer, kPart79Scenes.table, kPart79Scenes.base, frame);
}

void IgorEngine::part79DrawKiss(int frame) {
	part79DrawGroupFrame(_screenLayer2, kPart79Layer2Frames.table, kPart79Layer2Frames.base, frame);
}

void IgorEngine::part79DrawCircularClosingTransition(int frame) {
	const int offset = READ_LE_UINT16(_screenLayer2 + kPart79Layer2Frames2.table + frame * 2);
	decodeAnimFrame(_screenLayer2 + kPart79Layer2Frames2.base + offset, _screenVGA, false);
}

void IgorEngine::part79DrawPhilipSharkScene(const void *group, int frame) {
	const Part79FrameGroup *g = (const Part79FrameGroup *)group;
	part79DrawGroupFrame(_animFramesBuffer, g->table, g->base, frame);
}

void IgorEngine::part79AnimateIgorHead(int frame) {
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

void IgorEngine::part79UpdateSpeakerDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part79DrawBoomerangScene(_part79EndFrame);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part79DrawBoomerangScene(_part79BaseFrame + getRandomNumber(3));
		break;
	}
}

void IgorEngine::part79UpdateLauraDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part79DrawBoomerangScene(kPart79FirstSpeakerEndFrame);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part79DrawBoomerangScene(kPart79FirstSpeakerBase + getRandomNumber(3));
		break;
	}
}

void IgorEngine::part79UpdatePhilipDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part79DrawKiss(kPart79ThirdSpeakerEndFrame);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part79DrawKiss(kPart79ThirdSpeakerBase + getRandomNumber(3));
		break;
	}
}

void IgorEngine::part79AnimatePhilipShaking() {
	if (compareGameTick(0x3D)) {
		part79DrawBoomerangScene(_roomAmbientIndex);
		_roomAmbientIndex = (_roomAmbientIndex == kPart79AmbientFirst) ? kPart79AmbientSecond : kPart79AmbientFirst;
	}
}

void IgorEngine::part79SaySpeaker(int startX, int startY, int x, int y, int r, int g, int b, int endFrame, int baseFrame,
									 const Common::Array<DialogueText> &lines) {
	_part79EndFrame = endFrame;
	_part79BaseFrame = baseFrame;
	cutsceneSayStartWithCallback(startX, startY, r, g, b, lines, &IgorEngine::part79UpdateSpeakerDialogue);
	waitForEndOfCutsceneDialogue(x, y, r, g, b);
	_updateDialogue = nullptr;
}

void IgorEngine::part79SayLaura(int startX, int startY, const Common::Array<DialogueText> &lines) {
	cutsceneSayStartWithCallback(startX, startY, 63, 0, 38, lines, &IgorEngine::part79UpdateLauraDialogue);
	waitForEndOfCutsceneDialogue(145, 86, 63, 0, 38);
	_updateDialogue = nullptr;
}

void IgorEngine::part79SayPhilip(const Common::Array<DialogueText> &lines) {
	cutsceneSayStartWithCallback(220, 91, 63, 0, 38, lines, &IgorEngine::part79UpdatePhilipDialogue);
	waitForEndOfCutsceneDialogue(220, 91, 63, 0, 38);
	_updateDialogue = nullptr;
}

void IgorEngine::part79SayIgor(const Common::Array<DialogueText> &lines) {
	igorSay(lines);
	waitForEndOfIgorDialogue();
}

void IgorEngine::part79SetIgorPos(int x, int y) {
	_walkData[0].x = x;
	_walkData[0].y = y;
}

void IgorEngine::part79FadeColors(int first, int last, bool fadeInColors) {
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

void IgorEngine::part79FadeInPair(int firstColor) {
	memcpy(_paletteBuffer + firstColor * 3, _game.version == kIdSpaFloppy ? kPart79SpaFloppyFadeColors : kPart79FadeColors, 6);
	part79FadeColors(firstColor, firstColor + 1, true);
}

void IgorEngine::part79BeachScene() {
	if (isCDVersion())
		playMusic(13);
	_gameState.enableLight = 2;
	// The ending uses the beach clothes, including when entered directly.
	loadIgorBeachFrames();
	loadData(IMG_Part79b, _screenLayer1);
	uint8 *text = loadData(TXT_Part79b);
	decodeRoomStrings(text, true);
	free(text);
	loadData(ANM_Part79bFrames, _animFramesBuffer);
	loadData(ANM_Part79bLayer2, _screenLayer2);

	part79SaySpeaker(178, 82, 178, 82, 63, 63, 0, 2, 2, { { 201, 2, 753 }, { 203, 1, 754 }, { 204, 1, 755 } });
	part79SayLaura(145, 86, { { 205, 2, 756 }, { 207, 2, 757 } });
	part79SaySpeaker(172, 82, 178, 82, 63, 63, 0, 2, 2, { { 209, 2, 758 } });
	for (int frame = 0x10; frame <= 0x1C; ++frame) {
		part79DrawBoomerangScene(frame);
		waitForTimer(31);
	}
	part79SayLaura(145, 86, { { 211, 1, 759 } });
	for (int frame = 0xB; frame <= 0xF; ++frame) {
		part79DrawBoomerangScene(frame);
		waitForTimer(11);
	}
	part79DrawBoomerangScene(29);
	part79SaySpeaker(178, 82, 178, 82, 63, 63, 0, 0x1D, 0x1E, { { 212, 1, 760 }, { 213, 1, 761 } });

	part79SetIgorPos(229, 137);
	_walkData[0].posNum = 4;
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 30;
	_walkData[0].scaleWidth = 50;
	_walkData[0].scaleHeight = 50;
	_walkDataLastIndex = 1;
	part79SayIgor({ { 214, 2, 762 }, { 216, 2, 763 } });
	part79SaySpeaker(178, 82, 178, 82, 63, 63, 0, 0x1D, 0x1E, { { 218, 2, 764 } });
	for (int frame = 0x21; frame <= 0x26; ++frame) {
		part79DrawBoomerangScene(frame);
		waitForTimer(31);
	}
	part79DrawBoomerangScene(46);

	part79SetIgorPos(236, 138);
	part79SayIgor({ { 220, 2, 765 } });
	part79SaySpeaker(195, 83, 195, 83, 63, 63, 0, 40, 41, { { 222, 2, 766 } });
	part79DrawBoomerangScene(39);
	waitForTimer(31);
	part79DrawBoomerangScene(46);
	part79SayIgor({ { 224, 2, 767 } });
	for (int frame = 0x2F; frame <= 0x32; ++frame) {
		part79DrawBoomerangScene(frame);
		waitForTimer(31);
	}
	part79SaySpeaker(206, 85, 206, 85, 63, 63, 0, 0x35, 0x36, { { 226, 2, 768 } });
	part79DrawBoomerangScene(51);
	waitForTimer(31);

	part79SetIgorPos(246, 138);
	part79SayIgor({ { 228, 2, 769 }, { 230, 2, 770 } });
	part79SaySpeaker(206, 85, 206, 85, 63, 63, 0, 0x35, 0x36, { { 232, 1, 771 } });
	playSound(28, 1);
	for (int frame = 0x3A; frame <= 0x56; ++frame) {
		part79DrawBoomerangScene(frame);
		if (frame == 0x4F) {
			playSound(29, 1);
		}
		waitForTimer(17);
	}
	part79DrawBoomerangScene(52);
	waitForTimer(33);
	// the sound goes on until it ends
	while (_mixer->isSoundHandleActive(_sfxHandle) || isDialogueSpeechPlaying()) {
		waitForTimer();
	}
	_roomAmbientIndex = kPart79AmbientFirst;
	igorSay(233, 2, 772);
	_updateRoomBackground = &IgorEngine::part79AnimatePhilipShaking;
	waitForEndOfIgorDialogue();
	_updateRoomBackground = nullptr;
	for (int frame = 0x58; frame <= 0x64; ++frame) {
		part79DrawBoomerangScene(frame);
		if (frame < 0x64) {
			waitForTimer(31);
		}
		if (frame == 0x59) {
			playRoomSound(48, 0);
		}
	}

	for (int frame = 1; frame <= 3; ++frame) {
		part79DrawKiss(frame);
		waitForTimer(31);
	}
	part79SetIgorPos(178, 142);
	_walkData[0].posNum = 2;
	part79SayIgor({ { 235, 1, 773 }, { 236, 2, 774 } });
	for (int frame = 4; frame <= 0x12; ++frame) {
		part79DrawKiss(frame);
		waitForTimer(31);
	}
	part79SayIgor({ { 238, 1, 775 } });
	// the texts 201 to 215 are replaced by the ones of the main resource
	text = loadData(TXT_Part79bMain);
	decodeRoomStrings(text, true);
	free(text);
	for (int frame = 0x13; frame <= 0x15; ++frame) {
		part79DrawKiss(frame);
		waitForTimer(31);
	}

	part79SetIgorPos(196, 141);
	part79SayIgor({ { 201, 2, 776 }, { 203, 2, 777 } });
	part79SayPhilip({ { 205, 1, 778 }, { 206, 1, 779 }, { 207, 1, 780 } });
	part79SayIgor({ { 208, 1, 781 }, { 209, 1, 782 } });
	part79SayPhilip({ { 210, 1, 783 } });
	part79SayIgor({ { 211, 1, 784 } });
	part79SayPhilip({ { 212, 1, 785 } });
	part79SayIgor({ { 213, 1, 786 } });
	part79SayPhilip({ { 214, 1, 787 } });
	for (int frame = 0x1B; frame <= 0x1D; ++frame) {
		part79DrawKiss(frame);
		waitForTimer(31);
	}
	for (int frame = 1; frame <= 0xE; ++frame) {
		part79DrawCircularClosingTransition(frame);
		waitForTimer(31);
	}
	waitForTimer(255);
	part79DrawKiss(30);
	waitForTimer(255);
	for (int frame = 0xF; frame <= 0x11; ++frame) {
		part79DrawCircularClosingTransition(frame);
		waitForTimer(31);
	}

	// the second animation resource replaces the first one
	loadData(ANM_Part79bScenes, _animFramesBuffer);
	const bool englishFloppy = _game.version == kIdEngFloppy;
	const bool spanishFloppy = _game.version == kIdSpaFloppy;
	const int titleOffset = englishFloppy ? kPart79Block1FloppyOffset : kPart79Block1Offset;
	const int titleWidth = englishFloppy ? kPart79Block1FloppyWidth : kPart79Block1Width;
	if (spanishFloppy) {
		copyArea(_screenVGA, kPart79Block1SpaFloppyOffset, 320, _animFramesBuffer + kPart79Block1Frames, kPart79Block1SpaFloppyWidth,
				 kPart79Block1SpaFloppyWidth, kPart79Block1SpaFloppyHeight);
		memset(_screenVGA + kPart79Block1SpaFloppyMark1, 0xFE, 2);
		memset(_screenVGA + kPart79Block1SpaFloppyMark2, 0xFE, 2);
	} else {
		copyArea(_screenVGA, titleOffset, 320, _animFramesBuffer + kPart79Block1Frames, titleWidth,
				 titleWidth, kPart79Block1Height);
	}

	part79FadeInPair(0xF5);
	for (int i = 0; i < (spanishFloppy ? 3 : 5); ++i) {
		waitForTimer(255);
	}
	part79FadeInPair(0xF7);
	waitForTimer(255);
	part79FadeInPair(0xF9);
	waitForTimer(255);
	part79FadeInPair(0xFB);
	waitForTimer(255);
	part79FadeInPair(0xFD);
	for (int i = 0; i < 3; ++i) {
		waitForTimer(255);
	}
	part79FadeColors(kPart79FadeFirstColor, kPart79FadeLastColor, false);

	for (int frame = 1; frame <= 7; ++frame) {
		part79DrawPhilipSharkScene(&kPart79Anim2A, frame);
		waitForTimer(31);
	}
	for (int frame = 2; frame <= 0x12; ++frame) {
		part79DrawPhilipSharkScene(&kPart79Anim2B, frame);
		waitForTimer(31);
	}
	for (int frame = 1; frame <= 0x12; ++frame) {
		part79DrawPhilipSharkScene(&kPart79Anim2B, frame);
		waitForTimer(31);
	}
	part79DrawPhilipSharkScene(&kPart79Anim2C, 1);
	waitForTimer(31);
	for (int y = 0; y < kPart79Block2Height; ++y) {
		memset(_screenVGA + kPart79Block2Offset + y * 320, kPart79Block2Color, kPart79Block2Width);
	}
	for (int round = 1; round <= 5; ++round) {
		for (int frame = 0x10; frame <= 0x11; ++frame) {
			part79DrawPhilipSharkScene(&kPart79Anim2C, frame);
			waitForTimer(61);
		}
	}
	for (int frame = 2; frame <= 0xF; ++frame) {
		part79DrawPhilipSharkScene(&kPart79Anim2C, frame);
		waitForTimer(31);
	}
	cutsceneSayStart(206, 89, 63, 32, 0, { { 215, 1, 788 } });
	waitForEndOfCutsceneDialogue(206, 89, 63, 32, 0);
	waitForTimer(255);
	for (int frame = 1; frame <= 7; ++frame) {
		part79DrawPhilipSharkScene(&kPart79Anim2D, frame);
		waitForTimer(31);
	}
	const int finalTitleOffset = englishFloppy ? kPart79Block3FloppyOffset : kPart79Block3Offset;
	const int finalTitleWidth = englishFloppy ? kPart79Block3FloppyWidth : kPart79Block3Width;
	const int finalTitleFrames = englishFloppy ? kPart79Block3FloppyFrames : kPart79Block3Frames;
	if (spanishFloppy) {
		copyArea(_screenVGA, kPart79Block3SpaFloppyOffset, 320, _animFramesBuffer + kPart79Block3SpaFloppyFrames, kPart79Block3SpaFloppyWidth,
				 kPart79Block3SpaFloppyWidth, kPart79Block3SpaFloppyHeight);
	} else {
		copyArea(_screenVGA, finalTitleOffset, 320, _animFramesBuffer + finalTitleFrames, finalTitleWidth,
				 finalTitleWidth, kPart79Block3Height);
	}
	part79FadeColors(kPart79FadeFirstColor, kPart79FadeLastColor, true);
	for (int i = 0; i < (spanishFloppy ? 5 : 8); ++i) {
		waitForTimer(255);
	}
	part79FadeColors(kPart79FadeFirstColor, kPart79FadeLastColor, false);

	loadIgorFrames();
	_creditsPass = 0;
	_currentPart = 910;
}

void IgorEngine::part79() {
	const bool savingWasAllowed = _canSaveGame;
	_canSaveGame = false;
	// the picture is shown below a black band (the Spanish floppy draws it at the top of the screen)
	if (_currentPart == 790) {
		fillScreenRows(0, 200, 0);
	}
	if (_game.version != kIdSpaFloppy) {
		uint8 band[320 * kPart79BandRows];
		memset(band, 0, sizeof(band));
		_system->copyRectToScreen(band, 320, 0, 0, 320, kPart79BandRows);
		_screenVGAVOffset = kPart79BandRows;
	}
	if (_currentPart == 790) {
		part79LauraInWater();
	} else {
		part79BeachScene();
	}
	_screenVGAVOffset = 0;
	_canSaveGame = savingWasAllowed;
}

} // End of namespace Igor
