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

void IgorEngine::ADD_DIALOGUE_TEXT(int num, int count, int sound) {
	assert(_dialogueTextsCount < MAX_DIALOGUE_TEXTS);
	DialogueText *dt = &_dialogueTextsTable[_dialogueTextsCount];
	dt->num = num;
	dt->count = count;
	dt->sound = sound;
	++_dialogueTextsCount;
}

void IgorEngine::SET_DIALOGUE_TEXT(int start, int count) {
	_dialogueTextsStart = start - 1;
	_dialogueTextsCount = count;
}

void IgorEngine::SET_EXEC_ACTION_FUNC(int i, ExecuteActionProc p) {
	switch (i) {
	case 0:
		_executeMainAction = p;
		break;
	case 1:
		_executeRoomAction = p;
		break;
	}
}

void IgorEngine::EXEC_MAIN_ACTION(int action) {
	switch (action) {
	case 0:
	case 1:
		break;
	case 2: {
			int num, rnd = getRandomNumber(100);
			if (rnd < 34) {
				num = 11;
			} else if (rnd < 69) {
				num = 12;
			} else if (rnd < 94) {
				num = 13;
			} else {
				num = 14;
			}
			ADD_DIALOGUE_TEXT(num, 1, num);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 3:
		ADD_DIALOGUE_TEXT(15, 1, 15);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 4:
		ADD_DIALOGUE_TEXT(10, 1, 10);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 5:
		ADD_DIALOGUE_TEXT(9, 1, 9);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 6:
		ADD_DIALOGUE_TEXT(8, 1, 8);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 7:
		ADD_DIALOGUE_TEXT(6, 1, 6);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 8:
		ADD_DIALOGUE_TEXT(7, 1, 7);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 9: {
			int num = 16 + getRandomNumber(2);
			ADD_DIALOGUE_TEXT(num, 1, num);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 10:
		ADD_DIALOGUE_TEXT(18, 1, 18);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 11:
		ADD_DIALOGUE_TEXT(19, 1, 19);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 12: {
			int num = 20 + getRandomNumber(2);
			ADD_DIALOGUE_TEXT(num, 1, num);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 13:
		ADD_DIALOGUE_TEXT(22, 1, 22);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 14:
		ADD_DIALOGUE_TEXT(23, 1, 23);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 15:
		ADD_DIALOGUE_TEXT(24, 1, 24);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 16:
		ADD_DIALOGUE_TEXT(25, 1, 25);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 17: {
			int num = 26 + getRandomNumber(2);
			ADD_DIALOGUE_TEXT(num, 1, num);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 18:
		ADD_DIALOGUE_TEXT(28, 1, 28);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 19:
		ADD_DIALOGUE_TEXT(4, 1, 4);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 20:
		ADD_DIALOGUE_TEXT(5, 1, 5);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 21: {
			int num = 1 + getRandomNumber(3);
			ADD_DIALOGUE_TEXT(num, 1, num);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 22:
		ADD_DIALOGUE_TEXT(51, 1, 32);
		ADD_DIALOGUE_TEXT(52, 1, 33);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 23:
		ADD_DIALOGUE_TEXT(53, 1, 34);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 24:
		ADD_DIALOGUE_TEXT(54, 1, 35);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 25:
		ADD_DIALOGUE_TEXT(55, 3, 36);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 26:
		ADD_DIALOGUE_TEXT(58, 1, 37);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 27:
		ADD_DIALOGUE_TEXT(59, 2, 38);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 28:
		ADD_DIALOGUE_TEXT(61, 1, 39);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 29:
		ADD_DIALOGUE_TEXT(62, 1, 40);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 30:
		ADD_DIALOGUE_TEXT(64, 1, 42);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 31:
		ADD_DIALOGUE_TEXT(63, 1, 41);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 32:
		ADD_DIALOGUE_TEXT(65, 1, 43);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 33:
		ADD_DIALOGUE_TEXT(66, 1, 44);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 34:
		ADD_DIALOGUE_TEXT(30, 1, 30);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 35:
		ADD_DIALOGUE_TEXT(29, 1, 29);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 36:
		ADD_DIALOGUE_TEXT(67, 2, 45);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 37:
		ADD_DIALOGUE_TEXT(69, 1, 46);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 38:
		EXEC_MAIN_ACTION_38();
		break;
	case 39:
		ADD_DIALOGUE_TEXT(70, 1, 47);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 40:
		ADD_DIALOGUE_TEXT(71, 1, 48);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 41:
		ADD_DIALOGUE_TEXT(72, 1, 49);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 42:
		ADD_DIALOGUE_TEXT(73, 1, 50);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 43:
		EXEC_MAIN_ACTION_43();
		break;
	case 44:
		ADD_DIALOGUE_TEXT(156, 1, 80);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 45:
		ADD_DIALOGUE_TEXT(92, 1, 64);
		ADD_DIALOGUE_TEXT(93, 3, 65);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 46:
		if (_objectsState[3] == 0) {
			ADD_DIALOGUE_TEXT(85, 1, 60);
		} else {
			ADD_DIALOGUE_TEXT(86, 2, 61);
		}
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 48:
		ADD_DIALOGUE_TEXT(77 + _objectsState[0], 1, 53 + _objectsState[0]);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
	case 50:
		if (_objectsState[1] == 0) {
			ADD_DIALOGUE_TEXT(80, 1, 56);
			ADD_DIALOGUE_TEXT(81, 1, 57);
			SET_DIALOGUE_TEXT(1, 2);
			startIgorDialogue();
		} else {
			ADD_DIALOGUE_TEXT(82, 1, 58);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 51:
		if (_objectsState[7] == 0) {
			ADD_DIALOGUE_TEXT(157, 1, 81);
		} else {
			ADD_DIALOGUE_TEXT(158, 1, 82);
		}
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 52:
		ADD_DIALOGUE_TEXT(89, 2, 63);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 53:
		ADD_DIALOGUE_TEXT(88, 1, 62);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 54:
		EXEC_MAIN_ACTION_54();
		break;
	case 55:
		ADD_DIALOGUE_TEXT(159, 1, 83);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 56:
		ADD_DIALOGUE_TEXT(160, 1, 84);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 57:
		ADD_DIALOGUE_TEXT(31, 1, 31);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 58:
		ADD_DIALOGUE_TEXT(140, 1, 70);
		ADD_DIALOGUE_TEXT(141, 1, 71);
		ADD_DIALOGUE_TEXT(142, 1, 72);
		ADD_DIALOGUE_TEXT(143, 1, 73);
		SET_DIALOGUE_TEXT(1, 4);
		startIgorDialogue();
		_objectsState[4] = 2;
		break;
	case 59:
		ADD_DIALOGUE_TEXT(97, 2, 67);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 60:
		ADD_DIALOGUE_TEXT(161, 1, 85);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 61:
		if (_inventoryInfo[51] != 0) {
			ADD_DIALOGUE_TEXT(76, 1, 52);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		} else {
			ADD_DIALOGUE_TEXT(74, 2, 51);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
			addObjectToInventory(16, 51);
		}
		break;
	case 62:
		_inventoryImages[_inventoryInfo[52] - 1] = 0;
		_inventoryInfo[52] = 0;
		packInventory();
		_inventoryImages[_inventoryInfo[59] - 1] = 0;
		_inventoryInfo[59] = 0;
		packInventory();
		_objectsState[1] = 1;
		UPDATE_OBJECT_STATE(2);
		addObjectToInventory(24, 59);
		ADD_DIALOGUE_TEXT(169, 1, 93);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 63:
		ADD_DIALOGUE_TEXT(170, 1, 94);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		_inventoryImages[_inventoryInfo[68] - 1] = 0;
		_inventoryInfo[68] = 0;
		packInventory();
		_inventoryImages[_inventoryInfo[69] - 1] = 0;
		_inventoryInfo[69] = 0;
		packInventory();
		_objectsState[6] = 1;
		UPDATE_OBJECT_STATE(7);
		addObjectToInventory(34, 69);
		break;
	case 64:
		if (_objectsState[6] == 0) {
			ADD_DIALOGUE_TEXT(162, 1, 86);
		} else if (_objectsState[6] == 1) {
			ADD_DIALOGUE_TEXT(163, 1, 87);
		} else if (_objectsState[6] == 2) {
			ADD_DIALOGUE_TEXT(164, 1, 88);
		}
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 65:
		ADD_DIALOGUE_TEXT(165, 1, 89);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 66:
		ADD_DIALOGUE_TEXT(166, 1, 90);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 67:
		ADD_DIALOGUE_TEXT(167, 1, 91);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 68:
		ADD_DIALOGUE_TEXT(168, 1, 92);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 69:
		ADD_DIALOGUE_TEXT(96, 1, 66);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	default:
		warning("EXEC_MAIN_ACTION() Unhandled action %d", action);
		break;
	}
}

void IgorEngine::EXEC_MAIN_ACTION_38() {
	memcpy(_paletteBuffer, _currentPalette, 624);
	fadeOutPalette(624);
	uint8 *tmp = (uint8 *)malloc(64000 + 768);
	if (tmp) {
		memcpy(tmp, _screenVGA, 64000);
		memcpy(tmp + 64000, _paletteBuffer, 768);
	}
	loadData(IMG_NewsPaper, _screenVGA);
	loadData(PAL_NewsPaper, _paletteBuffer);
	fadeInPalette(624);
	WalkData *wd = &_walkData[_walkDataLastIndex - 1];
	int _walkDataCurrentPosX2 = wd->x;
	int _walkDataCurrentPosY2 = wd->y;
	int _walkDataCurrentWScale = wd->scaleWidth;
	wd->x = 80;
	wd->y = 130;
	wd->scaleWidth = 50;
	ADD_DIALOGUE_TEXT(144, 1, 74);
	ADD_DIALOGUE_TEXT(145, 3, 75);
	ADD_DIALOGUE_TEXT(148, 2, 76);
	ADD_DIALOGUE_TEXT(150, 2, 77);
	SET_DIALOGUE_TEXT(1, 4);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	wd->x = 240;
	wd->y = 130;
	ADD_DIALOGUE_TEXT(152, 1, 78);
	ADD_DIALOGUE_TEXT(153, 3, 79);
	SET_DIALOGUE_TEXT(1, 2);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	wd->x = _walkDataCurrentPosX2;
	wd->y = _walkDataCurrentPosY2;
	wd->scaleWidth = _walkDataCurrentWScale;
	fadeOutPalette(624);
	if (tmp) {
		memcpy(_screenVGA, tmp, 64000);
		memcpy(_paletteBuffer, tmp + 64000, 768);
		free(tmp);
	}
	fadeInPalette(624);
	_objectsState[2] = 1;
}

void IgorEngine::EXEC_MAIN_ACTION_43() {
	memcpy(_paletteBuffer, _currentPalette, 624);
	fadeOutPalette(624);
	uint8 *tmp = (uint8 *)malloc(64000 + 768);
	if (tmp) {
		memcpy(tmp, _screenVGA, 64000);
		memcpy(tmp + 64000, _paletteBuffer, 768);
	}
	loadData(IMG_PhotoHarrisonMargaret, _screenVGA);
	loadData(PAL_PhotoHarrisonMargaret, _paletteBuffer);
	fadeInPalette(624);
	WalkData *wd = &_walkData[_walkDataLastIndex - 1];
	int _walkDataCurrentPosX2 = wd->x;
	int _walkDataCurrentPosY2 = wd->y;
	int _walkDataCurrentWScale = wd->scaleWidth;
	wd->x = 160;
	wd->y = 130;
	wd->scaleWidth = 50;
	ADD_DIALOGUE_TEXT(83, 2, 59);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	_currentAction.object1Num = 0;
	wd->x = _walkDataCurrentPosX2;
	wd->y = _walkDataCurrentPosY2;
	wd->scaleWidth = _walkDataCurrentWScale;
	fadeOutPalette(624);
	if (tmp) {
		memcpy(_screenVGA, tmp, 64000);
		memcpy(_paletteBuffer, tmp + 64000, 768);
		free(tmp);
	}
	fadeInPalette(624);
}

void IgorEngine::EXEC_MAIN_ACTION_54() {
	memcpy(_paletteBuffer, _currentPalette, 624);
	fadeOutPalette(624);
	uint8 *tmp = (uint8 *)malloc(64000 + 768);
	if (tmp) {
		memcpy(tmp, _screenVGA, 64000);
		memcpy(tmp + 64000, _paletteBuffer, 768);
	}
	loadData(IMG_RomanNumbersPaper, _screenVGA);
	loadData(PAL_RomanNumbersPaper, _paletteBuffer);
	PART_UPDATE_FIGURES_ON_PAPER(0);
	fadeInPalette(624);
	WalkData *wd = &_walkData[_walkDataLastIndex - 1];
	int _walkDataCurrentPosX2 = wd->x;
	int _walkDataCurrentPosY2 = wd->y;
	int _walkDataCurrentWScale = wd->scaleWidth;
	wd->x = 160;
	wd->y = 130;
	wd->scaleWidth = 50;
	if (_objectsState[2] == 0) {
		ADD_DIALOGUE_TEXT(99, 1, 68);
	} else {
		ADD_DIALOGUE_TEXT(100, 2, 69);
	}
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	wd->x = _walkDataCurrentPosX2;
	wd->y = _walkDataCurrentPosY2;
	wd->scaleWidth = _walkDataCurrentWScale;
	fadeOutPalette(624);
	if (tmp) {
		memcpy(_screenVGA, tmp, 64000);
		memcpy(_paletteBuffer, tmp + 64000, 768);
		free(tmp);
	}
	fadeInPalette(624);
}

void IgorEngine::CHECK_FOR_END_OF_DEMO() {
	if (_demoActionsCounter >= 10) {
		ADD_DIALOGUE_TEXT(104, 2);
		ADD_DIALOGUE_TEXT(106, 2);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
	}
}

void IgorEngine::SET_PAL_208_96_1() {
	memcpy(_paletteBuffer + 208 * 3, PAL_96_1, 96);
}

void IgorEngine::SET_PAL_240_48_1() {
	memcpy(_paletteBuffer + 240 * 3, PAL_48_1, 48);
}

void IgorEngine::UPDATE_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		switch (_objectsState[0]) {
		case 0:
			strcpy(_globalObjectNames[23], getString(STR_BottleOfWhisky));
			break;
		case 1:
			strcpy(_globalObjectNames[23], getString(STR_EmptyBottle));
			break;
		case 2:
			strcpy(_globalObjectNames[23], getString(STR_BottleOfWater));
			break;
		}
	}
	if (num == 2 || num == 255) {
		switch (_objectsState[1]) {
		case 0:
			_inventoryImages[23] = 27;
			strcpy(_globalObjectNames[24], getString(STR_Lizard));
			break;
		default:
			_inventoryImages[23] = 35;
			strcpy(_globalObjectNames[24], getString(STR_FatLizard));
			break;
		}
	}
	if (num == 4 || num == 255) {
		switch (_objectsState[3]) {
		case 0:
			strcpy(_globalObjectNames[22], getString(STR_CarolineFolder));
			break;
		case 1:
			strcpy(_globalObjectNames[22], getString(STR_PhilipFolder));
			break;
		}
	}
	if (num == 7 || num == 255) {
		switch (_objectsState[6]) {
		case 0:
			_inventoryImages[33] = 21;
			break;
		case 1:
			_inventoryImages[33] = 14;
			break;
		case 2:
			_inventoryImages[33] = 7;
			break;
		}
	}
	if (num == 8 || num == 255) {
		if (_objectsState[7] == 0) {
			strcpy(_globalObjectNames[25], getString(STR_Statuette));
			_inventoryImages[24] = 29;
		} else {
			strcpy(_globalObjectNames[25], getString(STR_Reward));
			_inventoryImages[24] = 39;
		}
	}
}

void IgorEngine::PART_MEANWHILE() {
	hideCursor();
	memset(_currentPalette, 0, 768);
	setPaletteRange(208, 255);
	loadData(IMG_Meanwhile, _screenVGA);
	_paletteBuffer[3] = 63;
	_paletteBuffer[4] = 32;
	_paletteBuffer[5] = 0;
	_paletteBuffer[6] = 44;
	_paletteBuffer[7] = 12;
	_paletteBuffer[8] = 0;
	fadeInPalette(9);
	for (int i = 0; i <= 5 && !_inputVars[kInputEscape]; ++i) {
		waitForTimer(250);
	}
	_inputVars[kInputEscape] = 0;
	fadeOutPalette(9);
}

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
	fadeInPalette(768);
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
	fadeOutPalette(768);
	memset(_screenVGA, 0, 46080);
	if (_objectsState[110] < 9 && _currentPart != 331) {
		setupDefaultPalette();
		SET_PAL_240_48_1();
		SET_PAL_208_96_1();
		drawVerbsPanel();
		drawInventory(_inventoryInfo[72], 0);
	}
}

void IgorEngine::PART_UPDATE_FIGURES_ON_PAPER(int delay) {
	uint8 *framesData = loadData(FRM_NumbersPaper1);
	uint8 *framesOffsets = loadData(FRM_NumbersPaper2);
	for (int i = 1; i <= 20; ++i) {
		const uint8 *p = framesData + READ_LE_UINT16(framesOffsets + (i - 1) * 2) - 1;
		decodeAnimFrame(p, _screenVGA, true);
		waitForTimer(delay);
	}
	free(framesData);
	free(framesOffsets);
}

void IgorEngine::PART_MAIN() {
	SET_EXEC_ACTION_FUNC(0, &IgorEngine::EXEC_MAIN_ACTION);
	memset(_objectsState, 0, 112);
	_objectsState[21] = 1;
	_objectsState[49] = 1;
	memset(_inventoryInfo, 0, 36);
	_inventoryInfo[0] = 1; // ordering
	_inventoryInfo[1] = 2;
	_inventoryInfo[2] = 4;
	_inventoryInfo[36] = 1;
	_inventoryInfo[37] = 2;
	_inventoryInfo[39] = 3;
	_inventoryInfo[72] = 1; // first object
	_inventoryInfo[73] = 3; // last object
	UPDATE_OBJECT_STATE(255);
	if (_currentPart != kStartupPart) { // boot param
		SET_PAL_208_96_1();
		SET_PAL_240_48_1();
		drawVerbsPanel();
		drawInventory(1, 0);
	}
	do {
		debugC(9, kDebugGame, "PART_MAIN _currentPart %d", _currentPart);
		switch (_currentPart) {
		case 40:
			PART_04();
			break;
		case 50:
		case 51:
		case 52:
			PART_05();
			break;
		case 60:
		case 61:
		case 62:
			PART_06();
			break;
		case 120:
		case 121:
		case 122:
			PART_12();
			break;
		case 130:
		case 131:
			PART_13();
			break;
		case 140:
		case 141:
		case 142:
			PART_14();
			break;
		case 150:
			PART_15();
			break;
		case 160:
			PART_16();
			break;
		case 170:
		case 171:
			PART_17();
			break;
		case 180:
		case 181:
			PART_18();
			break;
		case 190:
		case 191:
			PART_19();
			break;
		case 210:
		case 211:
		case 212:
			PART_21();
			break;
		case 220:
		case 221:
			PART_22();
			break;
		case 230:
		case 231:
		case 232:
			PART_23();
			break;
		case 240:
		case 241:
		case 242:
			PART_24();
			break;
		case 250:
		case 251:
		case 252:
			PART_25();
			break;
		case 260:
		case 261:
			PART_26();
			break;
		case 270:
		case 271:
			PART_27();
			break;
		case 280:
		case 281:
			PART_28();
			break;
		case 300:
		case 301:
		case 302:
		case 303:
			PART_30();
			break;
		case 310:
		case 311:
		case 312:
		case 313:
			PART_31();
			break;
		case 330:
		case 331:
			PART_33();
			break;
		case 360:
			PART_36();
			break;
		case 370:
			PART_37();
			break;
		case 750:
			PART_75();
			break;
		case 850:
			memset(_screenVGA, 0, 64000);
			_screenVGAVOffset = 24;
			_system->copyRectToScreen(_screenVGA, 320, 0, 0, 320, _screenVGAVOffset);
			PART_85();
			memset(_screenVGA + 46080, 0, 17920);
			_nextTimer = _system->getMillis() + 1000 / 60;
			for (int y = _screenVGAVOffset; y >= 0; --y) {
				_system->copyRectToScreen(_screenVGA, 320, 0, y, 320, 145);
				_system->updateScreen();
				int diff = _nextTimer - _system->getMillis();
				if (diff > 0) {
					_system->delayMillis(diff);
				}
				_nextTimer = _system->getMillis() + 1000 / 60;
			}
			_screenVGAVOffset = 0;
			_inputVars[kInputCursorXPos] = 160;
			_inputVars[kInputCursorYPos] = 72;
			_system->warpMouse(_inputVars[kInputCursorXPos], _inputVars[kInputCursorYPos]);
			break;
		case 900:
		case 901:
		case 902:
		case 903:
		case 904:
			PART_90();
			break;
		default:
			warning("PART_MAIN() Unhandled part %d", _currentPart);
			_currentPart = kInvalidPart;
			break;
		}
		if (_currentPart >= 10) {
			if (_currentPart <= 24 || _currentPart == 51) {
				continue;
			}
			if (_currentPart == 60 || _currentPart == 102 || _currentPart == 110) {
				continue;
			}
		}
		if (_currentPart == 340 || _currentPart == 351 || _currentPart == 750) {
			continue;
		}
		if (_currentPart == 122 || _currentPart == 255) {
			continue;
		}
		if (_objectsState[110] < 1 || _objectsState[110] > 7) {
			continue;
		}
		if (_gameState.nextMusicCounter != 5) {
			++_gameState.nextMusicCounter;
		} else {
			if (_gameState.musicNum != 11) {
				_previousMusic = _gameState.musicNum;
				playMusic(11);
			}
			PART_MEANWHILE();
			PART_MARGARET_ROOM_CUTSCENE();
			if (_previousMusic != 11) {
				playMusic(_previousMusic);
			}
			_gameState.nextMusicCounter = 0;
		}
	} while (_currentPart != kInvalidPart && !_eventQuitGame);
	if (_game.flags & kFlagDemo) {
		for (_currentPart = kSharewarePart; !_eventQuitGame && _currentPart <= kSharewarePart + 6; ++_currentPart) {
			PART_95();
		}
	}
}

} // namespace Igor
