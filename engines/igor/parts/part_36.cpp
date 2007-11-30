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

void IgorEngine::PART_36_EXEC_ACTION(int action) {
	switch (action) {
	case 101:
		ADD_DIALOGUE_TEXT(201, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 102:
		PART_36_ACTION_102();
		break;
	case 103:
		ADD_DIALOGUE_TEXT(202, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(203, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		ADD_DIALOGUE_TEXT(204, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 106:
		ADD_DIALOGUE_TEXT(209, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107:
		ADD_DIALOGUE_TEXT(206, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 108:
		ADD_DIALOGUE_TEXT(208, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 109:
		_currentPart = 242;
		break;
	default:
		error("PART_36_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_36_ACTION_102() {
	for (int i = 1; i >= 0; --i) {
		for (int j = 0; j <= 48; ++j) {
			for (int k = 0; k <= 33; ++k) {
				uint8 color = _animFramesBuffer[14 + i * 1666 + j * 34 + k];
				int offset = (j + 74) * 320 + k + 70;
				if (color < 192 || color > 207) {
					_screenTempLayer[100 * j + k] = color;
					continue;
				}
				RoomObjectArea *roa = &_roomObjectAreasTable[_screenLayer2[offset]];
				if (roa->y1Lum > 0) {
					_screenTempLayer[100 * j + k] = _screenLayer1[offset];
				} else {
					if (roa->y2Lum > 0) {
						color -= roa->deltaLum;
					}
					_screenTempLayer[100 * j + k] = color;
				}
			}
		}
		for (int j = 0; j <= 48; ++j) {
			memcpy(_screenVGA + j * 320 + 23750, _screenTempLayer + j * 100, 34);
		}
		waitForTimer(45);
	}
	addObjectToInventory(30, 65);
	_objectsState[88] = 1;
	PART_36_HELPER_1(255);
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
}

void IgorEngine::PART_36_HELPER_1(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[88] == 0) {
			PART_36_HELPER_4(1);
		} else {
			PART_36_HELPER_4(0);
			_roomObjectAreasTable[6].object = 1;
			_roomObjectAreasTable[7].object = 1;
		}
	}
}

void IgorEngine::PART_36_HELPER_2() {
	_walkData[0].setPos(319, 143, 4, 0);
	_walkData[0].setDefaultScale();
	_walkDataLastIndex = 0;
	_walkCurrentFrame = 1;
	buildWalkPath(319, 143, 259, 143);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_36_HELPER_4(int frame) {
	if (frame == 0) {
		memcpy(_screenLayer1 + 31430, _animFramesBuffer, 7);
	} else if (frame == 1) {
		memcpy(_screenLayer1 + 31430, _animFramesBuffer + 7, 7);
	}
}

void IgorEngine::PART_36() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_ChemistryClassroom, IMG_ChemistryClassroom, BOX_ChemistryClassroom, MSK_ChemistryClassroom, TXT_ChemistryClassroom);
	static const int anm[] = { FRM_ChemistryClassroom1, FRM_ChemistryClassroom2, 0 };
	loadAnimData(anm);
	loadActionData(DAT_ChemistryClassroom);
	_roomDataOffsets = PART_36_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(90, 0, 319, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_36_EXEC_ACTION);
	PART_36_HELPER_1(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeInPalette(768);
	PART_36_HELPER_2();
	enterPartLoop();
	while (_currentPart == 360) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

} // namespace Igor
