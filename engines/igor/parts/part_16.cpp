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

void IgorEngine::PART_16_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_16_EXEC_ACTION %d", action);
	switch (action) {
	case 101:
		PART_16_ACTION_101();
		break;
	case 102:
		ADD_DIALOGUE_TEXT(208, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 103:
		_currentPart = 261;
		break;
	default:
		error("PART_16_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_16_ACTION_101() {
	if (_objectsState[52] == 1) {
		ADD_DIALOGUE_TEXT(207, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		return;
	}
	ADD_DIALOGUE_TEXT(201, 1);
	ADD_DIALOGUE_TEXT(202, 2);
	SET_DIALOGUE_TEXT(1, 2);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	for (int i = 1; i <= 2; ++i) {
		int offset = 20332;
		for (int j = 0; j <= 48; ++j) {
			const uint8 *src = _animFramesBuffer + j * 23 + (i - 1) * 1127;
			memcpy(_screenVGA + j * 320 + offset, src, 23);
		}
		waitForTimer(120);
	}
	addObjectToInventory(20, 55);
	_objectsState[52] = 1;
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
	ADD_DIALOGUE_TEXT(204, 3);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
}

void IgorEngine::PART_16_UPDATE_DIALOGUE_MARGARET_HARRISON(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_16_HELPER_6((VAR_CURRENT_TALKING_ACTOR == 0) ? 34 : 48);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_16_HELPER_6((VAR_CURRENT_TALKING_ACTOR == 0) ? getRandomNumber(5) + 30 : getRandomNumber(5) + 44);
		break;
	}
}

void IgorEngine::PART_16_UPDATE_DIALOGUE_MARGARET(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_16_HELPER_6(49);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_16_HELPER_6(getRandomNumber(4) + 49);
		break;
	}
}

void IgorEngine::PART_16_HELPER_1(int num) {
}

void IgorEngine::PART_16_HELPER_2() {
	WalkData *wd = &_walkData[0];
	wd->setPos(0, 135, 2, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 30;
	wd->scaleWidth = 50;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 50;
	_walkDataLastIndex = 0;
	buildWalkPath(0, 135, 90, 135);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_16_HELPER_3() {
	memset(_screenVGA + 46080, 0, 17920);
	ADD_DIALOGUE_TEXT(210, 1);
	ADD_DIALOGUE_TEXT(211, 1);
	ADD_DIALOGUE_TEXT(212, 1);
	ADD_DIALOGUE_TEXT(213, 2);
	SET_DIALOGUE_TEXT(1, 4);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	--_walkDataLastIndex;
	buildWalkPath(90, 135, 200, 143);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_walkDataCurrentIndex = 0;
	for (int i = 15; i >= 0; --i) {
		if (i == 15) {
			_walkCurrentFrame = 0;
		}
		_walkData[0].setPos(200, 143, 3, _walkCurrentFrame);
		WalkData::setNextFrame(3, _walkCurrentFrame);
		_walkData[0].clipSkipX = 1;
		_walkData[0].clipWidth = 30;
		_walkData[0].scaleWidth = i * 3 + 5;
		_walkData[0].xPosChanged = 1;
		_walkData[0].dxPos = 0;
		_walkData[0].yPosChanged = 1;
		_walkData[0].dyPos = 3;
		_walkData[0].scaleHeight = 50;
		moveIgor(_walkData[0].posNum, _walkData[0].frameNum);
		waitForTimer(15);
	}
	fadeOutPalette(768);
	_currentPart = 331;
	PART_33();
	memset(_currentPalette, 0, 768);
	setPaletteRange(208, 255);
	do {
		PART_MARGARET_ROOM_CUTSCENE();
	} while (_objectsState[110] != 9);
	setupDefaultPalette();
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	loadResourceData__ROOM_Laboratory();
	loadResourceData__ANIM_Laboratory();
	memcpy(_screenVGA, _screenLayer1, 46080);
	PART_16_HELPER_1(255);
	fadeInPalette(768);
	waitForTimer(255);
	for (int i = 1; i <= 29; ++i) {
		decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x6F36 + i * 2) - 1, _screenVGA, true);
		waitForTimer(25);
	}
	ADD_DIALOGUE_TEXT(215, 1);
	ADD_DIALOGUE_TEXT(216, 1);
	ADD_DIALOGUE_TEXT(217, 1);
	SET_DIALOGUE_TEXT(1, 3);
	startCutsceneDialogue(175, 78, 0, 58, 40);
	VAR_CURRENT_TALKING_ACTOR = 0;
	_updateDialogue = &IgorEngine::PART_16_UPDATE_DIALOGUE_MARGARET_HARRISON;
	waitForEndOfCutsceneDialogue(175, 78, 0, 58, 40);
	_updateDialogue = 0;
	for (int i = 35; i <= 41; ++i) {
		decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x6F36 + i * 2) - 1, _screenVGA, true);
		waitForTimer(35);
	}
	ADD_DIALOGUE_TEXT(218, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(88, 69, 63, 32, 0);
	_updateDialogue = &IgorEngine::PART_16_UPDATE_DIALOGUE_MARGARET;
	waitForEndOfCutsceneDialogue(88, 69, 63, 32, 0);
	_updateDialogue = 0;
	for (int i = 42; i <= 43; ++i) {
		decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x6F36 + i * 2) - 1, _screenVGA, true);
		waitForTimer(35);
	}
	ADD_DIALOGUE_TEXT(220, 1);
	ADD_DIALOGUE_TEXT(221, 1);
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(175, 78, 0, 58, 40);
	VAR_CURRENT_TALKING_ACTOR = 1;
	_updateDialogue = &IgorEngine::PART_16_UPDATE_DIALOGUE_MARGARET_HARRISON;
	waitForEndOfCutsceneDialogue(175, 78, 0, 58, 40);
	_updateDialogue = 0;
	ADD_DIALOGUE_TEXT(222, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(88, 69, 63, 32, 0);
	_updateDialogue = &IgorEngine::PART_16_UPDATE_DIALOGUE_MARGARET;
	waitForEndOfCutsceneDialogue(88, 69, 63, 32, 0);
	_updateDialogue = 0;
	playSound(26, 1);
	for (int i = 53; i <= 61; ++i) {
		decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x6F36 + i * 2) - 1, _screenVGA, true);
		waitForTimer(35);
	}
	playSound(25, 1);
	for (int i = 1; i <= 20; ++i) {
		int frame = getRandomNumber(4) + 61;
		decodeAnimFrame(_animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x6F36 + frame * 2) - 1, _screenVGA, true);
		waitForTimer(15);
	}
	playSound(27, 1);
	PART_16_HELPER_5();
	for (int i = 1; i <= 3; ++i) {
		waitForTimer(255);
	}
	PART_16_HELPER_1(255);
	addObjectToInventory(19, 54);
	_objectsState[53] = 1;
	_objectsState[65] = 3;
	playMusic(3);
	_currentPart = 261;
}

void IgorEngine::PART_16_HELPER_5() {
	memset(&_currentPalette[3], 63, 621);
	setPaletteRange(1, 207);
	loadData(IMG_PhotoHarrisonMargaret, _screenVGA);
	loadData(PAL_PhotoHarrisonMargaret, _paletteBuffer);
	for (int m = 1; m <= 63; ++m) {
		for (int i = 3; i <= 207 * 3; ++i) {
			if (_paletteBuffer[i] <= m && _paletteBuffer[i] >= _currentPalette[i]) {
				--_currentPalette[i];
			}
		}
		setPaletteRange(1, 207);
		waitForTimer();
	}
}

void IgorEngine::PART_16_HELPER_6(int frame) {
	const uint8 *src = _animFramesBuffer + 0x8CE + READ_LE_UINT16(_animFramesBuffer + 0x6F36 + frame * 2) - 1;
	decodeAnimFrame(src, _screenVGA, true);
}

void IgorEngine::PART_16() {
	_gameState.enableLight = 1;
	loadResourceData__ROOM_Laboratory();
	loadResourceData__ANIM_Laboratory();
	loadActionData(DAT_Laboratory);
	_roomDataOffsets = PART_16_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 248, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_16_EXEC_ACTION);
	PART_16_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeInPalette(768);
	PART_16_HELPER_2();
	if (_objectsState[65] == 2 && _inventoryInfo[71] != 0 && _objectsState[76] == 1) {
		PART_16_HELPER_3();
	}
	enterPartLoop();
	while (_currentPart == 160) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

void IgorEngine::loadResourceData__ROOM_Laboratory() {
	loadRoomData(PAL_Laboratory, IMG_Laboratory, BOX_Laboratory, MSK_Laboratory, TXT_Laboratory);
}

void IgorEngine::loadResourceData__ANIM_Laboratory() {
	static const int anm[] = { FRM_Laboratory1, FRM_Laboratory2, FRM_Laboratory3, 0 };
	loadAnimData(anm);
}

} // namespace Igor
