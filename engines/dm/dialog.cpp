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

#include "dm/dialog.h"
#include "dm/gfx.h"
#include "dm/text.h"
#include "dm/eventman.h"

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

	EventManager &evtMan = *_vm->_eventMan;
	DisplayMan &displMan = *_vm->_displayMan;
	TextMan &txtMan = *_vm->_textMan;

	displMan.loadIntoBitmap(kDMGraphicIdxDialogBox, displMan._bitmapViewport);
	//Strangerke: the version should be replaced by a ScummVM/RogueVM (?) string
	// TODO: replace with ScummVM version string
	txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, 192, 7, kDMColorLightGray, kDMColorDarkGary, "V2.2", k136_heightViewport);
	int16 choiceCount = 1;
	if (choice2)
		choiceCount++;

	if (choice3)
		choiceCount++;

	if (choice4)
		choiceCount++;

	if (fading)
		displMan.startEndFadeToPalette(displMan._blankBuffer);

	if (clearScreen)
		displMan.fillScreen(kDMColorBlack);

	displMan._useByteBoxCoordinates = false;
	if (choiceCount == 1) {
		displMan.blitToBitmap(displMan._bitmapViewport, displMan._bitmapViewport, constBox1, 0, 64, k112_byteWidthViewport, k112_byteWidthViewport, kDMColorNoTransparency, k136_heightViewport, k136_heightViewport);
		displMan.blitToBitmap(displMan._bitmapViewport, displMan._bitmapViewport, constBox2, 0, 39, k112_byteWidthViewport, k112_byteWidthViewport, kDMColorNoTransparency, k136_heightViewport, k136_heightViewport);
		displMan.blitToBitmap(displMan._bitmapViewport, displMan._bitmapViewport, constBox3, 0, 14, k112_byteWidthViewport, k112_byteWidthViewport, kDMColorNoTransparency, k136_heightViewport, k136_heightViewport);
		printCenteredChoice(displMan._bitmapViewport, choice1, 112, 114);
	} else if (choiceCount == 2) {
		displMan.blitToBitmap(displMan._bitmapViewport, displMan._bitmapViewport, dialog2ChoicesPatch, 102, 52, k112_byteWidthViewport, k112_byteWidthViewport, kDMColorNoTransparency, k136_heightViewport, k136_heightViewport);
		printCenteredChoice(displMan._bitmapViewport, choice1, 112, 77);
		printCenteredChoice(displMan._bitmapViewport, choice2, 112, 114);
	} else if (choiceCount == 3) {
		printCenteredChoice(displMan._bitmapViewport, choice1, 112, 77);
		printCenteredChoice(displMan._bitmapViewport, choice2, 59, 114);
		printCenteredChoice(displMan._bitmapViewport, choice3, 166, 114);
	} else if (choiceCount == 4) {
		displMan.blitToBitmap(displMan._bitmapViewport, displMan._bitmapViewport, dialog4ChoicesPatch, 102, 99, k112_byteWidthViewport, k112_byteWidthViewport, kDMColorNoTransparency, k136_heightViewport, k136_heightViewport);
		printCenteredChoice(displMan._bitmapViewport, choice1, 59, 77);
		printCenteredChoice(displMan._bitmapViewport, choice2, 166, 77);
		printCenteredChoice(displMan._bitmapViewport, choice3, 59, 114);
		printCenteredChoice(displMan._bitmapViewport, choice4, 166, 114);
	}

	int16 textPosX;
	int16 textPosY = 29;
	if (msg1) {
		char L1312_ac_StringPart1[70];
		char L1313_ac_StringPart2[70];
		if (isMessageOnTwoLines(msg1, L1312_ac_StringPart1, L1313_ac_StringPart2)) {
			textPosY = 21;
			textPosX = 113 - ((strlen(L1312_ac_StringPart1) * 6) >> 1);
			txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, kDMColorYellow, kDMColorLightBrown, L1312_ac_StringPart1, k136_heightViewport);
			textPosY += 8;
			textPosX = 113 - ((strlen(L1313_ac_StringPart2) * 6) >> 1);
			txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, kDMColorYellow, kDMColorLightBrown, L1313_ac_StringPart2, k136_heightViewport);
			textPosY += 8;
		} else {
			textPosX = 113 - ((strlen(msg1) * 6) >> 1);
			txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, kDMColorYellow, kDMColorLightBrown, msg1, k136_heightViewport);
			textPosY += 8;
		}
	}
	if (msg2) {
		char L1312_ac_StringPart1[70];
		char L1313_ac_StringPart2[70];
		if (isMessageOnTwoLines(msg2, L1312_ac_StringPart1, L1313_ac_StringPart2)) {
			textPosX = 113 - ((strlen(L1312_ac_StringPart1) * 6) >> 1);
			txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, kDMColorGold, kDMColorLightBrown, L1312_ac_StringPart1, k136_heightViewport);
			textPosY += 8;
			textPosX = 113 - ((strlen(L1313_ac_StringPart2) * 6) >> 1);
			txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, kDMColorGold, kDMColorLightBrown, L1313_ac_StringPart2, k136_heightViewport);
		} else {
			textPosX = 113 - ((strlen(msg2) * 6) >> 1);
			txtMan.printTextToBitmap(displMan._bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, kDMColorGold, kDMColorLightBrown, msg2, k136_heightViewport);
		}
	}
	if (screenDialog) {
		Box displayBox(47, 270, 33, 168);
		evtMan.showMouse();
		displMan.blitToScreen(displMan._bitmapViewport, &displayBox, k112_byteWidthViewport, kDMColorNoTransparency, k136_heightViewport);
		evtMan.hideMouse();
	} else {
		displMan.drawViewport(k0_viewportNotDungeonView);
		_vm->delay(1);
	}

	if (fading)
		displMan.startEndFadeToPalette(displMan._paletteTopAndBottomScreen);

	displMan._drawFloorAndCeilingRequested = true;
	displMan.updateScreen();
}

void DialogMan::printCenteredChoice(byte *bitmap, const char *str, int16 posX, int16 posY) {
	if (str) {
		posX -= (strlen(str) * 6) >> 1;
		_vm->_textMan->printTextToBitmap(bitmap, k112_byteWidthViewport, posX, posY, kDMColorGold, kDMColorLightBrown, str, k136_heightViewport);
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
	EventManager &evtMan = *_vm->_eventMan;
	DisplayMan &displMan = *_vm->_displayMan;

	evtMan.hideMouse();
	MouseInput *primaryMouseInputBackup = evtMan._primaryMouseInput;
	MouseInput *secondaryMouseInputBackup = evtMan._secondaryMouseInput;
	KeyboardInput *primaryKeyboardInputBackup = evtMan._primaryKeyboardInput;
	KeyboardInput *secondaryKeyboardInputBackup = evtMan._secondaryKeyboardInput;
	evtMan._secondaryMouseInput = nullptr;
	evtMan._primaryKeyboardInput = nullptr;
	evtMan._secondaryKeyboardInput = nullptr;
	evtMan._primaryMouseInput = evtMan._primaryMouseInputDialogSets[dialogSetIndex][choiceCount - 1];
	evtMan.discardAllInput();
	_selectedDialogChoice = 99;
	do {
		Common::Event key;
		Common::EventType eventType = evtMan.processInput(&key);
		evtMan.processCommandQueue();
		_vm->delay(1);
		displMan.updateScreen();
		if ((_selectedDialogChoice == 99) && (choiceCount == 1)
			&& (eventType != Common::EVENT_INVALID) && key.kbd.keycode == Common::KEYCODE_RETURN) {
			/* If a choice has not been made yet with the mouse and the dialog has only one possible choice and carriage return was pressed on the keyboard */
			_selectedDialogChoice = kDMDialogChoice1;
		}
	} while (_selectedDialogChoice == 99);
	displMan._useByteBoxCoordinates = false;
	Box boxA = evtMan._primaryMouseInput[_selectedDialogChoice - 1]._hitbox;
	boxA._rect.left -= 3;
	boxA._rect.right += 3;
	boxA._rect.top -= 3;
	boxA._rect.bottom += 4;
	evtMan.showMouse();
	displMan._drawFloorAndCeilingRequested = true;
	Box boxB(0, 0, boxA._rect.right - boxA._rect.left + 3, boxA._rect.bottom - boxA._rect.top + 3);
	displMan.blitToBitmap(displMan._bitmapScreen, displMan._bitmapViewport,
										boxB, boxA._rect.left, boxA._rect.top, k160_byteWidthScreen, k160_byteWidthScreen, kDMColorNoTransparency, 200, 25);
	_vm->delay(1);
	boxB = boxA;
	boxB._rect.bottom = boxB._rect.top;
	displMan.fillScreenBox(boxB, kDMColorLightBrown);
	boxB = boxA;
	boxB._rect.right = boxB._rect.left;
	boxB._rect.bottom--;
	displMan.fillScreenBox(boxB, kDMColorLightBrown);
	boxB = boxA;
	boxB._rect.bottom--;
	boxB._rect.top = boxB._rect.bottom;
	boxB._rect.left -= 2;
	displMan.fillScreenBox(boxB, kDMColorBlack);
	boxB = boxA;
	boxB._rect.left = boxB._rect.right;
	displMan.fillScreenBox(boxB, kDMColorBlack);
	_vm->delay(2);
	boxB = boxA;
	boxB._rect.top++;
	boxB._rect.bottom = boxB._rect.top;
	boxB._rect.right -= 2;
	displMan.fillScreenBox(boxB, kDMColorLightBrown);
	boxB = boxA;
	boxB._rect.left++;
	boxB._rect.right = boxB._rect.left;
	boxB._rect.bottom--;
	displMan.fillScreenBox(boxB, kDMColorLightBrown);
	boxB = boxA;
	boxB._rect.right--;
	boxB._rect.left = boxB._rect.right;
	displMan.fillScreenBox(boxB, kDMColorBlack);
	boxB = boxA;
	boxB._rect.top = boxB._rect.bottom = boxB._rect.bottom - 2;
	boxB._rect.left++;
	displMan.fillScreenBox(boxB, kDMColorBlack);
	boxB = boxA;
	boxB._rect.top = boxB._rect.bottom = boxB._rect.bottom + 2;
	boxB._rect.left--;
	boxB._rect.right += 2;
	displMan.fillScreenBox(boxB, kDMColorLightestGray);
	boxB = boxA;
	boxB._rect.left = boxB._rect.right = boxB._rect.right + 3;
	boxB._rect.bottom += 2;
	displMan.fillScreenBox(boxB, kDMColorLightestGray);
	_vm->delay(2);
	boxA._rect.right += 3;
	boxA._rect.bottom += 3;
	displMan.blitToBitmap(displMan._bitmapViewport, displMan._bitmapScreen,
										boxA, 0, 0, k160_byteWidthScreen, k160_byteWidthScreen, kDMColorNoTransparency, 25, k200_heightScreen);
	evtMan.hideMouse();
	evtMan._primaryMouseInput = primaryMouseInputBackup;
	evtMan._secondaryMouseInput = secondaryMouseInputBackup;
	evtMan._primaryKeyboardInput = primaryKeyboardInputBackup;
	evtMan._secondaryKeyboardInput = secondaryKeyboardInputBackup;
	evtMan.discardAllInput();
	evtMan.showMouse();
	return _selectedDialogChoice;
}
}
