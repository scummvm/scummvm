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


void IgorEngine::PART_28_EXEC_ACTION(int action) {
	switch (action) {
	case 101: // leave
		_currentPart = 230;
		break;
	case 102: // read plaque
		igorSay(201, 1, 1226);
		break;
	case 103: // look at door
		igorSay(202, 1, 1227);
		break;
	case 104: // look at file
		igorSay(203, 1, 1228);
		break;
	case 105: // look at Caroline
		igorSay(204, 1, 1229);
		break;
	case 106: // look at message board
		if (_objectsState[111] == 0) {
			igorSay({ { 205, 1, 1230 }, { 206, 1, 1231 } });
		} else {
			igorSay(234, 1, 1253);
		}
		break;
	case 107: // look at plant
		igorSay(237, 1, 1256);
		break;
	case 108:
		PART_28_ACTION_108_talkCaroline();
		break;
	case 109:
		PART_28_ACTION_109_takeFolder();
		break;
	case 110: // open door
		igorSay({ { 228, 1, 1248 }, { 229, 1, 1249 } });
		break;
	case 111: // give folder to caroline
		if (_objectsState[3] == 0) {
			igorSay(226, 1, 1246);
		} else {
			igorSay(227, 1, 1247);
		}
		break;
	case 112: // use caroline
		igorSay(235, 1, 1254);
		break;
	case 113:
		igorSay(236, 1, 1255);
		break;
	default:
		error("PART_28_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

// Caroline's idle animation is only driven by the room loop and while Igor speaks;
// her own talking loops never run it, otherwise it fights the talking frames.
void IgorEngine::PART_28_ACTION_108_talkCaroline() {
	const UpdateRoomBackgroundProc updateRoomBackground = _updateRoomBackground;
	if (_objectsState[111] == 1) {
		igorSayAndWait(218, 1, 1241);
		_updateRoomBackground = 0;
		cutsceneSayStart(182, 81, 63, 17, 17, { { 211, 1, 1235 }, { 212, 1, 1236 }, { 213, 2, 1237 }, { 215, 1, 1238 }, { 216, 1, 1239 } });
		_updateDialogue = &IgorEngine::PART_28_UPDATE_DIALOGUE_CAROLINE;
		waitForEndOfCutsceneDialogue(182, 81, 63, 17, 17);
		ADD_DIALOGUE_TEXT(217, 1, 1240);
		SET_DIALOGUE_TEXT(1, 5);
		startCutsceneDialogue(182, 81, 63, 17, 17);
		waitForEndOfCutsceneDialogue(182, 81, 63, 17, 17);
		_updateDialogue = 0;
		_updateRoomBackground = updateRoomBackground;
		--_walkDataLastIndex;
		buildWalkPath(213, 131, 160, 140);
		_walkDataCurrentIndex = 1;
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkData[_walkDataLastIndex].posNum = 3;
		_gameState.igorMoving = true;
		waitForIgorMove();
		igorSay(222, 2, 1244);
		return;
	}
	if (_objectsState[87] < 2) {
		igorSayAndWait(207, 1, 1232);
	} else {
		igorSayAndWait(218, 1, 1241);
	}
	_updateRoomBackground = 0;
	if (_objectsState[87] < 2) {
		cutsceneSayWithCallback(182, 81, 63, 17, 17, 208, 1, 1233, &IgorEngine::PART_28_UPDATE_DIALOGUE_CAROLINE);
	} else {
		cutsceneSayWithCallback(182, 81, 63, 17, 17, { { 219, 2, 1242 }, { 221, 1, 1243 } }, &IgorEngine::PART_28_UPDATE_DIALOGUE_CAROLINE);
	}
	_updateRoomBackground = updateRoomBackground;
}

void IgorEngine::PART_28_ACTION_109_takeFolder() {
	for (int i = 1; i <= 7; ++i) {
		const int offset = 25067;
		drawAnimRect(offset, i * 1085 + 0x5EC9, 31, 35);
		if (i < 7) {
			waitForTimer(45);
		}
	}
	addObjectToInventory(22, 57);
	_objectsState[87] = 1;
	PART_28_HELPER_1_OBJECT_STATE(255);
	igorSay(230, 1, 1250);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
}

void IgorEngine::PART_28_UPDATE_DIALOGUE_CAROLINE(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_28_HELPER_5_drawCarolineTalkingFrame(7);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_28_HELPER_5_drawCarolineTalkingFrame(getRandomNumber(4) + 7);
		break;
	}
}

void IgorEngine::PART_28_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(61)) {
		if (_gameState.unk10 > 2) {
			PART_28_HELPER_5_drawCarolineTalkingFrame(_gameState.unk10);
			if (_gameState.unk10 < 6) {
				++_gameState.unk10;
			} else {
				_gameState.unk10 = 1;
			}
		} else {
			PART_28_HELPER_5_drawCarolineTalkingFrame(_gameState.unk10);
			if (_gameState.unk10 == 1) {
				_gameState.unk10 = 2;
			} else {
				_gameState.unk10 = 1;
			}
			if (getRandomNumber(7) == 0) {
				_gameState.unk10 = 3;
			}
		}
	}
}

void IgorEngine::PART_28_HELPER_1_OBJECT_STATE(int num) {
	WRITE_LE_UINT16(_roomActionsTable + 0x56, 42133);
	if (num == 2 || num == 255) {
		if (_objectsState[87] == 0) {
			PART_28_HELPER_8(1);
		} else {
			PART_28_HELPER_8(0);
			_roomObjectAreasTable[7].object = 0;
		}
	}
}

void IgorEngine::PART_28_HELPER_2_drawCaroline() {
	const int offset = 27374;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer, 30, 30, 44);
}

void IgorEngine::PART_28_HELPER_3_enterFromRight() {
	_walkData[0].setPos(319, 142, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(319, 142, 289, 142);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_28_HELPER_5_drawCarolineTalkingFrame(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 43; ++i) {
		for (int j = 0; j <= 29; ++j) {
			int offset = (i + 85) * 320 + j + 174;
			uint8 color = _screenVGA[offset];
			if ((color >= 192 && color <= 207) || color == 240 || color == 241) {
				_screenTempLayer[i * 100 + j] = _screenVGA[offset];
			} else {
				_screenTempLayer[i * 100 + j] = _animFramesBuffer[(frame - 1) * 1320 + i * 30 + j];
			}
		}
	}
	int offset = 27374;
	copyArea(_screenVGA, offset, 320, _screenTempLayer, 100, 30, 44);
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + (frame - 1) * 1320, 30, 30, 44);
	if (_gameState.dialogueTextRunning) {
		memcpy(_screenTextLayer + 23040, _screenLayer1 + _dialogueDirtyRectY, _dialogueDirtyRectSize);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_28_HELPER_6_enterAndCheckGrades() {
	setupDefaultPalette();
	SET_PAL_240_48_1();
	SET_PAL_208_96_1();
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	PART_28_HELPER_1_OBJECT_STATE(255);
	fadeIn(768);
	_walkData[0].setPos(319, 142, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 1;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(319, 142, 214, 130);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkData[_walkDataLastIndex].posNum = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	igorSayAndWait(209, 2, 1234);
	waitForTimer(255);
	igorSayAndWait(231, 1, 1251);
	--_walkDataLastIndex;
	buildWalkPath(214, 130, 214, 140);
	_walkDataCurrentIndex = 1;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
	igorSayAndWait({ { 224, 2, 1245 }, { 232, 2, 1252 } });
}

void IgorEngine::PART_28_HELPER_8(int frame) {
	const int offset = 33387;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + frame * 135 + 0x61F8, 15, 15, 9);
}

void IgorEngine::PART_28() {
	playMusic(2);
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeCorridorCaroline, IMG_CollegeCorridorCaroline, BOX_CollegeCorridorCaroline, MSK_CollegeCorridorCaroline, TXT_CollegeCorridorCaroline);
	static const int anm[] = { FRM_CollegeCorridorCaroline1, FRM_CollegeCorridorCaroline2, FRM_CollegeCorridorCaroline3, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorCaroline);
	_roomDataOffsets = PART_28_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(15, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_28_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_28_UPDATE_ROOM_BACKGROUND;
	PART_28_HELPER_2_drawCaroline();
	PART_28_HELPER_1_OBJECT_STATE(255);
	memcpy(_screenVGA, _screenLayer1, 46080);

	if (!restoreRoomAfterLoad()) {
		_gameState.unk10 = 1;
		_currentAction.verb = kVerbWalk;
		if (_currentPart == 280) {
			fadeIn(768);
			PART_28_HELPER_3_enterFromRight();
		} else if (_currentPart == 281) {
			PART_28_HELPER_6_enterAndCheckGrades();
		}
	}
	enterPartLoop();
	while ((_currentPart == 280 || _currentPart == 281) && !_gameStateLoaded) {
		runPartLoop();
	}
	if (!_gameStateLoaded && _objectsState[87] == 1) {
		_objectsState[87] = 2;
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}
} // End of namespace Igor
