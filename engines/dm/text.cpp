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

#define kLetterWidth 5
#define kLetterHeight 6

void TextMan::printTextToBitmap(byte* destBitmap, uint16 destPixelWidth, uint16 destX, uint16 destY,
								Color textColor, Color bgColor, const char* text, uint16 destHeight, Viewport &viewport) {
	destX -= 1; // fixes missalignment, to be checked
	destY -= 4; // fixes missalignment, to be checked

	uint16 textLength = strlen(text);
	uint16 nextX = destX;
	uint16 nextY = destY;
	byte *srcBitmap = _vm->_displayMan->getBitmap(kFontGraphicIndice);

	byte *tmp = _vm->_displayMan->_tmpBitmap;
	for (uint16 i = 0; i < (kLetterWidth + 1) * kLetterHeight * 128; ++i) {
		tmp[i] = srcBitmap[i] ? textColor : bgColor;
	}
	srcBitmap = tmp;

	for (const char *begin = text, *end = text + textLength; begin != end; ++begin) {
		if (nextX + kLetterWidth + 1 >= (viewport._posX + viewport._width) || (*begin == '\n')) {
			nextX = destX;
			nextY += kLetterHeight + 1;
		}
		if (nextY + kLetterHeight >= (viewport._posY + viewport._height))
			break;
		uint16 srcX = (1 + 5) * toupper(*begin); // 1 + 5 is not the letter width, arbitrary choice of the unpacking code
		_vm->_displayMan->blitToBitmap(srcBitmap, 6 * 128, (nextX == destX) ? (srcX + 1) : srcX, 0, destBitmap, destPixelWidth,
									   (nextX == destX) ? (nextX + 1) : nextX, nextX + kLetterWidth + 1, nextY, nextY + kLetterHeight, kColorNoTransparency, viewport);
		nextX += kLetterWidth + 1;
	}
}

void TextMan::printTextToScreen(uint16 destX, uint16 destY, Color textColor, Color bgColor, const char* text, Viewport &viewport) {
	printTextToBitmap(_vm->_displayMan->_vgaBuffer, _vm->_displayMan->_screenWidth, destX, destY, textColor, bgColor, text, _vm->_displayMan->_screenHeight, viewport);
}

void TextMan::printToViewport(int16 posX, int16 posY, Color textColor, const char* text) {
	printTextToScreen(posX, posY, textColor, kColorDarkestGray, text, gDungeonViewport);
}

}
