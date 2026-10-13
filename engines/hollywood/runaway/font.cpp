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

#include "common/endian.h"
#include "common/str-enc.h"
#include "common/util.h"
#include "graphics/surface.h"
#include "hollywood/runaway/font.h"
#include "hollywood/runaway/graphics.h"

namespace Hollywood {
namespace {

// The German demo's character table at 0x4641f0 uses DOS 850, with additional
// punctuation in otherwise unused positions. 254 denotes a space.
const byte kGlyphMap[256] = {
	149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149,
	149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149,
	254, 83, 149, 89, 112, 90, 113, 149, 68, 69, 149, 71, 77, 70, 79, 75,
	58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 80, 78, 73, 72, 74, 85,
	118, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 87, 76, 88, 149, 107,
	149, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 41,
	42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 149, 149, 149, 91, 149,
	94, 108, 54, 149, 139, 96, 149, 95, 149, 140, 97, 141, 149, 98, 144, 149,
	102, 149, 149, 149, 142, 99, 149, 100, 143, 147, 109, 149, 149, 149, 149, 149,
	53, 55, 56, 57, 40, 86, 149, 149, 84, 114, 149, 149, 149, 82, 110, 111,
	149, 149, 149, 149, 149, 101, 149, 149, 106, 149, 149, 149, 149, 149, 149, 149,
	149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 115,
	116, 149, 149, 145, 149, 149, 103, 149, 146, 149, 149, 149, 149, 149, 149, 149,
	104, 93, 149, 149, 149, 149, 149, 149, 149, 105, 149, 149, 149, 149, 149, 149,
	92, 81, 149, 149, 149, 149, 149, 149, 149, 149, 117, 149, 149, 149, 149, 149,
};

} // End of anonymous namespace

bool RunawayFont::load() {
	Common::Array<byte> metrics;
	if (!readRunawayGlobalChunk(125, _pixels) || !readRunawayGlobalChunk(126, metrics) ||
			(metrics.size() != 150 * 5 && metrics.size() != 156 * 5))
		return false;
	_glyphs.resize(metrics.size() / 5);
	for (uint i = 0; i < _glyphs.size(); ++i) {
		const byte *data = metrics.data() + i * 5;
		Glyph &glyph = _glyphs[i];
		glyph.offset = READ_LE_UINT16(data);
		glyph.top = data[2];
		glyph.height = data[3];
		glyph.width = data[4];
		if (glyph.width > 28 || glyph.top + glyph.height > 28 ||
				glyph.offset + glyph.width * glyph.height > _pixels.size())
			return false;
	}
	memcpy(_glyphMap, kGlyphMap, sizeof(_glyphMap));
	// The French table at 0x46f1e0 adds circumflex vowels and moves the
	// missing-character box to 155. Glyphs 153/154 are the CP1252 ligatures.
	if (_glyphs.size() == 156) {
		for (uint i = 0; i < ARRAYSIZE(_glyphMap); ++i) {
			if (_glyphMap[i] == 149)
				_glyphMap[i] = 155;
		}
		_glyphMap[0x83] = 148;
		_glyphMap[0x88] = 149;
		_glyphMap[0x8c] = 150;
		_glyphMap[0x93] = 151;
		_glyphMap[0x96] = 152;
	}
	char bytes[128];
	for (uint i = 0; i < 128; ++i)
		bytes[i] = i + 128;
	_characters = Common::U32String(bytes, 128, Common::kDos850);
	// Native 0x40ee40 packs adjacent contours, trying up to five pixels
	// of overlap. Value 17 contributes only to the left glyph's spacing.
	_kerning.resize(_glyphs.size() * _glyphs.size());
	for (uint left = 0; left < _glyphs.size(); ++left) {
		const Glyph &a = _glyphs[left];
		for (uint right = 0; right < _glyphs.size(); ++right) {
			const Glyph &b = _glyphs[right];
			int shift = 1;
			for (; shift < 5; ++shift) {
				bool collision = false;
				for (uint y = 0; y < b.height && !collision; ++y) {
					const int ay = b.top + y - a.top;
					if (ay < 0 || ay >= a.height)
						continue;
					for (uint x = 0; x < b.width; ++x) {
						const int ax = a.width - shift + x;
						if (ax >= 0 && ax < a.width && _pixels[b.offset + y * b.width + x] < 17 &&
								_pixels[a.offset + ay * a.width + ax] < 18) {
							collision = true;
							break;
						}
					}
				}
				if (collision)
					break;
				if (shift >= b.width) {
					shift = 1;
					break;
				}
			}
			_kerning[left * _glyphs.size() + right] = -shift;
		}
	}
	return true;
}

uint RunawayFont::glyphIndex(uint32 chr) const {
	// These signs use game-specific positions rather than their DOS codes.
	switch (chr) {
	case 0x201c: return 92;
	case 0x201d: return 81;
	case 0x201e: return 92;
	case '\'': return 90;
	case '"': return 81;
	case 0x0152: return _glyphs.size() == 156 ? 153 : 149;
	case 0x0153: return _glyphs.size() == 156 ? 154 : 149;
	}
	if (chr < 128)
		return _glyphMap[chr];
	for (uint i = 0; i < _characters.size(); ++i) {
		if (_characters[i] == chr)
			return _glyphMap[i + 128];
	}
	return _glyphs.size() - 1;
}

int RunawayFont::getCharWidth(uint32 chr) const {
	const uint glyph = glyphIndex(chr);
	return glyph == 254 ? 8 : _glyphs[glyph].width;
}

int RunawayFont::getKerningOffset(uint32 left, uint32 right) const {
	const uint a = glyphIndex(left), b = glyphIndex(right);
	return !left || !right || a == 254 || b == 254 ? 0 : _kerning[a * _glyphs.size() + b];
}

void RunawayFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	const uint index = glyphIndex(chr);
	if (index == 254 || dst->format.bytesPerPixel != 2)
		return;
	const Glyph &glyph = _glyphs[index];
	const uint r = (color >> 11) & 31, g = (color >> 5) & 63, b = color & 31;
	for (uint row = 0; row < glyph.height; ++row) {
		const int dy = y + glyph.top + row;
		if (dy < 0 || dy >= dst->h)
			continue;
		for (uint column = 0; column < glyph.width; ++column) {
			const int dx = x + column;
			const byte level = _pixels[glyph.offset + row * glyph.width + column];
			if (dx < 0 || dx >= dst->w || level >= 17)
				continue;
			const uint16 pixel = (((r * level + 8) / 16) << 11) |
				(((g * level + 8) / 16) << 5) | ((b * level + 8) / 16);
			uint16 *out = (uint16 *)dst->getBasePtr(dx, dy);
			// The native text buffer keeps the brighter of overlapping glyphs.
			*out = MAX<uint16>(*out, pixel);
		}
	}
}

} // End of namespace Hollywood
