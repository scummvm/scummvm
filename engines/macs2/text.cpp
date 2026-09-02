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

#include "macs2/text.h"
#include "common/util.h"

namespace Macs2 {

void GlyphData::readFromeFile(Common::File &file) {
	_ascii = file.readByte();
	_width = file.readUint16LE();
	_height = file.readUint16LE();
	_data.resize(_width * _height);
	file.read(_data.data(), _width * _height);
}

void GlyphData::readFromMemory(Common::SeekableReadStream *stream) {
	_ascii = stream->readByte();
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_data.resize(_width * _height);
	stream->read(_data.data(), _width * _height);
}

bool Text::findGlyph(char c, GlyphData &out, const GlyphData *glyphs, uint16 numGlyphs) const {
	for (uint16 i = 0; i < numGlyphs; i++) {
		if (glyphs[i]._ascii == c) {
			out = glyphs[i];
			return true;
		}
	}
	return false;
}

bool Text::findGlyph(char c, GlyphData &out) const {
	return findGlyph(c, out, _glyphs, _numGlyphs);
}

int Text::measureString(const Common::String &s) const {
	int sum = 0;
	GlyphData currentGlyph;
	uint16 widestGlyph = 0;
	for (auto current = s.begin(); current != s.end(); current++) {
		if (findGlyph(*current, currentGlyph)) {
			widestGlyph = MAX(widestGlyph, currentGlyph._width);
		}
	}

	for (auto current = s.begin(); current != s.end(); current++) {
		if (!findGlyph(*current, currentGlyph)) {
			sum += widestGlyph;
		} else {
			sum += currentGlyph._width + 1;
		}
	}
	return sum;
}

int Text::measureString(const Common::String &s, const GlyphData *glyphs, uint16 numGlyphs) const {
	int width = 0;
	uint16 widestGlyph = 1;
	for (uint i = 0; i < numGlyphs; i++) {
		widestGlyph = MAX(widestGlyph, glyphs[i]._width);
	}
	for (auto iter = s.begin(); iter != s.end(); iter++) {
		GlyphData glyph;
		if (findGlyph(*iter, glyph, glyphs, numGlyphs)) {
			width += glyph._width + 1;
		} else {
			width += widestGlyph;
		}
	}
	return width;
}

int Text::measureStrings(const Common::StringArray &sa) const {
	int max = -1;
	for (auto iter = sa.begin(); iter != sa.end(); iter++) {
		max = MAX(measureString(*iter), max);
	}
	return max;
}

int Text::measureStringsVertically(const Common::StringArray &sa, int lineHeight) const {
	return (int)sa.size() * lineHeight;
}

} // namespace Macs2
