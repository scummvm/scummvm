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

Font::Font(BladeRunnerEngine *vm) : _vm(vm) {
	reset();
}

Font::~Font() {
	close();
}

bool Font::open(const Common::String &fileName, int screenWidth, int screenHeight, int spacing1, int spacing2, uint16 color) {
	reset();

	_screenWidth = screenWidth;
	_screenHeight = screenHeight;
	_spacing1 = spacing1;
	_spacing2 = spacing2;
	_color = color;

	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->getResourceStream(fileName));
	if (!stream) {
		debug("Font::open failed to open '%s'", fileName.c_str());
		return false;
	}

	_characterCount = stream->readUint32LE();
	_maxWidth = stream->readUint32LE();
	_maxHeight = stream->readUint32LE();
	_dataSize = stream->readUint32LE();
	_data = new uint16[_dataSize];
	if (!_data) {
		debug("Font::open failed to allocate font buffer");
		return false;
	}

	for (int i = 0; i < _characterCount; i++) {
		_characters[i]._x = stream->readUint32LE();
		_characters[i]._y = stream->readUint32LE();
		_characters[i]._width = stream->readUint32LE();
		_characters[i]._height = stream->readUint32LE();
		_characters[i]._dataOffset = stream->readUint32LE();
	}
	for (int i = 0; i < _dataSize; i++) {
		_data[i] = stream->readUint16LE();
	}
	return true;
}

void Font::close() {
	if (_data != nullptr) {
		delete[] _data;
	}
	reset();
}

void Font::setSpacing(int spacing1, int spacing2) {
	if (_data) {
		_spacing1 = spacing1;
		_spacing2 = spacing2;
	}
}

void Font::setColor(uint16 color) {
	if (_data && _color != color) {
		replaceColor(_color, color);
		_color = color;
	}
}

void Font::draw(const Common::String &text, Graphics::Surface &surface, int x, int y) {
	if (!_data) {
		return;
	}

	x = CLIP(x, 0, _screenWidth - getTextWidth(text) + 1);
	y = CLIP(y, 0, _screenHeight - _maxHeight);

	const char *character = text.c_str();
	while (*character != 0) {
		drawCharacter(*character, surface, x, y);
		x += _spacing1 + _characters[*character + 1]._width;
		character++;
	}

}

void Font::drawColor(const Common::String &text, Graphics::Surface &surface, int x, int y, uint16 color) {
	if (_color != color) {
		setColor(color);
	}
	draw(text, surface, x, y);
}

int Font::getTextWidth(const Common::String &text) {
	const char *character = text.c_str();

	if (!_data) {
		return 0;
	}
	int totalWidth = 0;
	if (*character == 0) {
		return 0;
	}
	while (*character != 0) {
		totalWidth = _spacing1 + _characters[*character + 1]._width;
		character++;
	}
	return totalWidth - _spacing1;
}

void Font::reset() {
	_maxWidth = 0;
	_maxHeight = 0;
	_characterCount = 0;
	_data = nullptr;
	_dataSize = 0;
	_screenWidth = 0;
	_screenHeight = 0;
	_spacing1 = 0;
	_spacing2 = 0;
	_color = 0x7FFF;
	_intersperse = 0;

	memset(_characters, 0, 256 * sizeof(FontCharacter));
}

void Font::replaceColor(uint16 oldColor, uint16 newColor) {
	if (!_data || !_dataSize) {
		return;
	}
	for (int i = 0; i < _dataSize; i++) {
		if (_data[i] == oldColor) {
			_data[i] = newColor;
		}
	}
}

void Font::drawCharacter(const char character, Graphics::Surface &surface, int x, int y) {
	uint8 characterIndex = (uint8)character + 1;
	if (x < 0 || x >= _screenWidth || y < 0 || y >= _screenHeight || !_data || characterIndex >= _characterCount) {
		return;
	}

	uint16 *dstPtr = (uint16*)surface.getBasePtr(x + _characters[characterIndex]._x, y + _characters[characterIndex]._y);
	uint16 *srcPtr = &_data[_characters[characterIndex]._dataOffset];
	int width = _characters[characterIndex]._width;
	int height = _characters[characterIndex]._height;
	if (_intersperse && y & 1) {
		dstPtr += surface.pitch / 2;
	}

	int endY = height + y - 1;
	int currentY = y;
	while (currentY <= endY && currentY < _screenHeight) {
		int currentX = x;
		int endX = width + x - 1;
		while (currentX <= endX && currentX < _screenWidth) {
			if ((*srcPtr & 0x8000) == 0) {
				*dstPtr = *srcPtr;
			}
			dstPtr++;
			srcPtr++;
			currentX++;
		}
		dstPtr += surface.pitch / 2 - width;
		if (_intersperse) {
			srcPtr += width;
			dstPtr += surface.pitch / 2;
			currentY++;
		}
		currentY++;
	}
}
} // End of namespace BladeRunner
