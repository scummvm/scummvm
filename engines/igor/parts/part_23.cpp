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

static const uint8 PART_23_ANIM_DATA_1[3] = { 0, 4, 1 };

void IgorEngine::part23ExecuteAction(int action) {
	switch (action) {
	case 101:
		_currentPart = 280;
		break;
	case 102: // look at portrait
		igorSay(201, 2, 1217);
		break;
	case 105:
		part23EnterLadiesRoom();
		break;
	case 106: // look at door
		igorSay(205, 1, 1220);
		break;
	case 107: // open door
		part23OpenDoor();
		break;
	case 108: // close door
		part23CloseDoor();
		break;
	case 109: // go through right corridor
		_currentPart = 300;
		break;
	case 110: // Look at Lucas
		igorSay({ { 203, 1, 1218 }, { 204, 1, 1219 } });
		break;
	case 111:
		igorSay(208, 2, 1222);
		break;
	case 112:
		igorSay({ { 210, 1, 1223 }, { 211, 1, 1224 } });
		break;
	case 113:
		igorSay(212, 1, 1225);
		break;
	default:
		error("part23ExecuteAction unhandled action %d", action);
		break;
	}
}

void IgorEngine::part23EnterLadiesRoom() {
	if (_objectsState[kLadiesRoomDoorStateIndex] != 0) {
		_roomObjectAreasTable[_screenLayer2[38633]].area = 1;
		--_walkDataLastIndex;
		buildWalkPathSimple(233, 132, 233, 120);
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
		_currentPart = 190;
	}
}

void IgorEngine::part23OpenDoor() {
	if (_objectsState[kLadiesRoomDoorStateIndex] == 1) {
		partMainExecuteAction(11);
		return;
	}
	const int offset = 23901;
	for (int i = 2; i <= 3; ++i) {
		drawAnimRect(offset, i * 1620 + 0x895, 30, 54);
		if (i == 3) {
			playSound(13, 1);
		}
		if (i < 3) {
			waitForTimer(100);
		}
	}
	_objectsState[kLadiesRoomDoorStateIndex] = 1;
	part23ApplyObjectState(1);
}

void IgorEngine::part23CloseDoor() {
	if (_objectsState[kLadiesRoomDoorStateIndex] == 0) {
		partMainExecuteAction(14);
		return;
	}
	const int offset = 23901;
	for (int i = 1; i <= 2; ++i) {
		drawAnimRect(offset, PART_23_ANIM_DATA_1[i] * 1620 + 0x895, 30, 54);
		if (i == 2) {
			playSound(14, 1);
		}
		if (i < 2) {
			waitForTimer(100);
		}
	}
	_objectsState[kLadiesRoomDoorStateIndex] = 0;
	part23ApplyObjectState(1);
}

void IgorEngine::part23UpdateBackground() {
	if (compareGameTick(29) || compareGameTick(61)) {
		int rnd = getRandomNumber(10);
		if (rnd <= 4) {
			_gameState.unk10 = 1;
		} else if (rnd <= 9) {
			_gameState.unk10 = 2;
		} else if (rnd == 10) {
			if (_gameState.unk10 < 3) {
				_gameState.unk10 = getRandomNumber(1) + 3;
			} else {
				_gameState.unk10 = getRandomNumber(1) + 1;
			}
		}
		part23DrawLucasFrame(_gameState.unk10);
	}
}

void IgorEngine::part23ApplyObjectState(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[kLadiesRoomDoorStateIndex] == 0) {
			part23DrawDoor(1);
			_roomActionsTable[4] = 6;
		} else {
			part23DrawDoor(2);
			_roomActionsTable[4] = 7;
		}
	}
	part23DrawLucas();
}

void IgorEngine::part23DrawDoor(int frame) {
	const int offset = 23907;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + (frame - 1) * 1296, 24, 24, 54);
}

void IgorEngine::part23DrawLucas() {
	const int offset = 25763;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + 0xA20, 25, 25, 49);
}

void IgorEngine::part23WalkFromDoor() {
	_walkData[0].setPos(233, 120, 3, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[38633]].area = 1;
	buildWalkPathSimple(233, 120, 233, 137);
	_roomObjectAreasTable[_screenLayer2[38633]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	if (_objectsState[kToiletGirlsHaveLeftStateIndex] == 0) {
		drawAnimRect(23907, 0, 24, 54, false, kBlendBehindIgor);
		playSound(14, 1);
		_objectsState[kLadiesRoomDoorStateIndex] = 0;
		part23ApplyObjectState(1);
		igorSay(206, 2, 1221);
	}
}

void IgorEngine::part23WalkFromLeft() {
	_walkData[0].setPos(0, 134, 2, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipSkipX = 15;
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(0, 134, 100, 140);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part23WalkFromRight() {
	_walkData[0].setPos(319, 134, 4, 0);
	_walkData[0].setDefaultScale();
	_walkData[0].clipWidth = 15;
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPathSimple(319, 134, 289, 134);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::part23DrawLucasFrame(int frame) {
	_roomCursorOn = false;
	for (int i = 0; i <= 18; ++i) {
		for (int j = 0; j <= 14; ++j) {
			int offset = (i + 80) * 320 + j + 162;
			uint8 color = _screenVGA[offset];
			if ((color >= 192 && color <= 207) || color == 240 || color == 241) {
				_screenTempLayer[i * 100 + j] = color;
			} else {
				_screenTempLayer[i * 100 + j] = _animFramesBuffer[frame * 285 + i * 15 + j + 0x271C];
			}
		}
	}
	int offset = 25762;
	copyArea(_screenVGA, offset, 320, _screenTempLayer, 100, 15, 19);
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + frame * 285 + 0x271C, 15, 15, 19);
	if (_dialogueCursorOn) {
		_roomCursorOn = true;
	}
}

void IgorEngine::part23() {
	playMusic(2);
	_gameState.enableLight = 2;
	loadRoomData(PAL_CollegeCorridorLucas, IMG_CollegeCorridorLucas, BOX_CollegeCorridorLucas, MSK_CollegeCorridorLucas, TXT_CollegeCorridorLucas);
	static const int anm[] = {
		FRM_CollegeCorridorLucas1, FRM_CollegeCorridorLucas2,
		FRM_CollegeCorridorLucas3, FRM_CollegeCorridorLucas4,
		0
	};
	loadAnimData(anm);
	loadActionData(DAT_CollegeCorridorLucas);
	_roomDataOffsets = PART_23_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(15, 0, 319, 143);

	partMainSetExecuteAction(1, &IgorEngine::part23ExecuteAction);
	_updateRoomBackground = &IgorEngine::part23UpdateBackground;
	part23ApplyObjectState(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	if (!restoreRoomAfterLoad()) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		if (_currentPart == 230) {
			part23WalkFromLeft();
		} else if (_currentPart == 231) {
			part23WalkFromRight();
		} else if (_currentPart == 232) {
			part23WalkFromDoor();
		}
	}
	enterPartLoop();
	while (_currentPart >= 230 && _currentPart <= 232 && !_gameStateLoaded) {
		runPartLoop();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
