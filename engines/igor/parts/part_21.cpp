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
		igorSay(201, 1, 1177);
		break;
	case 104:
		igorSay(202, 1, 1178);
		break;
	case 105:
		igorSay(203, 1, 1179);
		break;
	case 106:
		igorSay(204, 1, 1180);
		break;
	case 107:
		PART_21_ACTION_107();
		break;
	case 108:
		PART_21_ACTION_108();
		break;
	case 109:
		igorSay(205, 1, 1181);
		break;
	case 110:
		PART_21_ACTION_110();
		break;
	case 111:
		PART_21_ACTION_111();
		break;
	case 112:
		igorSay(222, 1, 1195);
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
		copyArea(_screenVGA, offset, 320, _screenLayer1 + offset, 320, 20, 51);
		PART_21_HELPER_10();
	}
	_currentPart = 303;
}

void IgorEngine::PART_21_ACTION_102() {
	PART_21_HELPER_6(1);
	igorSayAndWait(226, 1, 1197);
	cutsceneSayStart(76, 87, 63, 32, 0, 227, 1, 1198);
	_updateDialogue = &IgorEngine::PART_21_UPDATE_DIALOGUE_MARGARET_2;
	// The cutscene dialogue wait does not run Margaret's idle animation
	_updateRoomBackground = 0;
	waitForEndOfCutsceneDialogue(76, 87, 63, 32, 0);
	_updateRoomBackground = &IgorEngine::PART_21_UPDATE_ROOM_BACKGROUND;
	_updateDialogue = 0;
	PART_21_HANDLE_DIALOGUE_MARGARET();
	PART_21_HELPER_1_OBJECT_STATE(255);
}

void IgorEngine::PART_21_ACTION_107() {
	if (_objectsState[64] == 1) {
		EXEC_MAIN_ACTION(11);
		return;
	}
	for (int i = 2; i <= 3; ++i) {
		const int offset = 21902;
		drawAnimRect(offset, (i - 1) * 1782, 33, 54);
		if (i == 3) {
			playSound(13, 1);
		} else {
			waitForTimer(100);
		}
	}
	_objectsState[64] = 1;
	PART_21_HELPER_1_OBJECT_STATE(1);
}

void IgorEngine::PART_21_ACTION_108() {
	if (_objectsState[64] == 0) {
		EXEC_MAIN_ACTION(14);
		return;
	}
	for (int i = 2; i >= 1; --i) {
		const int offset = 21902;
		drawAnimRect(offset, (i - 1) * 1782, 33, 54);
		if (i == 2) {
			playSound(14, 1);
			waitForTimer(100);
		}
	}
	_objectsState[64] = 0;
	PART_21_HELPER_1_OBJECT_STATE(1);
}

void IgorEngine::PART_21_ACTION_110() {
	if (_objectsState[65] == 1) {
		const int offset = 27180;
		copyArea(_screenVGA, offset, 320, _screenLayer1 + offset, 320, 20, 51);
		PART_21_HELPER_10();
	}
	_currentPart = 270;
}

void IgorEngine::PART_21_ACTION_111() {
	if (_inventoryInfo[65] == 0) {
		igorSay(223, 2, 1196);
		return;
	}
	IN_ACTION_111 = true;
	igorSay({ { 208, 1, 1184 }, { 209, 1, 1185 } });
	// Igor's talking head is not animated here; the writing frames take its place
	waitForEndOfIgorDialogue(false);
	IN_ACTION_111 = false;
	const int offset = 28183;
	drawAnimRect(offset, 0xCBE1, 26, 50);
	waitForTimer(100);
	int k = 1;
	do {
		if (compareGameTick(3, 24)) {
			drawAnimRect(offset, 0xBCA5 + PART_21_ANIM_DATA_2[k] * 1300, 26, 50);
			++k;
		}
		PART_21_UPDATE_ROOM_BACKGROUND();
		waitForTimer();
	} while (k != 10);
	removeObjectFromInventory(56);
	_objectsState[65] = 1;
	PART_21_HELPER_1_OBJECT_STATE(255);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
	igorSay(210, 2, 1186);
}

void IgorEngine::PART_21_ACTION_113() {
	if (_objectsState[64] != 0) {
		 _roomObjectAreasTable[_screenLayer2[34679]].area = 1;
		 --_walkDataLastIndex;
		 buildWalkPathSimple(165, 126, 119, 108);
		 _walkDataCurrentIndex = 1;
		 _gameState.igorMoving = true;
		PART_21_HELPER_5();
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
		PART_21_HELPER_11(getRandomNumber(4) + 6);
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
		PART_21_HELPER_6(getRandomNumber(4) + 6);
		break;
	}
}

void IgorEngine::PART_21_UPDATE_DIALOGUE_MARGARET_3(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_21_HELPER_6(15);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_21_HELPER_6(getRandomNumber(4) + 15);
		break;
	}
}

void IgorEngine::PART_21_HANDLE_DIALOGUE_MARGARET() {
	loadDialogueData(DLG_CollegeCorridorMargaret);
	_updateDialogue = &IgorEngine::PART_21_UPDATE_DIALOGUE_MARGARET_1;
	handleDialogue(81, 76, 63, 32, 0);
	_updateDialogue = 0;
}

void IgorEngine::PART_21_HELPER_1_OBJECT_STATE(int num) {
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
		if (_objectsState[65] <= 1) {
			PART_21_HELPER_9();
		}
		if (_objectsState[65] == 2 || _objectsState[65] == 4) {
			_roomObjectAreasTable[9].object = 0;
			_roomObjectAreasTable[10].object = 0;
		}
	}
}

void IgorEngine::PART_21_HELPER_2_enterFromLeft() {
	_walkData[0].setPos(0, 141, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 15;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPathSimple(0, 141, 32, 137);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	PART_21_HELPER_5();
}

void IgorEngine::PART_21_HELPER_3_enterFromRight() {
	_walkData[0].setPos(319, 133, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	buildWalkPathSimple(319, 133, 239, 133);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	PART_21_HELPER_5();
}

void IgorEngine::PART_21_HELPER_4_enterFromPhysicsClass() {
	_walkData[0].setPos(119, 108, 2, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[34679]].area = 1;
	buildWalkPathSimple(119, 108, 175, 130);
	_roomObjectAreasTable[_screenLayer2[34679]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	PART_21_HELPER_5();
}

void IgorEngine::PART_21_HELPER_5() {
	// Same walk loop as waitForIgorMove(), but the room background keeps
	// animating (Margaret) while Igor walks.
	_gameTicks = 0;
	do {
		if (compareGameTick(1, 16)) {
			if (_walkDataCurrentIndex > _walkDataLastIndex) {
				_gameState.igorMoving = false;
				_walkDataLastIndex = _walkDataCurrentIndex;
			}
			if (_gameState.igorMoving) {
				moveIgor(_walkData[_walkDataCurrentIndex].posNum, _walkData[_walkDataCurrentIndex].frameNum);
				++_walkDataCurrentIndex;
			}
		}
		PART_21_UPDATE_ROOM_BACKGROUND();
		waitForTimer();
	} while (_gameState.igorMoving);
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
	copyArea(_screenVGA, offset, 320, _screenTempLayer, 100, 49, 43);
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + frame * 2107 + 0xCA7, 49, 49, 43);
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_21_HELPER_7() {
	const int offset = 21901;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + 0xE031, 33, 33, 54);
}

void IgorEngine::PART_21_HELPER_8() {
	const int offset = 21901;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + 0xE727, 33, 33, 54);
}

void IgorEngine::PART_21_HELPER_9() {
	const int offset = 28520;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + 0x14E2, 49, 49, 43);
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
	cutsceneSayStart(76, 87, 63, 32, 0, { { 212, 1, 1187 }, { 213, 1, 1188 }, { 214, 1, 1189 }, { 215, 2, 1190 }, { 217, 2, 1191 } });
	_updateDialogue = &IgorEngine::PART_21_UPDATE_DIALOGUE_MARGARET_3;
	_updateRoomBackground = 0;
	waitForEndOfCutsceneDialogue(76, 87, 63, 32, 0);
	_updateRoomBackground = &IgorEngine::PART_21_UPDATE_ROOM_BACKGROUND;
	_updateDialogue = 0;
	cutsceneSayStart(76, 87, 63, 32, 0, { { 219, 1, 1192 }, { 220, 1, 1193 }, { 221, 1, 1194 } });
	_updateDialogue = &IgorEngine::PART_21_UPDATE_DIALOGUE_MARGARET_3;
	_updateRoomBackground = 0;
	waitForEndOfCutsceneDialogue(76, 87, 63, 32, 0);
	_updateRoomBackground = &IgorEngine::PART_21_UPDATE_ROOM_BACKGROUND;
	_updateDialogue = 0;
	_objectsState[65] = 2;
	drawVerbsPanel();
	_currentAction.verb = kVerbWalk;
	drawInventory(_inventoryInfo[72], 0);
	PART_21_HELPER_1_OBJECT_STATE(255);
	_objectsState[110] = 1;
}

void IgorEngine::PART_21_HELPER_11(int frame) {
	const int offset = 28520;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + frame * 2107 + 0xCA7, 49, 49, 43);
}

void IgorEngine::PART_21_UPDATE_ROOM_BACKGROUND() {
	if (IN_ACTION_111 && compareGameTick(3, 24)) {
		const int offset = 28183;
		int i = getRandomNumber(1) + 1;
		drawAnimRect(offset, 0xBCA5 + i * 1300, 26, 50);
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
			if (_gameState.unk10 == 3 && getRandomNumber(5) != 0) {
				_gameState.unk10 = 1;
			}
		}
	}
}

void IgorEngine::PART_21() {
	playMusic(2);
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeCorridorMargaret, IMG_CollegeCorridorMargaret, BOX_CollegeCorridorMargaret, MSK_CollegeCorridorMargaret, TXT_CollegeCorridorMargaret);
	static const int anm[] = { FRM_CollegeCorridorMargaret1, FRM_CollegeCorridorMargaret2, FRM_CollegeCorridorMargaret3, FRM_CollegeCorridorMargaret4, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorMargaret);
	_roomDataOffsets = PART_21_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_21_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_21_UPDATE_ROOM_BACKGROUND;
	PART_21_HELPER_1_OBJECT_STATE(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	if (!restoreRoomAfterLoad()) {
		_gameState.unk10 = 1;
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 210) {
			PART_21_HELPER_2_enterFromLeft();
		} else if (_currentPart == 211) {
			PART_21_HELPER_3_enterFromRight();
		} else if (_currentPart == 212) {
			PART_21_HELPER_4_enterFromPhysicsClass();
		}
	}
	enterPartLoop();
	while (_currentPart >= 210 && _currentPart <= 212 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
