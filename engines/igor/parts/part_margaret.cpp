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


static int VAR_MARGARET_ROOM_ANIM_COUNTER;

void IgorEngine::PART_MARGARET_ROOM_CUTSCENE_HELPER_1() {
	const uint8 *src = _animFramesBuffer + READ_LE_UINT16(_animFramesBuffer + 0x2A63 + VAR_MARGARET_ROOM_ANIM_COUNTER * 2) - 1;
	decodeAnimFrame(src, _screenVGA);
}

void IgorEngine::PART_MARGARET_ROOM_CUTSCENE_HELPER_2(int frame) {
	const int offset = ((VAR_MARGARET_ROOM_ANIM_COUNTER - 1) * 5 + frame) * 2;
	const uint8 *src = _animFramesBuffer + 0x2A75 + READ_LE_UINT16(_animFramesBuffer + offset + 0xD887) - 1;
	decodeAnimFrame(src, _screenVGA);
}

void IgorEngine::PART_MARGARET_ROOM_CUTSCENE_UPDATE_DIALOGUE_MARGARET(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_MARGARET_ROOM_CUTSCENE_HELPER_2(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_MARGARET_ROOM_CUTSCENE_HELPER_2(getRandomNumber(5) + 1);
		break;
	}
}

void IgorEngine::PART_MARGARET_ROOM_CUTSCENE() {
	_gameState.enableLight = 1;
	VAR_MARGARET_ROOM_ANIM_COUNTER = _objectsState[110] + 1;
	if (VAR_MARGARET_ROOM_ANIM_COUNTER == 9) {
		VAR_MARGARET_ROOM_ANIM_COUNTER = 1;
	}
	loadRoomData(PAL_MargaretRoom, IMG_MargaretRoom, BOX_MargaretRoom, MSK_MargaretRoom, TXT_MargaretRoom);
	static const int anm[] = { FRM_MargaretRoom1, FRM_MargaretRoom2, FRM_MargaretRoom3, FRM_MargaretRoom4, 0 };
	loadAnimData(anm);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_updateDialogue = &IgorEngine::PART_MARGARET_ROOM_CUTSCENE_UPDATE_DIALOGUE_MARGARET;
	PART_MARGARET_ROOM_CUTSCENE_HELPER_1();
	memset(_currentPalette, 0, 768);
	fadeIn(768);
	_gameState.igorMoving = false;
	switch (VAR_MARGARET_ROOM_ANIM_COUNTER) {
	case 1:
		ADD_DIALOGUE_TEXT(223, 1, 1118);
		ADD_DIALOGUE_TEXT(224, 1, 1119);
		ADD_DIALOGUE_TEXT(225, 1, 1120);
		break;
	case 2:
		ADD_DIALOGUE_TEXT(201, 1, 1097);
		ADD_DIALOGUE_TEXT(202, 1, 1098);
		ADD_DIALOGUE_TEXT(203, 1, 1099);
		break;
	case 3:
		ADD_DIALOGUE_TEXT(204, 1, 1100);
		ADD_DIALOGUE_TEXT(205, 1, 1101);
		ADD_DIALOGUE_TEXT(206, 1, 1102);
		break;
	case 4:
		ADD_DIALOGUE_TEXT(207, 1, 1103);
		ADD_DIALOGUE_TEXT(208, 1, 1104);
		ADD_DIALOGUE_TEXT(209, 1, 1105);
		break;
	case 5:
		ADD_DIALOGUE_TEXT(210, 1, 1106);
		ADD_DIALOGUE_TEXT(211, 1, 1107);
		ADD_DIALOGUE_TEXT(212, 1, 1108);
		break;
	case 6:
		ADD_DIALOGUE_TEXT(213, 2, 1109);
		ADD_DIALOGUE_TEXT(215, 1, 1110);
		ADD_DIALOGUE_TEXT(216, 1, 1111);
		break;
	case 7:
		ADD_DIALOGUE_TEXT(217, 1, 1112);
		ADD_DIALOGUE_TEXT(218, 1, 1113);
		ADD_DIALOGUE_TEXT(219, 1, 1114);
		break;
	case 8:
		ADD_DIALOGUE_TEXT(220, 1, 1115);
		ADD_DIALOGUE_TEXT(221, 1, 1116);
		ADD_DIALOGUE_TEXT(222, 1, 1117);
		break;
	}
	SET_DIALOGUE_TEXT(1, 3);
	startCutsceneDialogue(200, 73, 63, 32, 0);
	waitForEndOfCutsceneDialogue(200, 73, 63, 32, 0);
	waitForTimer(255);
	_updateDialogue = 0;
	++_objectsState[110];
	memcpy(_paletteBuffer, _currentPalette, 768);
	fadeOut(768);
	memset(_screenVGA, 0, 46080);
	if (_objectsState[110] < 9 && _currentPart != 331) {
		setupDefaultPalette();
		SET_PAL_240_48_1();
		SET_PAL_208_96_1();
		drawVerbsPanel();
		drawInventory(_inventoryInfo[72], 0);
	}
}

} // End of namespace Igor
