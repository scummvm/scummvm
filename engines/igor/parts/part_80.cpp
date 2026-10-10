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

// A 50x29 area of the picture is redrawn from the animation resource, whose frames are 50x29 (1450 bytes) each.
static const int kPart80PatchOffset = 62 * 320 + 135;
static const int kPart80PatchWidth = 50;
static const int kPart80PatchHeight = 29;
static const int kPart80PatchFrameSize = 1450;

// Four frames of 28x69 are drawn into the picture at the end of the scene; they start at this offset of the animation
// resource.
static const int kPart80ObjectOffset = 68 * 320 + 121;
static const int kPart80ObjectWidth = 28;
static const int kPart80ObjectHeight = 69;
static const int kPart80ObjectFrameSize = 0x78C;
static const int kPart80ObjectFramesBase = 0x25C4;

// Where and in which color each of the two persons talk.
static const int kPart80FirstX = 149;
static const int kPart80FirstY = 59;
static const int kPart80SecondX = 171;
static const int kPart80SecondY = 60;

void IgorEngine::PART_80_DRAW_PATCH(int frame) {
	copyArea(_screenVGA, kPart80PatchOffset, 320, _animFramesBuffer + (frame - 1) * kPart80PatchFrameSize, kPart80PatchWidth,
			 kPart80PatchWidth, kPart80PatchHeight);
}

void IgorEngine::PART_80_UPDATE_DIALOGUE_FIRST(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_80_DRAW_PATCH(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_80_DRAW_PATCH(getRandomNumber(3) + 1);
		break;
	}
}

void IgorEngine::PART_80_UPDATE_DIALOGUE_SECOND(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_80_DRAW_PATCH(5);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_80_DRAW_PATCH(getRandomNumber(3) + 5);
		break;
	}
}

void IgorEngine::PART_80_SAY(int who, const Common::Array<DialogueText> &lines) {
	if (who == 0) {
		cutsceneSayWithCallback(kPart80FirstX, kPart80FirstY, 0, 44, 63, lines, &IgorEngine::PART_80_UPDATE_DIALOGUE_FIRST);
	} else {
		cutsceneSayWithCallback(kPart80SecondX, kPart80SecondY, 63, 0, 38, lines, &IgorEngine::PART_80_UPDATE_DIALOGUE_SECOND);
	}
}

/**
 * Scene of two persons talking, followed by four frames drawn into the picture. It is played at the end of state 730.
 */
void IgorEngine::PART_80() {
	playMusic(8);
	_gameState.enableLight = 1;
	loadRoomData(PAL_Part80, IMG_Part80, BOX_Part80, MSK_Part80, TXT_Part80);
	static const int anim[] = { ANM_Part80, 0 };
	loadAnimData(anim);
	memcpy(_screenVGA, _screenLayer1, 46080);
	memset(_screenVGA + 46080, 0, 17920);
	fadeIn(768);
	_gameState.igorMoving = false;

	PART_80_SAY(0, { { 201, 2, 1131 } });
	PART_80_SAY(1, { { 203, 1, 1132 } });
	PART_80_SAY(0, { { 204, 2, 1133 } });
	PART_80_SAY(1, { { 206, 1, 1134 } });
	PART_80_SAY(0, { { 207, 1, 1135 } });
	PART_80_SAY(1, { { 208, 1, 1136 } });
	PART_80_SAY(0, { { 209, 1, 1137 } });
	PART_80_SAY(1, { { 210, 1, 1138 } });
	PART_80_SAY(0, { { 211, 1, 1139 }, { 212, 1, 1140 } });
	PART_80_SAY(1, { { 213, 1, 1141 } });
	PART_80_SAY(0, { { 214, 2, 1142 } });
	PART_80_SAY(1, { { 216, 1, 1143 }, { 217, 1, 1144 } });
	PART_80_SAY(0, { { 218, 1, 1145 } });
	PART_80_SAY(1, { { 219, 1, 1146 }, { 220, 1, 1147 }, { 221, 1, 1148 } });
	waitForTimer(255);

	for (int i = 1; i <= 4; ++i) {
		copyArea(_screenVGA, kPart80ObjectOffset, 320,
				 _animFramesBuffer + kPart80ObjectFramesBase + i * kPart80ObjectFrameSize, kPart80ObjectWidth,
				 kPart80ObjectWidth, kPart80ObjectHeight);
		if (i < 4) {
			waitForTimer(21);
		} else {
			playSound(15, 1);
		}
	}
	for (int i = 1; i <= 3; ++i) {
		waitForTimer(255);
	}
	memcpy(_paletteBuffer, _currentPalette, 624);
	fadeOut(768);
}

} // End of namespace Igor
