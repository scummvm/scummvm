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

void IgorEngine::PART_06_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_06_EXEC_ACTION %d", action);
	switch (action) {
	case 101:
		ADD_DIALOGUE_TEXT(201, 2);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 102:
		PART_06_ACTION_102();
		break;
	case 103:
		PART_06_ACTION_103();
		break;
	case 104:
		ADD_DIALOGUE_TEXT(203, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		PART_06_ACTION_105();
		break;
	case 106:
		ADD_DIALOGUE_TEXT(204, 1);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107:
		PART_06_ACTION_107();
		break;
	case 108:
		PART_06_ACTION_108();
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

void IgorEngine::PART_06_ACTION_103() {
	ADD_DIALOGUE_TEXT(215, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	PART_06_HELPER_8(0);
	ADD_DIALOGUE_TEXT(216, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(170, 69, 55, 37, 63);
	waitForEndOfCutsceneDialogue(170, 69, 55, 37, 63);
	PART_06_HANDLE_DIALOGUE_PHOTOGRAPHER();
	PART_06_HELPER_6(255);
}

void IgorEngine::PART_06_ACTION_105() {
	_gameTicks = 0;
	for (int i = 0; i <= 3; ++i) {
		if (compareGameTick(1)) {
			const int offset = 22568;
			for (int j = 0; j <= 48; ++j) {
				const uint8 *src = _animFramesBuffer + 0x81AE + i * 1715 + j * 35;
				memcpy(_screenVGA + 320 * j + offset, src, 35);
			}
		}
		PART_06_UPDATE_ROOM_BACKGROUND();
		waitForTimer();
	}
	addObjectToInventory(36, 71);
	_objectsState[62] = 0;
	PART_06_HELPER_6(255);
}

void IgorEngine::PART_06_ACTION_107() {
	PART_06_HELPER_8(0);
	ADD_DIALOGUE_TEXT(205, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	ADD_DIALOGUE_TEXT(206, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(170, 69, 55, 37, 63);
	waitForEndOfCutsceneDialogue(170, 69, 55, 37, 63);
}

void IgorEngine::PART_06_ACTION_108() {
	PART_06_HELPER_8(0);
	ADD_DIALOGUE_TEXT(208, 2);
	ADD_DIALOGUE_TEXT(210, 2);
	SET_DIALOGUE_TEXT(1, 2);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	ADD_DIALOGUE_TEXT(212, 1);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(170, 69, 55, 37, 63);
	waitForEndOfCutsceneDialogue(170, 69, 55, 37, 63);
	int i = 7;
	_gameTicks = 0;
	do {
		if (compareGameTick(1, 32)) {
			const uint8 *src = _animFramesBuffer + 0xA763 + READ_LE_UINT16(_animFramesBuffer + 0xDB95 + i * 2) - 1;
			decodeAnimFrame(src, _screenVGA, true);
			++i;
		}
		PART_06_UPDATE_ROOM_BACKGROUND();
		if (i == 7) {
			stopSound();
			playSound(19, 1);
		}
		waitForTimer();
	} while (i != 28);
	removeObjectFromInventory(61);
	_objectsState[61] = 0;
	PART_06_HELPER_6(255);
	_gameState.unkF = false;
	ADD_DIALOGUE_TEXT(213, 2);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
}

void IgorEngine::PART_06_UPDATE_DIALOGUE_PHOTOGRAPHER(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_06_HELPER_7(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_06_HELPER_7(getRandomNumber(6) + 1);
		break;
	case kUpdateDialogueAnimStanding:
		PART_06_HELPER_7(1);
		break;
	}
}

void IgorEngine::PART_06_HANDLE_DIALOGUE_PHOTOGRAPHER() {
	loadDialogueData(DLG_SpringPhotographer);
	_updateDialogue = &IgorEngine::PART_06_UPDATE_DIALOGUE_PHOTOGRAPHER;
	handleDialogue(170, 69, 55, 37, 63);
	_updateDialogue = 0;
}

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

void IgorEngine::PART_06_HELPER_6(int num) {
	if (num == 2 || num == 255) {
		if (_objectsState[61] == 1) {
			PART_06_HELPER_14();
			_roomObjectAreasTable[3].object = 0;
			_roomObjectAreasTable[13].area = 0;
			_roomObjectAreasTable[15].area = 0;
			_roomObjectAreasTable[17].area = 0;
			_roomObjectAreasTable[18].area = 0;
		} else {
			PART_06_HELPER_12();
			_roomObjectAreasTable[2].object = 0;
			_roomObjectAreasTable[15].object = 0;
			_roomObjectAreasTable[18].object = 0;
			_roomObjectAreasTable[3].object = 0;
			_roomObjectAreasTable[13].area = 3;
			_roomObjectAreasTable[15].area = 3;
			_roomObjectAreasTable[17].area = 3;
			_roomObjectAreasTable[18].area = 3;
		}
	}
	if (num == 3 || num == 255) {
		if (_objectsState[62] == 1) {
			PART_06_HELPER_13(0);
		} else {
			PART_06_HELPER_13(1);
			_roomObjectAreasTable[3].object = 0;
		}
	}
}

void IgorEngine::PART_06_HELPER_7(int frame) {
	const uint8 *src = _animFramesBuffer + 0xA763 + READ_LE_UINT16(_animFramesBuffer + 0xDB95 + frame * 2) - 1;
	decodeAnimFrame(src, _screenVGA, true);
}

void IgorEngine::PART_06_HELPER_8(int frame) {
	const int offset = 23521;
	for (int i = 0; i <= 48; ++i) {
		const uint8 *src = _animFramesBuffer + 0x95C7 + i * 23 + frame * 1127;
		memcpy(_screenVGA + i * 320 + offset, src, 23);
	}
}

void IgorEngine::PART_06_HELPER_12() {
	const int offset = 23521;
	for (int i = 0; i <= 48; ++i) {
		const uint8 *src = _animFramesBuffer + 0xDEA7 + i * 23;
		memcpy(_screenLayer1 + i * 320 + offset, src, 23);
	}
}

void IgorEngine::PART_06_HELPER_13(int frame) {
	const int offset = 26756;
	for (int i = 0; i <= 5; ++i) {
		const uint8 *src = _animFramesBuffer + 0x7E00 + frame * 42 + i * 7;
		memcpy(_screenLayer1 + i * 320 + offset, src, 7);
	}
}

void IgorEngine::PART_06_HELPER_14() {
	const int offset = 23521;
	for (int i = 0; i <= 48; ++i) {
		const uint8 *src = _animFramesBuffer + 0x95C7 + i * 23;
		memcpy(_screenLayer1 + i * 320 + offset, src, 23);
	}
}

void IgorEngine::PART_06_HELPER_15(int frame) {
	const uint8 *src = _animFramesBuffer + 0xA763 + READ_LE_UINT16(_animFramesBuffer + 0xDB95 + frame * 2) - 1;
	decodeAnimFrame(src, _screenVGA, true);
}

void IgorEngine::PART_06() {
	_gameState.enableLight = 1;
	loadRoomData(PAL_SpringBridge, IMG_SpringBridge, BOX_SpringBridge, MSK_SpringBridge, TXT_SpringBridge);
	static const int anm1[] = { FRM_SpringBridge1, FRM_SpringBridge2, 0 };
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
	PART_06_HELPER_6(255);
	if (_objectsState[63] == 1) {
		PART_06_HELPER_3();
	}
	if (_currentPart == 61) {
		SET_PAL_208_96_1();
		drawVerbsPanel();
		drawInventory(1, 0);
		_currentAction.verb = kVerbWalk;
		memcpy(_paletteBuffer, _currentPalette, 624);
		fadeInPalette(768);
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
		if (compareGameTick(19, 32)) {
			handleRoomDialogue();
		}
		if (compareGameTick(4, 8)) {
			handleRoomInventoryScroll();
		}
		if (compareGameTick(1)) {
			handleRoomLight();
		}
		PART_06_UPDATE_ROOM_BACKGROUND();
		if (compareGameTick(61) && _gameState.unkF && getRandomNumber(10) == 0) {
			PART_06_HELPER_8(getRandomNumber(4));
		}
		waitForTimer();
	}
	leavePartLoop();
}

} // namespace Igor
