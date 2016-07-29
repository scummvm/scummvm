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

DialogMan::DialogMan(DMEngine* vm) : _vm(vm) {}

void DialogMan::f427_dialogDraw(char* msg1, char* msg2, char* choice1, char* choice2, char* choice3, char* choice4, bool screenDialog, bool clearScreen, bool fading) {
	static Box constBox1 = Box(0, 223, 101, 125);
	static Box constBox2 = Box(0, 223,  76, 100);
	static Box constBox3 = Box(0, 223,  51,  75);
	static Box dialog2ChoicesPatch = Box(102, 122, 89, 125);
	static Box dialog4ChoicesPatch = Box(102, 122, 62, 97);

	_vm->_displayMan->f466_loadIntoBitmap(k0_dialogBoxGraphicIndice, _vm->_displayMan->_g296_bitmapViewport);
	//Strangerke: the version should be replaced by a ScummVM/RogueVM (?) string
	// TODO: replace with ScummVM version string
	_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, 192, 7, k2_ColorLightGray, k1_ColorDarkGary, "V2.2", k136_heightViewport);
	int16 choiceCount = 1;
	if (choice2)
		choiceCount++;

	if (choice3)
		choiceCount++;

	if (choice4)
		choiceCount++;

	if (fading)
		_vm->_displayMan->f436_STARTEND_FadeToPalette(_vm->_displayMan->_g345_aui_BlankBuffer);

	if (clearScreen)
		_vm->_displayMan->fillScreen(k0_ColorBlack);

	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	if (choiceCount == 1) {
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, constBox1, 0, 64, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, constBox2, 0, 39, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, constBox3, 0, 14, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice1, 112, 114);
	} else if (choiceCount == 2) {
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, dialog2ChoicesPatch, 102, 52, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice1, 112, 77);
		f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice2, 112, 114);
	} else if (choiceCount == 3) {
		f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice1, 112, 77);
		f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice2, 59, 114);
		f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice3, 166, 114);
	} else if (choiceCount == 4) {
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, dialog4ChoicesPatch, 102, 99, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice1, 59, 77);
		f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice2, 166, 77);
		f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice3, 59, 114);
		f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice4, 166, 114);
	}

	int16 textPosX;
	int16 textPosY = 29;
	if (msg1) {
		char L1312_ac_StringPart1[70];
		char L1313_ac_StringPart2[70];
		if (f426_isMessageOnTwoLines(msg1, L1312_ac_StringPart1, L1313_ac_StringPart2)) {
			textPosY = 21;
			textPosX = 113 - ((strlen(L1312_ac_StringPart1) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k11_ColorYellow, k5_ColorLightBrown, L1312_ac_StringPart1, k136_heightViewport);
			textPosY += 8;
			textPosX = 113 - ((strlen(L1313_ac_StringPart2) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k11_ColorYellow, k5_ColorLightBrown, L1313_ac_StringPart2, k136_heightViewport);
			textPosY += 8;
		} else {
			textPosX = 113 - ((strlen(msg1) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k11_ColorYellow, k5_ColorLightBrown, msg1, k136_heightViewport);
			textPosY += 8;
		}
	}
	if (msg2) {
		char L1312_ac_StringPart1[70];
		char L1313_ac_StringPart2[70];
		if (f426_isMessageOnTwoLines(msg2, L1312_ac_StringPart1, L1313_ac_StringPart2)) {
			textPosX = 113 - ((strlen(L1312_ac_StringPart1) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k9_ColorGold, k5_ColorLightBrown, L1312_ac_StringPart1, k136_heightViewport);
			textPosY += 8;
			textPosX = 113 - ((strlen(L1313_ac_StringPart2) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k9_ColorGold, k5_ColorLightBrown, L1313_ac_StringPart2, k136_heightViewport);
		} else {
			textPosX = 113 - ((strlen(msg2) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, textPosX, textPosY, k9_ColorGold, k5_ColorLightBrown, msg2, k136_heightViewport);
		}
	}
	if (screenDialog) {
		Box displayBox;
		displayBox._y1 = 33;
		displayBox._y2 = 168;
		displayBox._x1 = 47;
		displayBox._x2 = 270;
		_vm->_eventMan->f78_showMouse();
		_vm->_displayMan->f21_blitToScreen(_vm->_displayMan->_g296_bitmapViewport, &displayBox, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport);
		_vm->_eventMan->f77_hideMouse();
	} else {
		_vm->_displayMan->f97_drawViewport(k0_viewportNotDungeonView);
		_vm->f22_delay(1);
	}

	if (fading)
		_vm->_displayMan->f436_STARTEND_FadeToPalette(_vm->_displayMan->_g347_paletteTopAndBottomScreen);

	_vm->_displayMan->_g297_drawFloorAndCeilingRequested = true;
}

void DialogMan::f425_printCenteredChoice(byte* bitmap, char* str, int16 posX, int16 posY) {
	if (str) {
		posX -= (strlen(str) * 6) >> 1;
		_vm->_textMan->f40_printTextToBitmap(bitmap, k112_byteWidthViewport, posX, posY, k9_ColorGold, k5_ColorLightBrown, str, k136_heightViewport);
	}
}

bool DialogMan::f426_isMessageOnTwoLines(char* str, char* part1, char* part2) {
	uint16 L1305_ui_StringLength;
	uint16 L1306_ui_SplitPosition;


	L1305_ui_StringLength = strlen(str);
	if (L1305_ui_StringLength <= 30) {
		return false;
	}
	strcpy(part1, str);
	L1306_ui_SplitPosition = L1305_ui_StringLength >> 1;
	while ((part1[L1306_ui_SplitPosition] != ' ') && L1306_ui_SplitPosition < L1305_ui_StringLength) {
		L1306_ui_SplitPosition++;
	}
	part1[L1306_ui_SplitPosition] = '\0';
	strcpy(part2, &part1[L1306_ui_SplitPosition + 1]);
	return true;
}
}
