/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "graphics/paletteman.h"
#include "common/system.h"
#include "efh/efh.h"

namespace Efh {

void EfhEngine::initPalette() {
	// Strangerke - values from a tool I wrote in 2008. I can't remember if it's guess work or not.
	static const uint8 pal[3 * 16] = {
		0x00, 0x00, 0x00,
		0x00, 0x00, 0xAA,
		0x00, 0xAA, 0x00,
		0x00, 0xAA, 0xAA,
		0xAA, 0x00, 0x00,
		0xAA, 0x00, 0xAA,
		0xAA, 0x55, 0x00,
		0xAA, 0xAA, 0xAA,
		0x55, 0x55, 0x55,
		0x55, 0x55, 0xFF,
		0x01, 0x01, 0x01, // Color 0xA is for transparency
		0x55, 0xFF, 0xFF,
		0xFF, 0x55, 0x55,
		0xFF, 0x55, 0xFF,
		0xFF, 0xFF, 0x55,
		0xFF, 0xFF, 0xFF
	};

	debugC(1, kDebugGraphics, "initPalette");
	_system->getPaletteManager()->setPalette(pal, 0, 16);
	_system->updateScreen();
}

void EfhEngine::drawLeftCenterBox() {
	debugC(1, kDebugGraphics, "drawLeftCenterBox");
	drawColoredRect(16, 8, 111, 135, 0);
}

void EfhEngine::displayNextAnimFrame() {
	debugC(6, kDebugGraphics, "displayNextAnimFrame");

	if (++_unkAnimRelatedIndex >= 15)
		_unkAnimRelatedIndex = 0;

	displayAnimFrame();
}

void EfhEngine::displayAnimFrame() {
	debugC(1, kDebugGraphics, "displayAnimFrame");
	// The original had a parameter. As it was always equal to zero, it was removed in ScummVM

	if (_animImageSetId == 0xFF)
		return;

	if (_animImageSetId == 0xFE) {
		displayRawDataAtPos(_portraitSubFilesArray[0], 16, 8);
		return;
	}

	displayRawDataAtPos(_portraitSubFilesArray[0], 16, 8);
	for (int i = 0; i < 4; ++i) {
		int8 var2 = _animInfo[_animImageSetId]._frameList[_unkAnimRelatedIndex]._subFileId[i];
		if (var2 == -1)
			continue;
		displayRawDataAtPos(_portraitSubFilesArray[var2 + 1], _animInfo[_animImageSetId]._posX[var2] + 16, _animInfo[_animImageSetId]._posY[var2] + 8);
	}
}

void EfhEngine::displayAnimFrames(int16 animId, bool displayMenuBoxFl) {
	debugC(1, kDebugGraphics, "displayAnimFrames %d %s", animId, displayMenuBoxFl ? "True" : "False");
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
	debugC(1, kDebugGraphics, "displayFctFullScreen");

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
	debugC(1, kDebugGraphics, "displayBufferBmAtPos %d %d", posX, posY);
	// CHECKME: Quick code to display stuff, may require to really reverse the actual function
	uint8 *destPtr = (uint8 *)_mainSurface->getBasePtr(posX, posY);
	int counter = 0;
	for (int line = 0; line < bufferBM->_height; ++line) {
		for (int col = 0; col < bufferBM->_lineDataSize; ++col) { // _lineDataSize = _width / 2
			if (bufferBM->_dataPtr[counter] >> 4 != 0xA)
				destPtr[320 * line + 2 * col] = bufferBM->_dataPtr[counter] >> 4;
			if ((bufferBM->_dataPtr[counter] & 0xF) != 0xA)
				destPtr[320 * line + 2 * col + 1] = bufferBM->_dataPtr[counter] & 0xF;
			++counter;
		}
	}

	//	_system->copyRectToScreen((uint8 *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
	//	_system->updateScreen();
}

void EfhEngine::drawRect(int16 minX, int16 minY, int16 maxX, int16 maxY) {
	debugC(1, kDebugGraphics, "drawRect %d-%d %d-%d", minX, minY, maxX, maxY);

	if (minY > maxY)
		SWAP(minY, maxY);

	if (minX > maxX)
		SWAP(minX, maxX);

	minX = CLIP<int16>(minX, 0, 319);
	maxX = CLIP<int16>(maxX, 0, 319);
	minY = CLIP<int16>(minY, 0, 199);
	maxY = CLIP<int16>(maxY, 0, 199);

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

void EfhEngine::drawColoredRect(int16 minX, int16 minY, int16 maxX, int16 maxY, int16 color) {
	debugC(1, kDebugGraphics, "drawColoredRect %d-%d %d-%d %d", minX, minY, maxX, maxY, color);

	uint8 oldValue = _defaultBoxColor;
	_defaultBoxColor = color;
	drawRect(minX, minY, maxX, maxY);
	_defaultBoxColor = oldValue;
}

void EfhEngine::clearScreen(int16 color) {
	debugC(1, kDebugGraphics, "clearScreen %d", color);
	drawColoredRect(0, 0, 320, 200, color);
}

void EfhEngine::displayRawDataAtPos(uint8 *imagePtr, int16 posX, int16 posY) {
	debugC(1, kDebugGraphics, "displayRawDataAtPos %d %d", posX, posY);
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

void EfhEngine::drawString(const char *str, int16 startX, int16 startY, uint16 textColor) {
	debugC(1, kDebugGraphics, "drawString %s %d %d %d", str, startX, startY, textColor);
	const uint8 *curPtr = (const uint8 *)str;
	uint16 lineHeight = _fontDescr._charHeight + _fontDescr._extraVerticalSpace;
	int16 minX = startX;

	if (textColor & 0x8000) {
		warning("STUB - drawString - 0x8000");
		// int16 minY = startY;                                 // Used in case 0x8000
		// int16 var6 = _fontDescr._extraLines[0] + startY - 1; // Used in case 0x8000
	}

	for (uint curChar = *curPtr++; curChar != 0; curChar = *curPtr++) {
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

void EfhEngine::displayCenteredString(Common::String str, int16 minX, int16 maxX, int16 posY) {
	debugC(1, kDebugGraphics, "displayCenteredString %s %d-%d %d", str.c_str(), minX, maxX, posY);
	uint16 length = getStringWidth(str.c_str());
	int16 startCenteredDisplayX = minX + (maxX - minX - length) / 2;
	drawString(str.c_str(), startCenteredDisplayX, posY, _textColor);
}

void EfhEngine::displayMenuAnswerString(const char *str, int16 minX, int16 maxX, int16 posY) {
	debugC(1, kDebugGraphics, "displayMenuAnswerString %s %d-%d %d", str, minX, maxX, posY);
	displayCenteredString(str, minX, maxX, posY);
	displayFctFullScreen();
	displayCenteredString(str, minX, maxX, posY);
}

void EfhEngine::drawMapWindow() {
	debugC(1, kDebugGraphics, "drawMapWindow");
	drawColoredRect(128, 8, 303, 135, 0);
}

void EfhEngine::displayGameScreen() {
	debugC(1, kDebugGraphics, "displayGameScreen");
	clearScreen(0);
	drawUpperLeftBorders();
	drawUpperRightBorders();
	drawBottomBorders();
	displayAnimFrame();
	displayLowStatusScreen(false);
}

void EfhEngine::drawUpperLeftBorders() {
	debugC(1, kDebugGraphics, "drawUpperLeftBorders");
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 0);
	displayRawDataAtPos(_circleImageSubFileArray[1], 112, 0);
	displayRawDataAtPos(_circleImageSubFileArray[3], 16, 0);
}

void EfhEngine::drawUpperRightBorders() {
	debugC(1, kDebugGraphics, "drawUpperRightBorders");
	displayRawDataAtPos(_circleImageSubFileArray[2], 304, 0);
	displayRawDataAtPos(_circleImageSubFileArray[4], 128, 0);
}

void EfhEngine::drawBottomBorders() {
	debugC(1, kDebugGraphics, "drawBottomBorders");
	displayRawDataAtPos(_circleImageSubFileArray[7], 16, 136);
	displayRawDataAtPos(_circleImageSubFileArray[8], 16, 192);
	displayRawDataAtPos(_circleImageSubFileArray[5], 0, 136);
	displayRawDataAtPos(_circleImageSubFileArray[6], 304, 136);
}

void EfhEngine::drawChar(uint8 curChar, int16 posX, int16 posY) {
	debugC(1, kDebugGraphics, "drawChar %c %d %d", curChar, posX, posY);

	// CHECKME: Quick hacked display, may require rework

	// Note: The original is making of a variable which is set to _textColor & 0x3FFF
	// It seems _textColor is always set to a small value and thus this variable
	// has been removed.

	uint8 *destPtr = (uint8 *)_mainSurface->getBasePtr(posX, posY);

	int16 charId = curChar - 0x20;
	uint8 width = _fontDescr._widthArray[charId];

	for (uint line = 0; line < 8; ++line) {
		int16 x = 0;
		for (int i = 7; i >= 7 - width; --i) {
			if (_fontDescr._fontData[charId]._lines[line] & (1 << i))
				destPtr[320 * line + x] = _textColor;
			++x;
		}
	}
}

void EfhEngine::setTextColorWhite() {
	debugC(1, kDebugGraphics, "setTextColorWhite");

	if (_videoMode == 8) // CGA
		_textColor = 0x3;
	else
		_textColor = 0xF;
}

void EfhEngine::setTextColorRed() {
	debugC(1, kDebugGraphics, "setTextColorRed");

	if (_videoMode == 8) // CGA
		_textColor = 0x2;
	else
		_textColor = 0xC;
}

void EfhEngine::setTextColorGrey() {
	debugC(1, kDebugGraphics, "setTextColorGrey");

	if (_videoMode == 8) // CGA
		_textColor = 0x1;
	else
		_textColor = 0x8;
}

void EfhEngine::displayStringAtTextPos(Common::String message) {
	debugC(1, kDebugGraphics, "displayStringAtTextPos %s", message.c_str());

	drawString(message.c_str(), _textPosX, _textPosY, _textColor);
	_textPosX += getStringWidth(message.c_str()) + 1;
	setNextCharacterPos();
}

void EfhEngine::clearBottomTextZone(int16 color) {
	debugC(1, kDebugGraphics, "clearBottomTextZone %d", color);

	drawColoredRect(16, 152, 302, 189, color);
}

void EfhEngine::clearBottomTextZone_2(int16 color) {
	debugC(1, kDebugGraphics, "clearBottomTextZone_2 %d", color);

	displayColoredMenuBox(16, 152, 302, 189, color);
}

void EfhEngine::setNextCharacterPos() {
	debugC(1, kDebugGraphics, "setNextCharacterPos");

	if (_textPosX <= 311)
		return;

	_textPosX = 0;
	_textPosY += 8;

	if (_textPosY > 191)
		_textPosY = 0;
}

void EfhEngine::displayCharAtTextPos(char character) {
	debugC(1, kDebugGraphics, "displayCharAtTextPos %c", character);

	char buffer[2];
	buffer[0] = character;
	buffer[1] = 0;

	drawString(buffer, _textPosX, _textPosY, _textColor);
	_textPosX += getStringWidth(buffer) + 1;
	setNextCharacterPos();
}

void EfhEngine::displayWindow(uint8 *buffer, int16 posX, int16 posY, uint8 *dest) {
	debugC(1, kDebugGraphics, "displayWindow %d %d", posX, posY);

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

void EfhEngine::displayColoredMenuBox(int16 minX, int16 minY, int16 maxX, int16 maxY, int16 color) {
	debugC(1, kDebugGraphics, "displayColoredMenuBox %d %d -> %d %d %d", minX, minY, maxX, maxY, color);

	drawColoredRect(minX, minY, maxX, maxY, color);
	displayFctFullScreen();
	drawColoredRect(minX, minY, maxX, maxY, color);
}

} // End of namespace Efh
