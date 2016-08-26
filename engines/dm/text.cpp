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

TextMan::TextMan(DMEngine* vm) : _vm(vm) {}

#define k5_LetterWidth 5
#define k6_LetterHeight 6

void TextMan::f40_printTextToBitmap(byte* destBitmap, uint16 destPixelWidth, uint16 destX, uint16 destY,
								Color textColor, Color bgColor, const char* text, uint16 destHeight) {
	destX -= 1; // fixes missalignment, to be checked
	destY -= 4; // fixes missalignment, to be checked

	uint16 textLength = strlen(text);
	uint16 nextX = destX;
	uint16 nextY = destY;
	byte *srcBitmap = _vm->_displayMan->f489_getBitmap(k557_FontGraphicIndice);

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
		_vm->_displayMan->f132_blitToBitmap(srcBitmap, 6 * 128, (nextX == destX) ? (srcX + 1) : srcX, 0, destBitmap, destPixelWidth,
			box, k255_ColorNoTransparency);

		nextX += k5_LetterWidth + 1;
	}
}

void TextMan::f53_printToLogicalScreen(uint16 destX, uint16 destY, Color textColor, Color bgColor, const char* text) {
	f40_printTextToBitmap(_vm->_displayMan->_g348_bitmapScreen, _vm->_displayMan->_screenWidth, destX, destY, textColor, bgColor, text, _vm->_displayMan->_screenHeight);
}

void TextMan::f52_printToViewport(int16 posX, int16 posY, Color textColor, const char* text, Color bgColor) {
	f40_printTextToBitmap(_vm->_displayMan->_g296_bitmapViewport, k112_byteWidthViewport * 2, posX, posY, textColor, bgColor, text, k200_heightScreen);
}

void TextMan::f41_printWithTrailingSpaces(byte* destBitmap, int16 destPixelWidth, int16 destX, int16 destY, Color textColor,
									  Color bgColor, const char* text, int16 requiredTextLength, int16 destHeight) {
	Common::String str = text;
	for (int16 i = str.size(); i < requiredTextLength; ++i)
		str += ' ';
	f40_printTextToBitmap(destBitmap, destPixelWidth, destX, destY, textColor, bgColor, str.c_str(), destHeight);
}

}
