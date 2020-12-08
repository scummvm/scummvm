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

// Since FreeType2 includes files, which contain forbidden symbols, we need to
// allow all symbols here.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"
#ifdef USE_FREETYPE2

#include "graphics/fonts/ttf.h"
#include "graphics/font.h"
#include "graphics/surface.h"

#include "common/ustr.h"
#include "common/file.h"
#include "common/config-manager.h"
#include "common/singleton.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/unzip.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_GLYPH_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H

#if (FREETYPE_MAJOR > 2 ||                                                          \
        (FREETYPE_MAJOR == 2 && (FREETYPE_MINOR > 3 ||                              \
                                 (FREETYPE_MINOR == 3 && FREETYPE_PATCH >= 8))))
// FT2.3.8+, nothing to do, FT_GlyphSlot_Own_Bitmap is in FT_BITMAP_H
#define FAKE_BOLD 2
#elif (FREETYPE_MAJOR > 2 ||                                                        \
        (FREETYPE_MAJOR == 2 && (FREETYPE_MINOR > 2 ||                              \
                                 (FREETYPE_MINOR == 2 && FREETYPE_PATCH >= 0))))
// FT2.2.0+ have FT_GlyphSlot_Own_Bitmap in FT_SYNTHESIS_H
#include FT_SYNTHESIS_H
#define FAKE_BOLD 2
#elif (FREETYPE_MAJOR > 2 ||                                                        \
        (FREETYPE_MAJOR == 2 && (FREETYPE_MINOR > 1 ||                              \
                                 (FREETYPE_MINOR == 1 && FREETYPE_PATCH >= 10))))
// FT2.1.10+ don't have FT_GlyphSlot_Own_Bitmap but they have FT_Bitmap_Embolden, do workaround
#define FAKE_BOLD 1
#else
// Older versions don't have FT_Bitmap_Embolden
#define FAKE_BOLD 0
#endif

#if FREETYPE_MAJOR > 2 || ( FREETYPE_MAJOR == 2 &&  FREETYPE_MINOR >= 9)
#include FT_TRUETYPE_DRIVER_H
#endif

namespace Graphics {

namespace {

inline int ftCeil26_6(FT_Pos x) {
	return (x + 63) / 64;
}

inline int divRoundToNearest(int dividend, int divisor) {
	return (dividend + (divisor / 2)) / divisor;
}

} // End of anonymous namespace

class TTFLibrary : public Common::Singleton<TTFLibrary> {
public:
	TTFLibrary();
	~TTFLibrary();

	/**
	 * Check whether FreeType2 is initialized properly.
	 */
	bool isInitialized() const { return _initialized; }

	bool loadFont(const uint8 *file, const int32 face_index, const uint32 size, FT_Face &face);
	void closeFont(FT_Face &face);
private:
	FT_Library _library;
	bool _initialized;
};

void shutdownTTF() {
	TTFLibrary::destroy();
}

#define g_ttf ::Graphics::TTFLibrary::instance()

TTFLibrary::TTFLibrary() : _library(), _initialized(false) {
	if (!FT_Init_FreeType(&_library))
		_initialized = true;
}

TTFLibrary::~TTFLibrary() {
	if (_initialized) {
		FT_Done_FreeType(_library);
		_initialized = false;
	}
}

bool TTFLibrary::loadFont(const uint8 *file, const int32 face_index, const uint32 size, FT_Face &face) {
	assert(_initialized);

	return (FT_New_Memory_Face(_library, file, size, face_index, &face) == 0);
}

void TTFLibrary::closeFont(FT_Face &face) {
	assert(_initialized);

	FT_Done_Face(face);
}

class TTFFont : public Font {
public:
	TTFFont();
	virtual ~TTFFont();

	bool load(Common::SeekableReadStream &stream, int size, TTFSizeMode sizeMode,
	          uint dpi, TTFRenderMode renderMode, const uint32 *mapping, bool stemDarkening);
	bool load(uint8 *ttfFile, uint32 sizeFile, int32 faceIndex, bool fakeBold, bool fakeItalic,
	          int size, TTFSizeMode sizeMode, uint dpi, TTFRenderMode renderMode, const uint32 *mapping, bool stemDarkening);

	virtual int getFontHeight() const;

	virtual int getMaxCharWidth() const;

	virtual int getCharWidth(uint32 chr) const;

	virtual int getKerningOffset(uint32 left, uint32 right) const;

	virtual Common::Rect getBoundingBox(uint32 chr) const;

	virtual void drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const;
private:
	bool _initialized;
	FT_Face _face;

	uint8 *_ttfFile;
	uint32 _size;

	int _width, _height;
	int _ascent, _descent;

	struct Glyph {
		Surface image;
		int xOffset, yOffset;
		int advance;
		FT_UInt slot;
	};

	bool cacheGlyph(Glyph &glyph, uint32 chr) const;
	typedef Common::HashMap<uint32, Glyph> GlyphCache;
	mutable GlyphCache _glyphs;
	bool _allowLateCaching;
	void assureCached(uint32 chr) const;

	Common::SeekableReadStream *readTTFTable(FT_ULong tag) const;

	int computePointSize(int size, TTFSizeMode sizeMode) const;
	int readPointSizeFromVDMXTable(int height) const;
	int computePointSizeFromHeaders(int height) const;

	FT_Int32 _loadFlags;
	FT_Render_Mode _renderMode;
	bool _hasKerning;

	bool _fakeBold;
	bool _fakeItalic;
};

TTFFont::TTFFont()
    : _initialized(false), _face(), _ttfFile(0), _size(0), _width(0), _height(0), _ascent(0),
      _descent(0), _glyphs(), _loadFlags(FT_LOAD_TARGET_NORMAL), _renderMode(FT_RENDER_MODE_NORMAL),
      _hasKerning(false), _allowLateCaching(false), _fakeBold(false), _fakeItalic(false) {
}

TTFFont::~TTFFont() {
	if (_initialized) {
		g_ttf.closeFont(_face);

		delete[] _ttfFile;
		_ttfFile = 0;

		for (GlyphCache::iterator i = _glyphs.begin(), end = _glyphs.end(); i != end; ++i)
			i->_value.image.free();

		_initialized = false;
	}
}

bool TTFFont::load(Common::SeekableReadStream &stream, int size, TTFSizeMode sizeMode,
                   uint dpi, TTFRenderMode renderMode, const uint32 *mapping, bool stemDarkening) {
	if (!g_ttf.isInitialized())
		return false;

	uint32 sizeFile = stream.size();
	if (!sizeFile)
		return false;

	uint8 *ttfFile = new uint8[sizeFile];
	assert(ttfFile);

	if (stream.read(ttfFile, sizeFile) != sizeFile) {
		delete[] ttfFile;
		return false;
	}

	if (!load(ttfFile, sizeFile, 0, false, false, size, sizeMode, dpi, renderMode, mapping, stemDarkening)) {
		delete[] ttfFile;
		return false;
	}

	// Don't delete ttfFile as it's now owned by the class
	return true;
}

bool TTFFont::load(uint8 *ttfFile, uint32 sizeFile, int32 faceIndex, bool bold, bool italic,
                   int size, TTFSizeMode sizeMode, uint dpi, TTFRenderMode renderMode, const uint32 *mapping, bool stemDarkening) {
	_initialized = false;

	if (!g_ttf.isInitialized())
		return false;

	_size = sizeFile;
	if (!_size)
		return false;

	_ttfFile = ttfFile;
	assert(_ttfFile);

	if (!g_ttf.loadFont(_ttfFile, faceIndex, _size, _face)) {
		// Don't delete ttfFile as we return fail
		_ttfFile = 0;

		return false;
	}

	// We only support scalable fonts.
	if (!FT_IS_SCALABLE(_face)) {
		g_ttf.closeFont(_face);

		// Don't delete ttfFile as we return fail
		_ttfFile = 0;

		return false;
	}
	if (stemDarkening) {
#if FREETYPE_MAJOR > 2 || ( FREETYPE_MAJOR == 2 &&  FREETYPE_MINOR >= 9)
		FT_Parameter param;
		param.tag = FT_PARAM_TAG_STEM_DARKENING;
		param.data = &stemDarkening;
		FT_Face_Properties(_face, 1, &param);
#else
		warning("Stem darkening is not available with this version of FreeType");
#endif
	}

	// Check whether we have kerning support
	_hasKerning = (FT_HAS_KERNING(_face) != 0);

	if (FT_Set_Char_Size(_face, 0, computePointSize(size, sizeMode) * 64, dpi, dpi)) {
		g_ttf.closeFont(_face);

		// Don't delete ttfFile as we return fail
		_ttfFile = 0;

		return false;
	}

	bool fontBold = ((_face->style_flags & FT_STYLE_FLAG_BOLD) != 0);
	_fakeBold = bold && !fontBold;
	bool fontItalic = ((_face->style_flags & FT_STYLE_FLAG_ITALIC) != 0);
	_fakeItalic = italic && !fontItalic;

	switch (renderMode) {
	case kTTFRenderModeNormal:
		_loadFlags = FT_LOAD_TARGET_NORMAL;
		_renderMode = FT_RENDER_MODE_NORMAL;
		break;

	case kTTFRenderModeLight:
		_loadFlags = FT_LOAD_TARGET_LIGHT;
		_renderMode = FT_RENDER_MODE_LIGHT;
		break;

	case kTTFRenderModeMonochrome:
		_loadFlags = FT_LOAD_TARGET_MONO;
		_renderMode = FT_RENDER_MODE_MONO;
		break;

	default:
		break;
	}

	FT_Fixed yScale = _face->size->metrics.y_scale;
	_ascent = ftCeil26_6(FT_MulFix(_face->ascender, yScale));
	_descent = ftCeil26_6(FT_MulFix(_face->descender, yScale));

	_width = ftCeil26_6(FT_MulFix(_face->max_advance_width, _face->size->metrics.x_scale));
	_height = _ascent - _descent + 1;

#if FAKE_BOLD > 0
	// Width isn't modified when we can't fake bold
	if (_fakeBold) {
		// Embolden by 1 pixel width
		_width += 1;
	}
#endif

	// Apply a matrix transform for all loaded glyphs
	if (_fakeItalic) {
		// This matrix is taken from Wine source code
		// 16.16 fixed-point
		FT_Matrix slantMat;
		slantMat.xx = (1 << 16);      // 1.
		slantMat.xy = (1 << 16) >> 2; // .25
		slantMat.yx = 0;              // 0.
		slantMat.yy = (1 << 16);      // 1.
		FT_Set_Transform(_face, &slantMat, nullptr);

		// Don't try to load bitmap version of font as we have to transform the strokes
		_loadFlags |= FT_LOAD_NO_BITMAP;
	}

	if (!mapping) {
		// Allow loading of all unicode characters.
		_allowLateCaching = true;

		// Load all ISO-8859-1 characters.
		for (uint i = 0; i < 256; ++i) {
			if (!cacheGlyph(_glyphs[i], i)) {
				_glyphs.erase(i);
			}
		}
	} else {
		// We have a fixed map of characters do not load more later.
		_allowLateCaching = false;

		for (uint i = 0; i < 256; ++i) {
			const uint32 unicode = mapping[i] & 0x7FFFFFFF;
			const bool isRequired = (mapping[i] & 0x80000000) != 0;
			// Check whether loading an important glyph fails and error out if
			// that is the case.
			if (!cacheGlyph(_glyphs[i], unicode)) {
				_glyphs.erase(i);
				if (isRequired) {
					g_ttf.closeFont(_face);

					// Don't delete ttfFile as we return fail
					_ttfFile = 0;

					return false;
				}
			}
		}
	}

	if (_glyphs.size() == 0) {
		g_ttf.closeFont(_face);

		// Don't delete ttfFile as we return fail
		_ttfFile = 0;

		return false;
	} else {
		_initialized = true;
		// At this point we get ownership of _ttfFile
		return true;
	}
}

int TTFFont::computePointSize(int size, TTFSizeMode sizeMode) const {
	int ptSize = 0;
	switch (sizeMode) {
	case kTTFSizeModeCell: {
		ptSize = readPointSizeFromVDMXTable(size);

		if (ptSize == 0) {
			ptSize = computePointSizeFromHeaders(size);
		}

		if (ptSize == 0) {
			warning("Unable to compute point size for font '%s'", _face->family_name);
			ptSize = 1;
		}
		break;
	}
	case kTTFSizeModeCharacter:
		ptSize = size;
		break;
	default:
		break;
	}

	return ptSize;
}

Common::SeekableReadStream *TTFFont::readTTFTable(FT_ULong tag) const {
	// Find the required buffer size by calling the load function with nullptr
	FT_ULong size = 0;
	FT_Error err = FT_Load_Sfnt_Table(_face, tag, 0, nullptr, &size);
	if (err) {
		return nullptr;
	}

	byte *buf = (byte *)malloc(size);
	if (!buf) {
		return nullptr;
	}

	err = FT_Load_Sfnt_Table(_face, tag, 0, buf, &size);
	if (err) {
		free(buf);
		return nullptr;
	}

	return new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES);
}

int TTFFont::readPointSizeFromVDMXTable(int height) const {
	// The Vertical Device Metrics table matches font heights with point sizes.
	// FreeType does not expose it, we have to parse it ourselves.
	// See https://www.microsoft.com/typography/otspec/vdmx.htm

	Common::ScopedPtr<Common::SeekableReadStream> vdmxBuf(readTTFTable(TTAG_VDMX));
	if (!vdmxBuf) {
		return 0;
	}

	// Read the main header
	vdmxBuf->skip(4); // Skip the version
	uint16 numRatios = vdmxBuf->readUint16BE();

	// Compute the starting position for the group table positions table
	int32 offsetTableStart = vdmxBuf->pos() + 4 * numRatios;

	// Search the ratio table for the 1:1 ratio, or the default record (0, 0, 0)
	int32 selectedRatio = -1;
	for (uint16 i = 0; i < numRatios; i++) {
		vdmxBuf->skip(1); // Skip the charset subset
		uint8 xRatio = vdmxBuf->readByte();
		uint8 yRatio1 = vdmxBuf->readByte();
		uint8 yRatio2 = vdmxBuf->readByte();

		if ((xRatio == 1 && yRatio1 <= 1 && yRatio2 >= 1)
		    || (xRatio == 0 && yRatio1 == 0 && yRatio2 == 0)) {
			selectedRatio = i;
			break;
		}
	}
	if (selectedRatio < 0) {
		return 0;
	}

	// Read from group table positions table to get the group table offset
	vdmxBuf->seek(offsetTableStart + sizeof(uint16) * selectedRatio);
	uint16 groupOffset = vdmxBuf->readUint16BE();

	// Read the group table header
	vdmxBuf->seek(groupOffset);
	uint16 numRecords = vdmxBuf->readUint16BE();
	vdmxBuf->skip(2); // Skip the table bounds

	// Search a record matching the required height
	for (uint16 i = 0; i < numRecords; i++) {
		uint16 pointSize = vdmxBuf->readUint16BE();
		int16 yMax = vdmxBuf->readSint16BE();
		int16 yMin = vdmxBuf->readSint16BE();

		if (yMax + -yMin > height) {
			return 0;
		}
		if (yMax + -yMin == height) {
			return pointSize;
		}
	}

	return 0;
}

int TTFFont::computePointSizeFromHeaders(int height) const {
	TT_OS2 *os2Header = (TT_OS2 *)FT_Get_Sfnt_Table(_face, ft_sfnt_os2);
	TT_HoriHeader *horiHeader = (TT_HoriHeader *)FT_Get_Sfnt_Table(_face, ft_sfnt_hhea);

	if (os2Header && (os2Header->usWinAscent + os2Header->usWinDescent != 0)) {
		return divRoundToNearest(_face->units_per_EM * height, os2Header->usWinAscent + os2Header->usWinDescent);
	} else if (horiHeader && (horiHeader->Ascender + horiHeader->Descender != 0)) {
		return divRoundToNearest(_face->units_per_EM * height, horiHeader->Ascender + horiHeader->Descender);
	}

	return 0;
}

int TTFFont::getFontHeight() const {
	return _height;
}

int TTFFont::getMaxCharWidth() const {
	return _width;
}

int TTFFont::getCharWidth(uint32 chr) const {
	assureCached(chr);
	GlyphCache::const_iterator glyphEntry = _glyphs.find(chr);
	if (glyphEntry == _glyphs.end())
		return 0;
	else
		return glyphEntry->_value.advance;
}

int TTFFont::getKerningOffset(uint32 left, uint32 right) const {
	if (!_hasKerning)
		return 0;

	assureCached(left);
	assureCached(right);

	FT_UInt leftGlyph, rightGlyph;
	GlyphCache::const_iterator glyphEntry;

	glyphEntry = _glyphs.find(left);
	if (glyphEntry != _glyphs.end()) {
		leftGlyph = glyphEntry->_value.slot;
	} else {
		return 0;
	}

	glyphEntry = _glyphs.find(right);
	if (glyphEntry != _glyphs.end()) {
		rightGlyph = glyphEntry->_value.slot;
	} else {
		return 0;
	}

	if (!leftGlyph || !rightGlyph)
		return 0;

	FT_Vector kerningVector;
	FT_Get_Kerning(_face, leftGlyph, rightGlyph, FT_KERNING_DEFAULT, &kerningVector);
	return (kerningVector.x / 64);
}

Common::Rect TTFFont::getBoundingBox(uint32 chr) const {
	assureCached(chr);
	GlyphCache::const_iterator glyphEntry = _glyphs.find(chr);
	if (glyphEntry == _glyphs.end()) {
		return Common::Rect();
	} else {
		const int xOffset = glyphEntry->_value.xOffset;
		const int yOffset = glyphEntry->_value.yOffset;
		const Graphics::Surface &image = glyphEntry->_value.image;
		return Common::Rect(xOffset, yOffset, xOffset + image.w, yOffset + image.h);
	}
}

namespace {

template<typename ColorType>
static void renderGlyph(uint8 *dstPos, const int dstPitch, const uint8 *srcPos, const int srcPitch, const int w, const int h, ColorType color, const PixelFormat &dstFormat) {
	uint8 sR, sG, sB;
	dstFormat.colorToRGB(color, sR, sG, sB);

	for (int y = 0; y < h; ++y) {
		ColorType *rDst = (ColorType *)dstPos;
		const uint8 *src = srcPos;

		for (int x = 0; x < w; ++x) {
			if (*src == 255) {
				*rDst = color;
			} else if (*src) {
				const uint8 a = *src;

				uint8 dR, dG, dB;
				dstFormat.colorToRGB(*rDst, dR, dG, dB);

				dR = ((255 - a) * dR + a * sR) / 255;
				dG = ((255 - a) * dG + a * sG) / 255;
				dB = ((255 - a) * dB + a * sB) / 255;

				*rDst = dstFormat.RGBToColor(dR, dG, dB);
			}

			++rDst;
			++src;
		}

		dstPos += dstPitch;
		srcPos += srcPitch;
	}
}

} // End of anonymous namespace

void TTFFont::drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	assureCached(chr);
	GlyphCache::const_iterator glyphEntry = _glyphs.find(chr);
	if (glyphEntry == _glyphs.end())
		return;

	const Glyph &glyph = glyphEntry->_value;

	x += glyph.xOffset;
	y += glyph.yOffset;

	if (x > dst->w)
		return;
	if (y > dst->h)
		return;

	int w = glyph.image.w;
	int h = glyph.image.h;

	const uint8 *srcPos = (const uint8 *)glyph.image.getPixels();

	// Make sure we are not drawing outside the screen bounds
	if (x < 0) {
		srcPos -= x;
		w += x;
		x = 0;
	}

	if (x + w > dst->w)
		w = dst->w - x;

	if (w <= 0)
		return;

	if (y < 0) {
		srcPos -= y * glyph.image.pitch;
		h += y;
		y = 0;
	}

	if (y + h > dst->h)
		h = dst->h - y;

	if (h <= 0)
		return;

	uint8 *dstPos = (uint8 *)dst->getBasePtr(x, y);

	if (dst->format.bytesPerPixel == 1) {
		for (int cy = 0; cy < h; ++cy) {
			uint8 *rDst = dstPos;
			const uint8 *src = srcPos;

			for (int cx = 0; cx < w; ++cx) {
				// We assume a 1Bpp mode is a color indexed mode, thus we can
				// not take advantage of anti-aliasing here.
				if (*src >= 0x80)
					*rDst = color;

				++rDst;
				++src;
			}

			dstPos += dst->pitch;
			srcPos += glyph.image.pitch;
		}
	} else if (dst->format.bytesPerPixel == 2) {
		renderGlyph<uint16>(dstPos, dst->pitch, srcPos, glyph.image.pitch, w, h, color, dst->format);
	} else if (dst->format.bytesPerPixel == 4) {
		renderGlyph<uint32>(dstPos, dst->pitch, srcPos, glyph.image.pitch, w, h, color, dst->format);
	}
}

bool TTFFont::cacheGlyph(Glyph &glyph, uint32 chr) const {
	FT_UInt slot = FT_Get_Char_Index(_face, chr);
	if (!slot)
		return false;

	glyph.slot = slot;

	// We use the light target and render mode to improve the looks of the
	// glyphs. It is most noticable in FreeSansBold.ttf, where otherwise the
	// 't' glyph looks like it is cut off on the right side.
	if (FT_Load_Glyph(_face, slot, _loadFlags))
		return false;

	if (FT_Render_Glyph(_face->glyph, _renderMode))
		return false;

	if (_face->glyph->format != FT_GLYPH_FORMAT_BITMAP)
		return false;

	glyph.xOffset = _face->glyph->bitmap_left;
	glyph.yOffset = _ascent - _face->glyph->bitmap_top;

	glyph.advance = ftCeil26_6(_face->glyph->advance.x);

	const FT_Bitmap *bitmap;
#if FAKE_BOLD == 1
	FT_Bitmap ownBitmap;
#endif

	if (_fakeBold) {
#if FAKE_BOLD >= 2
		// Embolden by 1 pixel in x and 0 in y
		glyph.advance += 1;

		if (FT_GlyphSlot_Own_Bitmap(_face->glyph))
			return false;

		// That's 26.6 fixed-point units
		if (FT_Bitmap_Embolden(_face->glyph->library, &_face->glyph->bitmap, 1 << 6, 0))
			return false;
		
		bitmap = &_face->glyph->bitmap;
#elif FAKE_BOLD >= 1
		FT_Bitmap_New(&ownBitmap);

		if (FT_Bitmap_Copy(_face->glyph->library, &_face->glyph->bitmap, &ownBitmap))
			return false;

		// Embolden by 1 pixel in x and 0 in y
		glyph.advance += 1;

		// That's 26.6 fixed-point units
		if (FT_Bitmap_Embolden(_face->glyph->library, &ownBitmap, 1 << 6, 0))
			return false;

		bitmap = &ownBitmap;
#else
		// Can't do anything, just don't fake bold
		bitmap = &_face->glyph->bitmap;
#endif
	} else {
		bitmap = &_face->glyph->bitmap;
	}


	glyph.image.create(bitmap->width, bitmap->rows, PixelFormat::createFormatCLUT8());

	const uint8 *src = bitmap->buffer;
	int srcPitch = bitmap->pitch;
	if (srcPitch < 0) {
		src += (bitmap->rows - 1) * srcPitch;
		srcPitch = -srcPitch;
	}

	uint8 *dst = (uint8 *)glyph.image.getPixels();
	memset(dst, 0, glyph.image.h * glyph.image.pitch);

	switch (bitmap->pixel_mode) {
	case FT_PIXEL_MODE_MONO:
		for (int y = 0; y < (int)bitmap->rows; ++y) {
			const uint8 *curSrc = src;
			uint8 mask = 0;

			for (int x = 0; x < (int)bitmap->width; ++x) {
				if ((x % 8) == 0)
					mask = *curSrc++;

				if (mask & 0x80)
					*dst = 255;

				mask <<= 1;
				++dst;
			}

			src += srcPitch;
		}
		break;

	case FT_PIXEL_MODE_GRAY:
		for (int y = 0; y < (int)bitmap->rows; ++y) {
			memcpy(dst, src, bitmap->width);
			dst += glyph.image.pitch;
			src += srcPitch;
		}
		break;

	default:
		warning("TTFFont::cacheGlyph: Unsupported pixel mode %d", bitmap->pixel_mode);
		glyph.image.free();
		return false;
	}

#if FAKE_BOLD == 1
	if (_fakeBold) {
		FT_Bitmap_Done(_face->glyph->library, &ownBitmap);
	}
#endif

	return true;
}

void TTFFont::assureCached(uint32 chr) const {
	if (!chr || !_allowLateCaching || _glyphs.contains(chr)) {
		return;
	}

	Glyph newGlyph;
	if (cacheGlyph(newGlyph, chr)) {
		_glyphs[chr] = newGlyph;
	}
}

Font *loadTTFFont(Common::SeekableReadStream &stream, int size, TTFSizeMode sizeMode, uint dpi, TTFRenderMode renderMode, const uint32 *mapping, bool stemDarkening) {
	TTFFont *font = new TTFFont();

	if (!font->load(stream, size, sizeMode, dpi, renderMode, mapping, stemDarkening)) {
		delete font;
		return 0;
	}

	return font;
}

Font *loadTTFFontFromArchive(const Common::String &filename, int size, TTFSizeMode sizeMode, uint dpi, TTFRenderMode renderMode, const uint32 *mapping) {
	Common::SeekableReadStream *archiveStream = nullptr;
	if (ConfMan.hasKey("extrapath")) {
		Common::FSDirectory extrapath(ConfMan.get("extrapath"));
		archiveStream = extrapath.createReadStreamForMember("fonts.dat");
	}

	if (!archiveStream) {
		archiveStream = SearchMan.createReadStreamForMember("fonts.dat");
	}

	Common::Archive *archive = Common::makeZipArchive(archiveStream);
	if (!archive) {
		return nullptr;
	}

	Common::File f;
	if (!f.open(filename, *archive)) {
		return nullptr;
	}

	Font *font = loadTTFFont(f, size, sizeMode, dpi, renderMode, mapping);

	delete archive;
	return font;
}

static bool matchFaceName(const Common::U32String &faceName, const FT_Face &face) {
	if (faceName == Common::U32String(face->family_name, Common::kASCII)) {
		// International name in ASCII match
		return true;
	}

	// Try to match with localized name
	// Loosely copied from freetype2-demos
	FT_SfntName aname;
	FT_UInt num_strings = FT_Get_Sfnt_Name_Count(face);
	for (FT_UInt j = 0; j < num_strings; j++) {
		if (FT_Get_Sfnt_Name(face, j, &aname)) {
			continue;
		}
		if (aname.name_id != TT_NAME_ID_FONT_FAMILY) {
			continue;
		}

		if (aname.platform_id == TT_PLATFORM_MICROSOFT &&
		        aname.language_id != TT_MS_LANGID_ENGLISH_UNITED_STATES) {
			if (aname.encoding_id == TT_MS_ID_SYMBOL_CS ||
			        aname.encoding_id == TT_MS_ID_UNICODE_CS) {
				// MS local name in UTF-16
				// string_len is in bytes length, we take wchar_t length
				Common::U32String localName = Common::U32String::decodeUTF16BE((uint16 *) aname.string, aname.string_len / 2);

				if (faceName == localName) {
					return true;
				}
			} else {
				// No conversion: try to match with 1 byte encoding
				if (faceName == Common::U32String((char *)aname.string, aname.string_len, Common::kLatin1)) {
					return true;
				}
			}
		} else if (aname.platform_id == TT_PLATFORM_MACINTOSH &&
		           aname.language_id != TT_MAC_LANGID_ENGLISH) {
			// No conversion
			if (faceName == Common::U32String((char *)aname.string, aname.string_len, Common::kLatin1)) {
				return true;
			}
		}
	}
	return false;
}

Font *findTTFace(const Common::Array<Common::String> &files, const Common::U32String &faceName,
                 bool bold, bool italic, int size, uint dpi, TTFRenderMode renderMode, const uint32 *mapping) {
	if (!g_ttf.isInitialized())
		return nullptr;

	uint8 *bestTTFFile = nullptr;
	uint32 bestSize = 0;
	uint32 bestFaceId = (uint32) -1;
	uint32 bestPenalty = (uint32) -1;

	for (Common::Array<Common::String>::const_iterator it = files.begin(); it != files.end(); it++) {
		Common::File ttf;
		if (!ttf.open(*it)) {
			continue;
		}
		uint32 sizeFile = ttf.size();
		if (!sizeFile) {
			continue;
		}
		uint8 *ttfFile = new uint8[sizeFile];
		assert(ttfFile);

		if (ttf.read(ttfFile, sizeFile) != sizeFile) {
			delete[] ttfFile;
			ttfFile = 0;

			continue;
		}

		ttf.close();

		FT_Face face;

		// Load face index -1 to get the count
		if (!g_ttf.loadFont(ttfFile, -1, sizeFile, face)) {
			delete[] ttfFile;
			ttfFile = 0;

			continue;
		}

		FT_Long num_faces = face->num_faces;

		g_ttf.closeFont(face);

		for (FT_Long i = 0; i < num_faces; i++) {
			if (!g_ttf.loadFont(ttfFile, i, sizeFile, face)) {
				continue;
			}

			if (!matchFaceName(faceName, face)) {
				// No match on names: we don't do like Windows, we don't take a random font
				g_ttf.closeFont(face);
				continue;
			}

			bool fontBold = ((face->style_flags & FT_STYLE_FLAG_BOLD) != 0);
			bool fontItalic = ((face->style_flags & FT_STYLE_FLAG_ITALIC) != 0);

			g_ttf.closeFont(face);

			// These scores are taken from Microsoft docs (table 1):
			// https://docs.microsoft.com/en-us/previous-versions/ms969909(v=msdn.10)
			uint32 penalty = 0;
			if (italic != fontItalic) {
				penalty += 4;
			}
			if (bold != fontBold) {
				penalty += 120;
			}
			if (penalty < bestPenalty) {
				// Better font
				// Cleanup old best font if it's not the same file as the current one
				if (bestTTFFile != ttfFile) {
					delete [] bestTTFFile;
				}

				bestPenalty = penalty;
				bestTTFFile = ttfFile;
				bestFaceId = i;
				bestSize = sizeFile;
			}
		}

		// Don't free the file if it has been elected the best
		if (bestTTFFile != ttfFile) {
			delete [] ttfFile;
		}
		ttfFile = nullptr;
	}

	if (!bestTTFFile) {
		return nullptr;
	}

	TTFFont *font = new TTFFont();

	TTFSizeMode sizeMode = kTTFSizeModeCell;
	if (size < 0) {
		size = -size;
		sizeMode = kTTFSizeModeCharacter;
	}
	if (dpi == 0) {
		dpi = 96;
	}

	if (!font->load(bestTTFFile, bestSize, bestFaceId, bold, italic, size, sizeMode,
	                dpi, renderMode, mapping, false)) {
		delete font;
		delete [] bestTTFFile;
		return nullptr;
	}

	return font;
}

} // End of namespace Graphics

namespace Common {
DECLARE_SINGLETON(Graphics::TTFLibrary);
} // End of namespace Common

#endif

