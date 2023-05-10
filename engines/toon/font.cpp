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

#include "common/debug.h"
#include "common/rect.h"

#include "toon/font.h"

namespace Toon {

FontRenderer::FontRenderer(ToonEngine *vm) : _vm(vm) {
	_currentFontColor[0] = 0;
	_currentFontColor[1] = 0xc8;
	_currentFontColor[2] = 0xcb;
	_currentFontColor[3] = 0xce;

	_currentFont = nullptr;
	_currentDemoFont = nullptr;
}

FontRenderer::~FontRenderer() {
	if (_currentDemoFont)
		delete _currentDemoFont;
}

// mapping extended characters required for foreign versions to font (animation)
static const byte map_textToFont[0x80] = {
	 '?',  '?',  '?',  '?', 0x03,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', // 0x8x
	 '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', // 0x9x
	 '?', 0x09,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', // 0xAx
	 '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', 0x0a, // 0xBx
	 '?',  '?',  '?',  '?', 0x1d,  '?',  '?', 0x02,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?', // 0xCx
	 '?', 0x0b,  '?',  '?',  '?',  '?', 0x1e,  '?',  '?',  '?',  '?', 0x20, 0x1f,  '?',  '?', 0x19, // 0xDx
	0x0d, 0x04, 0x0e,  '?', 0x1a,  '?',  '?', 0x18, 0x10, 0x0f, 0x12, 0x11, 0x09, 0x05, 0x14, 0x13, // 0xEx
	0x23, 0x08, 0x23, 0x06, 0x15, 0x23, 0x1b, 0x23, 0x23, 0x16, 0x07, 0x17, 0x1c, 0x23, 0x23, 0x23  // 0xFx
};

byte FontRenderer::textToFont(byte c) {
	// No need to remap simple characters.
	if (c < 0x80)
		return c;

	// The Spanish version shows grave accent over the 'e' when it should
	// be acute. This happens both in the original interpreter and when
	// using the common map which works for other languages, so we add a
	// special case for it.
	if (_vm->_language == Common::ES_ESP && c == 0xe9)
		return 0x10;

	// Use the common map to convert the extended characters.
	return map_textToFont[c - 0x80];
}

void FontRenderer::renderText(int16 x, int16 y, const Common::String &origText, int32 mode) {
	debugC(5, kDebugFont, "renderText(%d, %d, %s, %d)", x, y, origText.c_str(), mode);

	int16 xx, yy;
	computeSize(origText, &xx, &yy);

	if (mode & 2) {
		y -= yy / 2;
	} else if (mode & 4) {
		y -= yy;
	}

	if (mode & 1) {
		x -= xx / 2;
	}

	_vm->addDirtyRect(x, y, x + xx, y + yy);

	int16 curX = x;
	int16 curY = y;
	int32 height = 0;

	const byte *text = (const byte *)origText.c_str();
	while (*text) {
		byte curChar = *text;
		if (curChar == 13) {
			curY = curY + height;
			height = 0;
			curX = x;
		} else {
			curChar = textToFont(curChar);
			if (_currentFont) {
				_currentFont->drawFontFrame(_vm->getMainSurface(), curChar, curX, curY, _currentFontColor);
			} else {
				_currentDemoFont->drawGlyph(_vm->getMainSurface(), curChar, curX, curY, _currentFontColor);
			}
			curX = curX + MAX<int32>((_currentFont ? _currentFont->getFrameWidth(curChar) :
				_currentDemoFont->getGlyphWidth(curChar)) - 2, 0);
			height = MAX<int32>(height, _currentFont ? _currentFont->getFrameHeight(curChar) :
				_currentDemoFont->getHeight());
		}
		text++;
	}
}

void FontRenderer::computeSize(const Common::String &origText, int16 *retX, int16 *retY) {
	debugC(4, kDebugFont, "computeSize(%s, retX, retY)", origText.c_str());

	int16 lineWidth = 0;
	int16 lineHeight = 0;
	int16 totalHeight = 0;
	int16 totalWidth = 0;
	int16 lastLineHeight = 0;

	const byte *text = (const byte *)origText.c_str();
	while (*text) {
		byte curChar = *text;
		if (curChar == 13) {
			totalWidth = MAX(totalWidth, lineWidth);
			totalHeight += lineHeight;
			lineHeight = 0;
			lineWidth = 0;
			lastLineHeight = 0;
		} else if (curChar < 32) {
			text++;
			continue;
		} else {
			curChar = textToFont(curChar);
			int16 charWidth = (_currentFont ? _currentFont->getFrameWidth(curChar) :
				_currentDemoFont->getGlyphWidth(curChar)) - 1;
			int16 charHeight = _currentFont ? _currentFont->getFrameHeight(curChar) :
				_currentDemoFont->getHeight();
			lineWidth += charWidth;
			lineHeight = MAX(lineHeight, charHeight);

			// The character may be offset, so the height doesn't
			// really tell how far it will stick out. For now,
			// assume we only need to take the lower bound into
			// consideration.
			//Common::Rect charRect = _currentFont->getFrameRect(curChar);
			lastLineHeight = MAX(lastLineHeight, _currentFont ? _currentFont->getHeight() :
				(int16)_currentDemoFont->getHeight());
			
		}
		text++;
	}

	totalHeight += lastLineHeight;
	totalWidth = MAX(totalWidth, lineWidth);

	*retX = totalWidth;
	*retY = totalHeight;
}

void FontRenderer::setFont(Animation *font) {
	debugC(5, kDebugFont, "setFont(font)");

	_currentFont = font;
}

void FontRenderer::setFontColorByCharacter(int32 characterId) {
	debugC(5, kDebugFont, "setFontColorByCharacter(%d)", characterId);

	// unfortunately this table was hardcoded in the original executable
	static const byte colorsByCharacters[] = {
		0xe0, 0xdc, 0xc8,   0xd6, 0xc1, 0xc8,   0xe9, 0xde, 0xc8,   0xeb, 0xe8, 0xc8,
		0xd1, 0xcf, 0xc8,   0xdb, 0xd5, 0xc8,   0xfb, 0xfa, 0xc8,   0xd9, 0xd7, 0xc8,
		0xe8, 0xe4, 0xc8,   0xe9, 0xfa, 0xc8,   0xeb, 0xe4, 0xc8,   0xeb, 0xe4, 0xc8,
		0xd2, 0xea, 0xc8,   0xd3, 0xd0, 0xc8,   0xe1, 0xdd, 0xc8,   0xd9, 0xd7, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,
		0xd2, 0xcf, 0xc8,   0xd1, 0xcf, 0xc8,   0xd9, 0xd7, 0xc8,   0xe3, 0xdd, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,
		0xd9, 0xd7, 0xc8,   0xe6, 0xe4, 0xc8,   0xd9, 0xd7, 0xc8,   0xcd, 0xca, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xeb, 0xe8, 0xc8,   0xdb, 0xd5, 0xc8,
		0xe0, 0xdc, 0xc8,   0xd6, 0xc1, 0xc8,   0xd3, 0xd0, 0xc8,   0xd1, 0xcf, 0xc8,
		0xe6, 0xe4, 0xc8,   0xd1, 0xcf, 0xc8,   0xd2, 0xcf, 0xc8,   0xcc, 0xcb, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,
		0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8,   0xd9, 0xd7, 0xc8
	};

	setFontColor(colorsByCharacters[characterId * 3 + 2], colorsByCharacters[characterId * 3 + 1], colorsByCharacters[characterId * 3 + 0]);
}

void FontRenderer::setFontColor(int32 fontColor1, int32 fontColor2, int32 fontColor3) {
	debugC(5, kDebugFont, "setFontColor(%d, %d, %d)", fontColor1, fontColor2, fontColor3);

	// The English demo uses a different palette assignment.
	_currentFontColor[0] = 0;
	_currentFontColor[1] = _currentFont ? fontColor1 : fontColor3;
	_currentFontColor[2] = _currentFont ? fontColor2 : fontColor1;
	_currentFontColor[3] = _currentFont ? fontColor3 : fontColor2;
}

void FontRenderer::renderMultiLineText(int16 x, int16 y, const Common::String &origText, int32 mode, Graphics::Surface &frame) {
	debugC(5, kDebugFont, "renderMultiLineText(%d, %d, %s, %d)", x, y, origText.c_str(), mode);

	// divide the text in several lines
	// based on number of characters or size of lines.
	byte text[1024];
	memset(text, 0, 1024);
	Common::strlcpy((char *)text, origText.c_str(), 1024);

	byte *lines[16];
	memset(lines, 0, 16 * sizeof(byte *));
	int32 lineSize[16];
	memset(lineSize, 0, 16 * sizeof(int32));
	int32 numLines = 0;

	byte *it = text;

	int16 maxWidth = 0;
	int16 curWidth = 0;

	while (true) {
		byte *lastLine = it;
		byte *lastSpace = it;
		int32 lastSpaceX = 0;
		int32 curLetterNr = 0;
		curWidth = 0;

		while (*it && curLetterNr < 50 && curWidth < 580) {
			byte curChar = *it;
			if (curChar == 32) {
				lastSpace = it;
				lastSpaceX = curWidth;
			} else
				curChar = textToFont(curChar);

			int width = (_currentFont ? _currentFont->getFrameWidth(curChar) :
				_currentDemoFont->getGlyphWidth(curChar)) - 2;
			curWidth += MAX(width, 0);
			it++;
			curLetterNr++;
		}

		if (*lastLine == 0)
			break;

		lines[numLines] = lastLine;

		if (*it == 0)
			lineSize[numLines] = curWidth;
		else
			lineSize[numLines] = lastSpaceX;

		if (lineSize[numLines] > maxWidth)
			maxWidth = lineSize[numLines];

		lastLine = lastSpace + 1;
		numLines++;

		if (*it == 0)
			break;

		it = lastLine;
		*lastSpace = 0;

		if (numLines >= 16)
			break;
	}

	if (curWidth > maxWidth) {
		maxWidth = curWidth;
	}
	//numLines++;

	// get font height (assumed to be constant)
	int16 height = _currentFont ? _currentFont->getHeight() : _currentDemoFont->getHeight();
	int32 textSize = (height - 2) * numLines;
	y = y - textSize;
	if (y < 30)
		y = 30;
	if (y + textSize > 370)
		y = 370 - textSize;

	x -= _vm->state()->_currentScrollValue;

	// adapt x
	if (x - 30 - maxWidth / 2 < 0)
		x = maxWidth / 2 + 30;

	if (x + 30 + (maxWidth / 2) > TOON_SCREEN_WIDTH)
		x = TOON_SCREEN_WIDTH - (maxWidth / 2) - 30;

	// we have good coordinates now, we can render the multi line
	int16 curX = x;
	int16 curY = y;

	for (int32 i = 0; i < numLines; i++) {
		const byte *line = lines[i];
		curX = x - lineSize[i] / 2;
		_vm->addDirtyRect(curX + _vm->state()->_currentScrollValue, curY, curX + lineSize[i] + _vm->state()->_currentScrollValue + 2, curY + height);

		while (*line) {
			byte curChar = textToFont(*line);
			if (curChar != 32) {
				if (_currentFont) {
					_currentFont->drawFontFrame(frame, curChar, curX + _vm->state()->_currentScrollValue, curY, _currentFontColor);
				} else {
					_currentDemoFont->drawGlyph(frame, curChar, curX + _vm->state()->_currentScrollValue, curY, _currentFontColor);
				}
			}
			curX = curX + MAX<int32>((_currentFont ? _currentFont->getFrameWidth(curChar) : _currentDemoFont->getGlyphWidth(curChar)) - 2, 0);
			//height = MAX(height, _currentFont->getFrameHeight(curChar));
			line++;
		}
		curY += height;
	}
}

bool FontRenderer::loadDemoFont(const Common::String& filename) {
	uint32 fileSize = 0;
	uint8 *fileData = _vm->resources()->getFileData(filename, &fileSize);
	if (!fileData)
		return false;

	uint16 dataSize = READ_LE_UINT16(fileData);
	if (dataSize != fileSize)
		return false;

	// Offsets of the various parts of the font.
	uint16 fontDataOffset = READ_LE_UINT16(fileData + 4);
	uint16 glyphOffsetTableOffset = READ_LE_UINT16(fileData + 6);
	uint16 glyphWidthDataOffset = READ_LE_UINT16(fileData + 8);
	uint16 glyphDataOffset = READ_LE_UINT16(fileData + 10);
	uint16 glyphHeightDataOffset = READ_LE_UINT16(fileData + 12);

	// Generic font data.
	uint8 numGlyphs = *(fileData + fontDataOffset + 3);
	uint8 glyphWidth = *(fileData + fontDataOffset + 4);
	uint8 glyphHeight = *(fileData + fontDataOffset + 5);

	if (_currentDemoFont)
		delete _currentDemoFont;

	_currentDemoFont = new DemoFont(glyphWidth, glyphHeight, numGlyphs);

	// Copy the data for each glyph to the DemoFont.
	for (int i = 0; i < numGlyphs; i++) {
		// Read glyph dimensions.
		GlyphDimensions dimensions;
		dimensions.width = *(fileData + glyphWidthDataOffset + i);
		dimensions.heightOffset = *(fileData + glyphHeightDataOffset + (2 * i));
		dimensions.height = *(fileData + glyphHeightDataOffset + (2 * i) + 1);
		_currentDemoFont->setGlyphDimensions(i, dimensions);

		// Determine start of data for this glyph.
		uint16 currentGlyphDataOffset = READ_LE_UINT16(fileData + glyphOffsetTableOffset + (i * 2));
		assert(currentGlyphDataOffset >= glyphDataOffset);
		// Get pointers for the file and DemoFont data for this glyph.
		uint8 *srcGlyphDataPtr = fileData + currentGlyphDataOffset;
		uint8 *dstGlyphDataPtr = _currentDemoFont->getGlyphData(i);

		// Pad the glyph data with zeroes at the start for the height offset.
		uint16 offsetBytes = dimensions.heightOffset * dimensions.width;
		memset(dstGlyphDataPtr, 0, offsetBytes);
		dstGlyphDataPtr += offsetBytes;

		// Read each line of pixels.
		for (int j = 0; j < dimensions.height; j++) {
			// Each nibble has data for one pixel, so alternately read the low
			// and high nibble. If the number of pixels in a line is odd, one
			// nibble is discarded.
			bool lowNibble = true;
			uint8 glyphByte = 0;
			// Read each pixel in this line.
			for (int k = 0; k < dimensions.width; k++) {
				if (lowNibble) {
					// Copy the data byte.
					glyphByte = *srcGlyphDataPtr++;
					// Read the low nibble.
					*dstGlyphDataPtr++ = glyphByte & 0xF;
				} else {
					// Read the high nibble.
					*dstGlyphDataPtr++ = glyphByte >> 4;
				}
				// Switch to the other nibble.
				lowNibble = !lowNibble;
			}
		}
	}

	return true;
}

DemoFont::DemoFont(uint8 glyphWidth, uint8 glyphHeight, uint16 numGlyphs) {
	_glyphWidth = glyphWidth;
	_glyphHeight = glyphHeight;
	_numGlyphs = numGlyphs;

	// Allocate room for the full height and width for each glyph. A glyph
	// might not need this if the width is less, but it makes lookup easier.
	_glyphData = new uint8[_numGlyphs * _glyphWidth * _glyphHeight];
	_glyphDimensions = new GlyphDimensions[_numGlyphs];
}

DemoFont::~DemoFont() {
	delete[] _glyphData;
	delete[] _glyphDimensions;
}

uint8 *DemoFont::getGlyphData() {
	return _glyphData;
}

uint8 *DemoFont::getGlyphData(uint8 glyphNum) {
	assert(glyphNum < _numGlyphs);
	return _glyphData + (glyphNum * _glyphWidth * _glyphHeight);
}

uint8 DemoFont::getGlyphWidth(uint8 glyphNum) {
	assert(glyphNum < _numGlyphs);
	return _glyphDimensions[glyphNum].width;
}

uint8 DemoFont::getHeight() {
	return _glyphHeight;
}

void DemoFont::setGlyphDimensions(uint8 glyphNum, GlyphDimensions &glyphOffsets) {
	assert(glyphNum < _numGlyphs);
	_glyphDimensions[glyphNum] = glyphOffsets;
}

void DemoFont::drawGlyph(Graphics::Surface &surface, int32 glyphNum, int16 xx, int16 yy, byte *colorMap) {
	debugC(4, kDebugFont, "drawGlyph(surface, %d, %d, %d, colorMap)", glyphNum, xx, yy);
	// Clip character into range.
	if (glyphNum < 0)
		glyphNum = 0;

	if (glyphNum >= _numGlyphs)
		glyphNum = _numGlyphs - 1;

	if (_numGlyphs == 0)
		return;

	int16 rectX = getGlyphWidth(glyphNum);
	int16 rectY = getHeight();

	// Clip glyph dimensions to surface.
	if (rectX + xx >= surface.w)
		rectX = surface.w - xx;

	if (rectX < 0)
		return;

	if (rectY + yy >= surface.h)
		rectY = surface.h - yy;

	if (rectY < 0)
		return;

	// Copy the glyph data to the surface.
	int32 destPitch = surface.pitch;
	uint8 *c = getGlyphData(glyphNum);
	uint8 *curRow = (uint8 *)surface.getBasePtr(xx, yy);
	for (int16 y = 0; y < rectY; y++) {
		unsigned char *cur = curRow;
		for (int16 x = 0; x < rectX; x++) {
			if (*c && *c < 4)
				*cur = colorMap[*c];
			c++;
			cur++;
		}
		curRow += destPitch;
	}
}

} // End of namespace Toon
