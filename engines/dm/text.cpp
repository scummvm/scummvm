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

void TextMan::f40_printTextToBitmap(byte* destBitmap, uint16 destByteWidth, uint16 destX, uint16 destY,
									Color textColor, Color bgColor, const char* text, uint16 destHeight) {
	destX -= 1; // fixes missalignment, to be checked
	destY -= 4; // fixes missalignment, to be checked

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
		uint16 srcX = (1 + 5) * toupper(*begin); // 1 + 5 is not the letter width, arbitrary choice of the unpacking code

		Box box((nextX == destX) ? (nextX + 1) : nextX, nextX + k5_LetterWidth + 1, nextY, nextY + k6_LetterHeight - 1);
		_vm->_displayMan->f132_blitToBitmap(srcBitmap, destBitmap, box, (nextX == destX) ? (srcX + 1) : srcX, 0, 6 * 128 / 2, destByteWidth, kM1_ColorNoTransparency);

		nextX += k5_LetterWidth + 1;
	}
}

void TextMan::f53_printToLogicalScreen(uint16 destX, uint16 destY, Color textColor, Color bgColor, const char* text) {
	f40_printTextToBitmap(_vm->_displayMan->_g348_bitmapScreen, _vm->_displayMan->_screenWidth / 2, destX, destY, textColor, bgColor, text, _vm->_displayMan->_screenHeight);
}

void TextMan::f52_printToViewport(int16 posX, int16 posY, Color textColor, const char* text, Color bgColor) {
	f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport, posX, posY, textColor, bgColor, text, k200_heightScreen);
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
		warning(false, "MISSING CODE: F0561_SCROLLER_IsTextScrolling");
		memset(_g356_bitmapMessageAreaNewRow, k0_ColorBlack, 320 * 7);
		warning(false, "MISSING CODE: F0560_SCROLLER_SetCommand");
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
	warning(false, "MISSING CODE: F0561_SCROLLER_IsTextScrolling");
	if (true) { // there is a test here with F0561_SCROLLER_IsTextScrolling
		_vm->_textMan->f53_printToLogicalScreen(_g359_messageAreaCursorColumn * 6, (_g358_messageAreaCursorRow * 7) + 177, color, k0_ColorBlack, string);
	} else {
		f40_printTextToBitmap(_g356_bitmapMessageAreaNewRow, k160_byteWidthScreen, _g359_messageAreaCursorColumn * 6, 5, color, k0_ColorBlack, string, 7);
		warning(false, "MISSING CODE: F0561_SCROLLER_IsTextScrolling");
		warning(false, "MISSING CODE: F0560_SCROLLER_SetCommand");
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
}
