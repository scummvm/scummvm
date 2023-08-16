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

#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/managed_surface.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/fonts/macfont.h"

#define DEBUGSCALING 0

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

MacFontFamily::MacFontFamily(const Common::String &name) {
	_name = name;

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
	_ffNumKerns = 0;
	_ffNumStyleWidths = 0;
}

MacFontFamily::~MacFontFamily() {
	free(_ffOffsets);
}

bool MacFontFamily::load(Common::SeekableReadStream &stream) {
	_ffFlags     = stream.readUint16BE(); // flags for family
	_ffFamID     = stream.readUint16BE(); // family ID number
	_ffFirstChar = stream.readUint16BE(); // ASCII code of first character
	_ffLastChar  = stream.readUint16BE(); // ASCII code of last character
	_ffAscent    = stream.readSint16BE(); // maximum ascent for 1-pt font
	_ffDescent   = stream.readSint16BE(); // maximum descent for 1-pt font
	_ffLeading   = stream.readSint16BE(); // maximum leading for 1-pt font
	_ffWidMax    = stream.readSint16BE(); // maximum glyph width for 1-pt font
	_ffWTabOff   = stream.readUint32BE(); // offset to family glyph-width table
	_ffKernOff   = stream.readUint32BE(); // offset to kerning table
	_ffStylOff   = stream.readUint32BE(); // offset to style-mapping table

	debug(10, "flags: %x famid: %d first: %d last: %d", _ffFlags, _ffFamID, _ffFirstChar, _ffLastChar);
	debug(10, "ascent: %g descent: %g, leading: %g, widmax: %g", _ffAscent / (double)(1<<12),
			_ffDescent / (double)(1<<12), _ffLeading / (double)(1<<12), _ffWidMax / (double)(1<<12));

	debug(10, "wtaboff: %d kernoff: %d styloff: %d", _ffWTabOff, _ffKernOff, _ffStylOff);

	debugN(10, "Extra width: ");
	for (int i = 0; i < 9; i++) {		  // style properties info
		_ffProperty[i] = stream.readUint16BE();
		debugN(10, "%d ", _ffProperty[i]);
	}
	debug(10, "%s", "");

	_ffIntl[0]   = stream.readUint16BE(); // for international use
	_ffIntl[1]   = stream.readUint16BE(); // for international use
	_ffVersion   = stream.readUint16BE(); // version number

	debug(10, "version: %d", _ffVersion);

	_ffNumAssoc     = stream.readUint16BE(); // number of entries - 1
	_ffAssocEntries.resize(_ffNumAssoc + 1);

	debug(10, "association cnt: %d", _ffNumAssoc + 1);

	for (uint i = 0; i <= _ffNumAssoc; i++) {
		_ffAssocEntries[i]._fontSize  = stream.readUint16BE(); // point size of font
		_ffAssocEntries[i]._fontStyle = stream.readUint16BE(); // style of font
		_ffAssocEntries[i]._fontID    = stream.readUint16BE(); // font resource ID

		debug(10, "  size: %d style: %d id: %d", _ffAssocEntries[i]._fontSize, _ffAssocEntries[i]._fontStyle,
								_ffAssocEntries[i]._fontID);
	}

	if (_ffWTabOff || _ffStylOff || _ffKernOff) {
		_ffNumOffsets = stream.readUint16BE(); // number of entries - 1
		_ffOffsets = (uint32 *)calloc(_ffNumOffsets + 1, sizeof(uint32));
		debugN(10, "offset cnt: %d, OFF: ", _ffNumOffsets + 1);
		for (uint i = 0; i <= _ffNumOffsets; i++) {
			_ffOffsets[i] = stream.readUint32BE();
			debugN(10, "%d ", _ffOffsets[i]);
		}
		debug(10, "%s", "");

		_ffNumBBoxes = stream.readUint16BE(); // number of entries - 1
		_ffBBoxes.resize(_ffNumBBoxes + 1);
		debug(10, "num BBoxes: %d", _ffNumBBoxes + 1);
		for (uint i = 0; i <= _ffNumBBoxes; i++) {
			_ffBBoxes[i]._style  = stream.readUint16BE();
			_ffBBoxes[i]._left   = stream.readSint16BE();
			_ffBBoxes[i]._bottom = stream.readSint16BE();
			_ffBBoxes[i]._right  = stream.readSint16BE();
			_ffBBoxes[i]._top    = stream.readSint16BE();

			debug(10, "style: %d left: %g bottom: %g right: %g top: %g", _ffBBoxes[i]._style,
					_ffBBoxes[i]._left / (double)(1<<12), _ffBBoxes[i]._bottom / (double)(1<<12),
					_ffBBoxes[i]._right / (double)(1<<12), _ffBBoxes[i]._top / (double)(1<<12));
		}
	}

	if (_ffWTabOff) {
		stream.seek(_ffWTabOff);

		_ffNumStyleWidths = stream.readUint16BE() + 1;
		_ffStyleWidths.resize(_ffNumStyleWidths);

		debug(10, "style widths entries: %d", _ffNumStyleWidths);

		for (uint i = 0; i < _ffNumStyleWidths; i++) {
			uint size = _ffLastChar - _ffFirstChar + 3;
			_ffStyleWidths[i]._style = stream.readUint16BE();
			_ffStyleWidths[i]._widths.resize(size);

			for (uint j = 0; j < size; j++)
				_ffStyleWidths[i]._widths[j] = stream.readUint16BE();
		}
	}

	if (_ffStylOff) {
		// looks like this part is not useful for now.
		// stream.seek(_ffStylOff);

		/*uint16 classFlag =*/ stream.readUint16BE();
		/*uint8 plainIndex =*/ stream.readSByte();
		/*uint8 boldIndex =*/ stream.readSByte();
		/*uint8 italicIndex =*/ stream.readSByte();
	}

	if (_ffKernOff) {
		stream.seek(_ffKernOff);

		_ffNumKerns = stream.readUint16BE(); // number of entries - 1
		_ffKernEntries.resize(_ffNumKerns + 1);

		debug(10, "kern entries: %d", _ffNumKerns + 1);

		for (uint i = 0; i <= _ffNumKerns; i++) {
			_ffKernEntries[i]._style       = stream.readUint16BE();
			_ffKernEntries[i]._entryLength = stream.readUint16BE();
			_ffKernEntries[i]._kernPairs.resize(_ffKernEntries[i]._entryLength);

			debug(10, "  style: %x kernpairs: %u", _ffKernEntries[i]._style, _ffKernEntries[i]._entryLength);

			for (uint j = 0; j < _ffKernEntries[i]._entryLength; j++) {
				byte f, s;
				int16 d;
				f = _ffKernEntries[i]._kernPairs[j]._firstChar  = stream.readByte();
				s = _ffKernEntries[i]._kernPairs[j]._secondChar = stream.readByte();
				d = _ffKernEntries[i]._kernPairs[j]._distance   = stream.readSint16BE();

				_ffKernEntries[i]._kernTable[(f << 8) | s] = d;
			}
		}
	}

	return true;
 }

 int MacFontFamily::getGlyphWidth(uint style, uint c) {
	 for (uint i = 0; i < _ffStyleWidths.size(); i++) {
		 if (_ffKernEntries[i]._style == style) {
			 if (c < _ffFirstChar || c > _ffLastChar)
				 return -1;
			 return _ffStyleWidths[i]._widths[c - _ffFirstChar];
		 }
	 }
	 return -1;
 }

 int MacFontFamily::getKerningOffset(uint style, int32 left, uint32 right) const {
	uint16 idx = ((left & 0xff) << 8) | (right & 0xff);

	for (uint i = 0; i < _ffKernEntries.size(); i++) {
		if (_ffKernEntries[i]._style == style)
			if (_ffKernEntries[i]._kernTable.contains(idx))
				return _ffKernEntries[i]._kernTable[idx];
	}

	return 0;
 }


 MacFONTFont::MacFONTFont() {
	_data._fontType = 0;
	_data._firstChar = 0;
	_data._lastChar = 0;
	_data._maxWidth = 0;
	_data._kernMax = 0;
	_data._nDescent = 0;
	_data._fRectWidth = 0;
	_data._fRectHeight = 0;
	_data._owTLoc = 0;
	_data._ascent = 0;
	_data._descent = 0;
	_data._leading = 0;
	_data._rowWords = 0;
	_data._bitImage = nullptr;
	_data._surfHeight = 0;

	_data._family = nullptr;
	_data._size = 12;
	_data._style = 0;
 }

 MacFONTFont::MacFONTFont(const MacFONTdata &data) {
	 _data = data;
 }

 MacFONTFont::~MacFONTFont() {
	delete[] _data._bitImage;
 }

bool MacFONTFont::loadFont(Common::SeekableReadStream &stream, MacFontFamily *family, int size, int style) {
	_data._family = family;
	_data._size = size;
	_data._style = style;

	_data._fontType    = stream.readUint16BE();	// font type
	_data._firstChar   = stream.readUint16BE();	// character code of first glyph
	_data._lastChar    = stream.readUint16BE();	// character code of last glyph
	_data._maxWidth    = stream.readUint16BE();	// maximum glyph width
	_data._kernMax     = stream.readSint16BE();	// maximum glyph kern
	_data._nDescent    = stream.readSint16BE();	// negative of descent
	_data._fRectWidth  = stream.readUint16BE();	// width of font rectangle
	_data._fRectHeight = stream.readUint16BE();	// height of font rectangle
	_data._owTLoc      = stream.readUint16BE();	// offset to width/offset table
	_data._ascent      = stream.readUint16BE();	// maximum ascent measurement
	_data._descent     = stream.readUint16BE();	// maximum descent measurement
	_data._leading     = stream.readUint16BE();	// leading measurement
	_data._rowWords    = stream.readUint16BE() * 2; // row width of bit image in 16-bit wds

	_data._surfHeight = _data._fRectHeight;

	// i use this as a flag to indicate whether the font is generated. see more detail on drawChar.
	_data._slant = 0; // only used in generated font

	if (getDepth(_data._fontType) != 1) {
		warning("MacFONTFont: %dbpp fonts are not supported", getDepth(_data._fontType));

		return false;
	}

	// If positive, _nDescent holds the high bits of the offset to the
	// width/offset table.
	// https://web.archive.org/web/20080724120946/developer.apple.com/documentation/mac/Text/Text-252.html
	if (_data._nDescent > 0)
		_data._owTLoc |= _data._nDescent << 16;

	// Alignment is by word
	_data._owTLoc *= 2;
	_data._owTLoc += 16;

	uint16 glyphCount = _data._lastChar - _data._firstChar + 1;
	_data._glyphs.resize(glyphCount);

	// Bit image table
	uint bitImageSize = _data._rowWords * _data._surfHeight;
	_data._bitImage = new byte[bitImageSize];
	stream.read(_data._bitImage, bitImageSize);

	// Bitmap location table
	// Last glyph is the pic for the missing glyph
	// One more word for determinig width
	uint16 *bitmapOffsets = (uint16 *)calloc(glyphCount + 2, sizeof(uint16));

	for (uint16 i = 0; i < glyphCount + 2; i++)
		bitmapOffsets[i] = stream.readUint16BE();

	for (uint16 i = 0; i < glyphCount + 1; i++) {
		MacGlyph *glyph = (i == glyphCount) ? &_data._defaultChar : &_data._glyphs[i];
		glyph->bitmapOffset = bitmapOffsets[i];
		glyph->bitmapWidth = bitmapOffsets[i + 1] - bitmapOffsets[i];
	}

	// Width/offset table
	stream.seek(_data._owTLoc);

	for (uint16 i = 0; i < glyphCount; i++) {
		byte kerningOffset = stream.readByte();
		byte width = stream.readByte();

		// 0xFF designates missing glyph
		if (kerningOffset == 0xFF && width == 0xFF)
			continue;

		_data._glyphs[i].kerningOffset = _data._kernMax + kerningOffset;
		_data._glyphs[i].width = width;
	}

	_data._defaultChar.kerningOffset = _data._kernMax + stream.readByte();
	_data._defaultChar.width = _data._kernMax + stream.readByte();

	if (_data._fontType & kFontTypeGlyphWidthTable) {
		warning("Skipping glyph-width table");

		for (uint16 i = 0; i < glyphCount; i++)
			_data._glyphs[i].width1 = stream.readUint16BE();
	}

	if (_data._fontType & kFontTypeImageHeightTable) {
		for (uint16 i = 0; i < glyphCount; i++)
			_data._glyphs[i].height = stream.readUint16BE();
	}

	free(bitmapOffsets);

	return true;
}

int MacFONTFont::getCharWidth(uint32 chr) const {
	const MacGlyph *glyph = findGlyph(chr);

	if (!glyph)
		return _data._maxWidth;

	// this part looks wrong
	//	if (_data._fontType & kFontTypeGlyphWidthTable) {
	//		return glyph->width1;
	//	} else {
	//		if (_data._family) {
	//			int width = _data._family->getGlyphWidth(_data._style, chr);
	//			if (width != -1)
	//				return (width * 1000L + (1 << 11)) >> 12;
	//		}
	//	}
	return glyph->width;
}

void MacFONTFont::drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	assert(dst != 0);
	assert(dst->format.bytesPerPixel == 1 || dst->format.bytesPerPixel == 2 || dst->format.bytesPerPixel == 4);

	if (x > dst->w || y > dst->h)
		return;

	const MacGlyph *glyph = findGlyph(chr);
	if (!glyph || glyph->width == 0)
		return;

	if (x + glyph->bitmapWidth < 0 || y + _data._fRectHeight < 0)
		return;

	// Make sure we do not draw outside the surface
	uint16 yStart = (y < 0) ? -y : 0;
	uint16 yStop = _data._fRectHeight;
	uint16 xStart = (x < 0) ? -x : 0;
	uint16 xStop = glyph->bitmapWidth;

	// due to the way we are handling the generated fonts. we only add the kerning offset for the original font
	if (!_data._slant)
		x += glyph->kerningOffset;

	if (x >= dst->w)
		return;

	if (y + _data._fRectHeight >= dst->h)
		yStop = dst->h - y;
	if (x + glyph->bitmapWidth >= dst->w)
		xStop = dst->w - x;

	// for the underLine font, we need to draw the line at the whole area, which includes the kerning space.
	if ((_data._slant & kMacFontUnderline) && glyph->kerningOffset) {
		int underlineY = y + _data._ascent + 2;

		if (underlineY >= 0 && underlineY < dst->h) {
			int underlineXStart = x - glyph->kerningOffset;
			int underlineXEnd = x - 1;

			if (underlineXStart < 0)
				underlineXStart = 0;
			if (underlineXEnd >= dst->w)
				underlineXEnd = dst->w - 1;

			for (int ulx = underlineXStart; ulx <= underlineXEnd; ulx++) {
				if (dst->format.bytesPerPixel == 1)
					*((byte *)dst->getBasePtr(ulx, underlineY)) = color;
				else if (dst->format.bytesPerPixel == 2)
					*((uint16 *)dst->getBasePtr(ulx, underlineY)) = color;
				else if (dst->format.bytesPerPixel == 4)
					*((uint32 *)dst->getBasePtr(ulx, underlineY)) = color;
			}
		}
	}

	for (uint16 i = yStart; i < yStop; i++) {
		byte *srcRow = _data._bitImage + i * _data._rowWords;

		for (uint16 j = xStart; j < xStop; j++) {
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

const MacGlyph *MacFONTFont::findGlyph(uint32 c) const {
	if (_data._glyphs.empty())
		return 0;

	if (c < _data._firstChar || c > _data._lastChar)
		return &_data._defaultChar;

	return &_data._glyphs[c - _data._firstChar];
}

int MacFONTFont::getKerningOffset(uint32 left, uint32 right) const {
	if (_data._family) {
		int kerning = _data._family->getKerningOffset(_data._style, left, right);
		kerning *= _data._size;

		return (int)(kerning / (double)(1 << 12));
	}

	return 0;
}

static void makeBold(Surface *src, int *dstGray, MacGlyph *glyph, int height);
static void makeOutline(Surface *src, Surface *dst, MacGlyph *glyph, int height);
static void makeItalic(Surface *src, Surface *dst, MacGlyph *glyph, int height);
static void makeUnderLine(Surface *src, MacGlyph *glyph, int ascent);
static void makeShadow(Surface *src, Surface *dst, MacGlyph *glyph, int height);

MacFONTFont *MacFONTFont::scaleFont(const MacFONTFont *src, int newSize, int slant) {
	if (!src) {
		warning("Empty font reference in scale font");
		return NULL;
	}

	if (src->getFontSize() == 0) {
		warning("Requested to scale 0 size font");
		return NULL;
	}

	Graphics::Surface srcSurf, tmpSurf;
	srcSurf.create(MAX(src->getFontSize() * 2, newSize * 2), MAX(src->getFontSize() * 2, newSize * 2),
				PixelFormat::createFormatCLUT8());
	int dstGraySize = newSize * 20 * newSize;
	int *dstGray = (int *)malloc(dstGraySize * sizeof(int));

	tmpSurf.create(MAX(src->getFontSize() * 2, newSize * 2), MAX(src->getFontSize() * 2 + 2, newSize * 2 + 2),
				PixelFormat::createFormatCLUT8());

	float scale = (float)newSize / (float)src->getFontSize();

	MacFONTdata data;

	data._fontType = src->_data._fontType;
	data._firstChar = src->_data._firstChar;
	data._lastChar = src->_data._lastChar;
	data._maxWidth = (int)((float)src->_data._maxWidth * scale);
	data._kernMax = (int)((float)src->_data._kernMax * scale);
	data._nDescent = (int)((float)src->_data._nDescent * scale);
	data._fRectWidth = (int)((float)src->_data._fRectWidth * scale + data._lastChar * 2);
	data._fRectHeight = (int)((float)src->_data._fRectHeight * scale);
	data._owTLoc = src->_data._owTLoc;
	data._ascent = (int)((float)src->_data._ascent * scale);
	data._descent = (int)((float)src->_data._descent * scale);
	data._leading = (int)((float)src->_data._leading * scale);

	data._family = src->_data._family;
	data._size = src->_data._size;
	data._style = src->_data._style;

	data._glyphs.resize(src->_data._glyphs.size());

	// when we are generating the slant fonts. e.g. italic font, underLine font. we set this. more detail is in drawChar
	data._slant = slant;

	// update the height, for shadow and outline font, we are drawing the outline, thus we may have 2 more pixel height
	// for the underLine, we are drawing the line at ascent + 2, so we shall extend the height when we need.
	if ((slant & kMacFontShadow) || (slant & kMacFontOutline))
		data._fRectHeight += 2;
	else if (slant & kMacFontUnderline) {
		data._fRectHeight = MAX((int)data._fRectHeight, data._ascent + 2);
	}

	data._surfHeight = data._fRectHeight;

	// Determine width of the bit image table
	int newBitmapWidth = 0;

	// add the offset which we may use when we are making fonts
	int bitmapOffset = 2;

	// for italic, we need to calc our self. for shadow, it's 3
	// for bold and outline, it's 2
	if (slant & kMacFontItalic)
		bitmapOffset = (data._fRectHeight - 1) / SLANTDEEP;
	else if (slant & kMacFontShadow)
		bitmapOffset++;

	for (uint i = 0; i < src->_data._glyphs.size() + 1; i++) {
		MacGlyph *glyph = (i == src->_data._glyphs.size()) ? &data._defaultChar : &data._glyphs[i];
		const MacGlyph *srcglyph = (i == src->_data._glyphs.size()) ? &src->_data._defaultChar : &src->_data._glyphs[i];

		glyph->width = (int)((float)srcglyph->width * scale);
		glyph->kerningOffset = (int)((float)srcglyph->kerningOffset * scale);
		glyph->bitmapWidth = glyph->width; //(int)((float)srcglyph->bitmapWidth * scale);
		glyph->bitmapOffset = newBitmapWidth;

		// Align width to a byte
		newBitmapWidth += (glyph->bitmapWidth + 7 + bitmapOffset) & ~0x7;
	}

	data._rowWords = newBitmapWidth;

	uint bitImageSize = data._rowWords * data._fRectHeight;
	data._bitImage = new byte[bitImageSize]();

	int dstPitch = data._rowWords;

	for (uint i = 0; i < src->_data._glyphs.size() + 1; i++) {
		const MacGlyph *srcglyph = (i == src->_data._glyphs.size()) ? &src->_data._defaultChar : &src->_data._glyphs[i];

		int grayLevel = src->_data._fRectHeight * srcglyph->width / 4;

#if DEBUGSCALING
		int ccc = 'c';
#endif

		MacGlyph *glyph = (i == src->_data._glyphs.size()) ? &data._defaultChar : &data._glyphs[i];
		src->scaleSingleGlyph(&srcSurf, dstGray, dstGraySize, glyph->bitmapWidth, data._fRectHeight, 0, 0, grayLevel, i + src->_data._firstChar,
							src->_data._fRectHeight, srcglyph->width, scale);

		if (slant & kMacFontBold) {
			memset(dstGray, 0, dstGraySize * sizeof(int));
			makeBold(&srcSurf, dstGray, glyph, data._fRectHeight);

			for (uint16 y = 0; y < data._fRectHeight; y++) {
				int *srcPtr = &dstGray[y * glyph->bitmapWidth];
				byte *dstPtr = (byte *)srcSurf.getBasePtr(0, y);

				for (uint16 x = 0; x < glyph->bitmapWidth; x++, srcPtr++, dstPtr++) {
					if (*srcPtr)
						*dstPtr = 1;

#if DEBUGSCALING
					if (i == ccc)
						debugN("%c", *srcPtr ? '@' : '.');
#endif
				}

#if DEBUGSCALING
				if (i == ccc)
					debugN("\n");
#endif
			}
		}

		if (slant & kMacFontOutline) {
			tmpSurf.fillRect(Common::Rect(tmpSurf.w, tmpSurf.h), 0);
			makeOutline(&srcSurf, &tmpSurf, glyph, data._fRectHeight);
			srcSurf.copyFrom(tmpSurf);
		}

		if (slant & kMacFontItalic) {
			tmpSurf.fillRect(Common::Rect(tmpSurf.w, tmpSurf.h), 0);
			makeItalic(&srcSurf, &tmpSurf, glyph, data._fRectHeight);
			srcSurf.copyFrom(tmpSurf);
		}

		if (slant & kMacFontUnderline)
			makeUnderLine(&srcSurf, glyph, data._ascent);

		if (slant & kMacFontShadow) {
			tmpSurf.fillRect(Common::Rect(tmpSurf.w, tmpSurf.h), 0);
			makeShadow(&srcSurf, &tmpSurf, glyph, data._fRectHeight);
			srcSurf.copyFrom(tmpSurf);
		}

		byte *ptr = &data._bitImage[glyph->bitmapOffset / 8];

		for (int y = 0; y < data._fRectHeight; y++) {
			byte *dst = ptr;
			byte *srcPtr = (byte *)srcSurf.getBasePtr(0, y);
			byte b = 0;

			for (int x = 0; x < glyph->bitmapWidth; x++, srcPtr++) {
				b <<= 1;

				if (*srcPtr == 1)
					b |= 1;

				if (x % 8 == 7) {
					*dst++ = b;
					b = 0;
				}
			}

#if DEBUGSCALING
			if (i == ccc) {
				debugN(1, "--> %d ", grayLevel);

				grayPtr = &dstGray[y * glyph->width];
				for (int x = 0; x < glyph->width; x++, grayPtr++)
					debugN("%c", *grayPtr > grayLevel ? '#' : '.');
			}
#endif

			if (((glyph->bitmapWidth - 1) % 8)) {
#if DEBUGSCALING
				if (i == ccc)
					debugN("  --- %02x (w: %d bw: %d << %d)", b, glyph->width, glyph->bitmapWidth, 7 - ((glyph->width - 1) % 8));
#endif

				b <<= 7 - ((glyph->bitmapWidth - 1) % 8);
				*dst = b;

#if DEBUGSCALING
				if (i == ccc)
					debugN("  --- %02x ", b);
#endif
			}

#if DEBUGSCALING
			if (i == ccc) {
				byte *srcRow = data._bitImage + y * data._rowWords;

				for (uint16 x = 0; x < glyph->bitmapWidth; x++) {
					uint16 bitmapOffset = glyph->bitmapOffset + x;

					debugN("%c", srcRow[bitmapOffset / 8] & (1 << (7 - (bitmapOffset % 8))) ? '*' : '.');
				}

				debugN("\n");
			}
#endif

			ptr += dstPitch;
		}
	}

	srcSurf.free();
	tmpSurf.free();
	free(dstGray);

	return new MacFONTFont(data);
}

static void makeBold(Surface *src, int *dstGray, MacGlyph *glyph, int height) {
	glyph->width++;
	glyph->bitmapWidth++;

	for (uint16 y = 0; y < height; y++) {
		byte *srcPtr = (byte *)src->getBasePtr(0, y);
		int *dst = &dstGray[y * glyph->bitmapWidth];

		for (uint16 x = 0; x < glyph->bitmapWidth; x++, srcPtr++, dst++) {
			bool left = x ? *(srcPtr - 1) == 1 : false;
			bool center = *srcPtr == 1;
			bool right = x > glyph->bitmapWidth - 1 ? false : *(srcPtr + 1) == 1;

			bool edge, bold, res;

			bold = center || left;
			edge = !center && right;
			res = (bold && !edge);

			*dst = res ? 1 : 0;
		}
	}
}

static void makeOutline(Surface *src, Surface *dst, MacGlyph *glyph, int height) {
	glyph->bitmapWidth += 2;
	glyph->width++;

	int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
	int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

	for (uint16 y = 0; y < height + 2; y++) {
		byte *srcPtr = (byte *)src->getBasePtr(0, y);
		byte *dstPtr = (byte *)dst->getBasePtr(0, y);

		for (uint16 x = 0; x < glyph->bitmapWidth; x++, dstPtr++, srcPtr++) {
			if (*srcPtr)
				continue;
			// for every white pixel, if there is black pixel around it. It means that the white pixel is boundary, then we draw it as black pixel.
			for (int i = 0; i < 8; i++) {
				int nx = x + dx[i];
				int ny = y + dy[i];
				if (nx >= src->w || nx < 0 || ny >= src->h || ny < 0)
					continue;

				if (*((byte *)src->getBasePtr(nx, ny))) {
					*dstPtr = 1;
					break;
				}
			}
		}
	}
}

static void makeItalic(Surface *src, Surface *dst, MacGlyph *glyph, int height) {
	int dw = (height - 1) / SLANTDEEP;

	for (uint16 y = 0; y < height; y++) {
		int dx = dw - y / SLANTDEEP;
		byte *srcPtr = (byte *)src->getBasePtr(0, y);
		byte *dstPtr = (byte *)dst->getBasePtr(dx, y);

		for (uint16 x = 0; x < glyph->width; x++, srcPtr++, dstPtr++) {
			*dstPtr = *srcPtr;
		}
	}
	glyph->bitmapWidth += dw;
	glyph->kerningOffset -= dw / 2;
}

static void makeUnderLine(Surface *src, MacGlyph *glyph, int ascent) {
	// this case is for space, which has the same number of kerning offset and width.
	// inorder to draw the underLine for space, we need to disable the kerning offset of it.
	if (glyph->width == glyph->kerningOffset)
		glyph->kerningOffset = 0;

	for (int x = 0; x < glyph->width; x++)
		*((byte *) src->getBasePtr(x, ascent + 2)) = 1;
}

static void makeShadow(Surface *src, Surface *dst, MacGlyph *glyph, int height) {
	// makeShadow looks like just the outLine font with one more shadow at right-most edge and lowest edge
	makeOutline(src, dst, glyph, height);
	glyph->bitmapWidth++;
	glyph->width++;

	// right to left
	for (uint16 y = 0; y < height + 2; y++) {
		for (int x = dst->w - 1; x >= 0; x--) {
			byte *dstPtr = (byte *)dst->getBasePtr(x, y);
			if (*dstPtr)
				continue;

			// check the left pixel. if it's black, then we black the current one and break.
			byte *left = (byte *)dst->getBasePtr(MAX(x - 1, 0), y);
			if (*left) {
				*dstPtr = 1;
				break;
			}
		}
	}

	// down to up
	for (uint16 x = 0; x < glyph->bitmapWidth; x++) {
		for (int y = dst->h - 1; y >= 0; y--) {
			byte *dstPtr = (byte *) dst->getBasePtr(x, y);

			if (*dstPtr)
				continue;

			byte *up = (byte *)dst->getBasePtr(x, MAX(y - 1, 0));
			if (*up) {
				*dstPtr = 1;
				break;
			}
		}
	}
}

void MacFONTFont::testBlit(const MacFONTFont *src, ManagedSurface *dst, int color, int x0, int y0, int width) {
	for (int y = 0; y < src->_data._fRectHeight; y++) {
		byte *srcRow = src->_data._bitImage + y * src->_data._rowWords;

		for (int x = 0; x < width; x++) {
			uint16 bitmapOffset = x + 64;

			if (srcRow[bitmapOffset / 8] & (1 << (7 - (bitmapOffset % 8)))) {
				if (dst->format.bytesPerPixel == 1)
					*((byte *)dst->getBasePtr(x0 + x, y0 + y)) = color;
				else if (dst->format.bytesPerPixel == 2)
					*((uint16 *)dst->getBasePtr(x0 + x, y0 + y)) = color;
				else if (dst->format.bytesPerPixel == 4)
					*((uint32 *)dst->getBasePtr(x0 + x, y0 + y)) = color;
			}
		}
	}
}

} // End of namespace Graphics
