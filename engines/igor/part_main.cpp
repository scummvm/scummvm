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
void IgorEngine::EXEC_MAIN_ACTION(int action) {
	switch (action) {
	case 0:
	case 1:
		break;
	case 2: {
		int num, rnd = getRandomNumber(99);
		if (rnd < 34) {
			num = 11;
		} else if (rnd < 69) {
			num = 12;
		} else if (rnd < 94) {
			num = 13;
		} else {
			num = 14;
		}
		igorSay(num, 1, num);
	} break;
	case 3:
		igorSay(15, 1, 15);
		break;
	case 4:
		igorSay(10, 1, 10);
		break;
	case 5:
		igorSay(9, 1, 9);
		break;
	case 6:
		igorSay(8, 1, 8);
		break;
	case 7:
		igorSay(6, 1, 6);
		break;
	case 8:
		igorSay(7, 1, 7);
		break;
	case 9: {
			int num = 16 + getRandomNumber(1);
			igorSay(num, 1, num);
		}
		break;
	case 10:
		igorSay(18, 1, 18);
		break;
	case 11:
		igorSay(19, 1, 19);
		break;
	case 12: {
			int num = 20 + getRandomNumber(1);
			igorSay(num, 1, num);
		}
		break;
	case 13:
		igorSay(22, 1, 22);
		break;
	case 14:
		igorSay(23, 1, 23);
		break;
	case 15:
		igorSay(24, 1, 24);
		break;
	case 16:
		igorSay(25, 1, 25);
		break;
	case 17: {
			int num = 26 + getRandomNumber(1);
			igorSay(num, 1, num);
		}
		break;
	case 18:
		igorSay(28, 1, 28);
		break;
	case 19:
		igorSay(4, 1, 4);
		break;
	case 20:
		igorSay(5, 1, 5);
		break;
	case 21: {
			int num = 1 + getRandomNumber(2);
			igorSay(num, 1, num);
		}
		break;
	case 22:
		igorSay({ { 51, 1, 32 }, { 52, 1, 33 } });
		break;
	case 23:
		igorSay(53, 1, 34);
		break;
	case 24:
		igorSay(54, 1, 35);
		break;
	case 25:
		igorSay(55, 3, 36);
		break;
	case 26:
		igorSay(58, 1, 37);
		break;
	case 27:
		igorSay(59, 2, 38);
		break;
	case 28:
		igorSay(61, 1, 39);
		break;
	case 29:
		igorSay(62, 1, 40);
		break;
	case 30:
		igorSay(64, 1, 42);
		break;
	case 31:
		igorSay(63, 1, 41);
		break;
	case 32:
		igorSay(65, 1, 43);
		break;
	case 33:
		igorSay(66, 1, 44);
		break;
	case 34:
		igorSay(30, 1, 30);
		break;
	case 35:
		igorSay(29, 1, 29);
		break;
	case 36:
		igorSay(67, 2, 45);
		break;
	case 37:
		igorSay(69, 1, 46);
		break;
	case 38:
		// EXEC_MAIN_ACTION_38();
		break;
	case 39:
		igorSay(70, 1, 47);
		break;
	case 40:
		igorSay(71, 1, 48);
		break;
	case 41:
		igorSay(72, 1, 49);
		break;
	case 42:
		igorSay(73, 1, 50);
		break;
	case 43:
		EXEC_MAIN_ACTION_43_lookAtPhoto();
		break;
	case 44:
		igorSay(156, 1, 80);
		break;
	case 45:
		igorSay({ { 92, 1, 64 }, { 93, 3, 65 } });
		break;
	case 46: // look at folder
		if (_objectsState[3] == 0) {
			ADD_DIALOGUE_TEXT(85, 1, 60);
		} else {
			ADD_DIALOGUE_TEXT(86, 2, 61);
			_objectsState[5] = 1;
		}
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 48:
		igorSay(77 + _objectsState[0], 1, 53 + _objectsState[0]);
		break;
	case 50:
		if (_objectsState[1] == 0) {
			igorSay({ { 80, 1, 56 }, { 81, 1, 57 } });
		} else {
			igorSay(82, 1, 58);
		}
		break;
	case 51:
		if (_objectsState[7] == 0) {
			ADD_DIALOGUE_TEXT(157, 1, 81);
		} else {
			ADD_DIALOGUE_TEXT(158, 1, 82);
		}
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 52:
		igorSay(89, 2, 63);
		break;
	case 53:
		igorSay(88, 1, 62);
		break;
	case 54:
		lookAtPapyrus(false);
		break;
	case 55:
		igorSay(159, 1, 83);
		break;
	case 56:
		igorSay(160, 1, 84);
		break;
	case 57:
		igorSay(31, 1, 31);
		break;
	case 58:
		igorSay({ { 140, 1, 70 }, { 141, 1, 71 }, { 142, 1, 72 }, { 143, 1, 73 } });
		_objectsState[4] = 2;
		break;
	case 59:
		igorSay(97, 2, 67);
		break;
	case 60:
		igorSay(161, 1, 85);
		break;
	case 61:
		if (_inventoryInfo[51] != 0) {
			igorSay(76, 1, 52);
		} else {
			igorSay(74, 2, 51);
			addObjectToInventory(16, 51);
		}
		break;
	case 62:
		_inventoryInfo[_inventoryInfo[52] - 1] = 0;
		_inventoryInfo[52] = 0;
		packInventory();
		_inventoryInfo[_inventoryInfo[59] - 1] = 0;
		_inventoryInfo[59] = 0;
		packInventory();
		_objectsState[1] = 1;
		UPDATE_OBJECT_STATE(2);
		addObjectToInventory(24, 59);
		igorSay(169, 1, 93);
		break;
	case 63:
		igorSay(170, 1, 94);
		_inventoryInfo[_inventoryInfo[68] - 1] = 0;
		_inventoryInfo[68] = 0;
		packInventory();
		_inventoryInfo[_inventoryInfo[69] - 1] = 0;
		_inventoryInfo[69] = 0;
		packInventory();
		_objectsState[6] = 1;
		UPDATE_OBJECT_STATE(7);
		addObjectToInventory(34, 69);
		break;
	case 64:
		if (_objectsState[6] == 0) {
			ADD_DIALOGUE_TEXT(162, 1, 86);
		} else if (_objectsState[6] == 1) {
			ADD_DIALOGUE_TEXT(163, 1, 87);
		} else if (_objectsState[6] == 2) {
			ADD_DIALOGUE_TEXT(164, 1, 88);
		}
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 65:
		igorSay(165, 1, 89);
		break;
	case 66:
		igorSay(166, 1, 90);
		break;
	case 67:
		igorSay(167, 1, 91);
		break;
	case 68:
		igorSay(168, 1, 92);
		break;
	case 69:
		igorSay(96, 1, 66);
		break;

	default:
		warning("EXEC_MAIN_ACTION() Unhandled action %d", action);
		break;
	}
}

void IgorEngine::EXEC_MAIN_ACTION_43_lookAtPhoto() {
	memcpy(_paletteBuffer, _currentPalette, 624);
	fadeOut(624);
	uint8 *tmp = (uint8 *)malloc(64000 + 768);
	if (tmp) {
		memcpy(tmp, _screenVGA, 64000);
		memcpy(tmp + 64000, _paletteBuffer, 768);
	}
	loadData(IMG_PhotoHarrisonMargaret, _screenVGA);
	loadData(PAL_PhotoHarrisonMargaret, _paletteBuffer);
	fadeIn(624);
	WalkData *wd = &_walkData[_walkDataLastIndex - 1];
	int _walkDataCurrentPosX2 = wd->x;
	int _walkDataCurrentPosY2 = wd->y;
	int _walkDataCurrentWScale = wd->scaleWidth;
	wd->x = 160;
	wd->y = 130;
	wd->scaleWidth = 50;
	igorSayAndWait(83, 2, 59);
	_currentAction.object1Num = 0;
	wd->x = _walkDataCurrentPosX2;
	wd->y = _walkDataCurrentPosY2;
	wd->scaleWidth = _walkDataCurrentWScale;
	fadeOut(624);
	if (tmp) {
		memcpy(_screenVGA, tmp, 64000);
		memcpy(_paletteBuffer, tmp + 64000, 768);
		free(tmp);
	}
	fadeIn(624);
}

void IgorEngine::SET_EXEC_ACTION_FUNC(int i, ExecuteActionProc p) {
	switch (i) {
	case 0:
		_executeMainAction = p;
		break;
	case 1:
		_executeRoomAction = p;
		break;
	}
}

void IgorEngine::UPDATE_OBJECT_STATE(int num) {
	if (num == 1 || num == 255) {
		switch (_objectsState[0]) {
		case 0:
			Common::strlcpy(_globalObjectNames[23], getString(STR_BottleOfWhisky), sizeof(_globalObjectNames[23]));
			break;
		case 1:
			Common::strlcpy(_globalObjectNames[23], getString(STR_EmptyBottle), sizeof(_globalObjectNames[23]));
			break;
		case 2:
			Common::strlcpy(_globalObjectNames[23], getString(STR_BottleOfWater), sizeof(_globalObjectNames[23]));
			break;
		}
	}
	if (num == 2 || num == 255) {
		switch (_objectsState[1]) {
		case 0:
			_inventoryImages[23] = 27;
			Common::strlcpy(_globalObjectNames[24], getString(STR_Lizard), sizeof(_globalObjectNames[24]));
			break;
		default:
			_inventoryImages[23] = 35;
			Common::strlcpy(_globalObjectNames[24], getString(STR_FatLizard), sizeof(_globalObjectNames[24]));
			break;
		}
	}
	if (num == 4 || num == 255) {
		switch (_objectsState[3]) {
		case 0:
			Common::strlcpy(_globalObjectNames[22], getString(STR_CarolineFolder), sizeof(_globalObjectNames[22]));
			break;
		case 1:
			Common::strlcpy(_globalObjectNames[22], getString(STR_PhilipFolder), sizeof(_globalObjectNames[22]));
			break;
		}
	}
	if (num == 7 || num == 255) {
		switch (_objectsState[6]) {
		case 0:
			_inventoryImages[33] = 21;
			break;
		case 1:
			_inventoryImages[33] = 14;
			break;
		case 2:
			_inventoryImages[33] = 7;
			break;
		}
	}
	if (num == 8 || num == 255) {
		if (_objectsState[7] == 0) {
			Common::strlcpy(_globalObjectNames[25], getString(STR_Statuette), sizeof(_globalObjectNames[25]));
			_inventoryImages[24] = 29;
		} else {
			Common::strlcpy(_globalObjectNames[25], getString(STR_Reward), sizeof(_globalObjectNames[25]));
			_inventoryImages[24] = 39;
		}
	}
}

void IgorEngine::PART_MAIN() {
	SET_EXEC_ACTION_FUNC(0, &IgorEngine::EXEC_MAIN_ACTION);
	if (!_gameStateLoaded) {
		memset(_objectsState, 0, 112);
		_objectsState[21] = 1;
		_objectsState[49] = 1;
		memset(_inventoryInfo, 0, 36);
		_inventoryInfo[0] = 1; // ordering
		_inventoryInfo[1] = 2;
		_inventoryInfo[2] = 4;
		_inventoryInfo[36] = 1;
		_inventoryInfo[37] = 2;
		_inventoryInfo[39] = 3;
		_inventoryInfo[72] = 1; // first object
		_inventoryInfo[73] = 3; // last object
	}
	UPDATE_OBJECT_STATE(255);
	if (_currentPart != kStartupPart) { // boot param
		SET_PAL_208_96_1();
		SET_PAL_240_48_1();
		drawVerbsPanel();
		drawInventory(1, 0);
	}

	do {
		debugC(9, kDebugGame, "PART_MAIN _currentPart %d", _currentPart);
		switch (_currentPart) {
		case 0:
		case 1:
			PART_00(); // Student dormitory
			break;
		case 10:
		case 11:
		case 12:
			PART_01(); // Outside the dormitory window
			break;
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
			PART_02(); // Dormitory attic
			break;

		case 40:
			PART_04(); // Map
			break;
		case 50: // arrive from map
		case 51: // arrive from SpringRock through scroll
		case 52:
			PART_05(); // SpringBridge
			break;
		case 60:       // arrive through scroll
		case 61:       // arrive from intro
			PART_06(); // SpringRock
			break;
		case 70:       // enter from outside the administration corridor
		case 71:       // enter from deans door
		case 72:       // enter from secretary
			PART_07(); // Administration corridor
			break;
		case 80:
			PART_08(); // Dean Pepper office
			break;
		case 90:
			PART_09(); // Administration secretary room
			break;

		case 100: // enter from map
		case 101:
		case 102:
			PART_10(); // OutsideAdministrationBuilding street, right panel
			break;
		case 110:
			PART_11(); // OutsideAdministrationBuilding street, left panel
			break;
		case 120:
		case 121:
		case 122:
			PART_12(); // outside church
			break;
		case 130:
		case 131:
			PART_13();
			break;
		case 140:
		case 141:
		case 142:
			PART_14();
			break;
		case 150:
			PART_15();
			break;
		case 160:
			PART_16();
			break;
		case 170:
		case 171:
			PART_17(); // College entrance
			break;
		case 180:
		case 181:
			PART_18(); // men toilets
			break;
		case 190:
		case 191:
			PART_19(); // women toilets
			break;
		case 210:
		case 211:
		case 212:
			PART_21(); //college corridor margaret
			break;
		case 220:
		case 221:
			PART_22(); // church bell tower
			break;
		case 230:
		case 231:
		case 232:
			PART_23(); // college corridor lucas
			break;
		case 240:
		case 241:
		case 242:
			PART_24();
			break;
		case 250:
		case 251:
		case 252:
			PART_25();
			break;
		case 260:
		case 261:
			PART_26();
			break;
		case 270:
		case 271:
			PART_27();
			break;
		case 280:
		case 281:
			PART_28(); // college corridor caroline
			break;
		case 300:
		case 301:
		case 302:
		case 303:
			PART_30(); // college stairs first floor
			break;
		case 310:
		case 311:
		case 312:
		case 313:
			PART_31();
			break;
		case 330:
		case 331:
			PART_33();
			break;
		case 340:
			PART_34(); // Park, left panel
			break;
		case 350:
		case 351:
			PART_35(); // Park, right panel
			break;
		case 360:
			PART_36();
			break;
		case 370:
			PART_37();
			break;
		case 500:
		case 501:
			PART_50(); // outside the maze
			break;
		case 510:
		case 521:
		case 532:
		case 543:
		case 550:
		case 551:
		case 560:
		case 562:
		case 570:
		case 573:
		case 581:
		case 582:
		case 591:
		case 593:
		case 602:
		case 603:
		case 610:
		case 611:
		case 612:
		case 621:
		case 622:
		case 623:
		case 630:
		case 632:
		case 633:
		case 640:
		case 641:
		case 643:
		case 650:
		case 651:
		case 652:
		case 653:
		case 660:
		case 663:
		case 670:
		case 671:
			PART_MAZE(); // maze, 670 and 671 are the entrance
			break;
		case 700:
			PART_70();
			break;
		case 750:
			PART_75(); // philip vodka cutscene
			break;
		case 850: // Intro cutscene
			// Clear the entire screen buffer before starting the intro cutscene
			memset(_screenVGA, 0, 64000);
			g_engine->_screen->updateScreen();
			_screenVGAVOffset = 24;
			// copy the initial portion of the screen to the display, with the vertical offset
			_system->copyRectToScreen(_screenVGA, 320, 0, 0, 320, _screenVGAVOffset);
			g_engine->_screen->updateScreen();
			PART_85();
			// Clears the bottom portion of the screen
			memset(_screenVGA + 46080, 0, 17920);
			moveScreenUp(_screenVGAVOffset);
			_screenVGAVOffset = 0;
			_inputVars[kInputCursorXPos] = 160;
			_inputVars[kInputCursorYPos] = 72;
			_system->warpMouse(_inputVars[kInputCursorXPos], _inputVars[kInputCursorYPos]);
			break;
		case 900: // Logo slideshow
		case 901:
		case 902:
		case 903:
		case 904:
			PART_90();
			break;

		default:
			error("PART_MAIN() Unhandled part %d", _currentPart);
			break;
		}
		if (_currentPart >= 10) {
			if (_currentPart <= 24 || _currentPart == 51) {
				continue;
			}
			if (_currentPart == 60 || _currentPart == 102 || _currentPart == 110) {
				continue;
			}
		}
		if (_currentPart == 340 || _currentPart == 351 || _currentPart == 750) {
			continue;
		}
		if (_currentPart == 122 || _currentPart == 255) {
			continue;
		}
		// skip if we havent given the note to margaret yet
		if (_objectsState[110] < 1 || _objectsState[110] > 7) {
			continue;
		}
		debugC(9, kDebugEngine, "Checking meanwhile with part: %d, should show cutscene counter %d", _currentPart, _gameState.shouldShowCutsceneCounter);
		if (_gameState.shouldShowCutsceneCounter != 5) {
			++_gameState.shouldShowCutsceneCounter;
		} else {
			if (_gameState.musicNum != 11) {
				// _previousMusic = _gameState.musicNum;
				playMusic(11);
			}
			PART_MEANWHILE();
			PART_MARGARET_ROOM_CUTSCENE();
		// 	if (_previousMusic != 11) {
		// 		playMusic(_previousMusic);
		// 	}
			_gameState.shouldShowCutsceneCounter = 0;
		}
	} while (_currentPart != kInvalidPart && !_eventQuitGame);
}

/**
 * Moves the screen up line by line from the selected offset down to 0
 */
void IgorEngine::moveScreenUp(int offset) {
	_nextTimer = _system->getMillis() + 1000 / 60;
	for (int y = offset; y >= 0; --y) {
		_system->copyRectToScreen(_screenVGA, 320, 0, y, 320, 145);
		_system->updateScreen();
		int diff = _nextTimer - _system->getMillis();
		if (diff > 0) {
			_system->delayMillis(diff);
		}
		_nextTimer = _system->getMillis() + 1000 / 60;
	}
}

// Shows the papyrus full screen. With reveal set (dryer) the figures appear
// slowly and are remembered as seen; otherwise Igor comments on what is shown.
void IgorEngine::lookAtPapyrus(bool reveal) {
	memcpy(_paletteBuffer, _currentPalette, 624);
	fadeOut(624);
	// reserve for entire screen + palette, to save current screen
	uint8 *tmp = (uint8 *)malloc(64000 + 768);
	if (tmp) {
		memcpy(tmp, _screenVGA, 64000);
		memcpy(tmp + 64000, _paletteBuffer, 768);
	}
	// load image and palette
	loadData(IMG_RomanNumbersPaper, _screenVGA);
	loadData(PAL_RomanNumbersPaper, _paletteBuffer);
	if (reveal) {
		fadeIn(624);
		PART_UPDATE_FIGURES_ON_PAPER(60);
		_objectsState[2] = 1;
		waitForTimer(255);
	} else {
		// the figures are only shown from the second viewing on
		if (_objectsState[2] == 1) {
			PART_UPDATE_FIGURES_ON_PAPER(0);
		}
		fadeIn(624);
		WalkData *wd = &_walkData[_walkDataLastIndex - 1];
		int _walkDataCurrentPosX2 = wd->x;
		int _walkDataCurrentPosY2 = wd->y;
		int _walkDataCurrentWScale = wd->scaleWidth;
		wd->x = 160;
		wd->y = 130;
		wd->scaleWidth = 50;
		if (_objectsState[2] == 0) {
			ADD_DIALOGUE_TEXT(99, 1, 68);
		} else {
			ADD_DIALOGUE_TEXT(100, 2, 69);
		}
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		waitForEndOfIgorDialogue(false);
		wd->x = _walkDataCurrentPosX2;
		wd->y = _walkDataCurrentPosY2;
		wd->scaleWidth = _walkDataCurrentWScale;
	}
	fadeOut(624);

	// restore previous screen and palette
	if (tmp) {
		memcpy(_screenVGA, tmp, 64000);
		memcpy(_paletteBuffer, tmp + 64000, 768);
		free(tmp);
	}
	fadeIn(624);
}

// loads and displays the figures on the paper
void IgorEngine::PART_UPDATE_FIGURES_ON_PAPER(int delay) {
	uint8 *framesData = loadData(FRM_NumbersPaper1);
	uint8 *framesOffsets = loadData(FRM_NumbersPaper2);
	for (int i = 1; i <= 20; ++i) {
		const uint8 *p = framesData + READ_LE_UINT16(framesOffsets + (i - 1) * 2) - 1;
		decodeAnimFrame(p, _screenVGA, true);
		waitForTimer(delay);
	}
	free(framesData);
	free(framesOffsets);
}

void IgorEngine::PART_MEANWHILE() {
	hideCursor();
	memset(_currentPalette, 0, 768);
	setPaletteRange(208, 255);
	// clears the verbs and inventory panels area
	memset(_screenVGA + 46080, 0, 17920);
	loadData(IMG_Meanwhile, _screenVGA);
	_paletteBuffer[3] = 63;
	_paletteBuffer[4] = 32;
	_paletteBuffer[5] = 0;
	_paletteBuffer[6] = 44;
	_paletteBuffer[7] = 12;
	_paletteBuffer[8] = 0;
	fadeIn(9);
	for (int i = 0; i <= 5 && !_inputVars[kInputEscape]; ++i) {
		waitForTimer(250);
	}
	_inputVars[kInputEscape] = 0;
	fadeOut(9);
}

} // End of namespace Igor
