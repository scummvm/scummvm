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

void IgorEngine::PART_04_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_04_EXEC_ACTION %d", action);
	if (_game.flags & kFlagDemo) {
		if (action == 102 || action == 103 || action == 104) {
			ADD_DIALOGUE_TEXT(102, 2);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
			return;
		}
	}
	switch (action) {
	case 101:
		_currentPart = 120;
		break;
	case 102:
		_currentPart = 0;
		break;
	case 103:
		_currentPart = 350;
		break;
	case 104:
		_currentPart = 100;
		break;
	case 105:
		if (_objectsState[111] == 0) {
			_currentPart = 170;
		} else {
			_currentPart = 770;
		}
		break;
	case 106:
		_currentPart = 50;
		break;
	default:
		error("PART_04_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_04_CLEAR_OBJECT_STATE_84(int num) {
	_objectsState[84] = 0;
}

void IgorEngine::PART_04() {
	if (_objectsState[106] == 1) {
		_currentPart = 730;
		playMusic(1);
		return;
	}
	if (_objectsState[107] == 1) {
		_objectsState[107] = 0;
		_currentPart = 750;
		playMusic(1);
		return;
	}
	_gameState.enableLight = 1;
	loadRoomData(PAL_Map, IMG_Map, BOX_Map, MSK_Map, TXT_Map);
	loadActionData(DAT_Map);
	_roomDataOffsets = PART_04_ROOM_DATA_OFFSETS;
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_04_EXEC_ACTION);
	PART_04_CLEAR_OBJECT_STATE_84(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	fadeInPalette(768);
	_currentAction.verb = kVerbWalk;
	if (_gameState.musicNum != 2) {
		playMusic(2);
	}
	_walkData[0].x = 160;
	_walkData[0].y = 133;
	_walkData[0].scaleWidth = 49;
	_walkData[0].scaleHeight = 49;
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
	enterPartLoop();
	while (_currentPart == 40) {
		handleRoomInput();
		if (compareGameTick(19, 32)) {
			handleRoomDialogue();
		}
		if (compareGameTick(4, 8)) {
			handleRoomInventoryScroll();
		}
		scrollPalette(200, 207);
		setPaletteRange(200, 207);
		if (compareGameTick(1)) {
			handleRoomLight();
		}
		scrollPalette(184, 199);
		setPaletteRange(184, 199);
		waitForTimer();
	}
	leavePartLoop();
	fadeOutPalette(624);
}

} // namespace Igor
