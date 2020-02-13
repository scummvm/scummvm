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

#include "bladerunner/font.h"

#include "bladerunner/bladerunner.h"

#include "common/debug.h"

namespace BladeRunner {

Font::Font() {
	reset();
}

Font::~Font() {
	close();
}

Font* Font::load(BladeRunnerEngine *vm, const Common::String &fileName, int spacing, bool useFontColor) {
	Font *font = new Font();
	font->_spacing = spacing;
	font->_useFontColor = useFontColor;

	Common::ScopedPtr<Common::SeekableReadStream> stream(vm->getResourceStream(fileName));
	if (!stream) {
		warning("Font::open failed to open '%s'", fileName.c_str());
		delete font;
		return nullptr;
	}

	font->_characterCount = stream->readUint32LE();
	font->_maxWidth = stream->readUint32LE();
	font->_maxHeight = stream->readUint32LE();
	font->_dataSize = stream->readUint32LE();
	font->_data = new uint16[font->_dataSize];
	if (!font->_data) {
		warning("Font::open failed to allocate font buffer");
		delete font;
		return nullptr;
	}

	font->_characters.resize(font->_characterCount);
	for (uint32 i = 0; i < font->_characterCount; i++) {
		font->_characters[i].x = stream->readUint32LE();
		font->_characters[i].y = stream->readUint32LE();
		font->_characters[i].width = stream->readUint32LE();
		font->_characters[i].height = stream->readUint32LE();
		font->_characters[i].dataOffset = stream->readUint32LE();
	}

	for (int i = 0; i < font->_dataSize; i++) {
		font->_data[i] = stream->readUint16LE();
	}

	return font;
}

void Font::close() {
	if (_data != nullptr) {
		delete[] _data;
	}
	reset();
}

void Font::reset() {
	_maxWidth = 0;
	_maxHeight = 0;
	_characterCount = 0;
	_data = nullptr;
	_dataSize = 0;
	_screenWidth = 0;
	_screenHeight = 0;
	_spacing = 0;
	_useFontColor = false;

	_characters.clear();
}

int Font::getFontHeight() const {
	return _maxHeight;
}

int Font::getMaxCharWidth() const {
	return _maxWidth;
}

int Font::getCharWidth(uint32 chr) const {
	if (chr >= _characterCount) {
		return 0;
	}
	return _characters[chr + 1].width + _spacing;
}

void Font::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	uint32 characterIndex = chr + 1;
	if (x < 0 || x >= dst->w || y < 0 || y >= dst->h || !_data || characterIndex >= _characterCount) {
		return;
	}

	uint16 *srcPtr = &_data[_characters[characterIndex].dataOffset];
	int width = _characters[characterIndex].width;
	int height = _characters[characterIndex].height;

	int endY = height + y - 1;
	int currentY = y;

	// FIXME/TODO
	// This width and height check were added as a temporary bug fix -- a sanity check which is only needed for the internal TAHOMA18.FON font.
	// That font's glyph properties table is corrupted - the start of the file states that there are 0xF7 (=247) entries in the char properties table
	// but that table get corrupted past the 176th entry. The image data glyph part of the FON file also only covers the 176 entries.
	// So the following if clause-check will return here if the width and height values are unnaturally big.
	// The bug only affects debug cases where all character glyph need to be displayed...
	// ...or potential custom dialogue / translations that reference characters that are not within the range of ASCII values for the normal Latin characters.
	if (width > 100 || height > 100) {
		return;
	}

	while (currentY <= endY && currentY < dst->h) {
		int currentX = x;
		int endX = width + x - 1;
		while (currentX <= endX && currentX < dst->w) {
			uint8 a, r, g, b;
			getGameDataColor(*srcPtr, a, r, g, b);
			if (!a) { // Alpha is inversed
				uint32 outColor = color;
				if (_useFontColor) {
					// Ignore the alpha in the output as it is inversed in the input
					outColor = dst->format.RGBToColor(r, g, b);
				}
				void *dstPtr = dst->getBasePtr(CLIP(currentX + _characters[characterIndex].x, 0, dst->w - 1), CLIP(currentY + _characters[characterIndex].y, 0, dst->h - 1));
				drawPixel(*dst, dstPtr, outColor);
			}
			srcPtr++;
			currentX++;
		}
		currentY++;
	}
}

} // End of namespace BladeRunner
