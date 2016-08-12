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

#include "text.h"


namespace DM {

TextMan::TextMan(DMEngine* vm) : _vm(vm) {
	_g359_messageAreaCursorColumn = 0;
	_g358_messageAreaCursorRow = 0;
	for (uint16 i = 0; i < 4; ++i)
		_g360_messageAreaRowExpirationTime[i] = 0;
	_g356_bitmapMessageAreaNewRow = new byte[320 * 7];
}

TextMan::~TextMan() {
	delete[] _g356_bitmapMessageAreaNewRow;
}

#define k5_LetterWidth 5
#define k6_LetterHeight 6

void TextMan::f40_printTextToBitmap(byte* destBitmap, uint16 destByteWidth, int16 destX, int16 destY,
									Color textColor, Color bgColor, const char* text, uint16 destHeight) {
	if ((destX -= 1) < 0) // fixes missalignment, to be checked
		destX = 0;
	if ((destY -= 4) < 0) // fixes missalignment, to be checked
		destY = 0;

	uint16 destPixelWidth = destByteWidth * 2;

	uint16 textLength = strlen(text);
	uint16 nextX = destX;
	uint16 nextY = destY;
	byte *srcBitmap = _vm->_displayMan->f489_getNativeBitmapOrGraphic(k557_FontGraphicIndice);

	byte *tmp = _vm->_displayMan->_g74_tmpBitmap;
	for (uint16 i = 0; i < (k5_LetterWidth + 1) * k6_LetterHeight * 128; ++i) {
		tmp[i] = srcBitmap[i] ? textColor : bgColor;
	}
	srcBitmap = tmp;

	for (const char *begin = text, *end = text + textLength; begin != end; ++begin) {
		if (nextX + k5_LetterWidth + 1 >= destPixelWidth || (*begin == '\n')) {
			nextX = destX;
			nextY += k6_LetterHeight + 1;
		}
		if (nextY + k6_LetterHeight >= destHeight)
			break;
		uint16 srcX = (1 + 5) * *begin; // 1 + 5 is not the letter width, arbitrary choice of the unpacking code

		Box box((nextX == destX) ? (nextX + 1) : nextX, nextX + k5_LetterWidth + 1, nextY, nextY + k6_LetterHeight - 1);
		_vm->_displayMan->f132_blitToBitmap(srcBitmap, destBitmap, box, (nextX == destX) ? (srcX + 1) : srcX, 0, 6 * 128 / 2, destByteWidth, kM1_ColorNoTransparency,
											k6_LetterHeight, destHeight);

		nextX += k5_LetterWidth + 1;
	}
}

void TextMan::f53_printToLogicalScreen(uint16 destX, uint16 destY, Color textColor, Color bgColor, const char* text) {
	f40_printTextToBitmap(_vm->_displayMan->_g348_bitmapScreen, _vm->_displayMan->_screenWidth / 2, destX, destY, textColor, bgColor, text, _vm->_displayMan->_screenHeight);
}

void TextMan::f52_printToViewport(int16 posX, int16 posY, Color textColor, const char* text, Color bgColor) {
	f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, posX, posY, textColor, bgColor, text, k136_heightViewport);
}

void TextMan::f41_printWithTrailingSpaces(byte* destBitmap, int16 destByteWidth, int16 destX, int16 destY, Color textColor,
										  Color bgColor, const char* text, int16 requiredTextLength, int16 destHeight) {
	Common::String str = text;
	for (int16 i = str.size(); i < requiredTextLength; ++i)
		str += ' ';
	f40_printTextToBitmap(destBitmap, destByteWidth, destX, destY, textColor, bgColor, str.c_str(), destHeight);
}

void TextMan::f51_messageAreaPrintLineFeed() {
	f47_messageAreaPrintMessage(k0_ColorBlack, "\n");
}

void TextMan::f47_messageAreaPrintMessage(Color color, const char* string) {
	uint16 L0031_ui_CharacterIndex;
	char L0033_ac_String[54];


	while (*string) {
		if (*string == '\n') { /* New line */
			string++;
			if ((_g359_messageAreaCursorColumn != 0) || (_g358_messageAreaCursorRow != 0)) {
				_g359_messageAreaCursorColumn = 0;
				f45_messageAreaCreateNewRow();
			}
		} else {
			if (*string == ' ') {
				string++;
				if (_g359_messageAreaCursorColumn != 53) {
					f46_messageAreaPrintString(color, " "); // TODO: I'm not sure this is like the original
				}
			} else {
				L0031_ui_CharacterIndex = 0;
				do {
					L0033_ac_String[L0031_ui_CharacterIndex++] = *string++;
				} while (*string && (*string != ' ') && (*string != '\n')); /* End of string, space or New line */
				L0033_ac_String[L0031_ui_CharacterIndex] = '\0';
				if (_g359_messageAreaCursorColumn + L0031_ui_CharacterIndex > 53) {
					_g359_messageAreaCursorColumn = 2;
					f45_messageAreaCreateNewRow();
				}
				f46_messageAreaPrintString(color, L0033_ac_String);
			}
		}
	}
}

void TextMan::f45_messageAreaCreateNewRow() {
	uint16 L0029_ui_RowIndex;

	if (_g358_messageAreaCursorRow == 3) {
		f561_isTextScrolling(&_gK60_s_TextScroller, true);
		memset(_g356_bitmapMessageAreaNewRow, k0_ColorBlack, 320 * 7);
		f560_SCROLLER_setCommand(&_gK60_s_TextScroller, 1);

		for (L0029_ui_RowIndex = 0; L0029_ui_RowIndex < 3; L0029_ui_RowIndex++) {
			_g360_messageAreaRowExpirationTime[L0029_ui_RowIndex] = _g360_messageAreaRowExpirationTime[L0029_ui_RowIndex + 1];
		}
		_g360_messageAreaRowExpirationTime[3] = -1;
	} else {
		_g358_messageAreaCursorRow++;
	}
}

void TextMan::f46_messageAreaPrintString(Color color, const char* string) {
	int16 L0030_i_StringLength;

	L0030_i_StringLength = strlen(string);
	if (f561_isTextScrolling(&_gK60_s_TextScroller, false)) {
		f53_printToLogicalScreen(_g359_messageAreaCursorColumn * 6, (_g358_messageAreaCursorRow * 7 - 6) + 177, color, k0_ColorBlack, string);
	} else {
		f40_printTextToBitmap(_g356_bitmapMessageAreaNewRow, k160_byteWidthScreen, _g359_messageAreaCursorColumn * 6, 5, color, k0_ColorBlack, string, 7);
		if (f561_isTextScrolling(&_gK60_s_TextScroller, false))
			f560_SCROLLER_setCommand(&_gK60_s_TextScroller, 1);
	}
	_g359_messageAreaCursorColumn += L0030_i_StringLength;
	_g360_messageAreaRowExpirationTime[_g358_messageAreaCursorRow] = _vm->_g313_gameTime + 200;

}

void TextMan::f54_textInitialize() {
	f42_messageAreaMoveCursor(0, 3);
	for (uint16 i = 0; i < 4; ++i)
		_g360_messageAreaRowExpirationTime[i] = -1;
}

void TextMan::f42_messageAreaMoveCursor(int16 column, int16 row) {
	if (column < 0) {
		column = 0;
	} else {
		if (column >= 53) {
			column = 52;
		}
	}
	_g359_messageAreaCursorColumn = column;
	if (row < 0) {
		row = 0;
	} else {
		if (row >= 4) {
			row = 3;
		}
	}
	_g358_messageAreaCursorRow = row;
}

void TextMan::f44_messageAreaClearExpiredRows() {
	uint16 L0026_ui_RowIndex;
	int32 L0027_l_ExpirationTime;
	Box L0028_s_Box;

	_vm->_displayMan->_g578_useByteBoxCoordinates = false;
	L0028_s_Box._x1 = 0;
	L0028_s_Box._x2 = 319;
	for (L0026_ui_RowIndex = 0; L0026_ui_RowIndex < 4; L0026_ui_RowIndex++) {
		L0027_l_ExpirationTime = _g360_messageAreaRowExpirationTime[L0026_ui_RowIndex];
		if ((L0027_l_ExpirationTime == -1) || (L0027_l_ExpirationTime > _vm->_g313_gameTime))
			continue;
		L0028_s_Box._y2 = (L0028_s_Box._y1 = 172 + (L0026_ui_RowIndex * 7)) + 6;
		f561_isTextScrolling(&_gK60_s_TextScroller, true);
		_vm->_displayMan->f135_fillBoxBitmap(_vm->_displayMan->_g348_bitmapScreen, L0028_s_Box, k0_ColorBlack, k160_byteWidthScreen, k200_heightScreen);
		_g360_messageAreaRowExpirationTime[L0026_ui_RowIndex] = -1;
	}
}

void TextMan::f443_endgamePrintString(int16 x, int16 y, Color textColor, char* text) {
	char* L1407_pc_Character;
	char L1408_ac_ModifiedString[50];

	L1407_pc_Character = L1408_ac_ModifiedString;
	while (*L1407_pc_Character = *text++) {
		if ((*L1407_pc_Character >= 'A') && (*L1407_pc_Character <= 'Z')) {
			*L1407_pc_Character -= 64; /* Use the same font as the one used for scrolls */
		}
		L1407_pc_Character++;
	}
	f53_printToLogicalScreen(x, y, textColor, k12_ColorDarkestGray, L1408_ac_ModifiedString);
}

void TextMan::f43_messageAreaClearAllRows() {
	int16 L0023_i_RowIndex;

	f561_isTextScrolling(&_gK60_s_TextScroller, true);

	Box tmpBox(0, 319, 169, 199);
	_vm->_displayMan->D24_fillScreenBox(tmpBox, k0_ColorBlack);
	
	_g358_messageAreaCursorRow = 3;
	_g359_messageAreaCursorColumn = 0;
	for (L0023_i_RowIndex = 0; L0023_i_RowIndex < 4; L0023_i_RowIndex++) {
		_g360_messageAreaRowExpirationTime[L0023_i_RowIndex] = -1;
	}
}
}
