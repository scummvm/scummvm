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

void IgorEngine::PART_18_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		ADD_DIALOGUE_TEXT(201, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 102:
		ADD_DIALOGUE_TEXT(202, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 103:
		ADD_DIALOGUE_TEXT(205, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startCutsceneDialogue(99, 54, 43, 63, 0);
		waitForEndOfCutsceneDialogue(99, 54, 43, 63, 0);
		break;
	case 104:
		ADD_DIALOGUE_TEXT(206, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startCutsceneDialogue(132, 55, 63, 42, 0);
		waitForEndOfCutsceneDialogue(132, 55, 63, 42, 0);
		break;
	case 105:
		ADD_DIALOGUE_TEXT(207, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startCutsceneDialogue(167, 56, 0, 42, 42);
		waitForEndOfCutsceneDialogue(167, 56, 0, 42, 42);
		break;
	case 106:
		ADD_DIALOGUE_TEXT(208, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107:
		ADD_DIALOGUE_TEXT(209, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 108:
		ADD_DIALOGUE_TEXT(210, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 109:
		PART_18_ACTION_109();
		break;
	case 110:
		_currentPart = 252;
		break;
	case 111:
		PART_18_ACTION_111();
		break;
	default:
		error("PART_18_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_18_ACTION_109() {
	const int offset = 29480;
	for (int i = 1; i <= 2; ++i) {
		for (int j = 1; j <= 48; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + (i - 1) * 1519 + j * 31, 31);
		}
		waitForTimer(120);
	}
	int part = _currentPart;
	_currentPart = 191;
	fadeOutPalette(768);
	PART_19();
	memcpy(_screenVGA, _screenLayer2, 46080);
	loadResourceData__ROOM_MenToilets();
	loadResourceData__ANIM_MenToilets();
	loadActionData(DAT_MenToilets);
	_roomDataOffsets = PART_18_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_18_EXEC_ACTION);
	PART_18_HELPER_1(255);
	memset(_screenVGA + 46080, 0, 17920);
	drawVerbsPanel();
	removeObjectFromInventory(53);
	fadeInPalette(768);
	_currentPart = part;
	ADD_DIALOGUE_TEXT(215, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
}

void IgorEngine::PART_18_ACTION_111() {
	if (_objectsState[0] == 0) {
		ADD_DIALOGUE_TEXT(211, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		return;
	}
	if (_objectsState[0] == 2) {
		ADD_DIALOGUE_TEXT(212, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		return;
	}
	const int offset = 26798;
	for (int i = 0; i <= 28; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _animFramesBuffer + i * 28 + 0xF0A, 28);
	}
	playSound(47, 1);
	for (int i = 1; i <= 2; ++i) {
		waitForTimer(200);
	}
	for (int i = 0; i <= 28; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _animFramesBuffer + i * 28 + 0xBDE, 28);
	}
	ADD_DIALOGUE_TEXT(213, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	_objectsState[0] = 2;
	UPDATE_OBJECT_STATE(1);
}

void IgorEngine::PART_18_HELPER_1(int num) {
}

void IgorEngine::PART_18_HELPER_2() {
	_walkData[0].setPos(319, 142, 4, 0);
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 15;
	_walkData[0].scaleWidth = 50;
	_walkData[0].xPosChanged = 1;
	_walkData[0].dxPos = 0;
	_walkData[0].yPosChanged = 1;
	_walkData[0].dyPos = 0;
	_walkData[0].scaleHeight = 50;
	_walkDataLastIndex = 0;
	buildWalkPath(319, 142, 295, 142);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_18() {
	_gameState.enableLight = 1;
	loadResourceData__ROOM_MenToilets();
	loadResourceData__ANIM_MenToilets();
	loadActionData(DAT_MenToilets);
	_roomDataOffsets = PART_18_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_18_EXEC_ACTION);
	PART_18_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeInPalette(768);
	PART_18_HELPER_2();
	enterPartLoop();
	while (_currentPart == 180) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

void IgorEngine::loadResourceData__ROOM_MenToilets() {
	loadRoomData(PAL_MenToilets, IMG_MenToilets, BOX_MenToilets, MSK_MenToilets, TXT_MenToilets);
}

void IgorEngine::loadResourceData__ANIM_MenToilets() {
	static const int anm[] = { FRM_MenToilets1, FRM_MenToilets2, 0 };
	loadAnimData(anm);
}

} // namespace Igor
