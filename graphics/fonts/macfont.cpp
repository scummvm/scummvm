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

#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/surface.h"
#include "graphics/fonts/macfont.h"

namespace Graphics {

enum {
	k_fontTypeImageHeightTable = (1 << 0),
	k_fontTypeGlyphWidthTable  = (1 << 1),
	k_fontTypeFontColorTable   = (1 << 7),
	k_fontTypeSyntheticFont    = (1 << 8),
	k_fontTypeFixedWidthFont   = (1 << 13),
	k_fontTypeNotExpandable    = (1 << 14)
};

static int getDepth(uint16 _fontType) {
	// Returns 1, 2, 4, 8

	return 1 << ((_fontType >> 2) & 3);
}

MacFont::MacFont() {
	_fontType = 0;
	_firstChar = 0;
	_lastChar = 0;
	_maxWidth = 0;
	_kernMax = 0;
	_nDescent = 0;
	_fRectWidth = 0;
	_fRectHeight = 0;
	_owTLoc = 0;
	_ascent = 0;
	_descent = 0;
	_leading = 0;
	_rowWords = 0;
	_bitImage = nullptr;
}

MacFont::~MacFont() {
	free(_bitImage);
}

bool MacFont::loadFOND(Common::SeekableReadStream &stream) {
	_ffFlags     = stream.readUint16BE(); // flags for family
	_ffFamID     = stream.readUint16BE(); // family ID number
	_ffFirstChar = stream.readUint16BE(); // ASCII code of first character
	_ffLastChar  = stream.readUint16BE(); // ASCII code of last character
	_ffAscent    = stream.readUint16BE(); // maximum ascent for 1-pt font
	_ffDescent   = stream.readUint16BE(); // maximum descent for 1-pt font
	_ffLeading   = stream.readUint16BE(); // maximum leading for 1-pt font
	_ffWidMax    = stream.readUint16BE(); // maximum glyph width for 1-pt font
	_ffWTabOff   = stream.readUint32BE(); // offset to family glyph-width table
	_ffKernOff   = stream.readUint32BE(); // offset to kerning table
	_ffStylOff   = stream.readUint32BE(); // offset to style-mapping table

	for (int i = 0; i < 9; i++)			  // style properties info
		_ffProperty[i] = stream.readUint16BE();

	_ffIntl[0]   = stream.readUint16BE(); // for international use
	_ffIntl[1]   = stream.readUint16BE(); // for international use
	_ffVersion   = stream.readUint16BE(); // version number

	return true;
 }

bool MacFont::loadFont(Common::SeekableReadStream &stream) {
	_fontType = stream.readUint16BE();		// font type
	_firstChar = stream.readUint16BE();		// character code of first glyph
	_lastChar = stream.readUint16BE();		// character code of last glyph
	_maxWidth = stream.readUint16BE();		// maximum glyph width
	_kernMax = stream.readSint16BE();		// maximum glyph kern
	_nDescent = stream.readSint16BE();		// negative of descent
	_fRectWidth = stream.readUint16BE();	// width of font rectangle
	_fRectHeight = stream.readUint16BE();	// height of font rectangle
	_owTLoc = stream.readUint16BE();		// offset to width/offset table
	_ascent = stream.readUint16BE();		// maximum ascent measurement
	_descent = stream.readUint16BE();		// maximum descent measurement
	_leading = stream.readUint16BE();		// leading measurement
	_rowWords = stream.readUint16BE() * 2;

	if (getDepth(_fontType) != 1) {
		warning("MacFont: %dbpp fonts are not supported", getDepth(_fontType));

		return false;
	}

	// If positive, _nDescent holds the high bits of the offset to the
	// width/offset table.
	// https://developer.apple.com/legacy/library/documentation/mac/Text/Text-252.html
	if (_nDescent > 0)
		_owTLoc |= _nDescent << 16;

	// Alignment is by word
	_owTLoc *= 2;
	_owTLoc += 16;

	uint16 glyphCount = _lastChar - _firstChar + 1;
	_glyphs.resize(glyphCount);

	// Bit image table
	uint16 bitImageSize = _rowWords * _fRectHeight;
	_bitImage = new byte[bitImageSize];
	stream.read(_bitImage, bitImageSize);

	// Bitmap location table
	// Last glyph is the pic for the missing glyph
	// One more word for determinig width
	uint16 *bitmapOffsets = (uint16 *)calloc(glyphCount + 2, sizeof(uint16));

	for (uint16 i = 0; i < glyphCount + 2; i++)
		bitmapOffsets[i] = stream.readUint16BE();

	for (uint16 i = 0; i < glyphCount + 1; i++) {
		Glyph *glyph = (i == glyphCount) ? &_defaultChar : &_glyphs[i];
		glyph->bitmapOffset = bitmapOffsets[i];
		glyph->bitmapWidth = bitmapOffsets[i + 1] - bitmapOffsets[i];
	}

	// Width/offset table
	stream.seek(_owTLoc);

	for (uint16 i = 0; i < glyphCount; i++) {
		byte kerningOffset = stream.readByte();
		byte width = stream.readByte();

		// 0xFF designates missing glyph
		if (kerningOffset == 0xFF && width == 0xFF)
			continue;

		_glyphs[i].kerningOffset = _kernMax + kerningOffset;
		_glyphs[i].width = width;
	}

	_defaultChar.kerningOffset = _kernMax + stream.readByte();
	_defaultChar.width = _kernMax + stream.readByte();

	if (_fontType & k_fontTypeGlyphWidthTable) {
		warning("Skipping glyph-width table");

		for (uint16 i = 0; i < glyphCount; i++)
			stream.readUint16BE();
	}

	if (_fontType & k_fontTypeImageHeightTable) {
		warning("Skipping image height table");

		for (uint16 i = 0; i < glyphCount; i++)
			stream.readUint16BE();
	}

	return true;
}

int MacFont::getFontHeight() const {
	return _fRectHeight;
}

int MacFont::getMaxCharWidth() const {
	return _maxWidth;
}

int MacFont::getCharWidth(uint32 chr) const {
	const Glyph *glyph = findGlyph(chr);

	if (!glyph)
		return _maxWidth;

	return glyph->width;
}

void MacFont::drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	assert(dst != 0);
	assert(dst->format.bytesPerPixel == 1 || dst->format.bytesPerPixel == 2 || dst->format.bytesPerPixel == 4);

	const Glyph *glyph = findGlyph(chr);
	if (!glyph || glyph->width == 0)
		return;

	for (uint16 i = 0; i < _fRectHeight; i++) {
		byte *srcRow = _bitImage + i * _rowWords;

		for (uint16 j = 0; j < glyph->bitmapWidth; j++) {
			uint16 bitmapOffset = glyph->bitmapOffset + j;

			if (srcRow[bitmapOffset / 8] & (1 << (7 - (bitmapOffset % 8)))) {
				if (dst->format.bytesPerPixel == 1)
					*((byte *)dst->getBasePtr(x + j, y + i)) = color;
				else if (dst->format.bytesPerPixel == 2)
					*((uint16 *)dst->getBasePtr(x + j, y + i)) = color;
				else if (dst->format.bytesPerPixel == 4)
					*((uint32 *)dst->getBasePtr(x + j, y + i)) = color;
			}
		}
	}
}

const MacFont::Glyph *MacFont::findGlyph(uint32 c) const {
	if (_glyphs.empty())
		return 0;

	if (c < _firstChar || c > _lastChar)
		return &_defaultChar;

	return &_glyphs[c - _firstChar];
}

} // End of namespace Graphics
