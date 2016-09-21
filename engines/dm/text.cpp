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

#include "common/system.h"

#include "dm/text.h"

namespace DM {

TextMan::TextMan(DMEngine *vm) : _vm(vm) {
	_messageAreaCursorColumn = 0;
	_messageAreaCursorRow = 0;
	for (uint16 i = 0; i < 4; ++i)
		_messageAreaRowExpirationTime[i] = 0;

	_bitmapMessageAreaNewRow = new byte[320 * 7];
	_isScrolling = false;
	_startedScrollingAt = -1;
	_messageAreaCopy = new byte[320 * 7 * 4];
}

TextMan::~TextMan() {
	delete[] _bitmapMessageAreaNewRow;
	delete[] _messageAreaCopy;
}

void TextMan::printTextToBitmap(byte *destBitmap, uint16 destByteWidth, int16 destX, int16 destY,
								Color textColor, Color bgColor, const char *text, uint16 destHeight) {
	if ((destX -= 1) < 0) // fixes missalignment, to be checked
		destX = 0;
	if ((destY -= 4) < 0) // fixes missalignment, to be checked
		destY = 0;

	uint16 destPixelWidth = destByteWidth * 2;

	uint16 textLength = strlen(text);
	uint16 nextX = destX;
	uint16 nextY = destY;
	byte *srcBitmap = _vm->_displayMan->getNativeBitmapOrGraphic(kDMGraphicIdxFont);

	byte *tmp = _vm->_displayMan->_tmpBitmap;
	for (uint16 i = 0; i < (kDMFontLetterWidth + 1) * kDMFontLetterHeight * 128; ++i)
		tmp[i] = srcBitmap[i] ? textColor : bgColor;

	srcBitmap = tmp;

	for (const char *begin = text, *end = text + textLength; begin != end; ++begin) {
		if (nextX + kDMFontLetterWidth + 1 >= destPixelWidth || (*begin == '\n')) {
			nextX = destX;
			nextY += kDMFontLetterHeight + 1;
		}
		if (nextY + kDMFontLetterHeight >= destHeight)
			break;

		uint16 srcX = (1 + 5) * *begin; // 1 + 5 is not the letter width, arbitrary choice of the unpacking code

		Box box((nextX == destX) ? (nextX + 1) : nextX, nextX + kDMFontLetterWidth + 1, nextY, nextY + kDMFontLetterHeight - 1);
		_vm->_displayMan->blitToBitmap(srcBitmap, destBitmap, box, (nextX == destX) ? (srcX + 1) : srcX, 0, 6 * 128 / 2, destByteWidth, kDMColorNoTransparency,
									   kDMFontLetterHeight, destHeight);

		nextX += kDMFontLetterWidth + 1;
	}
}

void TextMan::printToLogicalScreen(uint16 destX, uint16 destY, Color textColor, Color bgColor, const char *text) {
	printTextToBitmap(_vm->_displayMan->_bitmapScreen, _vm->_displayMan->_screenWidth / 2, destX, destY, textColor, bgColor, text, _vm->_displayMan->_screenHeight);
}

void TextMan::printToViewport(int16 posX, int16 posY, Color textColor, const char *text, Color bgColor) {
	printTextToBitmap(_vm->_displayMan->_bitmapViewport, k112_byteWidthViewport, posX, posY, textColor, bgColor, text, k136_heightViewport);
}

void TextMan::printWithTrailingSpaces(byte *destBitmap, int16 destByteWidth, int16 destX, int16 destY, Color textColor,
									  Color bgColor, const char *text, int16 requiredTextLength, int16 destHeight) {
	Common::String str = text;
	for (int16 i = str.size(); i < requiredTextLength; ++i)
		str += ' ';
	printTextToBitmap(destBitmap, destByteWidth, destX, destY, textColor, bgColor, str.c_str(), destHeight);
}

void TextMan::printLineFeed() {
	printMessage(kDMColorBlack, "\n");
}

void TextMan::printMessage(Color color, const char *string, bool printWithScroll) {
	uint16 characterIndex;
	Common::String wrkString;

	while (*string) {
		if (*string == '\n') { /* New line */
			string++;
			if ((_messageAreaCursorColumn != 0) || (_messageAreaCursorRow != 0)) {
				_messageAreaCursorColumn = 0;
				createNewRow();
			}
		} else if (*string == ' ') {
			string++;
			if (_messageAreaCursorColumn != 53) {
				printString(color, " "); // I'm not sure if this is like the original
			}
		} else {
			characterIndex = 0;
			do {
				wrkString += *string++;
				characterIndex++;
			} while (*string && (*string != ' ') && (*string != '\n')); /* End of string, space or New line */
			wrkString += '\0';
			if (_messageAreaCursorColumn + characterIndex > 53) {
				_messageAreaCursorColumn = 2;
				createNewRow();
			}
			printString(color, wrkString.c_str());
		}
	}
}

void TextMan::createNewRow() {
	if (_messageAreaCursorRow == 3) {
		isTextScrolling(&_textScroller, true);
		memset(_bitmapMessageAreaNewRow, kDMColorBlack, 320 * 7);
		_isScrolling = true;
		setScrollerCommand(&_textScroller, 1);

		for (uint16 rowIndex = 0; rowIndex < 3; rowIndex++)
			_messageAreaRowExpirationTime[rowIndex] = _messageAreaRowExpirationTime[rowIndex + 1];

		_messageAreaRowExpirationTime[3] = -1;
	} else
		_messageAreaCursorRow++;
}

void TextMan::printString(Color color, const char* string) {
	int16 stringLength = strlen(string);
	if (isTextScrolling(&_textScroller, false))
		printToLogicalScreen(_messageAreaCursorColumn * 6, (_messageAreaCursorRow * 7 - 1) + 177, color, kDMColorBlack, string);
	else {
		printTextToBitmap(_bitmapMessageAreaNewRow, k160_byteWidthScreen, _messageAreaCursorColumn * 6, 0, color, kDMColorBlack, string, 7);
		_isScrolling = true;
		if (isTextScrolling(&_textScroller, false))
			setScrollerCommand(&_textScroller, 1);
	}
	_messageAreaCursorColumn += stringLength;
	_messageAreaRowExpirationTime[_messageAreaCursorRow] = _vm->_gameTime + 200;

}

void TextMan::initialize() {
	moveCursor(0, 3);
	for (uint16 i = 0; i < 4; ++i)
		_messageAreaRowExpirationTime[i] = -1;
}

void TextMan::moveCursor(int16 column, int16 row) {
	if (column < 0)
		column = 0;
	else if (column >= 53)
		column = 52;

	_messageAreaCursorColumn = column;
	if (row < 0)
		row = 0;
	else if (row >= 4)
		row = 3;

	_messageAreaCursorRow = row;
}

void TextMan::clearExpiredRows() {
	_vm->_displayMan->_useByteBoxCoordinates = false;
	Box displayBox;
	displayBox._rect.left = 0;
	displayBox._rect.right = 319;
	for (uint16 rowIndex = 0; rowIndex < 4; rowIndex++) {
		int32 expirationTime = _messageAreaRowExpirationTime[rowIndex];
		if ((expirationTime == -1) || (expirationTime > _vm->_gameTime) || _isScrolling)
			continue;
		displayBox._rect.top = 172 + (rowIndex * 7);
		displayBox._rect.bottom = displayBox._rect.top + 6;
		isTextScrolling(&_textScroller, true);
		_vm->_displayMan->fillBoxBitmap(_vm->_displayMan->_bitmapScreen, displayBox, kDMColorBlack, k160_byteWidthScreen, k200_heightScreen);
		_messageAreaRowExpirationTime[rowIndex] = -1;
	}
}

void TextMan::printEndGameString(int16 x, int16 y, Color textColor, const char* text) {
	char modifiedString[50];

	char *wrkStringPtr = modifiedString;
	*wrkStringPtr = *text++;
	while (*wrkStringPtr) {
		if ((*wrkStringPtr >= 'A') && (*wrkStringPtr <= 'Z'))
			*wrkStringPtr -= 64; /* Use the same font as the one used for scrolls */

		wrkStringPtr++;
		*wrkStringPtr = *text++;
	}
	printToLogicalScreen(x, y, textColor, kDMColorDarkestGray, modifiedString);
}

void TextMan::clearAllRows() {
	isTextScrolling(&_textScroller, true);

	Box tmpBox(0, 319, 169, 199);
	_vm->_displayMan->fillScreenBox(tmpBox, kDMColorBlack);

	_messageAreaCursorRow = 3;
	_messageAreaCursorColumn = 0;
	for (int16 rowIndex = 0; rowIndex < 4; rowIndex++)
		_messageAreaRowExpirationTime[rowIndex] = -1;
}

void TextMan::updateMessageArea() {
	if (_isScrolling) {
		if (_startedScrollingAt == -1) {
			_startedScrollingAt = g_system->getMillis();
			memcpy(_messageAreaCopy, _vm->_displayMan->_bitmapScreen + (200 - 7 * 4) * 320, 320 * 7 * 4);
		}

		int linesToCopy = (g_system->getMillis() - _startedScrollingAt) / 50;
		if (linesToCopy >= 7) {
			linesToCopy = 7;
			_startedScrollingAt = -1;
			_isScrolling = false;
		}
		memcpy(_vm->_displayMan->_bitmapScreen + (200 - 7 * 4) * 320, _messageAreaCopy + linesToCopy * 320,
				320 * (7 * 4 - linesToCopy));
		memcpy(_vm->_displayMan->_bitmapScreen + (200 - linesToCopy) * 320, _bitmapMessageAreaNewRow, 320 * linesToCopy);
	}
}

}
