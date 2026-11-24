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

#include "common/textconsole.h"

#include "dune/detection.h"
#include "dune/font.h"

namespace Dune {

enum {
	kWidthTableSize = 0x80,

	// Where each face's glyphs begin, which is what the pointers at DS:0x2514
	// and DS:0x2516 hold: the tall face after the two width tables, and the
	// small one after all 128 of its glyphs.
	kTallGlyphs = 0x100,
	kSmallGlyphs = kTallGlyphs + 128 * 9
};

bool Font::load(Archive &archive, const Common::String &name) {
	Resource file = archive.openMember(name);
	if (!file) {
		warning("Dune: the font %s is missing", name.c_str());
		return false;
	}

	_data.resize(file->size());
	file->read(_data.begin(), _data.size());

	return _data.size() > kSmallGlyphs;
}

const byte *Font::glyph(byte c) const {
	if (c >= 128)
		return nullptr;

	const uint32 rows = (uint32)glyphHeight();
	const uint32 offset = (_face == kFaceNormal ? kTallGlyphs : kSmallGlyphs) + rows * c;

	if (offset + rows > _data.size())
		return nullptr;

	return _data.begin() + offset;
}

int Font::charWidth(byte c) const {
	if (c >= kWidthTableSize || _data.empty())
		return 0;

	return _data[_face * kWidthTableSize + c];
}

int Font::stringWidth(const Common::String &text, Face *endingFace) const {
	Face face = _face;
	int width = 0;

	for (uint i = 0; i != text.size(); i++) {
		const byte c = text[i];

		if (c == kSwitchToAlternate) {
			face = kFaceAlternate;
			continue;
		}
		if (c == kSwitchToNormal) {
			face = kFaceNormal;
			continue;
		}

		if (c < kWidthTableSize && !_data.empty())
			width += _data[face * kWidthTableSize + c];
	}
	if (endingFace)
		*endingFace = face;

	return width;
}

int Font::wordWidth(const Common::String &text, Face *endingFace) const {
	Face face = _face;
	int width = 0;
	for (uint i = 0; i < text.size(); i++) {
		const byte c = text[i];
		if (c == kSwitchToAlternate)
			face = kFaceAlternate;
		else if (c == kSwitchToNormal)
			face = kFaceNormal;
		else if (c >= 9)
			width += charWidth(c);
	}
	if (endingFace)
		*endingFace = face;
	return width;
}

int Font::drawChar(byte *screen, int x, int y, byte c, byte colour, byte background) {
	if (c == kSwitchToAlternate) {
		setFace(kFaceAlternate);
		return x;
	}
	if (c == kSwitchToNormal) {
		setFace(kFaceNormal);
		return x;
	}

	const byte *rows = glyph(c);
	if (!rows)
		return x;

	const int height = glyphHeight();

	for (int row = 0; row != height; row++) {
		if (y + row < 0 || y + row >= 200)
			continue;

		byte *line = screen + 320 * (y + row) + x;

		for (int column = 0; column != (background ? charWidth(c) : 8); column++) {
			const bool ink = (rows[row] & (0x80 >> column)) != 0;
			if (!ink && !background)
				continue;
			if (x + column < 0 || x + column >= 320)
				continue;

			line[column] = ink ? colour : background;
		}
	}

	return x + charWidth(c);
}

int Font::drawString(byte *screen, int x, int y, const Common::String &text, byte colour, byte background) {
	for (uint i = 0; i != text.size(); i++)
		x = drawChar(screen, x, y, text[i], colour, background);

	return x;
}

} // End of namespace Dune
