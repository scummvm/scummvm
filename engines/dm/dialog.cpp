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
	static Box K0068_s_Box1 = {0, 223, 101, 125};
	static Box K0069_s_Box2 = {0, 223,  76, 100};
	static Box K0070_s_Box3 = {0, 223,  51,  75};
	static Box G0469_s_Graphic561_Box_Dialog2ChoicesPatch = {102, 122, 89, 125};
	static Box G0470_s_Graphic561_Box_Dialog4ChoicesPatch = {102, 122, 62, 97};

	int16 L1308_i_X;
	int16 L1309_i_Y;
	int16 L1310_i_ChoiceCount;
	char L1312_ac_StringPart1[70];
	char L1313_ac_StringPart2[70];
	Box L1314_s_Box;


	_vm->_displayMan->f466_loadIntoBitmap(k0_dialogBoxGraphicIndice, _vm->_displayMan->_g296_bitmapViewport);
	//Strangerke: the version should be replaced by a ScummVM/RogueVM (?) string
	// TODO: replace with ScummVM version string
	_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, 192, 7, k2_ColorLightGray, k1_ColorDarkGary, "V2.2", k136_heightViewport);
	L1310_i_ChoiceCount = 1;
	if (choice2 != nullptr) {
		L1310_i_ChoiceCount++;
	}
	if (choice3 != nullptr) {
		L1310_i_ChoiceCount++;
	}
	if (choice4 != nullptr) {
		L1310_i_ChoiceCount++;
	}
	if (fading) {
		warning(false, "MISSING CODE: F0436_STARTEND_FadeToPalette in f427_dialogDraw");
	}
	if (clearScreen) {
		_vm->_displayMan->fillScreen(k0_ColorBlack);
	}
	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	if (L1310_i_ChoiceCount == 1) {
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, K0068_s_Box1, 0, 64, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, K0069_s_Box2, 0, 39, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, K0070_s_Box3, 0, 14, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
		f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice1, 112, 114);
	} else {
		if (L1310_i_ChoiceCount == 2) {
			_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, G0469_s_Graphic561_Box_Dialog2ChoicesPatch, 102, 52, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
			f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice1, 112, 77);
			f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice2, 112, 114);
		} else {
			if (L1310_i_ChoiceCount == 3) {
				f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice1, 112, 77);
				f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice2, 59, 114);
				f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice3, 166, 114);
			} else {
				if (L1310_i_ChoiceCount == 4) {
					_vm->_displayMan->f132_blitToBitmap(_vm->_displayMan->_g296_bitmapViewport, _vm->_displayMan->_g296_bitmapViewport, G0470_s_Graphic561_Box_Dialog4ChoicesPatch, 102, 99, k112_byteWidthViewport, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport, k136_heightViewport);
					f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice1, 59, 77);
					f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice2, 166, 77);
					f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice3, 59, 114);
					f425_printCenteredChoice(_vm->_displayMan->_g296_bitmapViewport, choice4, 166, 114);
				}
			}
		}
	}
	L1309_i_Y = 29;
	if (msg1 != nullptr) {
		if (f426_isMessageOnTwoLines(msg1, L1312_ac_StringPart1, L1313_ac_StringPart2)) {
			L1309_i_Y = 21;
			L1308_i_X = 113 - ((strlen(L1312_ac_StringPart1) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, L1308_i_X, L1309_i_Y, k11_ColorYellow, k5_ColorLightBrown, L1312_ac_StringPart1, k136_heightViewport);
			L1309_i_Y += 8;
			L1308_i_X = 113 - ((strlen(L1313_ac_StringPart2) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, L1308_i_X, L1309_i_Y, k11_ColorYellow, k5_ColorLightBrown, L1313_ac_StringPart2, k136_heightViewport);
			L1309_i_Y += 8;
		} else {
			L1308_i_X = 113 - ((strlen(msg1) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, L1308_i_X, L1309_i_Y, k11_ColorYellow, k5_ColorLightBrown, msg1, k136_heightViewport);
			L1309_i_Y += 8;
		}
	}
	if (msg2 != nullptr) {
		if (f426_isMessageOnTwoLines(msg2, L1312_ac_StringPart1, L1313_ac_StringPart2)) {
			L1308_i_X = 113 - ((strlen(L1312_ac_StringPart1) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, L1308_i_X, L1309_i_Y, k9_ColorGold, k5_ColorLightBrown, L1312_ac_StringPart1, k136_heightViewport);
			L1309_i_Y += 8;
			L1308_i_X = 113 - ((strlen(L1313_ac_StringPart2) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, L1308_i_X, L1309_i_Y, k9_ColorGold, k5_ColorLightBrown, L1313_ac_StringPart2, k136_heightViewport);
		} else {
			L1308_i_X = 113 - ((strlen(msg2) * 6) >> 1);
			_vm->_textMan->f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, L1308_i_X, L1309_i_Y, k9_ColorGold, k5_ColorLightBrown, msg2, k136_heightViewport);
		}
	}
	if (screenDialog) {
		L1314_s_Box._y1 = 33;
		L1314_s_Box._y2 = 168;
		L1314_s_Box._x1 = 47;
		L1314_s_Box._x2 = 270;
		_vm->_eventMan->f78_showMouse();
		_vm->_displayMan->f21_blitToScreen(_vm->_displayMan->_g296_bitmapViewport, &L1314_s_Box, k112_byteWidthViewport, kM1_ColorNoTransparency, k136_heightViewport);
		_vm->_eventMan->f77_hideMouse();
	} else {
		_vm->_displayMan->f97_drawViewport(k0_viewportNotDungeonView);
		_vm->f22_delay(1);
	}
	if (fading) {
		warning(false, "MISSING CODE: F0436_STARTEND_FadeToPalette in f427_dialogDraw");
	}
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
