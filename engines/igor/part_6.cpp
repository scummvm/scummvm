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

void IgorEngine::PART_06_EXEC_ACTION(int action) {
debugC(9, kDebugGame, "PART_06_EXEC_ACTION %d", action);
	switch (action) {
        	case 102: // scroll right?
		    PART_06_ACTION_102();
		    break;
    default:
		error("PART_06_EXEC_ACTION unhandled action %d", action);
		break;
    }
}

void IgorEngine::PART_06_ACTION_102() {
	uint8 *walkTable1 = loadData(WLK_Bridge1);
	uint8 *walkTable2 = loadData(WLK_Bridge2);
	int xPos = 323;
	int yPos = 0;
	int i = 1;
	do {
		if (compareGameTick(1, 16)) {
			for (int y = 0; y <= 143; ++y) {
				memcpy(_screenLayer2 + y * 320 + i * 8, _screenLayer1 + y * 320, 320 - i * 8);
				memcpy(_screenLayer2 + y * 320, _animFramesBuffer + y * 224 + 224 - i * 8, i * 8);
			}
			if (i < 15) {
				xPos -= _walkScaleTable[0x8F9 + _walkCurrentFrame];
				assert(xPos >= 205);
				yPos = walkTable1[xPos - 205];
				WalkData::setNextFrame(kFacingPositionLeft, _walkCurrentFrame);
			} else {
				_walkCurrentFrame = 0;
			}
			int yOffset = (yPos - 50) * 320 + xPos - 239 + i * 8;
			for (_gameState.counter[1] = 0; _gameState.counter[1] <= 49; ++_gameState.counter[1]) {
				yOffset += 320;
				_gameState.counter[0] = yPos - 49 + _gameState.counter[1];
				for (_gameState.counter[2] = 0; _gameState.counter[2] <= 29; ++_gameState.counter[2]) {
					_gameState.counter[3] = xPos - 15 + _gameState.counter[2];
					const int offset = _gameState.counter[0] * 134 + _gameState.counter[3];
					if (_gameState.counter[0] >= 92 && _gameState.counter[0] <= 110 && offset >= 12533 && walkTable2[offset - 12533] == 1) {
						continue;
					}
					uint8 color = _facingIgorFrames[kFacingPositionLeft - 1][_walkCurrentFrame * 1500 + _gameState.counter[1] * 30 + _gameState.counter[2]];
					if (color != 0) {
						_screenLayer2[yOffset + _gameState.counter[2]] = color;
					}
				}
			}
			memcpy(_screenVGA, _screenLayer2, 46080);
			++i;
		}
		PART_06_UPDATE_ROOM_BACKGROUND();
		waitForTimer();
	} while (i != 29);
	free(walkTable1);
	free(walkTable2);
	WalkData *wd = &_walkData[0];
	wd->setPos(xPos, yPos, 4, 0);
	wd->setDefaultScale();
	_currentPart = 51;
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
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_06_EXEC_ACTION);
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
	loadActionData(DAT_SpringBridge);
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
