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
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#include "dialog.h"
#include "gfx.h"
#include "text.h"
#include "eventman.h"


namespace DM {

DialogMan::DialogMan(DMEngine *vm) : _vm(vm) {
	_selectedDialogChoice = 0;
}

void DialogMan::dialogDraw(const char *msg1, const char *msg2, const char *choice1, const char *choice2, const char *choice3, const char *choice4, bool screenDialog, bool clearScreen, bool fading) {
	static Box constBox1 = Box(0, 223, 101, 125);
	static Box constBox2 = Box(0, 223, 76, 100);
	static Box constBox3 = Box(0, 223, 51, 75);
	static Box dialog2ChoicesPatch = Box(102, 122, 89, 125);
	static Box dialog4ChoicesPatch = Box(102, 122, 62, 97);

	_vm->_displayMan->loadIntoBitmap(k0_dialogBoxGraphicIndice, _vm->_displayMan->_bitmapViewport);
	//Strangerke: the version should be replaced by a ScummVM/RogueVM (?) string
	// TODO: replace with ScummVM version string
	_vm->_textMan->printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, 192, 7, k2_ColorLightGray, k1_ColorDarkGary, "V2.2", k136_heightViewport);
	int16 choiceCount = 1;
	if (choice2)
		choiceCount++;

	if (choice3)
		choiceCount++;

	if (choice4)
		choiceCount++;

	if (fading)
		_vm->_displayMan->startEndFadeToPalette(_vm->_displayMan->_blankBuffer);

	if (clearScreen)
		_vm->_displayMan->fillScreen(k0_ColorBlack);

	_vm->_displayMan->_useByteBoxCoordinates = false;
	if (choiceCount == 1) {
		_vm->_displayMan->blitToBitmap(_vm->_displayMan->_bitmapViewport, _vm->_displayMan->_bitmapViewport, constBox1, 0, 64, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		_vm->_displayMan->blitToBitmap(_vm->_displayMan->_bitmapViewport, _vm->_displayMan->_bitmapViewport, constBox2, 0, 39, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		_vm->_displayMan->blitToBitmap(_vm->_displayMan->_bitmapViewport, _vm->_displayMan->_bitmapViewport, constBox3, 0, 14, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		printCenteredChoice(_vm->_displayMan->_bitmapViewport, choice1, 112, 114);
	} else if (choiceCount == 2) {
		_vm->_displayMan->blitToBitmap(_vm->_displayMan->_bitmapViewport, _vm->_displayMan->_bitmapViewport, dialog2ChoicesPatch, 102, 52, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		printCenteredChoice(_vm->_displayMan->_bitmapViewport, choice1, 112, 77);
		printCenteredChoice(_vm->_displayMan->_bitmapViewport, choice2, 112, 114);
	} else if (choiceCount == 3) {
		printCenteredChoice(_vm->_displayMan->_bitmapViewport, choice1, 112, 77);
		printCenteredChoice(_vm->_displayMan->_bitmapViewport, choice2, 59, 114);
		printCenteredChoice(_vm->_displayMan->_bitmapViewport, choice3, 166, 114);
	} else if (choiceCount == 4) {
		_vm->_displayMan->blitToBitmap(_vm->_displayMan->_bitmapViewport, _vm->_displayMan->_bitmapViewport, dialog4ChoicesPatch, 102, 99, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		printCenteredChoice(_vm->_displayMan->_bitmapViewport, choice1, 59, 77);
		printCenteredChoice(_vm->_displayMan->_bitmapViewport, choice2, 166, 77);
		printCenteredChoice(_vm->_displayMan->_bitmapViewport, choice3, 59, 114);
		printCenteredChoice(_vm->_displayMan->_bitmapViewport, choice4, 166, 114);
	}

	int16 textPosX;
	int16 textPosY = 29;
	if (msg1) {
		char L1312_ac_StringPart1[70];
		char L1313_ac_StringPart2[70];
		if (isMessageOnTwoLines(msg1, L1312_ac_StringPart1, L1313_ac_StringPart2)) {
			textPosY = 21;
			textPosX = 113 - ((strlen(L1312_ac_StringPart1) * 6) >> 1);
			_vm->_textMan->printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k11_ColorYellow, k5_ColorLightBrown, L1312_ac_StringPart1, k136_heightViewport);
			textPosY += 8;
			textPosX = 113 - ((strlen(L1313_ac_StringPart2) * 6) >> 1);
			_vm->_textMan->printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k11_ColorYellow, k5_ColorLightBrown, L1313_ac_StringPart2, k136_heightViewport);
			textPosY += 8;
		} else {
			textPosX = 113 - ((strlen(msg1) * 6) >> 1);
			_vm->_textMan->printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k11_ColorYellow, k5_ColorLightBrown, msg1, k136_heightViewport);
			textPosY += 8;
		}
	}
	if (msg2) {
		char L1312_ac_StringPart1[70];
		char L1313_ac_StringPart2[70];
		if (isMessageOnTwoLines(msg2, L1312_ac_StringPart1, L1313_ac_StringPart2)) {
			textPosX = 113 - ((strlen(L1312_ac_StringPart1) * 6) >> 1);
			_vm->_textMan->printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k9_ColorGold, k5_ColorLightBrown, L1312_ac_StringPart1, k136_heightViewport);
			textPosY += 8;
			textPosX = 113 - ((strlen(L1313_ac_StringPart2) * 6) >> 1);
			_vm->_textMan->printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k9_ColorGold, k5_ColorLightBrown, L1313_ac_StringPart2, k136_heightViewport);
		} else {
			textPosX = 113 - ((strlen(msg2) * 6) >> 1);
			_vm->_textMan->printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k9_ColorGold, k5_ColorLightBrown, msg2, k136_heightViewport);
		}
	}
	if (screenDialog) {
		Box displayBox;
		displayBox._y1 = 33;
		displayBox._y2 = 168;
		displayBox._x1 = 47;
		displayBox._x2 = 270;
		_vm->_eventMan->showMouse();
		_vm->_displayMan->blitToScreen(_vm->_displayMan->_bitmapViewport, &displayBox, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport);
		_vm->_eventMan->hideMouse();
	} else {
		_vm->_displayMan->drawViewport(k0_viewportNotDungeonView);
		_vm->delay(1);
	}

	if (fading)
		_vm->_displayMan->startEndFadeToPalette(_vm->_displayMan->_paletteTopAndBottomScreen);

	_vm->_displayMan->_drawFloorAndCeilingRequested = true;
	_vm->_displayMan->updateScreen();
}

void DialogMan::printCenteredChoice(byte *bitmap, const char *str, int16 posX, int16 posY) {
	if (str) {
		posX -= (strlen(str) * 6) >> 1;
		_vm->_textMan->printTextToBitmap(bitmap, k112_byteWidthViewport, posX, posY, k9_ColorGold, k5_ColorLightBrown, str, k136_heightViewport);
	}
}

bool DialogMan::isMessageOnTwoLines(const char *str, char *part1, char *part2) {
	uint16 strLength = strlen(str);
	if (strLength <= 30)
		return false;

	strcpy(part1, str);
	uint16 splitPosition = strLength >> 1;
	while ((splitPosition < strLength) && (part1[splitPosition] != ' '))
		splitPosition++;

	part1[splitPosition] = '\0';
	strcpy(part2, &part1[splitPosition + 1]);
	return true;
}

int16 DialogMan::getChoice(uint16 choiceCount, uint16 dialogSetIndex, int16 driveType, int16 automaticChoiceIfFlopyInDrive) {
	_vm->_eventMan->hideMouse();
	MouseInput *primaryMouseInputBackup = _vm->_eventMan->_primaryMouseInput;
	MouseInput *secondaryMouseInputBackup = _vm->_eventMan->_secondaryMouseInput;
	KeyboardInput *primaryKeyboardInputBackup = _vm->_eventMan->_primaryKeyboardInput;
	KeyboardInput *secondaryKeyboardInputBackup = _vm->_eventMan->_secondaryKeyboardInput;
	_vm->_eventMan->_secondaryMouseInput = nullptr;
	_vm->_eventMan->_primaryKeyboardInput = nullptr;
	_vm->_eventMan->_secondaryKeyboardInput = nullptr;
	_vm->_eventMan->_primaryMouseInput = _vm->_eventMan->_primaryMouseInputDialogSets[dialogSetIndex][choiceCount - 1];
	_vm->_eventMan->discardAllInput();
	_selectedDialogChoice = 99;
	do {
		Common::Event key;
		Common::EventType eventType = _vm->_eventMan->processInput(&key);
		_vm->_eventMan->processCommandQueue();
		_vm->delay(1);
		_vm->_displayMan->updateScreen();
		if ((_selectedDialogChoice == 99) && (choiceCount == 1) 
			&& (eventType != Common::EVENT_INVALID) && key.kbd.keycode == Common::KEYCODE_RETURN) {
			/* If a choice has not been made yet with the mouse and the dialog has only one possible choice and carriage return was pressed on the keyboard */
			_selectedDialogChoice = k1_DIALOG_CHOICE_1;
		}
	} while (_selectedDialogChoice == 99);
	_vm->_displayMan->_useByteBoxCoordinates = false;
	Box boxA = _vm->_eventMan->_primaryMouseInput[_selectedDialogChoice - 1]._hitbox;
	boxA._x1 -= 3;
	boxA._x2 += 3;
	boxA._y1 -= 3;
	boxA._y2 += 4;
	_vm->_eventMan->showMouse();
	_vm->_displayMan->_drawFloorAndCeilingRequested = true;
	Box boxB(0, 0, boxA._x2 - boxA._x1 + 3, boxA._y2 - boxA._y1 + 3);
	_vm->_displayMan->blitToBitmap(_vm->_displayMan->_bitmapScreen, _vm->_displayMan->_bitmapViewport,
										boxB, boxA._x1, boxA._y1, k160_byteWidthScreen, k160_byteWidthScreen, kM1_ColorNoTransparency, 200, 25);
	_vm->delay(1);
	boxB = boxA;
	boxB._y2 = boxB._y1;
	_vm->_displayMan->fillScreenBox(boxB, k5_ColorLightBrown);
	boxB = boxA;
	boxB._x2 = boxB._x1;
	boxB._y2--;
	_vm->_displayMan->fillScreenBox(boxB, k5_ColorLightBrown);
	boxB = boxA;
	boxB._y2--;
	boxB._y1 = boxB._y2;
	boxB._x1 -= 2;
	_vm->_displayMan->fillScreenBox(boxB, k0_ColorBlack);
	boxB = boxA;
	boxB._x1 = boxB._x2;
	_vm->_displayMan->fillScreenBox(boxB, k0_ColorBlack);
	_vm->delay(2);
	boxB = boxA;
	boxB._y1++;
	boxB._y2 = boxB._y1;
	boxB._x2 -= 2;
	_vm->_displayMan->fillScreenBox(boxB, k5_ColorLightBrown);
	boxB = boxA;
	boxB._x1++;
	boxB._x2 = boxB._x1;
	boxB._y2--;
	_vm->_displayMan->fillScreenBox(boxB, k5_ColorLightBrown);
	boxB = boxA;
	boxB._x2--;
	boxB._x1 = boxB._x2;
	_vm->_displayMan->fillScreenBox(boxB, k0_ColorBlack);
	boxB = boxA;
	boxB._y1 = boxB._y2 = boxB._y2 - 2;
	boxB._x1++;
	_vm->_displayMan->fillScreenBox(boxB, k0_ColorBlack);
	boxB = boxA;
	boxB._y1 = boxB._y2 = boxB._y2 + 2;
	boxB._x1--;
	boxB._x2 += 2;
	_vm->_displayMan->fillScreenBox(boxB, k13_ColorLightestGray);
	boxB = boxA;
	boxB._x1 = boxB._x2 = boxB._x2 + 3;
	boxB._y2 += 2;
	_vm->_displayMan->fillScreenBox(boxB, k13_ColorLightestGray);
	_vm->delay(2);
	boxA._x2 += 3;
	boxA._y2 += 3;
	_vm->_displayMan->blitToBitmap(_vm->_displayMan->_bitmapViewport, _vm->_displayMan->_bitmapScreen,
										boxA, 0, 0, k160_byteWidthScreen, k160_byteWidthScreen, kM1_ColorNoTransparency, 25, k200_heightScreen);
	_vm->_eventMan->hideMouse();
	_vm->_eventMan->_primaryMouseInput = primaryMouseInputBackup;
	_vm->_eventMan->_secondaryMouseInput = secondaryMouseInputBackup;
	_vm->_eventMan->_primaryKeyboardInput = primaryKeyboardInputBackup;
	_vm->_eventMan->_secondaryKeyboardInput = secondaryKeyboardInputBackup;
	_vm->_eventMan->discardAllInput();
	_vm->_eventMan->showMouse();
	return _selectedDialogChoice;
}
}
