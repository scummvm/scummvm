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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "efh/efh.h"
#include "graphics/palette.h"
#include "common/system.h"

namespace Efh {

void EfhEngine::initPalette() {
	// Strangerke - values from a tool I wrote in 2008. I can't remember if it's guess work or not.
	const uint8 pal[3 * 16] = {
		0, 0, 0,
		0, 0, 170,
		0, 170, 0,
		0, 170, 170,
		170, 0, 0,
		170, 0, 170,
		170, 85, 0,
		170, 170, 170,
		85, 85, 85,
		85, 85, 255,
		1, 1, 1,
		85, 255, 255,
		255, 85, 85,
		255, 85, 255,
		255, 255, 85,
		255, 255, 255
	};

	_system->getPaletteManager()->setPalette(pal, 0, 16);
	_system->updateScreen();
}

void EfhEngine::drawLeftCenterBox() {
	drawColoredRect(16, 8, 111, 135, 0);
}

void EfhEngine::displayAnimFrame() {
	// The original had a parameter. As it was always equal to zero, it was removed in ScummVM

	if (_animImageSetId == 0xFF)
		return;

	if (_animImageSetId == 0xFE) {
		displayRawDataAtPos(_portraitSubFilesArray[0], 16, 8);
		return;
	}

	displayRawDataAtPos(_portraitSubFilesArray[0], 16, 8);
	for (int i = 0; i < 4; ++i) {
		int16 var2 = _animInfo[_animImageSetId]._unkAnimArray[_unkAnimRelatedIndex].field0;
		if (var2 == 0xFF)
			continue;
		displayRawDataAtPos(_portraitSubFilesArray[var2 + 1], _animInfo[_animImageSetId]._field46_startX[var2] + 16, _animInfo[_animImageSetId]._field3C_startY[var2] + 8);
	}
}

void EfhEngine::displayAnimFrames(int16 animId, bool displayMenuBoxFl) {
	if (animId == 0xFF)
		return;

	_animImageSetId = animId;
	if (_animImageSetId == 0xFE)
		loadNewPortrait();
	else
		loadAnimImageSet();

	if (!displayMenuBoxFl)
		return;

	for (int i = 0; i < 2; ++i) {
		drawLeftCenterBox();
		displayAnimFrame();

		if (i == 0)
			displayFctFullScreen();
	}
}

void EfhEngine::displayFctFullScreen() {
	// CHECKME: 319 is in the original but looks suspicious.
	// copyDirtyRect(0, 0, 319, 200);

	_system->copyRectToScreen((uint8 *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
	_system->updateScreen();
}

void EfhEngine::copyDirtyRect(int16 minX, int16 minY, int16 maxX, int16 maxY) {
	_graphicsStruct->copy(_vgaGraphicsStruct2);
	_initRect = Common::Rect(minX, minY, maxX, maxY);
	copyGraphicBufferFromTo(_vgaGraphicsStruct2, _vgaGraphicsStruct1, _initRect, minX, minY);
}

void EfhEngine::copyGraphicBufferFromTo(EfhGraphicsStruct *efh_graphics_struct, EfhGraphicsStruct *efh_graphics_struct1, const Common::Rect &rect, int16 min_x, int16 min_y) {
	warning("STUB - copyGraphicBufferFromTo");
}

void EfhEngine::displayBufferBmAtPos(BufferBM *bufferBM, int16 posX, int16 posY) {
	// TODO: Quick code to display stuff, may require to really reverse the actual function
	uint8 *destPtr = (uint8 *)_mainSurface->getBasePtr(posX, posY);
	// warning("%d %d - startX %d startY %d width %d height %d lineDataSize %d fieldD %d", posX, posY, bufferBM->_startX, bufferBM->_startY, bufferBM->_width, bufferBM->_height, bufferBM->_lineDataSize, bufferBM->_fieldD);
	int counter = 0;
	for (int line = 0; line < bufferBM->_height; ++line) {
		for (int col = 0; col < bufferBM->_lineDataSize; ++col) { // _lineDataSize = _width / 2
			destPtr[320 * line + 2 * col] = bufferBM->_dataPtr[counter] >> 4;
			destPtr[320 * line + 2 * col + 1] = bufferBM->_dataPtr[counter] & 0xF;
			++counter;
		}
	}

	//	_system->copyRectToScreen((uint8 *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
	//	_system->updateScreen();
}

void EfhEngine::drawRect(int minX, int minY, int maxX, int maxY) {
	if (minY > maxY)
		SWAP(minY, maxY);

	if (minX > maxX)
		SWAP(minX, maxX);

	// warning("drawRect - _graphicsStruct x %d -> %d, y %d -> %d", _graphicsStruct->_area.left, _graphicsStruct->_area.right, _graphicsStruct->_area.top, _graphicsStruct->_area.bottom);

	minX = CLIP(minX, 0, 319);
	maxX = CLIP(maxX, 0, 319);
	minY = CLIP(minY, 0, 199);
	maxY = CLIP(maxY, 0, 199);

	int deltaY = 1 + maxY - minY;
	int deltaX = 1 + maxX - minX;

	uint8 color = _defaultBoxColor & 0xF;
	bool xorColor = (_defaultBoxColor & 0x40) != 0;
	uint8 *destPtr = (uint8 *)_mainSurface->getBasePtr(minX, minY);

	for (int line = 0; line < deltaY; ++line) {
		for (int col = 0; col < deltaX; ++col) {
			if (xorColor)
				destPtr[320 * line + col] ^= color;
			else
				destPtr[320 * line + col] = color;
		}
	}
}

void EfhEngine::drawColoredRect(int minX, int minY, int maxX, int maxY, int color) {
	uint8 oldValue = _defaultBoxColor;
	_defaultBoxColor = color;
	drawRect(minX, minY, maxX, maxY);
	_defaultBoxColor = oldValue;
}

void EfhEngine::clearScreen(int color) {
	drawColoredRect(0, 0, 320, 200, color);
}

void EfhEngine::displayRawDataAtPos(uint8 *imagePtr, int16 posX, int16 posY) {
	uint16 height = READ_LE_INT16(imagePtr);
	uint16 width = READ_LE_INT16(imagePtr + 2);
	uint8 *imageData = imagePtr + 4;

	_imageDataPtr._lineDataSize = width;
	_imageDataPtr._dataPtr = imageData;
	_imageDataPtr._height = height;
	_imageDataPtr._width = width * 2; // 2 pixels per byte
	_imageDataPtr._startX = _imageDataPtr._startY = 0;

	displayBufferBmAtPos(&_imageDataPtr, posX, posY);
}

void EfhEngine::drawString(const char *str, int16 startX, int16 startY, uint16 unkFl) {
	uint8 *curPtr = (uint8 *)str;
	uint16 lineHeight = _fontDescr._charHeight + _fontDescr._extraVerticalSpace;
	_unk_sub26437_flag = unkFl & 0x3FFF;
	int16 minX = startX;
	int16 minY = startY;                                 // Used in case 0x8000
	int16 var6 = _fontDescr._extraLines[0] + startY - 1; // Used in case 0x8000

	if (unkFl & 0x8000) {
		warning("STUB - drawString - 0x8000");
	}

	for (uint8 curChar = *curPtr++; curChar != 0; curChar = *curPtr++) {
		if (curChar == 0x0A) {
			startX = minX;
			startY += lineHeight;
			continue;
		}

		if (curChar < 0x20)
			continue;

		uint16 characterId = (curChar + 0xE0) & 0xFF;
		uint8 charWidth = _fontDescr._widthArray[characterId];

		if (startX + charWidth >= 319) {
			startX = minX;
			startY += lineHeight;
		}

		uint8 varC = _fontDescr._extraLines[characterId];
		drawChar(curChar, startX, startY + varC);
		startX += charWidth + _fontDescr._extraHorizontalSpace;
	}
}

void EfhEngine::displayCenteredString(const char *str, int16 minX, int16 maxX, int16 posY) {
	uint16 length = getStringWidth(str);
	int16 startCenteredDisplayX = minX + (maxX - minX - length) / 2;
	drawString(str, startCenteredDisplayX, posY, _textColor);
}

void EfhEngine::drawMapWindow() {
	drawColoredRect(128, 8, 303, 135, 0);
}

void EfhEngine::displayGameScreen() {
	clearScreen(0);
	drawUpperLeftBorders();
	drawUpperRightBorders();
	drawBottomBorders();
	displayAnimFrame();
	displayLowStatusScreen(false);
}

void EfhEngine::drawUpperLeftBorders() {
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 0);
	displayRawDataAtPos(_circleImageSubFileArray[1], 112, 0);
	displayRawDataAtPos(_circleImageSubFileArray[3], 16, 0);
}

void EfhEngine::drawUpperRightBorders() {
	displayRawDataAtPos(_circleImageSubFileArray[2], 304, 0);
	displayRawDataAtPos(_circleImageSubFileArray[4], 128, 0);
}

void EfhEngine::drawBottomBorders() {
	displayRawDataAtPos(_circleImageSubFileArray[7], 16, 136);
	displayRawDataAtPos(_circleImageSubFileArray[8], 16, 192);
	displayRawDataAtPos(_circleImageSubFileArray[5], 0, 136);
	displayRawDataAtPos(_circleImageSubFileArray[6], 304, 136);
}

void EfhEngine::drawChar(uint8 curChar, int16 posX, int posY) {
	// Quick hacked display, may require rework
	uint8 *destPtr = (uint8 *)_mainSurface->getBasePtr(posX, posY);

	int16 charId = curChar - 0x20;
	uint8 width = _fontDescr._widthArray[charId];

	for (int16 line = 0; line < 8; ++line) {
		int16 x = 0;
		for (int i = 7; i >= 7 - width; --i) {
			if (_fontDescr._fontData[charId]._lines[line] & (1 << i))
				destPtr[320 * line + x] = _textColor;
			++x;
		}
	}
}

void EfhEngine::setTextColorWhite() {
	if (_videoMode == 8) // CGA
		_textColor = 0x3;
	else
		_textColor = 0xF;
}

void EfhEngine::setTextColorRed() {
	if (_videoMode == 8) // CGA
		_textColor = 0x2;
	else
		_textColor = 0xC;
}

void EfhEngine::setTextColorGrey() {
	if (_videoMode == 8) // CGA
		_textColor = 0x1;
	else
		_textColor = 0x8;
}

void EfhEngine::displayStringAtTextPos(const char *message) {
	drawString(message, _textPosX, _textPosY, _textColor);
	_textPosX += getStringWidth(message) + 1;
	setNextCharacterPos();
}

void EfhEngine::unkFct_displayMenuBox_2(int16 color) {
	drawColoredRect(16, 152, 302, 189, color);
}

void EfhEngine::setNextCharacterPos() {
	if (_textPosX <= 311)
		return;

	_textPosX = 0;
	_textPosY += 8;

	if (_textPosY > 191)
		_textPosY = 0;
}

void EfhEngine::displayChar(char character) {
	char buffer[2];
	buffer[0] = character;
	buffer[1] = 0;

	drawString(buffer, _textPosX, _textPosY, _textColor);
	_textPosX += getStringWidth(buffer) + 1;
	setNextCharacterPos();
}

void EfhEngine::displayWindow(uint8 *buffer, int16 posX, int16 posY, uint8 *dest) {
	if (buffer == nullptr) {
		warning("Target Buffer Not Defined...DCImage!"); // That's the original message... And yes, it's wrong: it's checking the source buffer :)
		return;
	}

	// Only MCGA handled, the rest is skipped
	uncompressBuffer(buffer, dest);
	displayRawDataAtPos(dest, posX, posY);
	displayFctFullScreen();
	displayRawDataAtPos(dest, posX, posY);
}

} // End of namespace Efh
