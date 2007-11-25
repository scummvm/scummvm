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

#include "igor/igor.h"

namespace Igor {

static int VAR_CURRENT_TALKING_ACTOR;

void IgorEngine::PART_19_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		ADD_DIALOGUE_TEXT(201, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 102:
		ADD_DIALOGUE_TEXT(202, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startCutsceneDialogue(99, 54, 43, 63, 0);
		waitForEndOfCutsceneDialogue(99, 54, 43, 63, 0);
		break;
	case 103:
		ADD_DIALOGUE_TEXT(203, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startCutsceneDialogue(132, 55, 63, 42, 0);
		waitForEndOfCutsceneDialogue(132, 55, 63, 42, 0);
		break;
	case 104:
		ADD_DIALOGUE_TEXT(204, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startCutsceneDialogue(167, 56, 0, 42, 42);
		waitForEndOfCutsceneDialogue(167, 56, 0, 42, 42);
		break;
	case 105:
		ADD_DIALOGUE_TEXT(205, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106:
		_currentPart = 232;
		break;
	case 107:
		PART_19_ACTION_107();
		break;
	case 108:
		ADD_DIALOGUE_TEXT(208, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 109:
		PART_19_ACTION_109();
		break;
	default:
		error("PART_19_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_19_ACTION_107() {
	if (_objectsState[58] == 1) {
		ADD_DIALOGUE_TEXT(211, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		return;
	}
	ADD_DIALOGUE_TEXT(206, 1);
	ADD_DIALOGUE_TEXT(207, 1);
	SET_DIALOGUE_TEXT(1, 2);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	for (int i = 1; i <= 2; ++i) {
		const int offset = 27110;
		for (int j = 0; j <= 48; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + i * 1372 + j * 28 - 506, 28);
			waitForTimer(120);
		}
	}
	addObjectToInventory(12, 47);
	_objectsState[58] = 1;
	PART_19_HELPER_1(255);
}

void IgorEngine::PART_19_ACTION_109() {
	if (_objectsState[2] == 1) {
		ADD_DIALOGUE_TEXT(210, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		return;
	}
	for (int i = 1; i <= 4; ++i) {
		const uint8 *src = _animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0x354 + i * 2) - 1;
		decodeAnimFrame(src, _screenVGA, true);
		waitForTimer(45);
	}
	decodeAnimFrame(_animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0x35E) - 1, _screenVGA, true);
	playSound(45, 1);
	PART_19_HELPER_5();
	PART_19_HELPER_1(255);
	stopSound();
	decodeAnimFrame(_animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0x360) - 1, _screenVGA, true);
	ADD_DIALOGUE_TEXT(209, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
}

void IgorEngine::PART_19_UPDATE_DIALOGUE_WOMEN(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_19_HELPER_7((VAR_CURRENT_TALKING_ACTOR == 0) ? 35 : 40);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_19_HELPER_7((VAR_CURRENT_TALKING_ACTOR == 0) ? 35 + getRandomNumber(5) : 40 + getRandomNumber(5));
		break;
	}
}

void IgorEngine::PART_19_UPDATE_BACKGROUND_HELPER_9() {
	if (compareGameTick(3, 64) && _gameState.counter[1] <= 34) {
		const uint8 *src = _animFramesBuffer + 0xE1A + READ_LE_UINT16(_animFramesBuffer + 0x404F + _gameState.counter[1] * 2) - 1;
		decodeAnimFrame(src, _screenVGA, true);
		++_gameState.counter[1];
	}
}

void IgorEngine::PART_19_HELPER_1(int num) {
	if (num == 2 || num == 255) {
		if (_objectsState[59] == 0) {
			decodeAnimFrame(_animFramesBuffer + 0xE1A + READ_LE_UINT16(_animFramesBuffer + 0x4051) - 1, _screenLayer1, true);
		}
	}
}

void IgorEngine::PART_19_HELPER_2() {
	int talkSpeed = _gameState.talkSpeed;
	_gameState.talkSpeed = 5;
	memcpy(_screenLayer2, _screenVGA, 46080);
	memcpy(_screenVGA, _screenLayer1, 46080);
	const uint8 *src = _animFramesBuffer + 0xE1A + READ_LE_UINT16(_animFramesBuffer + 0x4051) - 1;
	decodeAnimFrame(src, _screenVGA, true);
	memcpy(_screenLayer1, _screenVGA, 46080);
	fadeInPalette(624);
	ADD_DIALOGUE_TEXT(212, 3);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(243, 92, 63, 23, 33);
	VAR_CURRENT_TALKING_ACTOR = 0;
	_updateDialogue = &IgorEngine::PART_19_UPDATE_DIALOGUE_WOMEN;
	waitForEndOfCutsceneDialogue(243, 92, 63, 23, 33);
	_updateDialogue = 0;
	for (int i = 2; i <= 9; ++i) {
		src = _animFramesBuffer + 0xE1A + READ_LE_UINT16(_animFramesBuffer + 0x404F + i * 2) - 1;
		waitForTimer(60);
	}
	ADD_DIALOGUE_TEXT(215, 1);
	ADD_DIALOGUE_TEXT(216, 2);
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(227, 91, 0, 63, 63);
	_gameState.counter[1] = 10;
	VAR_CURRENT_TALKING_ACTOR = 1;
	_updateRoomBackground = &IgorEngine::PART_19_UPDATE_BACKGROUND_HELPER_9;
	waitForEndOfCutsceneDialogue(227, 91, 0, 63, 63);
	_updateRoomBackground = 0;
	while (_gameState.counter[1] != 35) {
		PART_19_UPDATE_BACKGROUND_HELPER_9();
	}
	for (int i = 47; i <= 48; ++i) {
		src = _animFramesBuffer + 0xE1A + READ_LE_UINT16(_animFramesBuffer + 0x404F + i * 2) - 1;
		decodeAnimFrame(src, _screenVGA, true);
		waitForTimer(30);
	}
	for (int i = 1; i <= 3; ++i) {
		for (int j = 49; j <= 50; ++j) {
			src = _animFramesBuffer + 0xE1A + READ_LE_UINT16(_animFramesBuffer + 0x404F + j * 2) - 1;
			decodeAnimFrame(src, _screenVGA, true);
			waitForTimer(30);
			if (j == 50) {
				playSound(46, 1);
			}
		}
	}
	src = _animFramesBuffer + 0xE1A + READ_LE_UINT16(_animFramesBuffer + 0x40B5) - 1;
	decodeAnimFrame(src, _screenVGA, true);
	waitForTimer(30);
	ADD_DIALOGUE_TEXT(218, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(243, 92, 63, 23, 33);
	VAR_CURRENT_TALKING_ACTOR = 0;
	_updateDialogue = &IgorEngine::PART_19_UPDATE_DIALOGUE_WOMEN;
	waitForEndOfCutsceneDialogue(243, 92, 63, 23, 33);
	_updateDialogue = 0;
	ADD_DIALOGUE_TEXT(219, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(227, 91, 0, 63, 63);
	VAR_CURRENT_TALKING_ACTOR = 1;
	_updateDialogue = &IgorEngine::PART_19_UPDATE_DIALOGUE_WOMEN;
	waitForEndOfCutsceneDialogue(227, 91, 0, 63, 63);
	_updateDialogue = 0;
	ADD_DIALOGUE_TEXT(221, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(243, 92, 63, 23, 33);
	VAR_CURRENT_TALKING_ACTOR = 0;
	_updateDialogue = &IgorEngine::PART_19_UPDATE_DIALOGUE_WOMEN;
	waitForEndOfCutsceneDialogue(243, 92, 63, 23, 33);
	_updateDialogue = 0;
	ADD_DIALOGUE_TEXT(223, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(227, 91, 0, 63, 63);
	VAR_CURRENT_TALKING_ACTOR = 1;
	_updateDialogue = &IgorEngine::PART_19_UPDATE_DIALOGUE_WOMEN;
	waitForEndOfCutsceneDialogue(227, 91, 0, 63, 63);
	_updateDialogue = 0;
	_objectsState[59] = 1;
	fadeOutPalette(624);
	_gameState.talkSpeed = talkSpeed;
}

void IgorEngine::PART_19_HELPER_3() {
	_walkData[0].setPos(319, 142, 1, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPath(319, 142, 295, 142);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_19_HELPER_4() {
	decodeAnimFrame(_animFramesBuffer + 0xE1A + READ_LE_UINT16(_animFramesBuffer + 0x40AB) - 1, _screenVGA, true);
	waitForTimer(60);
	ADD_DIALOGUE_TEXT(225, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	--_walkDataLastIndex;
	buildWalkPath(295, 142, 319, 142);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 232;
}

void IgorEngine::PART_19_HELPER_5() {
	fadeOutPalette(624);
	uint8 *tmp = (uint8 *)malloc(64000 + 768);
	if (tmp) {
		memcpy(tmp, _screenVGA, 64000);
		memcpy(tmp + 64000, _paletteBuffer, 768);
	}
	loadData(IMG_RomanNumbersPaper, _screenVGA);
	loadData(PAL_RomanNumbersPaper, _paletteBuffer);
	fadeInPalette(624);
	PART_UPDATE_FIGURES_ON_PAPER(60);
	_objectsState[2] = 1;
	waitForTimer(255);
	if (tmp) {
		memcpy(_screenVGA, tmp, 64000);
		memcpy(_paletteBuffer, tmp + 64000, 768);
		free(tmp);
	}
}

void IgorEngine::PART_19_HELPER_7(int frame) {
	const uint8 *src = _animFramesBuffer + 0xE1A + READ_LE_UINT16(_animFramesBuffer + 0x404F + frame * 2) - 1;
	decodeAnimFrame(src, _screenVGA, true);
}

void IgorEngine::PART_19() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_WomenToilets, IMG_WomenToilets, BOX_WomenToilets, MSK_WomenToilets, TXT_WomenToilets);
	static const int anm[] = { FRM_WomenToilets1, FRM_WomenToilets2, FRM_WomenToilets3, FRM_WomenToilets4, FRM_WomenToilets5, 0 };
	loadAnimData(anm);
	loadActionData(DAT_WomenToilets);
	_roomDataOffsets = PART_19_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	if (_currentPart == 191) {
		PART_19_HELPER_2();
	} else {
		SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_19_EXEC_ACTION);
		PART_19_HELPER_1(255);
	}
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeInPalette(768);
	PART_19_HELPER_3();
	if (_objectsState[59] == 0) {
		PART_19_HELPER_4();
	} else {
		enterPartLoop();
		while (_currentPart == 190) {
			runPartLoop();
		}
		leavePartLoop();
	}
	fadeOutPalette(624);
}

} // namespace Igor
