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

static const uint8 PART_21_ANIM_DATA_1[7] = { 0, 11, 12, 13, 20, 21, 14 };

static const uint8 PART_21_ANIM_DATA_2[11] = { 0, 4, 5, 4, 5, 4, 5, 4, 5, 6, 0 };

static bool IN_ACTION_111;

void IgorEngine::PART_21_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		PART_21_ACTION_101();
		break;
	case 102:
		PART_21_ACTION_102();
		break;
	case 103:
		ADD_DIALOGUE_TEXT(201, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(202, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		ADD_DIALOGUE_TEXT(203, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106:
		ADD_DIALOGUE_TEXT(204, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107:
		PART_21_ACTION_107();
		break;
	case 108:
		PART_21_ACTION_108();
		break;
	case 109:
		ADD_DIALOGUE_TEXT(205, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 110:
		PART_21_ACTION_110();
		break;
	case 111:
		PART_21_ACTION_111();
		break;
	case 112:
		ADD_DIALOGUE_TEXT(222, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 113:
		PART_21_ACTION_113();
		break;
	default:
		error("PART_21_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_21_ACTION_101() {
	if (_objectsState[65] == 1) {
		const int offset = 29440;
		for (int i = 0; i <= 50; ++i) {
			memcpy(_screenVGA + i * 320 + offset, _screenLayer1 + i * 320 + offset, 20);
		}
		PART_21_HELPER_10();
	}
	_currentPart = 303;
}

void IgorEngine::PART_21_ACTION_102() {
	PART_21_HELPER_6(1);
	ADD_DIALOGUE_TEXT(226, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	ADD_DIALOGUE_TEXT(227, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(76, 87, 63, 32, 0);
	_updateDialogue = &IgorEngine::PART_21_UPDATE_DIALOGUE_MARGARET_2;
	waitForEndOfCutsceneDialogue(76, 87, 63, 32, 0);
	_updateDialogue = 0;
	PART_21_HANDLE_DIALOGUE_MARGARET();
	PART_21_HELPER_1(255);
}

void IgorEngine::PART_21_ACTION_107() {
	if (_objectsState[64] == 1) {
		EXEC_MAIN_ACTION(11);
		return;
	}
	for (int i = 2; i <= 3; ++i) {
		const int offset = 21902;
		for (int j = 0; j <= 53; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + (i - 1) * 1782 + j * 33, 33);
		}
		if (i == 3) {
			playSound(13, 1);
		} else {
			waitForTimer(100);
		}
	}
	_objectsState[64] = 1;
	PART_21_HELPER_1(1);
}

void IgorEngine::PART_21_ACTION_108() {
	if (_objectsState[64] == 0) {
		EXEC_MAIN_ACTION(14);
		return;
	}
	for (int i = 2; i >= 1; --i) {
		const int offset = 21902;
		for (int j = 0; j <= 53; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + (i - 1) * 1782 + j * 33, 33);
		}
		if (i == 2) {
			playSound(14, 1);
			waitForTimer(100);
		}
	}
	_objectsState[64] = 0;
	PART_21_HELPER_1(1);
}

void IgorEngine::PART_21_ACTION_110() {
	if (_objectsState[65] == 1) {
		const int offset = 27180;
		for (int i = 0; i <= 50; ++i) {
			memcpy(_screenVGA + i * 320 + offset, _screenLayer1 + i * 320 + offset, 20);
		}
		PART_21_HELPER_10();
	}
	_currentPart = 270;
}

void IgorEngine::PART_21_ACTION_111() {
	if (_inventoryInfo[65] == 0) {
		ADD_DIALOGUE_TEXT(223, 2);
		ADD_DIALOGUE_TEXT(225, 1);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		return;
	}
	IN_ACTION_111 = true;
	ADD_DIALOGUE_TEXT(208, 1);
	ADD_DIALOGUE_TEXT(209, 1);
	SET_DIALOGUE_TEXT(1, 2);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	const int offset = 28183;
	for (int i = 0; i <= 49; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _animFramesBuffer + 0xCBE1 + i * 26, 26);
	}
	waitForTimer(100);
	int k = 1;
	do {
		if (compareGameTick(3, 24)) {
			for (int j = 0; j <= 49; ++j) {
				memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + 0xBCA5 + PART_21_ANIM_DATA_2[k] * 1300 + j * 26, 26);
			}
			++k;
		}
		PART_21_UPDATE_ROOM_BACKGROUND();
		waitForTimer();
	} while (k != 10);
	IN_ACTION_111 = false;
	removeObjectFromInventory(56);
	_objectsState[65] = 1;
	PART_21_HELPER_1(255);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
	ADD_DIALOGUE_TEXT(210, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
}

void IgorEngine::PART_21_ACTION_113() {
	if (_objectsState[64] != 0) {
		 _roomObjectAreasTable[_screenLayer2[34679]].area = 1;
		 --_walkDataLastIndex;
		 buildWalkPathSimple(165, 126, 119, 108);
		 _walkDataCurrentIndex = 1;
		 _gameState.igorMoving = true;
		waitForIgorMove();
		if (_objectsState[65] == 1) {
			PART_21_HELPER_10();
		}
		_currentPart = 370;
	}
}

void IgorEngine::PART_21_UPDATE_DIALOGUE_MARGARET_1(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_21_HELPER_11(6);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_21_HELPER_11(getRandomNumber(5) + 6);
		break;
	case kUpdateDialogueAnimStanding:
		PART_21_HELPER_11(1);
		break;
	}
}

void IgorEngine::PART_21_UPDATE_DIALOGUE_MARGARET_2(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_21_HELPER_6(6);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_21_HELPER_6(getRandomNumber(5) + 6);
		break;
	}
}

void IgorEngine::PART_21_UPDATE_DIALOGUE_MARGARET_3(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_21_HELPER_6(15);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_21_HELPER_6(getRandomNumber(5) + 15);
		break;
	}
}

void IgorEngine::PART_21_HANDLE_DIALOGUE_MARGARET() {
	loadDialogueData(DLG_CollegeCorridorMargaret);
	_updateDialogue = &IgorEngine::PART_21_UPDATE_DIALOGUE_MARGARET_1;
	handleDialogue(81, 76, 63, 32, 0);
	_updateDialogue = 0;
}

void IgorEngine::PART_21_HELPER_1(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[64] == 0) {
			PART_21_HELPER_7();
			_roomActionsTable[4] = 6;
		} else {
			PART_21_HELPER_8();
			_roomActionsTable[4] = 7;
		}
	}
	if (num == 2 || num == 255) {
		if (_objectsState[65] > 1) {
			PART_21_HELPER_9();
		}
		if (_objectsState[65] == 2 || _objectsState[65] == 4) {
			_roomObjectAreasTable[9].object = 0;
			_roomObjectAreasTable[10].object = 0;
		}
	}
}

void IgorEngine::PART_21_HELPER_2() {
	_walkData[0].setPos(0, 141, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 15;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPathSimple(0, 141, 32, 137);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_21_HELPER_3() {
	_walkData[0].setPos(319, 133, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPathSimple(319, 133, 239, 133);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_21_HELPER_4() {
	_walkData[0].setPos(119, 108, 2, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[34679]].area = 1;
	buildWalkPathSimple(119, 108, 175, 130);
	_roomObjectAreasTable[_screenLayer2[34679]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_21_HELPER_6(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 42; ++i) {
		for (int j = 0; j <= 48; ++j) {
			int offset = (i + 89) * 320 + j + 40;
			uint8 color = _screenVGA[offset];
			if ((color >= 192 && color <= 207) || color == 240 || color == 241) {
				_screenTempLayer[i * 100 + j] = _screenVGA[offset];
			} else {
				_screenTempLayer[i * 100 + j] = _animFramesBuffer[frame * 2107 + i * 49 + j + 0xCA7];
			}
		}
	}
	int offset = 28520;
	for (int i = 0; i <= 42; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _screenTempLayer + i * 100, 49);
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + frame * 2107 + i * 49 + 0xCA7, 49);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_21_HELPER_7() {
	const int offset = 21901;
	for (int i = 0; i <= 53; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + i * 33 + 0xE031, 33);
	}
}

void IgorEngine::PART_21_HELPER_8() {
	const int offset = 21901;
	for (int i = 0; i <= 53; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + i * 33 + 0xE727, 33);
	}
}

void IgorEngine::PART_21_HELPER_9() {
	const int offset = 21901;
	for (int i = 0; i <= 42; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + i * 49 + 0x14E2, 49);
	}
}

void IgorEngine::PART_21_HELPER_10() {
	PART_21_HELPER_6(1);
	memset(_screenVGA + 46080, 0, 17920);
	waitForTimer(120);
	for (int i = 1; i <= 6; ++i) {
		PART_21_HELPER_6(PART_21_ANIM_DATA_1[i]);
		if (i < 6) {
			waitForTimer(100);
		}
	}
	ADD_DIALOGUE_TEXT(212, 1);
	ADD_DIALOGUE_TEXT(213, 1);
	ADD_DIALOGUE_TEXT(214, 1);
	ADD_DIALOGUE_TEXT(215, 2);
	ADD_DIALOGUE_TEXT(217, 2);
	SET_DIALOGUE_TEXT(1, 5);
	startCutsceneDialogue(76, 87, 63, 32, 0);
	_updateDialogue = &IgorEngine::PART_21_UPDATE_DIALOGUE_MARGARET_3;
	waitForEndOfCutsceneDialogue(76, 87, 63, 32, 0);
	_updateDialogue = 0;
	ADD_DIALOGUE_TEXT(219, 1);
	ADD_DIALOGUE_TEXT(220, 1);
	ADD_DIALOGUE_TEXT(221, 1);
	SET_DIALOGUE_TEXT(1, 3);
	startCutsceneDialogue(76, 87, 63, 32, 0);
	_updateDialogue = &IgorEngine::PART_21_UPDATE_DIALOGUE_MARGARET_3;
	waitForEndOfCutsceneDialogue(76, 87, 63, 32, 0);
	_updateDialogue = 0;
	_objectsState[65] = 2;
	drawVerbsPanel();
	_currentAction.verb = kVerbWalk;
	drawInventory(_inventoryInfo[72], 0);
	PART_21_HELPER_1(255);
	_objectsState[110] = 1;
}

void IgorEngine::PART_21_HELPER_11(int frame) {
	const int offset = 28520;
	for (int i = 0; i <= 42; ++i) {
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + frame * 2107 + i * 49 + 0xCA7, 49);
	}
}

void IgorEngine::PART_21_UPDATE_ROOM_BACKGROUND() {
	if (IN_ACTION_111 && compareGameTick(3, 24)) {
		const int offset = 28183;
		int i = getRandomNumber(2) + 1;
		for (int j = 0; j <= 49; ++j) {
			memcpy(_screenVGA + j * 320 + offset, _animFramesBuffer + 0xBCA5 + i * 1300 + j * 26, 26);
		}
		++i;
	}
	if (compareGameTick(61) && _objectsState[65] <= 1) {
		if (_gameState.unk10 > 2) {
			PART_21_HELPER_6(_gameState.unk10);
			++_gameState.unk10;
			if (_gameState.unk10 == 6) {
				++_gameState.counter[4];
				if (_gameState.counter[4] < 2) {
					_gameState.unk10 = 4;
				} else {
					_gameState.counter[4] = 0;
					_gameState.unk10 = 1;
				}
			}
		} else {
			PART_21_HELPER_6(_gameState.unk10);
			++_gameState.unk10;
			if (_gameState.unk10 == 3 && getRandomNumber(6) != 0) {
				_gameState.unk10 = 1;
			}
		}
	}
}

void IgorEngine::PART_21() {
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeCorridorMargaret, IMG_CollegeCorridorMargaret, BOX_CollegeCorridorMargaret, MSK_CollegeCorridorMargaret, TXT_CollegeCorridorMargaret);
	static const int anm[] = { FRM_CollegeCorridorMargaret1, FRM_CollegeCorridorMargaret2, FRM_CollegeCorridorMargaret3, FRM_CollegeCorridorMargaret4, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorMargaret);
	_roomDataOffsets = PART_21_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_21_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_21_UPDATE_ROOM_BACKGROUND;
	PART_21_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	_gameState.unk10 = 1;
	fadeInPalette(768);
	if (_currentPart == 210) {
		PART_21_HELPER_2();
	} else if (_currentPart == 211) {
		PART_21_HELPER_3();
	} else if (_currentPart == 212) {
		PART_21_HELPER_4();
	}
	enterPartLoop();
	while (_currentPart >= 210 && _currentPart <= 212) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

} // namespace Igor
