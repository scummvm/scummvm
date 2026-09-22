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

static int VAR_CURRENT_TALKING_ACTOR;
static bool VAR_WATER_SOUND_PLAYING;


void IgorEngine::PART_85_UPDATE_DIALOGUE_PHILIP_LAURA(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		decodeAnimFrame(_animFramesBuffer - 1 + READ_LE_UINT16(_animFramesBuffer + 0x7480), _screenVGA, true);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		if (VAR_CURRENT_TALKING_ACTOR != 0) {
			_gameState.counter[0] = getRandomNumber(5) + 8;
		} else {
			_gameState.counter[0] = getRandomNumber(5) + 13;
		}
		decodeAnimFrame(getAnimFrame(0, 0x7480, _gameState.counter[0]), _screenVGA, true);
		break;
	}
}


void IgorEngine::PART_85() {
	if(_game.flags & kFlagFloppy) {
		playMusic(2);
	} else playMusic(11);
	loadRoomData(PAL_SpringRock, IMG_SpringRock, BOX_SpringRock, MSK_SpringRock, TXT_SpringRock);
	memcpy(_screenLayer2, _screenLayer1, 46080);
	loadRoomData(PAL_SpringBridgeIntro, IMG_SpringBridgeIntro, 0, 0, TXT_SpringBridgeIntro);
	static const int anm[] = { ANM_PhilipLauraIntro, AOF_PhilipLauraIntro, ANM_LauraIntro, AOF_LauraIntro, 0 };
	loadAnimData(anm);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_updateDialogue = &IgorEngine::PART_85_UPDATE_DIALOGUE_PHILIP_LAURA;
	_updateRoomBackground = &IgorEngine::PART_85_UPDATE_ROOM_BACKGROUND;
	VAR_WATER_SOUND_PLAYING = true;
	// playSound(17, 1);
	decodeAnimFrame(getAnimFrame(0, 0x7480, 1), _screenVGA, true);
	fadeIn(768);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	PART_85_HELPER_1(0, 0x7481, 2, 7, 32);
	// if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	// ADD_DIALOGUE_TEXT(201, 3, 545);
	// ADD_DIALOGUE_TEXT(204, 1, 546);
	// SET_DIALOGUE_TEXT(1, 2);
	// startCutsceneDialogue(89, 56, 63, 63, 0);
	// VAR_CURRENT_TALKING_ACTOR = 0;
	// waitForEndOfCutsceneDialogue(89, 56, 63, 63, 0);
	// if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	// ADD_DIALOGUE_TEXT(205, 3, 547);
	// SET_DIALOGUE_TEXT(1, 1);
	// startCutsceneDialogue(113, 59, 63, 0, 38);
	// VAR_CURRENT_TALKING_ACTOR = 1;
	// waitForEndOfCutsceneDialogue(113, 59, 63, 0, 38);
	// if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	// ADD_DIALOGUE_TEXT(208, 1, 548);
	// SET_DIALOGUE_TEXT(1, 1);
	// startCutsceneDialogue(89, 56, 63, 63, 0);
	// VAR_CURRENT_TALKING_ACTOR = 0;
	// waitForEndOfCutsceneDialogue(89, 56, 63, 63, 0);
	// if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	// ADD_DIALOGUE_TEXT(209, 2, 549);
	// ADD_DIALOGUE_TEXT(211, 1, 550);
	// ADD_DIALOGUE_TEXT(212, 1, 551);
	// ADD_DIALOGUE_TEXT(213, 1, 552);
	// ADD_DIALOGUE_TEXT(214, 1, 553);
	// SET_DIALOGUE_TEXT(1, 5);
	// startCutsceneDialogue(113, 59, 63, 0, 38);
	// VAR_CURRENT_TALKING_ACTOR = 1;
	// waitForEndOfCutsceneDialogue(113, 59, 63, 0, 38);
	// if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	// PART_85_HELPER_1(0, 0x7481, 2, 7, 32);
	// if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	// ADD_DIALOGUE_TEXT(215, 1, 554);
	// ADD_DIALOGUE_TEXT(216, 2, 555);
	// ADD_DIALOGUE_TEXT(218, 1, 556);
	// SET_DIALOGUE_TEXT(1, 3);
	// startCutsceneDialogue(89, 56, 63, 63, 0);
	// VAR_CURRENT_TALKING_ACTOR = 0;
	// waitForEndOfCutsceneDialogue(89, 56, 63, 63, 0);
	// if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	// ADD_DIALOGUE_TEXT(219, 2, 557);
	// ADD_DIALOGUE_TEXT(221, 2, 558);
	// SET_DIALOGUE_TEXT(1, 2);
	// startCutsceneDialogue(113, 59, 63, 0, 38);
	// VAR_CURRENT_TALKING_ACTOR = 1;
	// waitForEndOfCutsceneDialogue(113, 59, 63, 0, 38);
	// if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	// decodeAnimFrame(getAnimFrame(0, 0x7480, 1), _screenVGA, true);
	// PART_85_HELPER_1(0, 0x7481, 2, 7, 32);
	// if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	// PART_85_HELPER_1(0, 0x7481, 18, 37, 20);
	// if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	PART_85_HELPER_2();
	VAR_WATER_SOUND_PLAYING = false;
	stopSound();
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	PART_85_HELPER_1(0x74CA, 0xA6C4, 1, 6, 32);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	for (int i = 0; i <= 200 / kTimerTicksCount; ++i) {
		PART_85_UPDATE_ROOM_BACKGROUND();
		if (_inputVars[kInputEscape]) goto PART_85_EXIT;
		waitForTimer();
	}
	_walkData[0].x = 251;
	_walkData[0].y = 125;
	_walkData[0].scaleWidth = 50;
	_walkDataLastIndex = 1;
	_walkCurrentPos = 4;
	ADD_DIALOGUE_TEXT(223, 2, 559);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	PART_85_HELPER_1(0x74CA, 0xA6C4, 7, 9, 32);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	_walkData[0].x = 250;
	_walkData[0].y = 119;
	_walkCurrentPos = 3;
	ADD_DIALOGUE_TEXT(225, 2, 560);
	ADD_DIALOGUE_TEXT(227, 3, 561);
	SET_DIALOGUE_TEXT(1, 2);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	PART_85_HELPER_1(0x74CA, 0xA6C4, 10, 24, 16);
	if (_game.version == kIdSpaCD) {
		displayLogo(); // cseg209:0x12B5
	}

PART_85_EXIT:
	stopSound();
	_gameState.dialogueTextRunning = false;
	_inputVars[kInputEscape] = 0;
	_walkData[0].setPos(155, 121, 4, 0);
	_walkData[0].setDefaultScale();
	copyArea(_screenLayer2, 23180, 320, _facingIgorFrames[3], 30, 30, 50, true);
	memcpy(_screenVGA, _screenLayer2, 46080);
	memset(_currentPalette + 0x270, 0, 0x8D);
	setPaletteRange(208, 254);
	_currentPart = 61;
	_updateDialogue = 0;
	_updateRoomBackground = 0;
}

void IgorEngine::displayLogo() {
	// The CD executable preserves the last intro frame while it displays a
	// 320x122 title card, then restores that frame before entering part 61.
	memcpy(_screenTextLayer, _screenVGA, 46080); // cseg209:0x03EE-0x0400
	playSound(65, 1); // cseg209:0x041C-0x0420

	memcpy(_paletteBuffer, _currentPalette, 768); // cseg209:0x0425-0x0432
	loadData(PAL_IgorIntroLogo, _currentPalette); // cseg209:0x0437-0x0460
	_currentPalette[765] = 63; // cseg209:0x0465
	_currentPalette[766] = 63; // cseg209:0x046A
	_currentPalette[767] = 63; // cseg209:0x046F
	updatePalette(768); // cseg209:0x0474-0x0486
	loadData(IMG_IgorIntroLogo, _screenVGA); // cseg209:0x048B-0x04B8

	for (int i = 0; i <= 200 / kTimerTicksCount; ++i) { // cseg209:0x04BD-0x04F2
		PART_85_UPDATE_ROOM_BACKGROUND();
		waitForTimer();
	}

	uint8 *subtitle = loadData(FRM_IgorIntroLogo); // cseg209:0x0002
	for (int y = 0; y < 13; ++y) {
		for (int x = 0; x < 206; ++x) {
			const uint8 color = subtitle[y * 206 + x];
			if (color != 255) {
				_screenVGA[(130 + y) * 320 + 57 + x] = (color == 241) ? 0 : 255;
			}
		}
	}
	free(subtitle);

	// Update the stored palette while exposing only color 255 to VGA.
	// cseg209:0x04F9-0x05CB
	for (int threshold = 63; threshold >= 0; threshold -= 3) {
		for (int i = 0; i < 768; ++i) {
			if (_paletteBuffer[i] >= threshold) {
				const int color = _currentPalette[i] + 3;
				_currentPalette[i] = MIN<int>(color, _paletteBuffer[i]);
			}
		}
		setPaletteRange(255, 255); // cseg209:0x05BB-0x05BF
		waitForTimer();
	}

	// The original counter runs in DOS timer units; waitForTimer() advances by
	// kTimerTicksCount units. cseg209:0x05CE-0x0635
	for (int i = 0; i <= 1500 / kTimerTicksCount; ++i) {
		PART_85_UPDATE_ROOM_BACKGROUND();
		waitForTimer();
		if (_inputVars[kInputClick] || _inputVars[kInputEscape] || _inputVars[kInputOptions]) {
			_inputVars[kInputClick] = 0;
			_inputVars[kInputEscape] = 0;
			_inputVars[kInputOptions] = 0;
			break;
		}
	}

	memcpy(_screenVGA, _screenTextLayer, 39040); // cseg209:0x0637-0x0649
	memcpy(_currentPalette, _paletteBuffer, 765); // cseg209:0x064E-0x065B
	setPaletteRange(0, 255); // cseg209:0x0660-0x0664

	// // The original updates only color 255 here. The other palette changes stay
	// // internal and therefore do not produce a visible fade. cseg209:0x0669-0x0705
	// for (int threshold = 0; threshold <= 63; threshold += 3) {
	// 	for (int i = 0; i < 768; ++i) {
	// 		if (_paletteBuffer[i] >= threshold) {
	// 			_currentPalette[i] = (_currentPalette[i] < 3) ? 0 : _currentPalette[i] - 3;
	// 		}
	// 	}
	// 	setPaletteRange(255, 255); // cseg209:0x06F0-0x06F4
	// 	waitForTimer();
	// }

	memcpy(_screenVGA, _screenTextLayer, 46080); // cseg209:0x0708-0x071A
	_currentPalette[765] = 63; // cseg209:0x071F
	_currentPalette[766] = 63; // cseg209:0x0724
	_currentPalette[767] = 63; // cseg209:0x0729
	setPaletteRange(255, 255); // cseg209:0x072E-0x0740
}

void IgorEngine::PART_85_HELPER_1(int frameOffset2, int frameOffset1, int firstFrame, int lastFrame, int delay) {
	do {
		if (compareGameTick(0, delay)) {
			decodeAnimFrame(_animFramesBuffer + frameOffset2 - 1 + READ_LE_UINT16(_animFramesBuffer + frameOffset1 + firstFrame * 2 - 3), _screenVGA, true);
			++firstFrame;
		}
		PART_85_UPDATE_ROOM_BACKGROUND();
		if (_inputVars[kInputEscape]) return;
		waitForTimer();
	} while (firstFrame <= lastFrame);
}

void IgorEngine::PART_85_HELPER_2() {
	int x = 1;
	do {
		if (compareGameTick(0, 16)) {
			for (int y = 0; y <= 143; ++y) {
				memcpy(_screenTextLayer + y * 320, _screenLayer1 + y * 320 + x * 8, 320 - x * 8);
				memcpy(_screenTextLayer + y * 320 + 320 - x * 8, _screenLayer2 + y * 320 + 96, x * 8);
			}
			memcpy(_screenVGA, _screenTextLayer, 46080);
			++x;
		}
		PART_85_UPDATE_ROOM_BACKGROUND();
		waitForTimer();
	} while (x != 29 && !_inputVars[kInputEscape]);
}

void IgorEngine::PART_85_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(61)) {
		scrollPalette(160, 167);
		setPaletteRange(160, 167);
	}
	if (compareGameTick(2, 8)) {
		scrollPalette(168, 178);
		setPaletteRange(168, 178);
	}
	if (compareGameTick(13, 16)) {
		scrollPalette(179, 184);
		setPaletteRange(179, 184);
	}
	if (compareGameTick(5, 32)) {
		scrollPalette(185, 191);
		setPaletteRange(185, 191);
	}
	if (!VAR_WATER_SOUND_PLAYING) {
		return;
	}
	if (_gameState.talkMode == kTalkModeTextOnly /*|| !_gameState.dialogueTextRunning*/) {
		playSound(17, 1);
	}
}

void IgorEngine::PART_85_HELPER_6(int frame) {
	const int offset = (_walkCurrentPos == 4) ? 24564 : 22643;
	copyArea(_screenVGA, offset, 320, _screenLayer2 + offset, 320, 14, 8);
	copyArea(_screenVGA, offset, 320, _igorHeadFrames + (_walkCurrentPos - 1) * 924 + frame * 154, 14, 14, 8, true);
}


void IgorEngine::copyArea(uint8 *dst, int dstOffset, int dstPitch, const uint8 *src, int srcPitch, int w, int h, bool transparent) {
	uint8 *p = dst + dstOffset;
	for (int y = 0; y < h; ++y) {
		if (transparent) {
			for (int x = 0; x < w; ++x) {
				if (src[x] != 0) {
					p[x] = src[x];
				}
			}
		} else {
			memcpy(p, src, w);
		}
		p += dstPitch;
		src += srcPitch;
	}
}

} // End of namespace Igor
