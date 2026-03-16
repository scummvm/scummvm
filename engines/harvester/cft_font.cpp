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

#include "common/algorithm.h"
#include "common/endian.h"
#include "graphics/surface.h"

namespace Harvester {

namespace {

static const uint32 kCftFontHeightOffset = 0x40;
static const uint32 kCftStartTableOffset = 0x48;
static const uint32 kCftWidthTableOffset = 0x248;
static const uint32 kCftGlyphCount = 128;

static uint32 normalizeGlyphChar(uint32 chr) {
	if (chr >= 'a' && chr <= 'z')
		return chr - ('a' - 'A');

	return chr;
}

} // End of anonymous namespace

HarvesterCftFont::HarvesterCftFont(const CftFontResource &resource, const char *glyphOrder, int spaceWidth)
		: _resource(resource) {
	Common::Array<GlyphSlice> atlasGlyphs;

	if (_resource.header.size() >= kCftWidthTableOffset + kCftGlyphCount * 2) {
		const byte *header = _resource.header.data();
		const int headerFontHeight = READ_LE_UINT16(header + kCftFontHeightOffset);
		if (headerFontHeight > 0)
			_fontHeight = MIN<int>(headerFontHeight, _resource.atlasHeight);
		if (_fontHeight <= 0)
			_fontHeight = _resource.atlasHeight;

		for (uint i = 0; i < kCftGlyphCount; ++i) {
			GlyphSlice glyph;
			glyph.x = READ_LE_UINT16(header + kCftStartTableOffset + i * 2);
			glyph.width = READ_LE_UINT16(header + kCftWidthTableOffset + i * 2);
			if (glyph.width <= 0 || glyph.x >= (int)_resource.atlasWidth)
				continue;

			glyph.width = MIN<int>(glyph.width, (int)_resource.atlasWidth - glyph.x);
			glyph.valid = glyph.width > 0;
			if (!glyph.valid)
				continue;

			bool duplicate = false;
			for (const GlyphSlice &existing : atlasGlyphs) {
				if (existing.x == glyph.x && existing.width == glyph.width) {
					duplicate = true;
					break;
				}
			}
			if (!duplicate)
				atlasGlyphs.push_back(glyph);
		}
	}

	Common::sort(atlasGlyphs.begin(), atlasGlyphs.end(), [](const GlyphSlice &left, const GlyphSlice &right) {
		return left.x < right.x;
	});

	const Common::String glyphOrderString(glyphOrder ? glyphOrder : "");
	const uint glyphCount = MIN<uint>(glyphOrderString.size(), atlasGlyphs.size());
	for (uint i = 0; i < glyphCount; ++i) {
		const byte chr = glyphOrderString[i];
		if (chr == ' ')
			continue;

		_glyphs[chr] = atlasGlyphs[i];
		_maxCharWidth = MAX(_maxCharWidth, _glyphs[chr].width);
	}

	if (spaceWidth > 0) {
		_spaceWidth = spaceWidth;
	} else if (_glyphs['I'].valid) {
		_spaceWidth = MAX<int>(1, _glyphs['I'].width / 2);
	} else {
		_spaceWidth = MAX<int>(1, _maxCharWidth / 2);
	}
}

int HarvesterCftFont::getCharWidth(uint32 chr) const {
	if (chr == ' ')
		return _spaceWidth;

	const GlyphSlice *glyph = findGlyph(chr);
	return glyph ? glyph->width : _spaceWidth;
}

void HarvesterCftFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	if (!dst || chr == ' ')
		return;

	const GlyphSlice *glyph = findGlyph(chr);
	if (!glyph)
		return;

	const int drawHeight = MIN<int>(_fontHeight, _resource.atlasHeight);
	for (int row = 0; row < drawHeight; ++row) {
		const int dstY = y + row;
		if (dstY < 0 || dstY >= dst->h)
			continue;

		const byte *srcRow = _resource.atlasPixels.data() + row * _resource.atlasWidth + glyph->x;
		for (int col = 0; col < glyph->width; ++col) {
			const int dstX = x + col;
			if (dstX < 0 || dstX >= dst->w || srcRow[col] == 0)
				continue;

			switch (dst->format.bytesPerPixel) {
			case 1:
				*((byte *)dst->getBasePtr(dstX, dstY)) = color;
				break;
			case 2:
				*((uint16 *)dst->getBasePtr(dstX, dstY)) = color;
				break;
			case 4:
				*((uint32 *)dst->getBasePtr(dstX, dstY)) = color;
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

	const GlyphSlice &glyph = _glyphs[normalizeGlyphChar(chr)];
	return glyph.valid ? &glyph : nullptr;
}

} // End of namespace Harvester
