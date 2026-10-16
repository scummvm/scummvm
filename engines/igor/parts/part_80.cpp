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

// Laura and friend talking are actually the same screen patch
static const int kPart80PatchOffset = 62 * 320 + 135;
static const int kPart80PatchWidth = 50;
static const int kPart80PatchHeight = 29;
static const int kPart80PatchFrameSize = 1450;

// animation of the girl falling at the end
static const int kPart80FallingGirlOffset = 68 * 320 + 121;
static const int kPart80FallingGirlWidth = 28;
static const int kPart80FallingGirlHeight = 69;
static const int kPart80FallingGirlFrameSize = 0x78C;
static const int kPart80FallingGirlFramesBase = 0x25C4;

// Where and in which color each of the two persons talk.
static const int kPart80FriendX = 149;
static const int kPart80FriendY = 59;
static const int kPart80LauraX = 171;
static const int kPart80LauraY = 60;

void IgorEngine::part80DrawFrame(int frame) {
	copyArea(_screenVGA, kPart80PatchOffset, 320, _animFramesBuffer + (frame - 1) * kPart80PatchFrameSize, kPart80PatchWidth,
			 kPart80PatchWidth, kPart80PatchHeight);
}

void IgorEngine::part80UpdateFriendDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part80DrawFrame(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part80DrawFrame(getRandomNumber(3) + 1);
		break;
	}
}

void IgorEngine::part80UpdateLauraDialogue(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		part80DrawFrame(5);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		part80DrawFrame(getRandomNumber(3) + 5);
		break;
	}
}

void IgorEngine::part80Say(int who, const Common::Array<DialogueText> &lines) {
	if (who == 0) {
		cutsceneSayWithCallback(kPart80FriendX, kPart80FriendY, 0, 44, 63, lines, &IgorEngine::part80UpdateFriendDialogue);
	} else {
		cutsceneSayWithCallback(kPart80LauraX, kPart80LauraY, 63, 0, 38, lines, &IgorEngine::part80UpdateLauraDialogue);
	}
}

/**
 * Larua and Friend discuss the great dosicua recovery.
 */
void IgorEngine::part80() {
	const bool savingWasAllowed = _canSaveGame;
	_canSaveGame = false;
	if (isCDVersion())
		playMusic(8);
	_gameState.enableLight = 1;
	loadRoomData(PAL_Part80, IMG_Part80, BOX_Part80, MSK_Part80, TXT_Part80);
	static const int anim[] = { ANM_Part80, 0 };
	loadAnimData(anim);
	memcpy(_screenVGA, _screenLayer1, 46080);
	memset(_screenVGA + 46080, 0, 17920);
	fadeIn(768);
	_gameState.igorMoving = false;

	part80Say(0, { { 201, 2, 1131 } });
	part80Say(1, { { 203, 1, 1132 } });
	part80Say(0, { { 204, 2, 1133 } });
	part80Say(1, { { 206, 1, 1134 } });
	part80Say(0, { { 207, 1, 1135 } });
	part80Say(1, { { 208, 1, 1136 } });
	part80Say(0, { { 209, 1, 1137 } });
	part80Say(1, { { 210, 1, 1138 } });
	part80Say(0, { { 211, 1, 1139 }, { 212, 1, 1140 } });
	part80Say(1, { { 213, 1, 1141 } });
	part80Say(0, { { 214, 2, 1142 } });
	part80Say(1, { { 216, 1, 1143 }, { 217, 1, 1144 } });
	part80Say(0, { { 218, 1, 1145 } });
	part80Say(1, { { 219, 1, 1146 }, { 220, 1, 1147 }, { 221, 1, 1148 } });
	// The Spanish floppy has no animation of the friend falling and goes straight to the final holds
	if (_game.version != kIdSpaFloppy) {
		waitForTimer(255);

		// left-most friend falls on the floor
		for (int i = 1; i <= 4; ++i) {
			copyArea(_screenVGA, kPart80FallingGirlOffset, 320,
					 _animFramesBuffer + kPart80FallingGirlFramesBase + i * kPart80FallingGirlFrameSize, kPart80FallingGirlWidth,
					 kPart80FallingGirlWidth, kPart80FallingGirlHeight);
			if (i < 4) {
				waitForTimer(21);
			} else {
				playSound(15, 1);
			}
		}
	}
	for (int i = 1; i <= 3; ++i) {
		waitForTimer(255);
	}
	memcpy(_paletteBuffer, _currentPalette, 624);
	fadeOut(768);
	_canSaveGame = savingWasAllowed;
}

} // End of namespace Igor
