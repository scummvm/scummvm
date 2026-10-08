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
		if (getRandomNumber(14) == 0) {
			switch (getRandomNumber(3)) {
			case 0:
				playSound(21, 1);
				break;
			case 1:
				playSound(22, 1);
				break;
			case 2:
				playSound(23, 1);
				break;
			case 3:
				playSound(18, 1);
				break;
			}
		}
	}
}

void IgorEngine::PART_06_HELPER_8_animatePhotographer(int frame) {
	const int offset = 23521;
	for (int i = 0; i <= 48; ++i) {
		const uint8 *src = _animFramesBuffer + 0x95C7 + i * 23 + frame * 1127;
		memcpy(_screenVGA + i * 320 + offset, src, 23);
	}
}


void IgorEngine::PART_06_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_06_EXEC_ACTION %d", action);
	switch (action) {
	case 101:
		//Look at water
		ADD_DIALOGUE_TEXT(201, 2, 480);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 102:
		PART_06_ACTION_102_scrollLeft();
		break;
	case 103:
		PART_06_ACTION_103_talkToPhotographer();
		break;
	case 104: // look at photographer
		ADD_DIALOGUE_TEXT(203, 1, 481);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 105:
		PART_06_ACTION_105();
		break;
	case 106: // Look at camera
		ADD_DIALOGUE_TEXT(204, 1, 482);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 107:
		PART_06_ACTION_107_giveAnythingToPhotographer();
		break;
	case 108:
		PART_06_ACTION_108_giveRocketToPhotographer();
		break;
	default:
		error("PART_06_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_06_ACTION_103_talkToPhotographer() {
	ADD_DIALOGUE_TEXT(215, 1, 489);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	PART_06_HELPER_8_animatePhotographer(0);
	ADD_DIALOGUE_TEXT(216, 1, 490);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(170, 69, 55, 37, 63);
	waitForEndOfCutsceneDialogue(170, 69, 55, 37, 63);
	PART_06_HANDLE_DIALOGUE_PHOTOGRAPHER();
	PART_06_HELPER_6_setPhotographerState(255);
}

void IgorEngine::PART_06_HANDLE_DIALOGUE_PHOTOGRAPHER() {
	loadDialogueData(DLG_SpringPhotographer);
	_updateDialogue = &IgorEngine::PART_06_UPDATE_DIALOGUE_PHOTOGRAPHER;
	handleDialogue(170, 69, 55, 37, 63);
	_updateDialogue = 0;
}


void IgorEngine::PART_06_UPDATE_DIALOGUE_PHOTOGRAPHER(int action) {
	switch (action) {
	case kUpdateDialogueAnimEndOfSentence:
		PART_06_HELPER_7_decodePhotographerTalkingFrame(1);
		break;
	case kUpdateDialogueAnimMiddleOfSentence:
		PART_06_HELPER_7_decodePhotographerTalkingFrame(getRandomNumber(5) + 1);
		break;
	case kUpdateDialogueAnimStanding:
		PART_06_HELPER_7_decodePhotographerTalkingFrame(1);
		break;
	}
}

void IgorEngine::PART_06_HELPER_7_decodePhotographerTalkingFrame(int frame) {
	const uint8 *src = _animFramesBuffer + 0xA763 + READ_LE_UINT16(_animFramesBuffer + 0xDB95 + frame * 2) - 1;
	decodeAnimFrame(src, _screenVGA, true);
}

void IgorEngine::PART_06_HELPER_6_setPhotographerState(int num) {
	if (num == 2 || num == 255) {
		if (_objectsState[61] == 1) {
			PART_05_06_DRAW_PHOTOGRAPHER();
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
			// the camera becomes a hotspot once the photographer is gone
			_roomObjectAreasTable[3].object = 4;
			_roomObjectAreasTable[13].area = 3;
			_roomObjectAreasTable[15].area = 3;
			_roomObjectAreasTable[17].area = 3;
			_roomObjectAreasTable[18].area = 3;
		}
	}
	if (num == 3 || num == 255) {
		if (_objectsState[62] == 1) {
			PART_05_06_DRAW_CAMERA(0);
		} else {
			PART_05_06_DRAW_CAMERA(1);
			_roomObjectAreasTable[3].object = 0;
		}
	}
}

void IgorEngine::PART_06_ACTION_105() {
	_gameTicks = 0;
	int i = 0;
	do {
		if (compareGameTick(1)) {
			const int offset = 22568;
			for (int j = 0; j <= 48; ++j) {
				const uint8 *src = _animFramesBuffer + 0x81AE + i * 1715 + j * 35;
				memcpy(_screenVGA + 320 * j + offset, src, 35);
			}
			++i;
		}
		PART_06_UPDATE_ROOM_BACKGROUND();
		waitForTimer();
	} while (i != 3);
	addObjectToInventory(36, 71);
	_objectsState[62] = 0;
	PART_06_HELPER_6_setPhotographerState(255);
}

void IgorEngine::PART_06_ACTION_107_giveAnythingToPhotographer() {
	PART_06_HELPER_8_animatePhotographer(0);
	ADD_DIALOGUE_TEXT(205, 1, 483);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	ADD_DIALOGUE_TEXT(206, 2, 484);
	SET_DIALOGUE_TEXT(1, 1);
	startCutsceneDialogue(170, 69, 55, 37, 63);
	waitForEndOfCutsceneDialogue(170, 69, 55, 37, 63);
}

void IgorEngine::PART_06_ACTION_108_giveRocketToPhotographer() {
	PART_06_HELPER_8_animatePhotographer(0);
	ADD_DIALOGUE_TEXT(208, 2, 485);
	ADD_DIALOGUE_TEXT(210, 2, 486);
	SET_DIALOGUE_TEXT(1, 2);
	startIgorDialogue();
	waitForEndOfIgorDialogue();
	ADD_DIALOGUE_TEXT(212, 1, 487);
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
			if (i == 8) {
				stopSound();
				playSound(19, 1);
			}
		}
		PART_06_UPDATE_ROOM_BACKGROUND();
		waitForTimer();
	} while (i != 28);
	removeObjectFromInventory(61);
	_objectsState[61] = 0;
	PART_06_HELPER_6_setPhotographerState(255);
	_gameState.unkF = false;
	ADD_DIALOGUE_TEXT(213, 2, 488);
	SET_DIALOGUE_TEXT(1, 1);
	startIgorDialogue();
}

void IgorEngine::PART_06_ACTION_102_scrollLeft() {
	uint8 *walkTable1 = loadData(WLK_Bridge1);
	uint8 *walkTable2 = loadData(WLK_Bridge2);
	int xPos = 323;
	int yPos = 0;
	int i = 1;
	_gameTicks = 15 & ~(kTimerTicksCount - 1);
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

void IgorEngine::PART_06_HELPER_12() {

	const int offset = 23521;
	for (int i = 0; i <= 48; ++i) {
		const uint8 *src = _animFramesBuffer + 0xDEA7 + i * 23;
		memcpy(_screenLayer1 + i * 320 + offset, src, 23);
	}
}

void IgorEngine::PART_06_HELPER_15(int frame) {
	const uint8 *src = _animFramesBuffer + 0xA763 + READ_LE_UINT16(_animFramesBuffer + 0xDB95 + frame * 2) - 1;
	decodeAnimFrame(src, _screenVGA, true);
}

void IgorEngine::PART_06() {
	debug("Entering PART_06");
	_gameState.enableLight = 1;

	loadRoomData(PAL_SpringBridge, IMG_SpringBridge, BOX_SpringBridge, MSK_SpringBridge, TXT_SpringBridge);
	static const int anm1[] = {FRM_SpringBridge1, FRM_SpringBridge2, 0};
	// loads from offset 32256
	loadAnimData(anm1, 0x7E00);
	if (_objectsState[60] == 0) {
		PART_05_06_DRAW_PAPER(0);
	}
	// copying a patch of 224 pixels width and 144 height into the backup buffer for later scroll
	// then it loads the actual current scene, SpringRock
	for (int i = 0; i <= 143; ++i) {
		memcpy(_animFramesBuffer + i * 224, _screenLayer1 + i * 320, 224);
	}
	loadRoomData(PAL_SpringRock, IMG_SpringRock, BOX_SpringRock, MSK_SpringRock, TXT_SpringRock);
	SET_PAL_240_48_1();
	// the room palette resource is longer than the palette the room really uses, restore the inventory colors
	SET_PAL_208_96_1();

	static const int anm2[] = {FRM_SpringRock1, FRM_SpringRock2, 0};
	loadAnimData(anm2, 0x7E00);
	static const int anm3[] = {FRM_SpringRock3, FRM_SpringRock4, 0};
	loadAnimData(anm3, 0x81AE);
	static const int anm4[] = {FRM_SpringRock5, FRM_SpringRock6, 0};
	loadAnimData(anm4, 0xA763);

	PART_05_06_SAVE_PHOTOGRAPHER_BACKGROUND();
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_06_EXEC_ACTION);
	_updateRoomBackground = &IgorEngine::PART_06_UPDATE_ROOM_BACKGROUND;
	PART_06_HELPER_6_setPhotographerState(255);

	if (_objectsState[63] == 1) {
		PART_05_06_DRAW_TRIPOD(true);
	}

	if (_currentPart == 61 && !_gameStateLoaded) {
		SET_PAL_208_96_1();
		drawVerbsPanel();
		drawInventory(1, 0);
		_currentAction.verb = kVerbWalk;
		memcpy(_paletteBuffer, _currentPalette, 624);
		// Part 61 is arriving from intro so it doesnt need to copy the screen buffer
		fadeIn(768);
	}
	loadActionData(DAT_SpringBridge);
	_roomDataOffsets = PART_06_ROOM_DATA_OFFSETS;
	if (!restoreRoomAfterLoad()) {
		_gameState.unkF = (_objectsState[61] == 1);
		_walkDataLastIndex = 1;
		_walkDataCurrentIndex = 1;
	}
	enterPartLoop();

	while (_currentPart >= 60 && _currentPart <= 61 && !_gameStateLoaded) {
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

		if (compareGameTick(61) && _gameState.unkF && getRandomNumber(9) == 0) {
			PART_06_HELPER_8_animatePhotographer(getRandomNumber(3));
		}

		waitForTimer();
	}
	leavePartLoop();
	if (_currentPart == 255 && !_gameStateLoaded) {
		fadeOut(768);
	}
}

} // End of namespace Igor
