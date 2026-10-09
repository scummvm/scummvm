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


static const uint8 PART_14_ANIM_DATA_1[6] = { 0, 2, 5, 1, 8, 3 };

static int VAR_NEW_CHURCH_MOSAIC_STONE;
static int VAR_CURRENT_CHURCH_MOSAIC_STONE;

void IgorEngine::PART_14_EXEC_ACTION(int action) {
	debugC(9, kDebugGame, "PART_14_EXEC_ACTION %d", action);
	switch (action) {
	case 101:
		PART_14_ACTION_101_goToChurchBell();
		break;
	case 102: // look at stairs
		igorSay(201, 1, 860);
		break;
	case 103:
		PART_14_ACTION_103_lookAtPuzzle();
		break;
	case 104: // look at candles
		igorSay(202, 1, 861);
		break;
	case 105:
		PART_14_ACTION_105_goBackToChurch();
		break;
	case 106:
		PART_14_ACTION_106_enterMaze();
		break;
	case 107:
		igorSay(203, 1, 862);
		break;
	case 108:
		PART_14_ACTION_108_useMatchesOnCandles();
		break;
	default:
		error("PART_14_EXEC_ACTION unhandled action %d", action);
		break;
	}
}

void IgorEngine::PART_14_ACTION_101_goToChurchBell() {
	_roomObjectAreasTable[_screenLayer2[22156]].area = 1;
	_roomObjectAreasTable[_screenLayer2[27895]].area = 1;
	--_walkDataLastIndex;
	buildWalkPath(50, 103, 55, 87);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	for (int i = 192 * 3; i <= 207 * 3; ++i) {
		if (_paletteBuffer[i] > 5) {
			_currentPalette[i] -= 5;
		} else {
			_currentPalette[i] = 0;
		}
	}
	setPaletteRange(192, 207);
	--_walkDataLastIndex;
	buildWalkPath(55, 87, 76, 69);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_currentPart = 220;
}

void IgorEngine::PART_14_ACTION_103_lookAtPuzzle() {
	fadeOut(768);
	PART_14_HELPER_9_showPuzzle();
	loadResourceData__ROOM_ChurchPuzzle();
	memset(_screenVGA + 46080, 0, 17920);
	drawVerbsPanel();
	drawInventory(_inventoryInfo[72], 0);
	fadeIn(768);
	if (_currentPart == 145) {
		for (int i = 1; i <= 10; ++i) {
			const uint8 *src = _animFramesBuffer + 0x9486 + READ_LE_UINT16(_animFramesBuffer + 0xCCE4 + i * 2) - 1;
			decodeAnimFrame(src, _screenVGA, true);
			waitForTimer(60);
		}
		stopSound();
		_objectsState[50] = 1;
	}
	PART_14_HELPER_1_OBJECT_STATE(255);
	_currentPart = 140;
}

void IgorEngine::PART_14_ACTION_105_goBackToChurch() {
	_walkDataCurrentIndex = 0;
	for (int i = 9; i >= 0; --i) {
		WalkData *wd = &_walkData[0];
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		wd->setPos(182, 143, 3, _walkCurrentFrame);
		WalkData::setNextFrame(3, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 30;
		wd->scaleWidth = 23 + i * 3;
		wd->xPosChanged = 1;
		wd->dxPos = 0;
		wd->yPosChanged = 1;
		wd->dyPos = 3;
		wd->scaleHeight = 50;
		moveIgor(wd->posNum, wd->frameNum);
		waitForTimer(15);
	}
	_currentPart = 131;
}

void IgorEngine::PART_14_ACTION_106_enterMaze() {
	--_walkDataLastIndex;
	_roomObjectAreasTable[_screenLayer2[35684]].area = 2;
	buildWalkPath(164, 126, 164, 111);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	for (int i = 192 * 3; i <= 207 * 3; ++i) {
		if (_paletteBuffer[i] > 5) {
			_currentPalette[i] -= 5;
		} else {
			_currentPalette[i] = 0;
		}
	}
	setPaletteRange(192, 207);
	_walkDataCurrentIndex = 0;
	for (int i = 9; i >= 0; --i) {
		WalkData *wd = &_walkData[0];
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		wd->setPos(164, 111, 1, _walkCurrentFrame);
		WalkData::setNextFrame(1, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 30;
		wd->scaleWidth = 23 + i * 3;
		wd->xPosChanged = 1;
		wd->dxPos = 0;
		wd->yPosChanged = 1;
		wd->dyPos = 3;
		wd->scaleHeight = 50;
		moveIgor(wd->posNum, wd->frameNum);
		waitForTimer(15);
	}
	_objectsState[71] = 0;
	_currentPart = 670;
}

// cutscene where the brother gives us the papyrus
void IgorEngine::PART_14_ACTION_108_useMatchesOnCandles() {
	_gameState.unkF = false;
	_updateRoomBackground = &IgorEngine::PART_14_UPDATE_ROOM_BACKGROUND_ACTION_108;
	igorSayAndWait(226, 1, 880);
	removeObjectFromInventory(43);
	for (int i = 1; i <= 3; ++i) {
		const uint8 *src = _animFramesBuffer + 0x2AAA + READ_LE_UINT16(_animFramesBuffer + 0x940C + i * 2) - 1;
		decodeAnimFrame(src, _screenVGA, true);
		waitForTimer(60);
	}
	_gameState.unkF = true;
	playSound(33, 1);
	PART_14_HELPER_8_brotherReveal(4, 24);
	stopSound();
	cutsceneSay(221, 85, 12, 33, 63, 205, 1, 864);
	PART_14_HELPER_8_brotherReveal(25, 26);
	PART_14_HELPER_8_brotherReveal(60, 60);
	WalkData *wd = &_walkData[_walkDataLastIndex - 1];
	wd->x = 250;
	wd->y = 138;
	wd->posNum = 4;
	igorSayAndWait({ { 206, 1, 865 }, { 207, 1, 866 } });
	cutsceneSay(221, 85, 12, 33, 63, 208, 2, 867);
	igorSayAndWait(210, 1, 868);
	cutsceneSay(221, 85, 12, 33, 63, { { 211, 2, 869 }, { 213, 3, 870 }, { 216, 1, 871 } });
	for (int i = 33; i <= 35; ++i) {
		const uint8 *src = _animFramesBuffer + 0x2AAA + READ_LE_UINT16(_animFramesBuffer + 0x940C + i * 2) - 1;
		decodeAnimFrame(src, _screenVGA, true);
		waitForTimer(60);
	}
	addObjectToInventory(28, 63);
	igorSayAndWait({ { 217, 1, 872 }, { 218, 1, 873 }, { 219, 1, 874 } });
	cutsceneSay(221, 85, 12, 33, 63, 220, 2, 875);
	PART_14_HELPER_7_candleFlicker(0);
	_gameState.unkF = false;
	_screenVGA[32865] = _screenLayer1[32865];
	playSound(33, 1);
	PART_14_HELPER_8_brotherReveal(36, 60);
	stopSound();
	igorSayAndWait({ { 222, 1, 876 }, { 223, 1, 877 }, { 224, 1, 878 }, { 225, 1, 879 } });
	PART_14_HELPER_1_OBJECT_STATE(255);
	_updateRoomBackground = 0;
}

void IgorEngine::PART_14_UPDATE_ROOM_BACKGROUND_ACTION_108() {
	if (compareGameTick(2, 16) && _gameState.unkF) {
		PART_14_HELPER_7_candleFlicker(getRandomNumber(1) + 1);
	}
}

void IgorEngine::PART_14_HELPER_1_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		if (_objectsState[50] == 1) {
			PART_14_HELPER_6_drawEntrance();
			_roomObjectAreasTable[3].object = 5;
			_roomObjectAreasTable[4].object = 5;
			_roomObjectAreasTable[7].object = 5;
			_roomObjectAreasTable[8].object = 5;
		} else {
			_roomObjectAreasTable[3].object = 0;
			_roomObjectAreasTable[4].object = 2;
			_roomObjectAreasTable[7].object = 0;
			_roomObjectAreasTable[8].object = 2;
		}
	}
}

void IgorEngine::PART_14_HELPER_2_enterFromChurch() {
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	for (int i = 0; i <= 9; ++i) {
		WalkData *wd = &_walkData[0];
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		wd->setPos(182, 143, 1, _walkCurrentFrame);
		WalkData::setNextFrame(1, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 30;
		wd->scaleWidth = 23 + i * 3;
		wd->xPosChanged = 1;
		wd->dxPos = 0;
		wd->yPosChanged = 1;
		wd->dyPos = 3;
		wd->scaleHeight = 50;
		moveIgor(wd->posNum, wd->frameNum);
		waitForTimer(15);
	}
	_walkDataLastIndex = 0;
	_walkDataCurrentIndex = 1;
	buildWalkPath(182, 143, 162, 138);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_14_HELPER_3_enterFromChurchBell() {
	for (int i = 192 * 3; i <= 207 * 3; ++i) {
		if (_paletteBuffer[i] > 5) {
			_currentPalette[i] -= 5;
		} else {
			_currentPalette[i] = 0;
		}
	}
	setPaletteRange(192, 207);
	WalkData *wd = &_walkData[0];
	wd->setPos(76, 69, 4, 0);
	wd->clipSkipX = 1;
	wd->clipWidth = 24;
	wd->scaleWidth = 40;
	wd->xPosChanged = 1;
	wd->dxPos = 0;
	wd->yPosChanged = 1;
	wd->dyPos = 0;
	wd->scaleHeight = 40;
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[22156]].area = 1;
	_roomObjectAreasTable[_screenLayer2[27895]].area = 1;
	buildWalkPath(76, 69, 55, 87);
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	for (int i = 192 * 3; i <= 207 * 3; ++i) {
		_currentPalette[i] = _paletteBuffer[i];
	}
	setPaletteRange(192, 207);
	--_walkDataLastIndex;
	buildWalkPath(55, 87, 50, 103);
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
	_roomObjectAreasTable[_screenLayer2[22156]].area = 0;
	_roomObjectAreasTable[_screenLayer2[27895]].area = 0;
}

void IgorEngine::PART_14_HELPER_4_enterFromMaze() {
	PART_14_HELPER_1_OBJECT_STATE(255);
	for (int i = 192 * 3; i <= 207 * 3; ++i) {
		if (_paletteBuffer[i] > 5) {
			_currentPalette[i] -= 5;
		} else {
			_currentPalette[i] = 0;
		}
	}
	setPaletteRange(192, 207);
	_walkDataCurrentIndex = 0;
	_walkCurrentFrame = 1;
	_walkCurrentPos = 3;
	for (int i = 0; i <= 9; ++i) {
		WalkData *wd = &_walkData[0];
		if (i == 9) {
			_walkCurrentFrame = 0;
		}
		wd->setPos(164, 111, 3, _walkCurrentFrame);
		WalkData::setNextFrame(3, _walkCurrentFrame);
		wd->clipSkipX = 1;
		wd->clipWidth = 30;
		wd->scaleWidth = 23 + i * 3;
		wd->xPosChanged = 1;
		wd->dxPos = 0;
		wd->yPosChanged = 1;
		wd->dyPos = 3;
		wd->scaleHeight = 50;
		moveIgor(3, wd->frameNum);
		waitForTimer(15);
	}
	for (int i = 192 * 3; i <= 207 * 3; ++i) {
		_currentPalette[i] = _paletteBuffer[i];
	}
	setPaletteRange(192, 207);
	_walkDataLastIndex = 0;
	_roomObjectAreasTable[_screenLayer2[35684]].area = 2;
	buildWalkPath(164, 111, 164, 126);
	_roomObjectAreasTable[_screenLayer2[35684]].area = 0;
	_walkData[_walkDataLastIndex].frameNum = 0;
	_walkDataCurrentIndex = 1;
	_gameState.igorMoving = true;
	waitForIgorMove();
}

void IgorEngine::PART_14_HELPER_6_drawEntrance() {
	const int offset = 18382;
	copyArea(_screenLayer1, offset, 320, _animFramesBuffer + 0xD5BA, 44, 44, 66);
}

void IgorEngine::PART_14_HELPER_7_candleFlicker(int frame) {
	const int offset = 38038;
	for (int i = 0; i <= 1; ++i) {
		memcpy(_screenVGA + i * 320 + offset, _animFramesBuffer + 0x2AA4 + frame * 2 + i, 1);
	}
}

void IgorEngine::PART_14_HELPER_8_brotherReveal(int start, int end) {
	_gameState.counter[0] = start - 1;
	do {
		if (compareGameTick(1, 16)) {
			++_gameState.counter[0];
			const uint8 *src = _animFramesBuffer + 0x2AAA + READ_LE_UINT16(_animFramesBuffer + 0x940C + _gameState.counter[0] * 2) - 1;
			decodeAnimFrame(src, _screenVGA, true);
		}
		if (compareGameTick(2, 16) && _gameState.unkF) {
			PART_14_HELPER_7_candleFlicker(getRandomNumber(1) + 1);
		}
		waitForTimer();
	} while (_gameState.counter[0] != end);
}

void IgorEngine::PART_14_HELPER_9_showPuzzle() {
	PART_14_HELPER_10_loadChurchMosaicData();
	memcpy(_screenTextLayer, _screenVGA, 46080);
	memcpy(_screenVGA, _screenLayer1, 46080);
	memcpy(_screenLayer1, _screenTextLayer, 46080);
	memset(_screenVGA + 46080, 0, 17920);
	fadeIn(768);
	_inputVars[kInputCursorXPos] = 160;
	_inputVars[kInputCursorYPos] = 72;
	showCursor();
	_gameState.counter[4] = 0;
	_currentPart = 145;
	_dialogueEnded = false;
	VAR_CURRENT_CHURCH_MOSAIC_STONE = 255;
	do {
		// the region mask only covers the room area; below it (verbs panel) there is no region
		int area = _inputVars[kInputCursorYPos] < 144 ? _screenLayer2[_inputVars[kInputCursorYPos] * 320 + _inputVars[kInputCursorXPos]] : 0;
		if (area == 0) {
			VAR_NEW_CHURCH_MOSAIC_STONE = 1;
		} else if (area >= 1 && area <= 8) {
			VAR_NEW_CHURCH_MOSAIC_STONE = 2;
		} else if (area == 9) {
			VAR_NEW_CHURCH_MOSAIC_STONE = 3;
		}
		if (VAR_NEW_CHURCH_MOSAIC_STONE != VAR_CURRENT_CHURCH_MOSAIC_STONE) {
			switch (VAR_NEW_CHURCH_MOSAIC_STONE) {
			case 1:
				drawActionSentence("", 253);
				break;
			case 2:
				drawActionSentence(getString(STR_PushStone), 253);
				break;
			case 3:
				drawActionSentence(getString(STR_Exit), 253);
				break;
			}
			VAR_CURRENT_CHURCH_MOSAIC_STONE = VAR_NEW_CHURCH_MOSAIC_STONE;
		}
		if (_inputVars[kInputClick]) {
			_inputVars[kInputClick] = 0;
			_dialogueEnded = area != 0;
			if (area == 9) {
				_currentPart = 146;
			}
			if (area >= 1 && area <= 8) {
				drawActionSentence(getString(STR_PushStone), 251);
				hideCursor();
				playSound(34, 1);
				switch (area) {
				case 1:
					PART_14_pushStone(33359, 28, 34, 0);
					break;
				case 2:
					PART_14_pushStone(22800, 27, 34, 0x3D4);
					break;
				case 3:
					PART_14_pushStone(10642, 39, 45, 0x785);
					break;
				case 4:
					PART_14_pushStone(3624, 56, 39, 0xCCFA);
					break;
				case 5:
					PART_14_pushStone(3679, 54, 38, 0xE87);
					break;
				case 6:
					PART_14_pushStone(11077, 35, 35, 0x1D8F);
					break;
				case 7:
					PART_14_pushStone(20688, 28, 36, 0x227B);
					break;
				case 8:
					PART_14_pushStone(31889, 27, 38, 0x2687);
					break;
				}
				stopSound();
				drawActionSentence(getString(STR_PushStone), 253);
				showCursor();
				_dialogueEnded = false;
				++_gameState.counter[4];
				if (_objectsState[2] == 0 || PART_14_ANIM_DATA_1[_gameState.counter[4]] != area) {
					_currentPart = 0;
				}
				if (_gameState.counter[4] == 5) {
					if (_currentPart == 145) {
						_dialogueEnded = true;
					} else {
						_currentPart = 145;
						_gameState.counter[4] = 0;
						playSound(35, 1);
						waitForTimer(255);
						stopSound();
						// The mosaic screen has no Igor sprite, so his head is not animated.
						igorSay(204, 1, 863);
						waitForEndOfIgorDialogue(false);
					}
				}
			}
		}
		if (_inputVars[kInputEscape]) {
			_inputVars[kInputEscape] = 0;
			_currentPart = 146;
			_dialogueEnded = true;
		}
		// The original reads mouse/keyboard state updated by interrupts; here
		// input is only gathered while waiting for the timer.
		waitForTimer();
	} while (!_eventQuitGame && !(_currentPart >= 145 && _currentPart <= 146 && _dialogueEnded));
	drawActionSentence("", 253);
	hideCursor();
	if (_currentPart == 145) {
		playSound(35, 1);
	}
	fadeOut(768);
	memcpy(_screenVGA, _screenLayer1, 46080);
	SET_PAL_208_96_1();
}

void IgorEngine::PART_14_HELPER_10_loadChurchMosaicData() {
	loadData(PAL_ChurchMosaic, _paletteBuffer);
	loadData(IMG_ChurchMosaic, _screenLayer1);
	uint8 *p = loadData(MSK_ChurchMosaic);
	decodeRoomMask(p);
	free(p);
}

void IgorEngine::PART_14_pushStone(int screenOffset, int w, int h, int animOffset) {
	for (int i = 0; i <= h; ++i) {
		memcpy(_screenTempLayer + i * 100, _screenVGA + i * 320 + screenOffset, w);
		memcpy(_screenVGA + i * 320 + screenOffset, _animFramesBuffer + animOffset + i * w, w);
	}
	waitForTimer(127);
	for (int i = 0; i <= h; ++i) {
		memcpy(_screenVGA + i * 320 + screenOffset, _screenTempLayer + i * 100, w);
	}
}

void IgorEngine::PART_14() {
	playMusic(3);
	_gameState.enableLight = 1;
	loadResourceData__ROOM_ChurchPuzzle();
	loadResourceData__ANIM_ChurchPuzzle();
	loadActionData(DAT_ChurchPuzzle);
	_roomDataOffsets = PART_14_ROOM_DATA_OFFSETS;
	setRoomWalkBounds(0, 0, 256, 143);
	SET_EXEC_ACTION_FUNC(1, &IgorEngine::PART_14_EXEC_ACTION);
	PART_14_HELPER_1_OBJECT_STATE(255);
	memcpy(_screenVGA, _screenLayer1, 46080);
	_currentAction.verb = kVerbWalk;
	fadeIn(768);
	if (_currentPart == 140) {
		PART_14_HELPER_2_enterFromChurch();
	}
	if (_currentPart == 141) {
		PART_14_HELPER_3_enterFromChurchBell();
	}
	if (_currentPart == 142) {
		PART_14_HELPER_4_enterFromMaze();
	}
	enterPartLoop();
	while (_currentPart >= 140 && _currentPart <= 142) {
		runPartLoop();
	}
	leavePartLoop();
	fadeOut(624);
}

void IgorEngine::loadResourceData__ROOM_ChurchPuzzle() {
	loadRoomData(PAL_ChurchPuzzle, IMG_ChurchPuzzle, BOX_ChurchPuzzle, MSK_ChurchPuzzle, TXT_ChurchPuzzle);
}

void IgorEngine::loadResourceData__ANIM_ChurchPuzzle() {
	static const int anm1[] = { FRM_ChurchPuzzle1, FRM_ChurchPuzzle2, FRM_ChurchPuzzle3, FRM_ChurchPuzzle4, 0 };
	loadAnimData(anm1, 0);
	static const int anm2[] = { FRM_ChurchPuzzle5, FRM_ChurchPuzzle6, FRM_ChurchPuzzle7, FRM_ChurchPuzzle8, FRM_ChurchPuzzle9, FRM_ChurchPuzzle10, FRM_ChurchPuzzle11, 0 };
	loadAnimData(anm2, 0x1D8F);
	static const int anm3[] = { FRM_ChurchPuzzle12, FRM_ChurchPuzzle13, FRM_ChurchPuzzle14, 0 };
	loadAnimData(anm3, 0xCCE6);
}


} // End of namespace Igor
