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

static const uint8 PART_15_ANIM_DATA_1[8] = { 5, 6, 0, 3, 3, 0, 0, 1 };

static const uint8 PART_15_ANIM_DATA_2[8] = { 0, 1, 2, 3, 4, 3, 5, 6 };

static const uint8 PART_15_ANIM_DATA_3[12] = { 0, 0, 1, 1, 1, 2, 1, 1, 1, 2, 1, 3 };

static const uint8 PART_15_ANIM_DATA_4[8] = { 0, 7, 8, 3, 4, 3, 5, 6 };

static const uint8 PART_15_ANIM_DATA_5[12] = { 0, 0, 1, 1, 1, 2, 1, 1, 1, 2, 1, 3 };

void IgorEngine::PART_15_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		PART_15_ACTION_101();
		break;
	case 102:
		ADD_DIALOGUE_TEXT(204, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 103:
		ADD_DIALOGUE_TEXT(205, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(207, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		ADD_DIALOGUE_TEXT(205, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106:
		ADD_DIALOGUE_TEXT(205, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107:
		PART_15_ACTION_107();
		break;
	case 108:
		ADD_DIALOGUE_TEXT(208, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 109:
		ADD_DIALOGUE_TEXT(201, 1);
		ADD_DIALOGUE_TEXT(202, 2);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 110:
		ADD_DIALOGUE_TEXT(234, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 111:
		ADD_DIALOGUE_TEXT(235, 1);
		ADD_DIALOGUE_TEXT(236, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 112:
		ADD_DIALOGUE_TEXT(237, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 113:
		ADD_DIALOGUE_TEXT(235, 1);
		ADD_DIALOGUE_TEXT(236, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 114:
		ADD_DIALOGUE_TEXT(240, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 115:
		PART_15_ACTION_115();
		break;
	case 116:
		PART_15_ACTION_116();
		break;
	default:
		error("PART_15_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_15_ACTION_101() {
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[34560]].area = 1;
	buildWalkPathSimple(34, 108, 0, 108);
	_roomObjectAreasTable[_screenLayer2[34560]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 271;
}

void IgorEngine::PART_15_ACTION_107() {
	if (_gameState.counter[3] == 1) {
		do {
			PART_15_HELPER_3();
			waitForTimer();
		} while (_gameState.counter[3] != 0);
	}
	PART_15_HELPER_7(6);
	PART_15_HANDLE_DIALOGUE_TOBIAS();
}

void IgorEngine::PART_15_ACTION_115() {
	if (_gameState.counter[3] == 1) {
		do {
			PART_15_HELPER_3();
			waitForTimer();
		} while (_gameState.counter[3] != 0);
	}
	PART_15_HELPER_7(6);
	ADD_DIALOGUE_TEXT(209, 1);
	ADD_DIALOGUE_TEXT(210, 1);
	SET_DIALOGUE_TEXT(1, 2);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	ADD_DIALOGUE_TEXT(212, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
	waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
	ADD_DIALOGUE_TEXT(213, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	ADD_DIALOGUE_TEXT(214, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
	waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
	for (int i = 1; i <= 11; ++i) {
		PART_15_HELPER_9(PART_15_ANIM_DATA_3[i]);
		waitForTimer(60);
	}
	PART_15_HELPER_7(6);
	if (_objectsState[37] == 0) {
		ADD_DIALOGUE_TEXT(215, 2);
		ADD_DIALOGUE_TEXT(217, 1);
	} else {
		ADD_DIALOGUE_TEXT(218, 2);
		ADD_DIALOGUE_TEXT(223, 1);
	}
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
	waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
	if (_objectsState[37] == 0) {
		return;
	}
	for (int i = 1; i <= 7; ++i) {
		PART_15_HELPER_7(PART_15_ANIM_DATA_2[i]);
		waitForTimer(40);
	}
	removeObjectFromInventory(70);
	_objectsState[46] = 1;
	if (_objectsState[47] == 0) {
		ADD_DIALOGUE_TEXT(221, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startCutsceneDialogue(133, 67, 0, 63, 19);
		_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
		waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
		_updateDialogue = 0;
		return;
	}
	ADD_DIALOGUE_TEXT(226, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
	waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
	ADD_DIALOGUE_TEXT(228, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	ADD_DIALOGUE_TEXT(229, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
	waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
	ADD_DIALOGUE_TEXT(230, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	_currentPart = 780;
}

void IgorEngine::PART_15_ACTION_116() {
	if (_gameState.counter[3] == 1) {
		do {
			PART_15_HELPER_3();
			waitForTimer();
		} while (_gameState.counter[3] != 0);
	}
	PART_15_HELPER_7(6);
	ADD_DIALOGUE_TEXT(209, 1);
	ADD_DIALOGUE_TEXT(211, 1);
	SET_DIALOGUE_TEXT(1, 2);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	ADD_DIALOGUE_TEXT(212, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
	waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
	ADD_DIALOGUE_TEXT(213, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	ADD_DIALOGUE_TEXT(214, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
	waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
	for (int i = 1; i <= 11; ++i) {
		PART_15_HELPER_9(PART_15_ANIM_DATA_5[i]);
		waitForTimer(60);
	}
	PART_15_HELPER_7(6);
	if (_objectsState[37] == 0) {
		ADD_DIALOGUE_TEXT(215, 2);
		ADD_DIALOGUE_TEXT(217, 1);
	} else {
		ADD_DIALOGUE_TEXT(218, 2);
		ADD_DIALOGUE_TEXT(223, 1);
	}
	SET_DIALOGUE_TEXT(1, 2);
	startCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
	waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
	if (_objectsState[37] == 0) {
		return;
	}
	for (int i = 1; i <= 7; ++i) {
		PART_15_HELPER_7(PART_15_ANIM_DATA_4[i]);
		waitForTimer(40);
	}
	removeObjectFromInventory(60);
	_objectsState[47] = 1;
	if (_objectsState[46] == 0) {
		ADD_DIALOGUE_TEXT(224, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startCutsceneDialogue(133, 67, 0, 63, 19);
		_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
		waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
		_updateDialogue = 0;
		return;
	}
	ADD_DIALOGUE_TEXT(226, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
	waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
	ADD_DIALOGUE_TEXT(228, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	ADD_DIALOGUE_TEXT(229, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
	waitForEndOfCutsceneDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
	ADD_DIALOGUE_TEXT(230, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	_currentPart = 780;
}

void IgorEngine::PART_15_UPDATE_ROOM_BACKGROUND() {
	PART_15_HELPER_5();
	if (_gameTicks == 38 || _gameTicks == 60) {
		if (_objectsState[48] != 1 && getRandomNumber(200) == 0 && _gameState.counter[3] == 0) {
			_gameState.counter[3] = 1;
		}
	}
	PART_15_HELPER_3();
}

void IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_15_HELPER_8(0);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_15_HELPER_8(getRandomNumber(5));
		break;
	case kUpdateDialogueAnimStanding:
		PART_15_HELPER_8(0);
		break;
	}
}

void IgorEngine::PART_15_HANDLE_DIALOGUE_TOBIAS() {
	loadDialogueData(DLG_TobiasOffice);
	_updateDialogue = &IgorEngine::PART_15_UPDATE_DIALOGUE_TOBIAS;
	handleDialogue(133, 67, 0, 63, 19);
	_updateDialogue = 0;
}

void IgorEngine::PART_15_HELPER_1(int num) {
}

void IgorEngine::PART_15_HELPER_2() {
	_walkData[0].setPos(0, 108, 2, 0);
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 15;
	_walkData[0].scaleWidth = 25;
	_walkData[0].xPosChanged = 1;
	_walkData[0].dxPos = 0;
	_walkData[0].yPosChanged = 1;
	_walkData[0].dyPos = 0;
	_walkData[0].scaleHeight = 25;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	_roomObjectAreasTable[_screenLayer2[34560]].area = 1;
	buildWalkPathSimple(0, 108, 34, 108);
	_roomObjectAreasTable[_screenLayer2[34560]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_15_HELPER_3() {
	if (compareGameTick(38) || compareGameTick(60)) {
		if (_objectsState[48] != 1 && _gameState.counter[3] == 1) {
			if (_gameState.unk11 == 0) {
				_objectsState[48] = 1;
				_objectsState[49] = 1;
				_gameState.counter[3] = 0;
			} else {
				PART_15_HELPER_6(_gameState.unk11);
			}
			if (_gameState.unk11 == 7) {
				_objectsState[49] = 2;
			}
			if (_objectsState[49] == 2) {
				if (_gameState.counter[4] == 0) {
					playSound(54, 1);
				}
				++_gameState.counter[4];
				if (_gameState.unk11 == 7) {
					_gameState.unk11	= 6;
				} else {
					_gameState.unk11 = 7;
				}
				if (_gameState.counter[4] > 15) {
					_objectsState[49] = 3;
					_gameState.unk11	= 6;
				}
			} else if (_objectsState[49] == 1) {
				++_gameState.unk11;
			} else {
				--_gameState.unk11;
			}
		}
	}
}

void IgorEngine::PART_15_HELPER_5() {
	if (compareGameTick(7) || compareGameTick(29) || compareGameTick(61)) {
		if (_gameState.unk10 >= 3) {
			PART_15_HELPER_9(PART_15_ANIM_DATA_1[_gameState.unk10]);
			++_gameState.unk10;
			if (_gameState.unk10 == 8) {
				_gameState.unk10 = 1;
			}
		}
	}
	if (compareGameTick(5)) {
		if (_gameState.unk10 >= 1 && _gameState.unk10 <= 2) {
			PART_15_HELPER_9(_gameState.unk10);
			if (_gameState.unk10 == 1 && getRandomNumber(10) == 0) {
				_gameState.unk10 = 2;
			} else {
				_gameState.unk10 = 1;
			}
			if (getRandomNumber(30) == 0) {
				_gameState.unk10 = 3;
			}
		}
	}
}

void IgorEngine::PART_15_HELPER_6(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 17; ++i) {
		for (int j = 0; j <= 52; ++j) {
			int offset = (i + 23) * 320 + j + 18;
			uint8 color = _screenVGA[offset];
			if (color < 0xF0 || color > 0xF1) {
				color = _animFramesBuffer[0x4B8C + frame * 954 + i * 53 + j];
			}
			_screenTempLayer[i * 100 + j] = color;
		}
	}
	int offset = 7378;
	for (int i = 0; i <= 17; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _screenTempLayer + i * 100, 53);
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + 0x4B8C + frame * 954 + i * 53, 53);
	}
	if (_gameState.dialogueTextRunning) {
		memcpy(_screenTextLayer + 23040, _screenLayer1 + _dialogueDirtyRectY, _dialogueDirtyRectSize);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_15_HELPER_7(int frame) {
	int offset = 20887;
	for (int i = 0; i <= 35; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _animFramesBuffer + 0x49A + frame * 2124 + i * 59, 59);
	}
}

void IgorEngine::PART_15_HELPER_8(int frame) {
	int offset = 22847;
	for (int i = 0; i <= 12; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _animFramesBuffer + 0x958 + frame * 182 + i * 14, 14);
	}
}

void IgorEngine::PART_15_HELPER_9(int frame) {
	int offset = 22835;
	for (int i = 0; i <= 22; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _animFramesBuffer + frame * 598 + i * 26, 26);
	}
}

void IgorEngine::PART_15() {
	_gameState.enableLight = 2;
	loadRoomData(PAL_TobiasOffice, IMG_TobiasOffice, BOX_TobiasOffice, MSK_TobiasOffice, TXT_TobiasOffice);
	static const int anm[] = { ANM_TobiasOffice1, AOF_TobiasOffice1, ANM_TobiasOffice2, AOF_TobiasOffice2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_TobiasOffice);
	_roomDataOffsets = PART_15_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(28, 0, 96, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_15_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_15_UPDATE_ROOM_BACKGROUND;
	PART_15_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeInPalette(768);
	_gameState.unk10 = 1;
	_gameState.unk11 = 2;
	_gameState.counter[3] = 0;
	_gameState.counter[4] = 0;
	PART_15_HELPER_2();
	enterPartLoop();
	while (_currentPart == 150) {
		runPartLoop();
	}
	leavePartLoop();
	if (_objectsState[48] == 1) {
		_objectsState[48] = 2;
	}
	fadeOutPalette(624);
}

} // namespace Igor
