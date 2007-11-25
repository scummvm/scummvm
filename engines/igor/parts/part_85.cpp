/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"

#include "igor/igor.h"

namespace Igor {

static bool VAR_WATER_SOUND_PLAYING;
static int VAR_CURRENT_TALKING_ACTOR;

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
	if (_gameState.talkMode == kTalkModeTextOnly || !_gameState.dialogueTextRunning) {
		playSound(17, 1);
	}
}

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

void IgorEngine::PART_85_HELPER_6(int frame) {
	const int offset = (_walkCurrentPos == 4) ? 24564 : 22643;
	copyArea(_screenVGA, offset, 320, _screenLayer2 + offset, 320, 14, 8);
	copyArea(_screenVGA, offset, 320, _igorHeadFrames + (_walkCurrentPos - 1) * 924 + frame * 154, 14, 14, 8, true);
}

void IgorEngine::PART_85() {
	playMusic(2);
	loadRoomData(PAL_SpringRock, IMG_SpringRock, BOX_SpringRock, MSK_SpringRock, TXT_SpringRock);
	memcpy(_screenLayer2, _screenLayer1, 46080);
	loadRoomData(PAL_SpringBridgeIntro, IMG_SpringBridgeIntro, 0, 0, TXT_SpringBridgeIntro);
	static const int anm[] = { ANM_PhilipLauraIntro, AOF_PhilipLauraIntro, ANM_LauraIntro, AOF_LauraIntro, 0 };
	loadAnimData(anm);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_updateDialogue = &IgorEngine::PART_85_UPDATE_DIALOGUE_PHILIP_LAURA;
	_updateRoomBackground = &IgorEngine::PART_85_UPDATE_ROOM_BACKGROUND;
	VAR_WATER_SOUND_PLAYING = true;
	playSound(17, 1);
	decodeAnimFrame(getAnimFrame(0, 0x7480, 1), _screenVGA, true);
	fadeInPalette(768);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	PART_85_HELPER_1(0, 0x7481, 2, 7, 32);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	ADD_DIALOGUE_TEXT(201, 3, 545);
	ADD_DIALOGUE_TEXT(204, 1, 546);
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(89, 56, 63, 63, 0);
	VAR_CURRENT_TALKING_ACTOR = 0;
	waitForEndOfCutsceneDialogue(89, 56, 63, 63, 0);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	ADD_DIALOGUE_TEXT(205, 3, 547);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(113, 59, 63, 0, 38);
	VAR_CURRENT_TALKING_ACTOR = 1;
	waitForEndOfCutsceneDialogue(113, 59, 63, 0, 38);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	ADD_DIALOGUE_TEXT(208, 1, 548);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(89, 56, 63, 63, 0);
	VAR_CURRENT_TALKING_ACTOR = 0;
	waitForEndOfCutsceneDialogue(89, 56, 63, 63, 0);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	ADD_DIALOGUE_TEXT(209, 2, 549);
	ADD_DIALOGUE_TEXT(211, 1, 550);
	ADD_DIALOGUE_TEXT(212, 1, 551);
	ADD_DIALOGUE_TEXT(213, 1, 552);
	ADD_DIALOGUE_TEXT(214, 1, 553);
	SET_DIALOGUE_TEXT(1, 5);
	startCutsceneDialogue(113, 59, 63, 0, 38);
	VAR_CURRENT_TALKING_ACTOR = 1;
	waitForEndOfCutsceneDialogue(113, 59, 63, 0, 38);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	PART_85_HELPER_1(0, 0x7481, 2, 7, 32);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	ADD_DIALOGUE_TEXT(215, 1, 554);
	ADD_DIALOGUE_TEXT(216, 2, 555);
	ADD_DIALOGUE_TEXT(218, 1, 556);
	SET_DIALOGUE_TEXT(1, 3);
	startCutsceneDialogue(89, 56, 63, 63, 0);
	VAR_CURRENT_TALKING_ACTOR = 0;
	waitForEndOfCutsceneDialogue(89, 56, 63, 63, 0);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	ADD_DIALOGUE_TEXT(219, 2, 557);
	ADD_DIALOGUE_TEXT(221, 2, 558);
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(113, 59, 63, 0, 38);
	VAR_CURRENT_TALKING_ACTOR = 1;
	waitForEndOfCutsceneDialogue(113, 59, 63, 0, 38);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	decodeAnimFrame(getAnimFrame(0, 0x7480, 1), _screenVGA, true);
	PART_85_HELPER_1(0, 0x7481, 2, 7, 32);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
	PART_85_HELPER_1(0, 0x7481, 18, 37, 20);
	if (_inputVars[kInputEscape]) goto PART_85_EXIT;
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

} // namespace Igor
