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


void IgorEngine::PART_06_UPDATE_ROOM_BACKGROUND() {
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
		switch (getRandomNumber(20)) {
		case 0:
			playSound(18, 1);
			break;
		case 3:
			playSound(21, 1);
			break;
		case 9:
			playSound(22, 1);
			break;
		case 17:
			playSound(23, 1);
			break;
		}
	}
}

void IgorEngine::PART_06_HELPER_1(int frame) {
	const int offset = 41926;
	for (int i = 0; i <= 2; ++i) {
		const uint8 *src = _animFramesBuffer + 0x7E00 + frame * 12 + i * 4;
		memcpy(_screenLayer1 + i * 320 + offset, src, 4);
	}
}

void IgorEngine::PART_06_HELPER_2() {
	const int offset = 23521;
	for (int i = 0; i <= 48; ++i) {
		const uint8 *src = _screenLayer1 + i * 320 + offset;
		memcpy(_animFramesBuffer + 0xDEA8 + i * 23 - 1, src, 23);
	}
}


void IgorEngine::PART_06_HELPER_3() {
	const int offset = 28668;
	for (int i = 0; i <= 32; ++i) {
		const uint8 *src = _animFramesBuffer + 0x7E54 + i * 23;
		memcpy(_screenLayer1 + i * 320 + offset, src, 23);
		memcpy(_screenVGA + i * 320 + offset, src, 23);
	}
}

void IgorEngine::PART_06_HELPER_8(int frame) {
	const int offset = 23521;
	for (int i = 0; i <= 48; ++i) {
		const uint8 *src = _animFramesBuffer + 0x95C7 + i * 23 + frame * 1127;
		memcpy(_screenVGA + i * 320 + offset, src, 23);
	}
}

void IgorEngine::PART_06() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_SpringBridge, IMG_SpringBridge, BOX_SpringBridge, MSK_SpringBridge, TXT_SpringBridge);
	static const int anm1[] = {FRM_SpringBridge1, FRM_SpringBridge2, 0};
	loadAnimData(anm1, 0x7E00);
	if (_objectsState[60] == 0) {
		PART_06_HELPER_1(0);
	}
    for (int i = 0; i <= 143; ++i) {
		memcpy(_animFramesBuffer + i * 224, _screenLayer1 + i * 320, 224);
	}
    loadRoomData(PAL_SpringRock, IMG_SpringRock, BOX_SpringRock, MSK_SpringRock, TXT_SpringRock);
	SET_PAL_240_48_1();
	static const int anm2[] = { FRM_SpringRock1, FRM_SpringRock2, 0 };
	loadAnimData(anm2, 0x7E00);
	static const int anm3[] = { FRM_SpringRock3, FRM_SpringRock4, 0 };
	loadAnimData(anm3, 0x81AE);
	static const int anm4[] = { FRM_SpringRock5, FRM_SpringRock6, 0 };
	loadAnimData(anm4, 0xA763);
	PART_06_HELPER_2();
	// SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_06_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_06_UPDATE_ROOM_BACKGROUND;
    // PART_06_HELPER_6(255);
	if (_objectsState[63] == 1) {
		PART_06_HELPER_3();
	}
	if (_currentPart == 61) {
		SET_PAL_208_96_1();
		drawVerbsPanel();
		// drawInventory(1, 0);
		_currentAction.verb = kVerbWalk;
		memcpy(_paletteBuffer, _currentPalette, 624);
		fadeIn(768);
	}
	// loadActionData(DAT_SpringBridge);
    _roomDataOffsets = PART_06_ROOM_DATA_OFFSETS;
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
	_gameState.unkF = (_objectsState[61] == 1);
	enterPartLoop();
    while (_currentPart >= 60 && _currentPart <= 61) {
		setRoomWalkBounds(0, 0, _objectsState[61] == 0 ? 234 : 142, 143);
		handleRoomInput();
		if (compareGameTick(1, 16)) {
			handleRoomIgorWalk();
		}
		// if (compareGameTick(19, 32)) {
		// 	handleRoomDialogue();
		// }
		// if (compareGameTick(4, 8)) {
		// 	handleRoomInventoryScroll();
		// }
		// if (compareGameTick(1)) {
		// 	handleRoomLight();
		// }
		PART_06_UPDATE_ROOM_BACKGROUND();
		if (compareGameTick(61) && _gameState.unkF && getRandomNumber(10) == 0) {
			PART_06_HELPER_8(getRandomNumber(4));
		}
		waitForTimer();
	}
	leavePartLoop();
}


} // End of namespace Igor
