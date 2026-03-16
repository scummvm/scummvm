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

#include "harvester/cft_font.h"

#include "common/endian.h"
#include "graphics/surface.h"

namespace Harvester {

namespace {

static const uint32 kCftFontHeightOffset = 0x40;
static const uint32 kCftStartTableOffset = 0x42;
static const uint32 kCftWidthTableOffset = 0x242;
static const uint32 kCftSpaceWidthOffset = 0x442;
static const uint32 kCftGlyphCount = 256;

} // End of anonymous namespace

HarvesterCftFont::HarvesterCftFont(const CftFontResource &resource) : _resource(resource) {
	if (_resource.header.size() < kCftWidthTableOffset + kCftGlyphCount * 2 || _resource.atlasWidth == 0 || _resource.atlasHeight == 0)
		return;

	const byte *header = _resource.header.data();
	_fontHeight = READ_LE_UINT16(header + kCftFontHeightOffset);
	_drawHeight = MAX<int>(0, (int)_resource.atlasHeight - 1);
	if (_fontHeight <= 0)
		_fontHeight = _drawHeight;
	_spaceWidth = READ_LE_UINT16(header + kCftSpaceWidthOffset);

	for (uint i = 0; i < kCftGlyphCount; ++i) {
		GlyphSlice &glyph = _glyphs[i];
		glyph.x = READ_LE_UINT16(header + kCftStartTableOffset + i * 2);
		glyph.width = READ_LE_UINT16(header + kCftWidthTableOffset + i * 2);
		if (glyph.width <= 0 || glyph.x >= (int)_resource.atlasWidth)
			continue;

		glyph.width = MIN<int>(glyph.width, (int)_resource.atlasWidth - glyph.x);
		glyph.valid = glyph.width > 0;
		if (glyph.valid)
			_maxCharWidth = MAX(_maxCharWidth, glyph.width);
	}

	if (_spaceWidth <= 0)
		_spaceWidth = MAX<int>(1, _maxCharWidth);
}

int HarvesterCftFont::getCharWidth(uint32 chr) const {
	if (chr == ' ' || chr == '_')
		return _spaceWidth;

	const GlyphSlice *glyph = findGlyph(chr);
	return glyph ? glyph->width : _spaceWidth;
}

void HarvesterCftFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	if (!dst || chr == ' ' || chr == '_')
		return;
	(void)color;

	const GlyphSlice *glyph = findGlyph(chr);
	if (!glyph)
		return;

	for (int row = 0; row < _drawHeight; ++row) {
		const int dstY = y + row;
		if (dstY < 0 || dstY >= dst->h)
			continue;

		const byte *srcRow = _resource.atlasPixels.data() + row * _resource.atlasWidth + glyph->x;
		for (int col = 0; col < glyph->width; ++col) {
			const int dstX = x + col;
			const byte srcColor = srcRow[col];
			if (dstX < 0 || dstX >= dst->w || srcColor == 0)
				continue;

			switch (dst->format.bytesPerPixel) {
			case 1:
				*((byte *)dst->getBasePtr(dstX, dstY)) = srcColor;
				break;
			case 2:
				*((uint16 *)dst->getBasePtr(dstX, dstY)) = srcColor;
				break;
			case 4:
				*((uint32 *)dst->getBasePtr(dstX, dstY)) = srcColor;
				break;
			default:
				break;
			}
		}
	}
}

const HarvesterCftFont::GlyphSlice *HarvesterCftFont::findGlyph(uint32 chr) const {
	if (chr >= ARRAYSIZE(_glyphs))
		return nullptr;

	const GlyphSlice &glyph = _glyphs[chr];
	return glyph.valid ? &glyph : nullptr;
}

} // End of namespace Harvester
