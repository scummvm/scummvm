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
	kFontTypeImageHeightTable = (1 << 0),
	kFontTypeGlyphWidthTable  = (1 << 1),
	kFontTypeFontColorTable   = (1 << 7),
	kFontTypeSyntheticFont    = (1 << 8),
	kFontTypeFixedWidthFont   = (1 << 13),
	kFontTypeNotExpandable    = (1 << 14)
};

enum {
	kFamilyGlyphWidthTable   = (1 << 1),
	kFamilyFractEnable       = (1 << 12),
	kFamilyIntegerExtra      = (1 << 13),
	kFamilyNoFractionalTable = (1 << 14),
	kFamilyFixedWidthFonts   = (1 << 15)
};

enum {
	kStylePropertyPlain     = 0,
	kStylePropertyBold      = 1,
	kStylePropertyItalic    = 2,
	kStylePropertyUnderline = 3,
	kStylePropertyOutline   = 4,
	kStylePropertyShadow    = 5,
	kStylePropertyCondensed = 6,
	kStylePropertyExtended  = 7,
	kStylePropertyUnused    = 8
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

	_ffFlags = 0;
	_ffFamID = 0;
	_ffFirstChar = 0;
	_ffLastChar = 0;
	_ffAscent = 0;
	_ffDescent = 0;
	_ffLeading = 0;
	_ffWidMax = 0;
	_ffWTabOff = 0;
	_ffKernOff = 0;
	_ffStylOff = 0;

	for (int i = 0; i < 9; i++)
		_ffProperty[i] = 0;

	_ffIntl[0] = 0;
	_ffIntl[1] = 0;
	_ffVersion = 0;

	_ffNumAssoc = 0;
	_ffNumOffsets = 0;
	_ffOffsets = nullptr;
	_ffNumBBoxes = 0;
}

MacFont::~MacFont() {
	free(_bitImage);
	free(_ffOffsets);
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

	_ffNumAssoc     = stream.readUint16BE(); // number of entries - 1
	_ffAssocEntries.resize(_ffNumAssoc + 1);
	for (uint i = 0; i <= _ffNumAssoc; i++) {
		_ffAssocEntries[i]._fontSize  = stream.readUint16BE(); // point size of font
		_ffAssocEntries[i]._fontStyle = stream.readUint16BE(); // style of font
		_ffAssocEntries[i]._fontID    = stream.readUint16BE(); // font resource ID
	}

	if (_ffWTabOff || _ffStylOff || _ffKernOff) {
		_ffNumOffsets = stream.readUint16BE(); // number of entries - 1
		_ffOffsets = (uint32 *)calloc(_ffNumOffsets + 1, sizeof(uint32));
		for (uint i = 0; i <= _ffNumOffsets; i++)
			_ffOffsets[i] = stream.readUint32BE();

		_ffNumBBoxes = stream.readUint16BE(); // number of entries - 1
		_ffBBoxes.resize(_ffNumBBoxes + 1);
		for (uint i = 0; i <= _ffNumBBoxes; i++) {
			_ffBBoxes[i]._style  = stream.readUint16BE();
			_ffBBoxes[i]._left   = stream.readUint16BE();
			_ffBBoxes[i]._bottom = stream.readUint16BE();
			_ffBBoxes[i]._right  = stream.readUint16BE();
			_ffBBoxes[i]._top    = stream.readUint16BE();
		}
	}

	if (_ffWTabOff) {
		// TODO: Read widths table
	}

	if (_ffStylOff) {
		// TODO: Read styles table
	}

	if (_ffKernOff) {
		stream.seek(_ffKernOff);

		_ffNumKerns = stream.readUint16BE(); // number of entries - 1
		_ffKernEntries.resize(_ffNumKerns + 1);

		for (uint i = 0; i <= _ffNumKerns; i++) {
			_ffKernEntries[i]._style       = stream.readUint16BE();
			_ffKernEntries[i]._entryLength = stream.readUint16BE();
			_ffKernEntries[i]._kernPairs.resize(_ffKernEntries[i]._entryLength / 4);

			for (uint j = 0; j < _ffKernEntries[i]._entryLength / 4; j++) {
				_ffKernEntries[i]._kernPairs[j]._firstChar = stream.readByte();
				_ffKernEntries[i]._kernPairs[j]._secondChar = stream.readByte();
				_ffKernEntries[i]._kernPairs[j]._distance = stream.readUint16BE();
			}
		}
	}

	return true;
 }

bool MacFont::loadFont(Common::SeekableReadStream &stream) {
	_fontType    = stream.readUint16BE();	// font type
	_firstChar   = stream.readUint16BE();	// character code of first glyph
	_lastChar    = stream.readUint16BE();	// character code of last glyph
	_maxWidth    = stream.readUint16BE();	// maximum glyph width
	_kernMax     = stream.readSint16BE();	// maximum glyph kern
	_nDescent    = stream.readSint16BE();	// negative of descent
	_fRectWidth  = stream.readUint16BE();	// width of font rectangle
	_fRectHeight = stream.readUint16BE();	// height of font rectangle
	_owTLoc      = stream.readUint16BE();	// offset to width/offset table
	_ascent      = stream.readUint16BE();	// maximum ascent measurement
	_descent     = stream.readUint16BE();	// maximum descent measurement
	_leading     = stream.readUint16BE();	// leading measurement
	_rowWords    = stream.readUint16BE() * 2; // row width of bit image in 16-bit wds

	if (getDepth(_fontType) != 1) {
		warning("MacFont: %dbpp fonts are not supported", getDepth(_fontType));

		return false;
	}

	// If positive, _nDescent holds the high bits of the offset to the
	// width/offset table.
	// http://mirror.informatimago.com/next/developer.apple.com/documentation/mac/Text/Text-252.html
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

	if (_fontType & kFontTypeGlyphWidthTable) {
		warning("Skipping glyph-width table");

		for (uint16 i = 0; i < glyphCount; i++)
			stream.readUint16BE();
	}

	if (_fontType & kFontTypeImageHeightTable) {
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
