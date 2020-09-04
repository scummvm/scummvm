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
#include "graphics/managed_surface.h"
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

MacFontFamily::MacFontFamily() {
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

			debug(10, "style: %d left: %g bottom: %g rigth: %g top: %g", _ffBBoxes[i]._style,
					_ffBBoxes[i]._left / (double)(1<<12), _ffBBoxes[i]._bottom / (double)(1<<12),
					_ffBBoxes[i]._right / (double)(1<<12), _ffBBoxes[i]._top / (double)(1<<12));
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

	if (getDepth(_data._fontType) != 1) {
		warning("MacFONTFont: %dbpp fonts are not supported", getDepth(_data._fontType));

		return false;
	}

	// If positive, _nDescent holds the high bits of the offset to the
	// width/offset table.
	// http://mirror.informatimago.com/next/developer.apple.com/documentation/mac/Text/Text-252.html
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
			stream.readUint16BE();
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

	return glyph->width;
}

void MacFONTFont::drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	assert(dst != 0);
	assert(dst->format.bytesPerPixel == 1 || dst->format.bytesPerPixel == 2 || dst->format.bytesPerPixel == 4);

	const MacGlyph *glyph = findGlyph(chr);
	if (!glyph || glyph->width == 0)
		return;

	for (uint16 i = 0; i < _data._fRectHeight; i++) {
		byte *srcRow = _data._bitImage + i * _data._rowWords;

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

#if DEBUGSCALING
bool dododo;
#endif

static void magnifyGray(Surface *src, int *dstGray, int width, int height, float scale);
static void makeBold(Surface *src, int *dstGray, MacGlyph *glyph, int height);
static void makeOutline(Surface *src, Surface *dst, MacGlyph *glyph, int height);

MacFONTFont *MacFONTFont::scaleFont(const MacFONTFont *src, int newSize, bool bold, bool italic, bool outline) {
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
	data._surfHeight = data._fRectHeight + 2;	// Outline
	data._owTLoc = src->_data._owTLoc;
	data._ascent = (int)((float)src->_data._ascent * scale);
	data._descent = (int)((float)src->_data._descent * scale);
	data._leading = (int)((float)src->_data._leading * scale);

	data._family = src->_data._family;
	data._size = src->_data._size;
	data._style = src->_data._style;

	data._glyphs.resize(src->_data._glyphs.size());

	// Determine width of the bit image table
	int newBitmapWidth = 0;
	for (uint i = 0; i < src->_data._glyphs.size() + 1; i++) {
		MacGlyph *glyph = (i == src->_data._glyphs.size()) ? &data._defaultChar : &data._glyphs[i];
		const MacGlyph *srcglyph = (i == src->_data._glyphs.size()) ? &src->_data._defaultChar : &src->_data._glyphs[i];

		glyph->width = (int)((float)srcglyph->width * scale);
		glyph->kerningOffset = (int)((float)srcglyph->kerningOffset * scale);
		glyph->bitmapWidth = glyph->width; //(int)((float)srcglyph->bitmapWidth * scale);
		glyph->bitmapOffset = newBitmapWidth;

		// Align width to a byte
		newBitmapWidth += (glyph->bitmapWidth + 7 + 2) & ~0x7; // Add 2 pixels for italic and bold
	}

	data._rowWords = newBitmapWidth;

	uint bitImageSize = data._rowWords * data._fRectHeight;
	data._bitImage = new byte[bitImageSize];

	int dstPitch = data._rowWords;

	for (uint i = 0; i < src->_data._glyphs.size() + 1; i++) {
		const MacGlyph *srcglyph = (i == src->_data._glyphs.size()) ? &src->_data._defaultChar : &src->_data._glyphs[i];

		int grayLevel = src->_data._fRectHeight * srcglyph->width / 4;

#if DEBUGSCALING
		int ccc = 'c';
		dododo = i == ccc;
#endif

		srcSurf.fillRect(Common::Rect(srcSurf.w, srcSurf.h), 0);
		src->drawChar(&srcSurf, i + src->_data._firstChar, 0, 0, 1);
		memset(dstGray, 0, dstGraySize * sizeof(int));
		magnifyGray(&srcSurf, dstGray, srcglyph->width, src->_data._fRectHeight, scale);

		MacGlyph *glyph = (i == src->_data._glyphs.size()) ? &data._defaultChar : &data._glyphs[i];
		int *grayPtr = dstGray;

		for (int y = 0; y < data._fRectHeight; y++) {
			byte *dst = (byte *)srcSurf.getBasePtr(0, y);

			for (int x = 0; x < glyph->width; x++, grayPtr++, dst++) {
#if DEBUGSCALING
				if (i == ccc) {
					if (*grayPtr)
						debugN(1, "%3d ", *grayPtr);
					else
						debugN(1, "    ");
				}
#endif
				if (*grayPtr > grayLevel)
					*dst = 1;
				else
					*dst = 0;
			}
#if DEBUGSCALING
			if (i == ccc)
				debug(1, "");
#endif
		}

		if (bold) {
			memset(dstGray, 0, dstGraySize * sizeof(int));
			makeBold(&srcSurf, dstGray, glyph, data._fRectHeight);

			for (uint16 y = 0; y < data._fRectHeight; y++) {
				int *srcPtr = &dstGray[y * glyph->width];
				byte *dstPtr = (byte *)srcSurf.getBasePtr(0, y);

				for (uint16 x = 0; x < glyph->width; x++, srcPtr++, dstPtr++) {
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

		if (outline) {
			makeOutline(&srcSurf, &tmpSurf, glyph, data._fRectHeight);
			srcSurf.copyFrom(tmpSurf);
		}

		byte *ptr = &data._bitImage[glyph->bitmapOffset / 8];

		for (int y = 0; y < data._fRectHeight; y++) {
			byte *dst = ptr;
			byte *srcPtr = (byte *)srcSurf.getBasePtr(0, y);
			byte b = 0;

			for (int x = 0; x < glyph->width; x++, srcPtr++) {
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

			if (((glyph->width - 1) % 8)) {
#if DEBUGSCALING
				if (i == ccc)
					debugN("  --- %02x (w: %d bw: %d << %d)", b, glyph->width, glyph->bitmapWidth, 7 - ((glyph->width - 1) % 8));
#endif

				b <<= 7 - ((glyph->width - 1) % 8);
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
	free(dstGray);

	return new MacFONTFont(data);
}

#define wholedivide(x, y)	(((x)+((y)-1))/(y))

static void countupScore(int *dstGray, int x, int y, int bbw, int bbh, float scale) {
	int newbbw = bbw * scale;
	int newbbh = bbh * scale;
	int x_ = x * newbbw;
	int y_ = y * newbbh;
	int x1 = x_ + newbbw;
	int y1 = y_ + newbbh;

	int newxbegin = x_ / bbw;
	int newybegin = y_ / bbh;
	int newxend = wholedivide(x1, bbw);
	int newyend = wholedivide(y1, bbh);

	for (int newy = newybegin; newy < newyend; newy++) {
		for (int newx = newxbegin; newx < newxend; newx++) {
			int newX = newx * bbw;
			int newY = newy * bbh;
			int newX1 = newX + bbw;
			int newY1 = newY + bbh;
			dstGray[newy * newbbw + newx] += (MIN(x1, newX1) - MAX(x_, newX)) *
											 (MIN(y1, newY1) - MAX(y_, newY));
		}
	}
}

static void magnifyGray(Surface *src, int *dstGray, int width, int height, float scale) {
	for (uint16 y = 0; y < height; y++) {
		for (uint16 x = 0; x < width; x++) {
			if (*((byte *)src->getBasePtr(x, y)) == 1)
				countupScore(dstGray, x, y, width, height, scale);
#if DEBUGSCALING
			if (dododo)
				debugN("%c", *((byte *)src->getBasePtr(x, y)) == 1 ? '*' : ' ');
#endif
		}

#if DEBUGSCALING
		if (dododo)
			debugN("\n");
#endif
	}
}

static void makeBold(Surface *src, int *dstGray, MacGlyph *glyph, int height) {
	glyph->width++;

	for (uint16 y = 0; y < height; y++) {
		byte *srcPtr = (byte *)src->getBasePtr(0, y);
		int *dst = &dstGray[y * glyph->width];

		for (uint16 x = 0; x < glyph->width; x++, srcPtr++, dst++) {
			bool left = x ? *(srcPtr - 1) == 1 : false;
			bool center = *srcPtr == 1;
			bool right = x > glyph->width - 1 ? false : *(srcPtr + 1) == 1;

			bool edge, bold, res;

			bold = center || left;
			edge = !center && right;
			res = (bold && !edge);

			*dst = res ? 1 : 0;
		}
	}
}

static void makeOutline(Surface *src, Surface *dst, MacGlyph *glyph, int height) {
	glyph->width += 2;
	glyph->height++;

	for (uint16 y = 0; y < height + 1; y++) {
		byte *srcPtr = (byte *)src->getBasePtr(0, y);
		byte *srcPtr2 = (byte *)src->getBasePtr(0, (y == height ? 0 : y + 1));
		byte *dstPtr = (byte *)dst->getBasePtr(0, y);

		for (uint16 x = 0; x < glyph->width; x++, dstPtr++) {
			byte pixX, pixR, pixB;
			pixX = (x == 0 || x == glyph->width - 2) ? 0 : *srcPtr++;
			pixB = (x == 0 || x == glyph->width - 2 || y == height) ? 0 : *srcPtr2++;
			pixR = (x == glyph->width - 1) ? 0 : *srcPtr;

			*dstPtr = (pixX ^ pixR) | (pixX ^ pixB);
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
