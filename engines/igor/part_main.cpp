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
		int num, rnd = getRandomNumber(100);
		if (rnd < 34) {
			num = 11;
		} else if (rnd < 69) {
			num = 12;
		} else if (rnd < 94) {
			num = 13;
		} else {
			num = 14;
		}
		ADD_DIALOGUE_TEXT(num, 1, num);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
	} break;
	case 3:
		ADD_DIALOGUE_TEXT(15, 1, 15);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 4:
		ADD_DIALOGUE_TEXT(10, 1, 10);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 5:
		ADD_DIALOGUE_TEXT(9, 1, 9);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 6:
		ADD_DIALOGUE_TEXT(8, 1, 8);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 7:
		ADD_DIALOGUE_TEXT(6, 1, 6);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 8:
		ADD_DIALOGUE_TEXT(7, 1, 7);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 9: {
			int num = 16 + getRandomNumber(2);
			ADD_DIALOGUE_TEXT(num, 1, num);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 10:
		ADD_DIALOGUE_TEXT(18, 1, 18);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 11:
		ADD_DIALOGUE_TEXT(19, 1, 19);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 12: {
			int num = 20 + getRandomNumber(2);
			ADD_DIALOGUE_TEXT(num, 1, num);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 13:
		ADD_DIALOGUE_TEXT(22, 1, 22);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 14:
		ADD_DIALOGUE_TEXT(23, 1, 23);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 15:
		ADD_DIALOGUE_TEXT(24, 1, 24);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 16:
		ADD_DIALOGUE_TEXT(25, 1, 25);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 17: {
			int num = 26 + getRandomNumber(2);
			ADD_DIALOGUE_TEXT(num, 1, num);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 18:
		ADD_DIALOGUE_TEXT(28, 1, 28);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 19:
		ADD_DIALOGUE_TEXT(4, 1, 4);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 20:
		ADD_DIALOGUE_TEXT(5, 1, 5);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 21: {
			int num = 1 + getRandomNumber(3);
			ADD_DIALOGUE_TEXT(num, 1, num);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		}
		break;
	case 22:
		ADD_DIALOGUE_TEXT(51, 1, 32);
		ADD_DIALOGUE_TEXT(52, 1, 33);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 23:
		ADD_DIALOGUE_TEXT(53, 1, 34);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 24:
		ADD_DIALOGUE_TEXT(54, 1, 35);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 25:
		ADD_DIALOGUE_TEXT(55, 3, 36);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 26:
		ADD_DIALOGUE_TEXT(58, 1, 37);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 27:
		ADD_DIALOGUE_TEXT(59, 2, 38);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 28:
		ADD_DIALOGUE_TEXT(61, 1, 39);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 29:
		ADD_DIALOGUE_TEXT(62, 1, 40);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 30:
		ADD_DIALOGUE_TEXT(64, 1, 42);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 31:
		ADD_DIALOGUE_TEXT(63, 1, 41);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 32:
		ADD_DIALOGUE_TEXT(65, 1, 43);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 33:
		ADD_DIALOGUE_TEXT(66, 1, 44);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 34:
		ADD_DIALOGUE_TEXT(30, 1, 30);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 35:
		ADD_DIALOGUE_TEXT(29, 1, 29);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 36:
		ADD_DIALOGUE_TEXT(67, 2, 45);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 37:
		ADD_DIALOGUE_TEXT(69, 1, 46);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 38:
		// EXEC_MAIN_ACTION_38();
		break;
	case 39:
		ADD_DIALOGUE_TEXT(70, 1, 47);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 40:
		ADD_DIALOGUE_TEXT(71, 1, 48);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 41:
		ADD_DIALOGUE_TEXT(72, 1, 49);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 42:
		ADD_DIALOGUE_TEXT(73, 1, 50);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 43:
		// EXEC_MAIN_ACTION_43();
		break;
	case 44:
		ADD_DIALOGUE_TEXT(156, 1, 80);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 45:
		ADD_DIALOGUE_TEXT(92, 1, 64);
		ADD_DIALOGUE_TEXT(93, 3, 65);
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 46:
		if (_objectsState[3] == 0) {
			ADD_DIALOGUE_TEXT(85, 1, 60);
		} else {
			ADD_DIALOGUE_TEXT(86, 2, 61);
		}
		SET_DIALOGUE_TEXT(1, 2);
		startIgorDialogue();
		break;
	case 48:
		ADD_DIALOGUE_TEXT(77 + _objectsState[0], 1, 53 + _objectsState[0]);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
	case 50:
		if (_objectsState[1] == 0) {
			ADD_DIALOGUE_TEXT(80, 1, 56);
			ADD_DIALOGUE_TEXT(81, 1, 57);
			SET_DIALOGUE_TEXT(1, 2);
			startIgorDialogue();
		} else {
			ADD_DIALOGUE_TEXT(82, 1, 58);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
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
		ADD_DIALOGUE_TEXT(89, 2, 63);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 53:
		ADD_DIALOGUE_TEXT(88, 1, 62);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 54:
		// EXEC_MAIN_ACTION_54();
		break;
	case 55:
		ADD_DIALOGUE_TEXT(159, 1, 83);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 56:
		ADD_DIALOGUE_TEXT(160, 1, 84);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 57:
		ADD_DIALOGUE_TEXT(31, 1, 31);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 58:
		ADD_DIALOGUE_TEXT(140, 1, 70);
		ADD_DIALOGUE_TEXT(141, 1, 71);
		ADD_DIALOGUE_TEXT(142, 1, 72);
		ADD_DIALOGUE_TEXT(143, 1, 73);
		SET_DIALOGUE_TEXT(1, 4);
		startIgorDialogue();
		_objectsState[4] = 2;
		break;
	case 59:
		ADD_DIALOGUE_TEXT(97, 2, 67);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 60:
		ADD_DIALOGUE_TEXT(161, 1, 85);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 61:
		if (_inventoryInfo[51] != 0) {
			ADD_DIALOGUE_TEXT(76, 1, 52);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
		} else {
			ADD_DIALOGUE_TEXT(74, 2, 51);
			SET_DIALOGUE_TEXT(1, 1);
			startIgorDialogue();
			addObjectToInventory(16, 51);
		}
		break;
	case 62:
		_inventoryImages[_inventoryInfo[52] - 1] = 0;
		_inventoryInfo[52] = 0;
		packInventory();
		_inventoryImages[_inventoryInfo[59] - 1] = 0;
		_inventoryInfo[59] = 0;
		packInventory();
		_objectsState[1] = 1;
		UPDATE_OBJECT_STATE(2);
		addObjectToInventory(24, 59);
		ADD_DIALOGUE_TEXT(169, 1, 93);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 63:
		ADD_DIALOGUE_TEXT(170, 1, 94);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		_inventoryImages[_inventoryInfo[68] - 1] = 0;
		_inventoryInfo[68] = 0;
		packInventory();
		_inventoryImages[_inventoryInfo[69] - 1] = 0;
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
		ADD_DIALOGUE_TEXT(165, 1, 89);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 66:
		ADD_DIALOGUE_TEXT(166, 1, 90);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 67:
		ADD_DIALOGUE_TEXT(167, 1, 91);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 68:
		ADD_DIALOGUE_TEXT(168, 1, 92);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;
	case 69:
		ADD_DIALOGUE_TEXT(96, 1, 66);
		SET_DIALOGUE_TEXT(1, 1);
		startIgorDialogue();
		break;

	default:
		warning("EXEC_MAIN_ACTION() Unhandled action %d", action);
		break;
	}
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
		case 170:
		case 171:
			PART_17();
			break;
		case 230:
		case 231:
		case 232:
			PART_23();
			break;
		case 300:
		case 301:
		case 302:
		case 303:
			PART_30();
			break;
		case 340:
			PART_34(); // Park, left panel; cseg101:1F85
			break;
		case 350:
		case 351:
			PART_35(); // Park, right panel; cseg100:1644
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

} // End of namespace Igor
