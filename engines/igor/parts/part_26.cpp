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


static const uint8 PART_26_ANIM_DATA_1[12] = { 6, 0, 2, 3, 4, 5, 6, 5, 4, 3, 2, 0 };

void IgorEngine::PART_26_EXEC_ACTION(int action) {
	switch (action) {
	case 101: // look at plaque
		igorSay(202, 1, 1279);
		break;
	case 102: // look at door
		igorSay(203, 1, 1280);
		break;
	case 103:
		PART_26_ACTION_103_openDoor();
		break;
	case 104:
		PART_26_ACTION_104_closeDoor();
		break;
	case 105: // look at picture
		igorSay(224, 2, 1290);
		break;
	case 106: // go to mens toilets corridor
		_currentPart = 250;
		break;
	case 107:
		PART_26_ACTION_107();
		break;
	case 108: // talk to miss sullivan
		igorSay({ { 216, 2, 1284 }, { 218, 1, 1285 } });
		break;
	case 109: // take miss sullivan
		igorSay(219, 1, 1286);
		break;
	case 110: // look at miss sullivan
		igorSay({ { 220, 2, 1287 }, { 222, 1, 1288 } });
		break;
	case 111: // use miss sullivan
		igorSay(223, 1, 1289);
		break;
	default:
		error("PART_26_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_26_ACTION_103_openDoor() {
	if (_objectsState[69] == 1) {
		EXEC_MAIN_ACTION(11);
		return;
	}
	for (int i = 1; i <= 2; ++i) {
		const int offset = 19870;
		drawAnimRect(offset, 0x16C0 + i * 2706, 41, 66);
		if (i == 2) {
			playSound(13, 1);
		} else if (i == 1) {
			waitForTimer(100);
		}
	}
	_objectsState[69] = 1;
	PART_26_HELPER_1_OBJECT_STATE(1);
}

void IgorEngine::PART_26_ACTION_104_closeDoor() {
	if (_objectsState[69] == 0) {
		EXEC_MAIN_ACTION(14);
		return;
	}
	for (int i = 3; i <= 4; ++i) {
		const int offset = 19870;
		drawAnimRect(offset, i * 2706 + 0x16C0, 41, 66);
		if (i == 4) {
			playSound(14, 1);
		} else if (i == 3) {
			waitForTimer(100);
		}
	}
	_objectsState[69] = 0;
	PART_26_HELPER_1_OBJECT_STATE(1);
}

void IgorEngine::PART_26_ACTION_107() {
	if (_objectsState[69] != 0) {
		_roomObjectAreasTable[_screenLayer2[36800]].area = 1;
		--_walkDataLastIndex;
		buildWalkPath(62, 127, 0, 115);
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
		_currentPart = 160;
	}
}

void IgorEngine::PART_26_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(29) || compareGameTick(61)) {
		if (_gameState.unk10 > 2) {
			if (_gameState.unk10 == 11) {
				_gameState.unk10 = 2;
			}
			PART_26_HELPER_7_drawMissSullivanFrame(PART_26_ANIM_DATA_1[_gameState.unk10]);
			if (_gameState.unk10 > 2) {
				++_gameState.unk10;
			}
		} else if (_gameState.unk10 == 1) {
			_gameState.unk10 = 2;
			PART_26_HELPER_7_drawMissSullivanFrame(_gameState.unk10);
		} else if (getRandomNumber(24) == 0) {
			_gameState.unk10 = 1;
			PART_26_HELPER_7_drawMissSullivanFrame(_gameState.unk10);
		} else if (getRandomNumber(29) == 0) {
			_gameState.unk10 = 3;
		}
	}
}

void IgorEngine::PART_26_HELPER_1_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[69] == 0) {
			PART_26_HELPER_2_drawClosedDoor();
			_roomActionsTable[0x1E] = 6;
		} else {
			PART_26_HELPER_3_drawOpenDoor();
			_roomActionsTable[0x1E] = 7;
		}
	}
}

void IgorEngine::PART_26_HELPER_2_drawClosedDoor() {
	const int offset = 19870;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer, 25, 25, 65);
}

void IgorEngine::PART_26_HELPER_3_drawOpenDoor() {
	const int offset = 19870;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + 0x659, 25, 25, 65);
}

void IgorEngine::PART_26_HELPER_4_enterFromRight() {
	_walkData[0].setPos(319, 125, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(319, 125, 269, 140);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_26_HELPER_5_enterFromLaboratory() {
	_walkData[0].setPos(0, 115, 3, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 2;
	_walkData[0].clipWidth = 30;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[36800]].area = 1;
	buildWalkPath(0, 115, 70, 127);
	_roomObjectAreasTable[_screenLayer2[36800]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	if (_objectsState[65] == 3) {
		igorSay({ { 212, 1, 1281 }, { 213, 2, 1282 }, { 215, 1, 1283 } });
		_objectsState[65] = 4;
		drawVerbsPanel();
		redrawVerb(kVerbWalk, true);
		_currentAction.verb = kVerbWalk;
		drawInventory(_inventoryInfo[72], 0);
		playSound(51, 1);
		PART_26_HELPER_1_OBJECT_STATE(255);
	}
}

void IgorEngine::PART_26_HELPER_7_drawMissSullivanFrame(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 39; ++i) {
		for (int j = 0; j <= 21; ++j) {
			int offset = (i + 91) * 320 + j + 169;
			uint8 color = _screenVGA[offset];
			if (color < 192 || (color > 207 && color != 240 && color != 241)) {
				color = _animFramesBuffer[0x942 + frame * 880 + i * 22 + j];
			}
			_screenTempLayer[100 * i + j] = color;
		}
	}
	for (int i = 0; i <= 39; ++i) {
		const int offset = i * 320 + 29289;
		memcpy(_screenVGA + offset, _screenTempLayer + i * 100, 22);
		memcpy(_screenLayer1 + offset, _animFramesBuffer + 0x942 + frame * 880 + i * 22, 22);
	}
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::PART_26() {
	playMusic(2);
	_gameState.enableLight = 1;
	loadRoomData(PAL_CollegeCorridorMissBarrymore, IMG_CollegeCorridorMissBarrymore, BOX_CollegeCorridorMissBarrymore, MSK_CollegeCorridorMissBarrymore, TXT_CollegeCorridorMissBarrymore);
	static const int anm[] = { FRM_CollegeCorridorMissBarrymore1, FRM_CollegeCorridorMissBarrymore2, FRM_CollegeCorridorMissBarrymore3, 0 };
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorMissBarrymore);
	_roomDataOffsets = PART_26_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(14, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_26_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_26_UPDATE_ROOM_BACKGROUND;
	PART_26_HELPER_1_OBJECT_STATE(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	_gameState.unk10 = true;
	fadeIn(768);
	if (_currentPart == 260) {
		PART_26_HELPER_4_enterFromRight();
	} else {
		PART_26_HELPER_5_enterFromLaboratory();
	}
	enterPartLoop();
	while (_currentPart >= 260 && _currentPart <= 261) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOut(624);
}


} // End of namespace Igor
