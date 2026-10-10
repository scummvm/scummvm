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

void IgorEngine::PART_04_EXEC_ACTION(int action) {
    debugC(9, kDebugGame, "PART_04_EXEC_ACTION %d", action);
    switch (action) {
	case 101: // church
		_currentPart = 120;
		break;
	case 102: // students room
		_currentPart = 0;
		break;
	case 103: // park
		_currentPart = 350;
		break;
	case 104: // decanato
		_currentPart = 100;
		break;
	case 105:
		if (_objectsState[111] == 0) {
			// college entrance
			_currentPart = 170;
		} else {
			_currentPart = 770;
		}
		break;
	case 106:
		// SpringBridge
		_currentPart = 50;
		break;
	default:
		error("PART_04_EXEC_ACTION unhandled action %d", action);
		break;
	}
}


/**
 * MAP
 * */
void IgorEngine::PART_04() {

	if (_objectsState[106] == 1) { // police custcene
		_currentPart = 730;
		return;
	}

	// if philip has drunk the chemical, trigger the cutscene
	if (_objectsState[107] == 1) {
		_objectsState[107] = 0;
		_currentPart = 750;
		return;
	}
	_gameState.enableLight = 1;
	loadRoomData(PAL_Map, IMG_Map, BOX_Map, MSK_Map, TXT_Map);
	loadActionData(DAT_Map);
	_roomDataOffsets = PART_04_ROOM_DATA_OFFSETS;
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_04_EXEC_ACTION);

	// closes Philips locker
	_objectsState[84] = 0;
	memcpy(_screenVGA, _screenLayer1, 46080);

	playMusic(1);
	if (!restoreRoomAfterLoad(false)) {
		_currentAction.verb = kVerbWalk;
		fadeIn(768);
		_walkData[0].x = 160;
		_walkData[0].y = 133;
		_walkData[0].scaleWidth = 49;
		_walkData[0].scaleHeight = 49;
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
	}
	enterPartLoop();
	while (_currentPart == 40 && !_gameStateLoaded) {
		handleRoomInput();
		if (compareGameTick(19, 32)) {
			handleRoomDialogue();
		}
		if (compareGameTick(4, 8)) {
			handleRoomInventoryScroll();
		}
		// palette animation for the water
		scrollPalette(200, 207);
		setPaletteRange(200, 207);

		if (compareGameTick(1)) {
			handleIgorIdleAnimation();
		}

		scrollPalette(184, 199);
		setPaletteRange(184, 199);
		waitForTimer();
	}
	leavePartLoop();
	if (!_gameStateLoaded)
		fadeOut(624);
}

} // End of namespace Igor
