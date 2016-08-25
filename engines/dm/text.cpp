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
	_messageAreaCursorColumn = 0;
	_messageAreaCursorRow = 0;
	for (uint16 i = 0; i < 4; ++i)
		_messageAreaRowExpirationTime[i] = 0;
	_bitmapMessageAreaNewRow = new byte[320 * 7];
}

TextMan::~TextMan() {
	delete[] _bitmapMessageAreaNewRow;
}

#define k5_LetterWidth 5
#define k6_LetterHeight 6

void TextMan::printTextToBitmap(byte* destBitmap, uint16 destByteWidth, int16 destX, int16 destY,
									Color textColor, Color bgColor, const char* text, uint16 destHeight) {
	if ((destX -= 1) < 0) // fixes missalignment, to be checked
		destX = 0;
	if ((destY -= 4) < 0) // fixes missalignment, to be checked
		destY = 0;

	uint16 destPixelWidth = destByteWidth * 2;

	uint16 textLength = strlen(text);
	uint16 nextX = destX;
	uint16 nextY = destY;
	byte *srcBitmap = _vm->_displayMan->getNativeBitmapOrGraphic(k557_FontGraphicIndice);

	byte *tmp = _vm->_displayMan->_tmpBitmap;
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
		_vm->_displayMan->blitToBitmap(srcBitmap, destBitmap, box, (nextX == destX) ? (srcX + 1) : srcX, 0, 6 * 128 / 2, destByteWidth, kM1_ColorNoTransparency,
											k6_LetterHeight, destHeight);

		nextX += k5_LetterWidth + 1;
	}
}

void TextMan::printToLogicalScreen(uint16 destX, uint16 destY, Color textColor, Color bgColor, const char* text) {
	printTextToBitmap(_vm->_displayMan->_bitmapScreen, _vm->_displayMan->_screenWidth / 2, destX, destY, textColor, bgColor, text, _vm->_displayMan->_screenHeight);
}

void TextMan::printToViewport(int16 posX, int16 posY, Color textColor, const char* text, Color bgColor) {
	printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, posX, posY, textColor, bgColor, text, k136_heightViewport);
}

void TextMan::printWithTrailingSpaces(byte* destBitmap, int16 destByteWidth, int16 destX, int16 destY, Color textColor,
										  Color bgColor, const char* text, int16 requiredTextLength, int16 destHeight) {
	Common::String str = text;
	for (int16 i = str.size(); i < requiredTextLength; ++i)
		str += ' ';
	printTextToBitmap(destBitmap, destByteWidth, destX, destY, textColor, bgColor, str.c_str(), destHeight);
}

void TextMan::printLineFeed() {
	printMessage(k0_ColorBlack, "\n");
}

void TextMan::printMessage(Color color, const char* string) {
	uint16 L0031_ui_CharacterIndex;
	char L0033_ac_String[54];


	while (*string) {
		if (*string == '\n') { /* New line */
			string++;
			if ((_messageAreaCursorColumn != 0) || (_messageAreaCursorRow != 0)) {
				_messageAreaCursorColumn = 0;
				createNewRow();
			}
		} else {
			if (*string == ' ') {
				string++;
				if (_messageAreaCursorColumn != 53) {
					printString(color, " "); // TODO: I'm not sure this is like the original
				}
			} else {
				L0031_ui_CharacterIndex = 0;
				do {
					L0033_ac_String[L0031_ui_CharacterIndex++] = *string++;
				} while (*string && (*string != ' ') && (*string != '\n')); /* End of string, space or New line */
				L0033_ac_String[L0031_ui_CharacterIndex] = '\0';
				if (_messageAreaCursorColumn + L0031_ui_CharacterIndex > 53) {
					_messageAreaCursorColumn = 2;
					createNewRow();
				}
				printString(color, L0033_ac_String);
			}
		}
	}
}

void TextMan::createNewRow() {
	uint16 L0029_ui_RowIndex;

	if (_messageAreaCursorRow == 3) {
		isTextScrolling(&_textScroller, true);
		memset(_bitmapMessageAreaNewRow, k0_ColorBlack, 320 * 7);
		setScrollerCommand(&_textScroller, 1);

		for (L0029_ui_RowIndex = 0; L0029_ui_RowIndex < 3; L0029_ui_RowIndex++) {
			_messageAreaRowExpirationTime[L0029_ui_RowIndex] = _messageAreaRowExpirationTime[L0029_ui_RowIndex + 1];
		}
		_messageAreaRowExpirationTime[3] = -1;
	} else {
		_messageAreaCursorRow++;
	}
}

void TextMan::printString(Color color, const char* string) {
	int16 L0030_i_StringLength;

	L0030_i_StringLength = strlen(string);
	if (isTextScrolling(&_textScroller, false)) {
		printToLogicalScreen(_messageAreaCursorColumn * 6, (_messageAreaCursorRow * 7 - 6) + 177, color, k0_ColorBlack, string);
	} else {
		printTextToBitmap(_bitmapMessageAreaNewRow, k160_byteWidthScreen, _messageAreaCursorColumn * 6, 5, color, k0_ColorBlack, string, 7);
		if (isTextScrolling(&_textScroller, false))
			setScrollerCommand(&_textScroller, 1);
	}
	_messageAreaCursorColumn += L0030_i_StringLength;
	_messageAreaRowExpirationTime[_messageAreaCursorRow] = _vm->_gameTime + 200;

}

void TextMan::initialize() {
	moveCursor(0, 3);
	for (uint16 i = 0; i < 4; ++i)
		_messageAreaRowExpirationTime[i] = -1;
}

void TextMan::moveCursor(int16 column, int16 row) {
	if (column < 0) {
		column = 0;
	} else {
		if (column >= 53) {
			column = 52;
		}
	}
	_messageAreaCursorColumn = column;
	if (row < 0) {
		row = 0;
	} else {
		if (row >= 4) {
			row = 3;
		}
	}
	_messageAreaCursorRow = row;
}

void TextMan::clearExpiredRows() {
	uint16 L0026_ui_RowIndex;
	int32 L0027_l_ExpirationTime;
	Box L0028_s_Box;

	_vm->_displayMan->_useByteBoxCoordinates = false;
	L0028_s_Box._x1 = 0;
	L0028_s_Box._x2 = 319;
	for (L0026_ui_RowIndex = 0; L0026_ui_RowIndex < 4; L0026_ui_RowIndex++) {
		L0027_l_ExpirationTime = _messageAreaRowExpirationTime[L0026_ui_RowIndex];
		if ((L0027_l_ExpirationTime == -1) || (L0027_l_ExpirationTime > _vm->_gameTime))
			continue;
		L0028_s_Box._y2 = (L0028_s_Box._y1 = 172 + (L0026_ui_RowIndex * 7)) + 6;
		isTextScrolling(&_textScroller, true);
		_vm->_displayMan->fillBoxBitmap(_vm->_displayMan->_bitmapScreen, L0028_s_Box, k0_ColorBlack, k160_byteWidthScreen, k200_heightScreen);
		_messageAreaRowExpirationTime[L0026_ui_RowIndex] = -1;
	}
}

void TextMan::printEndGameString(int16 x, int16 y, Color textColor, char* text) {
	char* L1407_pc_Character;
	char L1408_ac_ModifiedString[50];

	L1407_pc_Character = L1408_ac_ModifiedString;
	*L1407_pc_Character = *text++;
	while (*L1407_pc_Character) {
		if ((*L1407_pc_Character >= 'A') && (*L1407_pc_Character <= 'Z')) {
			*L1407_pc_Character -= 64; /* Use the same font as the one used for scrolls */
		}
		L1407_pc_Character++;
		*L1407_pc_Character = *text++;
	}
	printToLogicalScreen(x, y, textColor, k12_ColorDarkestGray, L1408_ac_ModifiedString);
}

void TextMan::clearAllRows() {
	int16 L0023_i_RowIndex;

	isTextScrolling(&_textScroller, true);

	Box tmpBox(0, 319, 169, 199);
	_vm->_displayMan->fillScreenBox(tmpBox, k0_ColorBlack);
	
	_messageAreaCursorRow = 3;
	_messageAreaCursorColumn = 0;
	for (L0023_i_RowIndex = 0; L0023_i_RowIndex < 4; L0023_i_RowIndex++) {
		_messageAreaRowExpirationTime[L0023_i_RowIndex] = -1;
	}
}
}
