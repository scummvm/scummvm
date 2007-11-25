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

static const uint8 PART_75_ANIM_DATA_1[18] = {
	0, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 10, 11, 12, 13, 14, 15, 16
};

static const uint8 PART_75_ANIM_DATA_2[37] = {
	 0, 31, 32, 33, 34, 31, 32, 33, 34, 31, 32, 33, 34, 31, 32, 33, 34, 31, 32,
	33, 34, 31, 32, 33, 34, 31, 32, 33, 34, 31, 32, 33, 34, 31, 32, 33, 34
};

void IgorEngine::PART_75_UPDATE_DIALOGUE_PHILIP(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_75_HELPER_1(2);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_75_HELPER_1(getRandomNumber(5) + 2);
		break;
	}
}

void IgorEngine::PART_75_HELPER_1(int frame) {
	const uint8 *src = _animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0x5E6D + frame * 2) - 1;
	decodeAnimFrame(src, _screenVGA);
}

void IgorEngine::PART_75() {
	PART_MEANWHILE();
	_gameState.enableLight = 1;
	loadRoomData(PAL_PhilipRoom, IMG_PhilipRoom, BOX_PhilipRoom, MSK_PhilipRoom, TXT_PhilipRoom);
	static const int anm[] = { ANM_PhilipVodka, AOF_PhilipVodka, 0 };
	loadAnimData(anm);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_updateDialogue = &IgorEngine::PART_75_UPDATE_DIALOGUE_PHILIP;
	PART_75_HELPER_1(1);
	PART_75_HELPER_1(7);
	memset(_currentPalette, 0, 768);
	fadeInPalette(768);
	_gameState.igorMoving = false;
	ADD_DIALOGUE_TEXT(201, 1, 1156);
	ADD_DIALOGUE_TEXT(202, 2, 1157);
	ADD_DIALOGUE_TEXT(204, 1, 1158);
	SET_DIALOGUE_TEXT(1, 3);
	startCutsceneDialogue(187, 82, 63, 63, 0);
	waitForEndOfCutsceneDialogue(187, 82, 63, 63, 0);
	for (int i = 1; i <= 17; ++i) {
		PART_75_HELPER_1(PART_75_ANIM_DATA_1[i]);
		waitForTimer(30);
	}
	ADD_DIALOGUE_TEXT(205, 1, 1159);
	ADD_DIALOGUE_TEXT(206, 1, 1160);
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(187, 82, 63, 63, 0);
	waitForEndOfCutsceneDialogue(187, 82, 63, 63, 0);
	for (int i = 17; i <= 30; ++i) {
		PART_75_HELPER_1(i);
		waitForTimer(15);
	}
	ADD_DIALOGUE_TEXT(208, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(187, 82, 63, 63, 0);
	playSound(62, 1);
	for (int i = 1; i <= 36; ++i) {
		PART_75_HELPER_1(PART_75_ANIM_DATA_2[i]);
		waitForTimer(15);
	}
	memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer + 23040, _dialogueDirtyRectSize);
	ADD_DIALOGUE_TEXT(209, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(187, 82, 63, 63, 0);
	PART_75_HELPER_1(35);
	waitForTimer(255);
	PART_75_HELPER_1(36);
	waitForTimer(140);
	memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer + 23040, _dialogueDirtyRectSize);
	ADD_DIALOGUE_TEXT(210, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(187, 82, 63, 63, 0);
	waitForEndOfCutsceneDialogue(187, 82, 63, 63, 0);
	for (int i = 1; i <= 40; ++i) {
		PART_75_HELPER_1(getRandomNumber(4) + 37);
		waitForTimer(30);
	}
	memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer + 23040, _dialogueDirtyRectSize);
	_gameState.dialogueTextRunning = false;
	_updateDialogue = 0;
	_currentPart = 122;
	fadeOutPalette(768);
}

} // namespace Igor
