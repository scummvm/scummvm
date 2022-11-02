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
 */

#ifndef NANCY_FONT_H
#define NANCY_FONT_H

#include "common/array.h"

#include "graphics/font.h"
#include "graphics/managed_surface.h"

namespace Common {
class SeekableReadStream;
}

namespace Nancy {

class NancyEngine;

class Font : public Graphics::Font {
public:
	Font() = default;
	~Font() = default;

	void read(Common::SeekableReadStream &stream);

	int getFontHeight() const override { return _fontHeight; }
	int getMaxCharWidth() const override { return _maxCharWidth; }
	int getCharWidth(uint32 chr) const override;
	int getKerningOffset(uint32 left, uint32 right) const override { return 1; }

	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

	// Custom word wrapping function to fix an edge case with overflowing whitespaces
	void wordWrap(const Common::String &str, int maxWidth, Common::Array<Common::String> &lines, int initWidth = 0) const;

private:
	Common::Rect getCharacterSourceRect(char chr) const;

	Common::String _description; // 0xA
	Common::Point _colorCoordsOffset; // 0x32

	uint16 _spaceWidth;              // 0x38

	uint16 _uppercaseOffset;         // 0x3C
	uint16 _lowercaseOffset;         // 0x3E
	uint16 _digitOffset;             // 0x40
	uint16 _periodOffset;            // 0x42
	uint16 _commaOffset;             // 0x44
	uint16 _equalitySignOffset;      // 0x46
	uint16 _colonOffset;             // 0x48
	uint16 _dashOffset;              // 0x4A
	uint16 _questionMarkOffset;      // 0x4C
	uint16 _exclamationMarkOffset;   // 0x4E
	uint16 _percentOffset;           // 0x50
	uint16 _ampersandOffset;         // 0x52
	uint16 _asteriskOffset;          // 0x54
	uint16 _leftBracketOffset;       // 0x56
	uint16 _rightBracketOffset;      // 0x58
	uint16 _plusOffset;              // 0x5A
	uint16 _apostropheOffset;        // 0x5C
	uint16 _semicolonOffset;         // 0x5E
	uint16 _slashOffset;             // 0x60

	Common::Array<Common::Rect> _symbolRects; // 0x62

	Graphics::ManagedSurface _image;

	int _fontHeight;
	int _maxCharWidth;
	uint _transColor;
};

} // End of namespace Nancy

#endif // NANCY_FONT_H
