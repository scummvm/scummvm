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

#include "graphics/big5.h"

namespace Graphics {

Big5Font::Big5Font() : _chineseTraditionalHeight(0) {
}

Big5Font::~Big5Font() {
	_chineseTraditionalFont.clear();
	_chineseTraditionalIndex.clear();
}

void Big5Font::ChineseTraditionalGlyph::makeOutline(int height) {
	outline[0][0] = 0;
	outline[0][1] = 0;
	// OR into outline the original bitmap moved by 1 pixel
	// 1 pixel down
	for (int y = 0; y < height - 1; y++) {
		outline[y+1][0] = bitmap[y][0];
		outline[y+1][1] = bitmap[y][1];
	}
	// 1 pixel up
	for (int y = 0; y < height - 1; y++) {
		outline[y][0] |= bitmap[y+1][0];
		outline[y][1] |= bitmap[y+1][1];
	}
	for (int y = 0; y < height; y++) {
		// 1 pixel right
		outline[y][0] |= bitmap[y][0] >> 1;
		outline[y][1] |= bitmap[y][0] << 7;
		outline[y][1] |= bitmap[y][1] >> 1;

		// 1 pixel left
		outline[y][0] |= bitmap[y][0] << 1;
		outline[y][0] |= bitmap[y][1] >> 7;
		outline[y][1] |= bitmap[y][1] << 1;
	}

	// Then AND-out the original bitmap
	for (int y = 0; y < height; y++) {
		outline[y][0] &= ~bitmap[y][0];
		outline[y][1] &= ~bitmap[y][1];
	}
}

void Big5Font::loadPrefixedRaw(Common::ReadStream &input, int height) {
	_chineseTraditionalHeight = height;
	_chineseTraditionalFont.clear();

	_chineseTraditionalIndex = Common::move(Common::Array<int>(0x8000, -1));
	// So far the smallest version had 1981 glyphs. Optimize a little bit for this number
	// but don't rely on it in any way
	_chineseTraditionalFont.reserve(1981);
	while(!input.eos()) {
		// Big-endian because it's not really a u16 but a big5 sequence.
		uint16 ch = input.readUint16BE();
		ChineseTraditionalGlyph glyph;
		if (ch == 0xffff)
			break;
		memset(&glyph.bitmap, 0, sizeof(glyph.bitmap));
		memset(&glyph.outline, 0, sizeof(glyph.outline));
		input.read(&glyph.bitmap, (kChineseTraditionalWidth / 8) * _chineseTraditionalHeight);
		glyph.makeOutline(height);
		_chineseTraditionalIndex[ch & 0x7fff] = _chineseTraditionalFont.size();
		_chineseTraditionalFont.push_back(glyph);
	}
}

template <class T> bool Big5Font::drawReal(byte *dest, uint16 textChar, int maxX, int maxY, uint32 destPitch, byte color, byte outlineColor, bool outline) const {
	int glyphIdx = _chineseTraditionalIndex[textChar & 0x7fff];
	if (glyphIdx < 0) {
		return false;
	}

	const ChineseTraditionalGlyph& glyph = _chineseTraditionalFont[glyphIdx];

	for (int y = 0; y < _chineseTraditionalHeight && y < maxY; y++) {
		T *cur = (T*) (dest + y * destPitch);
		T *curMax = cur + maxX;

		for (int byte = 0; byte < 2; byte++)
			for (int bit = 0; bit < 8 && cur < curMax; bit++, cur++)
				if ((glyph.bitmap[y][byte] << bit) & 0x80)
					*cur = color;
				else if (outline && (((glyph.outline[y][byte] << bit) & 0x80)))
					*cur = outlineColor;
	}
	return true;
}

bool Big5Font::drawBig5Char(byte *dest, uint16 ch, int maxX, int maxY, uint32 destPitch, byte color, byte outlineColor) const {
	return drawReal<uint8>(dest, ch, maxX, maxY, destPitch, color, outlineColor, true);
}

bool Big5Font::drawBig5Char(Graphics::Surface *surf, uint16 ch, const Common::Point &pt, uint32 color) const {
	switch(surf->format.bytesPerPixel) {
	case 4:
		return drawReal<uint32>((byte*)surf->getBasePtr(pt.x, pt.y), ch, surf->w - pt.x, surf->h - pt.y, surf->pitch, color, 0, false);
	case 2:
		return drawReal<uint16>((byte*)surf->getBasePtr(pt.x, pt.y), ch, surf->w - pt.x, surf->h - pt.y, surf->pitch, color, 0, false);
	case 1:
		return drawReal<uint8>((byte*)surf->getBasePtr(pt.x, pt.y), ch, surf->w - pt.x, surf->h - pt.y, surf->pitch, color, 0, false);
	default:
		error("Big5 font for bpp=%d is not supported", surf->format.bytesPerPixel);
	}

}

}
