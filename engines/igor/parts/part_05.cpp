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

#include "common/system.h"

#include "igor/igor.h"

namespace Igor {

void IgorEngine::PART_05_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_05_EXEC_ACTION %d", action);
	switch (action) {
	case 101:
		ADD_DIALOGUE_TEXT(201, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 102:
		PART_05_ACTION_102();
		break;
	case 103:
		PART_05_ACTION_103();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(203, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		_currentPart = 40;
		break;
	default:
		error("PART_05_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_05_ACTION_102() {
	uint8 *walkTable3 = loadData(WLK_Bridge3);
	uint8 *walkTable4 = loadData(WLK_Bridge4);
	int xPos = 220;
	int yPos = 0;
	int i = 1;
	do {
		if (compareGameTick(1, 16)) {
			for (int y = 0; y <= 143; ++y) {
				memcpy(_screenLayer2 + y * 320, _screenLayer1 + y * 320 + i * 8, 320 - i * 8);
				memcpy(_screenLayer2 + y * 320 + 320 - i * 8, _animFramesBuffer + y * 224, i * 8);
			}
			if (i < 15) {
				xPos += _walkScaleTable[0x8F9 + _walkCurrentFrame];
				assert(xPos >= 205);
				yPos = walkTable3[xPos - 205];
				WalkData::setNextFrame(kFacingPositionRight, _walkCurrentFrame);
			} else {
				_walkCurrentFrame = 0;
			}
			int yOffset = (yPos - 50) * 320 + xPos - 15 - i * 8;
			for (_gameState.counter[1] = 0; _gameState.counter[1] <= 49; ++_gameState.counter[1]) {
				yOffset += 320;
				_gameState.counter[0] = yPos - 49 + _gameState.counter[1];
				for (_gameState.counter[2] = 0; _gameState.counter[2] <= 29; ++_gameState.counter[2]) {
					_gameState.counter[3] = xPos - 15 + _gameState.counter[2];
					const int offset = _gameState.counter[0] * 134 + _gameState.counter[3];
					if (_gameState.counter[0] >= 92 && _gameState.counter[0] <= 110 && offset >= 12533 && walkTable4[offset - 12533] == 1) {
						continue;
					}
					uint8 color = _facingIgorFrames[kFacingPositionRight - 1][_walkCurrentFrame * 1500 + _gameState.counter[1] * 30 + _gameState.counter[2]];
					if (color != 0) {
						_screenLayer2[yOffset + _gameState.counter[2]] = color;
					}
				}
			}
			memcpy(_screenVGA, _screenLayer2, 46080);
			++i;
		}
		PART_05_UPDATE_ROOM_BACKGROUND();
		waitForTimer();
	} while (i != 29);
	free(walkTable3);
	free(walkTable4);
	WalkData *wd = &_walkData[0];
	wd->setPos(xPos - 224, yPos, 2, 0);
	wd->setDefaultScale();
	_currentPart = 60;
}

void IgorEngine::PART_05_ACTION_103() {
	int i = 0;
	do {
		if (compareGameTick(1)) {
			const int offset = 27526;
			for (int j = 0; j <= 48; ++j) {
				const uint8 *src = _animFramesBuffer + 0x7E18 + i * 1470 + j * 30;
				memcpy(_screenVGA + j * 320 + offset, src, 30);
			}
			++i;
		}
		PART_05_UPDATE_ROOM_BACKGROUND();
		waitForTimer();
	} while (i != 3);
	addObjectToInventory(21, 56);
	_objectsState[60] = 1;
	if (_game.version == kIdEngDemo110) {
		++_demoActionsCounter;
	}
	PART_05_HELPER_4(255);
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
		switch (getRandomNumber(200)) {
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
	if (_gameState.talkMode == kTalkModeTextOnly || !_gameState.dialogueTextRunning) {
		playSound(17, 1);
	}
}

void IgorEngine::PART_05_HELPER_1() {
	const int offset = 23521;
	for (int i = 0; i <= 48; ++i) {
		memcpy(_animFramesBuffer + 0xDEA8 + i * 23 - 1, _screenLayer1 + i * 320 + offset, 23);
		memcpy(_screenLayer1 + i * 320 + offset, _animFramesBuffer + 0x95C7 + i * 23, 23);
	}
}

void IgorEngine::PART_05_HELPER_2() {
	const int offset = 28668;
	for (int i = 0; i <= 32; ++i) {
		const uint8 *src = _animFramesBuffer + 0x7E54 + i * 23;
		memcpy(_screenLayer1 + i * 320 + offset, src, 23);
	}
}

void IgorEngine::PART_05_HELPER_3(int frame) {
	const int offset = 26756;
	for (int i = 0; i <= 5; ++i) {
		const uint8 *src = _animFramesBuffer + 0x7E00 + frame * 42 + i * 7;
		memcpy(_screenLayer1 + i * 320 + offset, src, 7);
	}
}

void IgorEngine::PART_05_HELPER_4(int num) {
	if (_objectsState[60] == 0) {
		PART_05_HELPER_5(0);
		return;
	}
	PART_05_HELPER_5(1);
	_roomObjectAreasTable[24].object = 0;
	for (int i = 27; i <= 29; ++i) {
		_roomObjectAreasTable[i].object = 0;
	}
}

void IgorEngine::PART_05_HELPER_5(int frame) {
	const int offset = 41926;
	for (int i = 0; i <= 2; ++i) {
		const uint8 *src = _animFramesBuffer + 0x7E00 + frame * 12 + i * 4;
		memcpy(_screenLayer1 + i * 320 + offset, src, 4);
	}
}

void IgorEngine::PART_05_HELPER_6() {
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

void IgorEngine::PART_05() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_SpringRock, IMG_SpringRock, BOX_SpringRock, MSK_SpringRock, TXT_SpringRock);
	SET_PAL_240_48_1();
	static const int anm2[] = { FRM_SpringRock1, FRM_SpringRock2, 0 };
	loadAnimData(anm2, 0x7E00);
	static const int anm3[] = { FRM_SpringRock3, FRM_SpringRock4, 0 };
	loadAnimData(anm3, 0x81AE);
	static const int anm4[] = { FRM_SpringRock5, FRM_SpringRock6, 0 };
	loadAnimData(anm4, 0xA763);
	if (_objectsState[61] != 0) {
		PART_05_HELPER_1();
	}
	if (_objectsState[63] != 0) {
		PART_05_HELPER_2();
	}
	if (_objectsState[62] != 0) {
		PART_05_HELPER_3(0);
	}
	for (int i = 0; i <= 143; ++i) {
		memcpy(_animFramesBuffer + i * 224, _screenLayer1 + i * 320 + 96, 224);
	}
	loadRoomData(PAL_SpringBridge, IMG_SpringBridge, BOX_SpringBridge, MSK_SpringBridge, TXT_SpringBridge);
	static const int anm1[] = { FRM_SpringBridge1, FRM_SpringBridge2, 0 };
	loadAnimData(anm1, 0x7E00);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_05_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_05_UPDATE_ROOM_BACKGROUND;
	PART_05_HELPER_4(255);
	loadActionData(DAT_SpringRock);
	_roomDataOffsets = PART_05_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 319, 143);
	_walkDataLastIndex = 1;
	_walkDataCurrentIndex = 1;
	if (_currentPart == 50) {
		PART_05_HELPER_6();
	}
	enterPartLoop();
	while (_currentPart >= 50 && _currentPart <= 52) {
		runPartLoop();
	}
	leavePartLoop();
	stopSound();
	if (_currentPart == 255) {
		fadeOutPalette(768);
	} else if (_currentPart != 60) {
		if (_objectsState[63] == 0) {
			_objectsState[61] = _objectsState[62] = _objectsState[63] = 1;
		}
		memcpy(_currentPalette, _paletteBuffer, 624);
		fadeOutPalette(624);
	}
}

} // namespace Igor
