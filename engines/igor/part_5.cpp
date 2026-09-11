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

void IgorEngine::PART_05() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_SpringRock, IMG_SpringRock, BOX_SpringRock, MSK_SpringRock, TXT_SpringRock);
	SET_PAL_240_48_1();

	loadRoomData(PAL_SpringBridge, IMG_SpringBridge, BOX_SpringBridge, MSK_SpringBridge, TXT_SpringBridge);
	_updateRoomBackground = &IgorEngine::PART_05_UPDATE_ROOM_BACKGROUND;

	//Enforce display of room
	{
		memcpy(_screenVGA, _screenLayer1, 46080);
		drawVerbsPanel();
		fadeInPalette(720);
	}


	setRoomWalkBounds(0, 0, 319, 143);
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;

	{
		_walkData[0].setPos(0, 141, 2, 0);
		_walkData[0].setDefaultScale();
		_walkDataLastIndex = 0;
		_walkDataCurrentIndex = 1;
		buildWalkPath(0, 141, 51, 123);
		_walkData[_walkDataLastIndex].frameNum = 0;
		_walkDataCurrentIndex = 1;
		_gameState.igorMoving = true;
		waitForIgorMove();
	}

	enterPartLoop();
	while (_currentPart >= 50 && _currentPart <= 52) {
		runPartLoop();
	}
	leavePartLoop();
	// if (_currentPart == 255) {
	// 	fadeOutPalette(768);
	// } else if (_currentPart != 60) {
	// 	// if (_objectsState[63] == 0) {
	// 	// 	_objectsState[61] = _objectsState[62] = _objectsState[63] = 1;
	// 	// }
	// 	memcpy(_currentPalette, _paletteBuffer, 624);
	// 	fadeOutPalette(624);
	// }
}

void IgorEngine::PART_05_UPDATE_ROOM_BACKGROUND() {
	if (compareGameTick(61)) {
		scrollPalette(160, 167);
		setPaletteRange(160, 167);
	}
	if (compareGameTick(2, 8)) {
		scrollPalette(168, 178);
		setPaletteRange(168, 178);
	}
	if (compareGameTick(13, 16)) {
		scrollPalette(179, 184);
		setPaletteRange(179, 184);
	}
	if (compareGameTick(5, 32)) {
		scrollPalette(185, 191);
		setPaletteRange(185, 191);
	}
	if (compareGameTick(1)) {
		// switch (getRandomNumber(200)) {
		// case 0:
		// 	playSound(18, 1);
		// 	break;
		// case 3:
		// 	playSound(21, 1);
		// 	break;
		// case 9:
		// 	playSound(22, 1);
		// 	break;
		// case 17:
		// 	playSound(23, 1);
		// 	break;
		// }
	}
	// if (_gameState.talkMode == kTalkModeTextOnly || !_gameState.dialogueTextRunning) {
	// 	playSound(17, 1);
	// }
}

} // End of namespace Igor
