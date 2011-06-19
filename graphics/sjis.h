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
 */

// The code in this file is currently only used in KYRA and SCI.
// So if neither of those is enabled, we will skip compiling it.
// We also enable this code for ScummVM builds including support
// for dynamic engine plugins.
// If you plan to use this code in another engine, you will have
// to add the proper define check here.
#if !(defined(ENABLE_KYRA) || defined(ENABLE_SCI) || defined(ENABLE_SCUMM) || defined(DYNAMIC_MODULES))

// If neither of the above mentioned is enabled, do not include the SJIS code.

#else

#ifndef GRAPHICS_SJIS_H
#define GRAPHICS_SJIS_H

#ifdef __DS__
/* This disables the flipped mode which is used in FM-Towns versions
 * of Monkey Island 1 (and maybe other SCUMM 5 games). These are not supported
 * on the DS, so it makes sense to have a corresponding setting here.
 */
#define DISABLE_FLIPPED_MODE
#endif

#include "common/scummsys.h"
#include "common/util.h"

namespace Graphics {

struct Surface;

/**
 * A font that is able to draw SJIS encoded characters.
 */
class FontSJIS {
public:
	virtual ~FontSJIS() {}

	/**
	 * Creates the first SJIS font, which ROM/font file is present.
	 * It will also call loadData, so the user can just start
	 * using the font.
	 *
	 * It'll prefer the platform specific ROM file, when platform
	 * is set to a value, which's font ROM is supported.
	 * So far that is only kPlatformFMTowns.
	 *
	 * The last file tried is ScummVM's SJIS.FNT file.
	 */
	static FontSJIS *createFont(const Common::Platform platform = Common::kPlatformUnknown);

	/**
	 * Load the font data.
	 */
	virtual bool loadData() = 0;

	/**
	 * Enable drawing with outline or shadow.
	 *
	 * After changing outline state, getFontHeight and getMaxFontWidth / getCharWidth might return
	 * different values!
	 */
	enum DrawingMode {
		kDefaultMode,
		kOutlineMode,
		kShadowMode,
		kFMTownsShadowMode
	};

	virtual void setDrawingMode(DrawingMode mode) {}

	/**
	 * Enable flipped character drawing (e.g. in the MI1 circus scene after Guybrush gets shot out of the cannon).
	 */
	virtual void toggleFlippedMode(bool enable) {}

	/**
	 * Returns the height of the font.
	 */
	virtual uint getFontHeight() const = 0;

	/**
	 * Returns the max. width of the font.
	 */
	virtual uint getMaxFontWidth() const = 0;

	/**
	 * Returns the width of a specific character.
	 */
	virtual uint getCharWidth(uint16 ch) const = 0;

	/**
	 * Draws a SJIS encoded character on the given surface.
	 */
	void drawChar(Graphics::Surface &dst, uint16 ch, int x, int y, uint32 c1, uint32 c2) const;

	/**
	 * Draws a SJIS char on the given raw buffer.
	 *
	 * @param dst   pointer to the destination
	 * @param ch    character to draw (in little endian)
	 * @param pitch pitch of the destination buffer (size in *bytes*)
	 * @param bpp   bytes per pixel of the destination buffer
	 * @param c1    forground color
	 * @param c2    outline color
	 * @param maxW  max draw width (to ensure that character drawing takes place within surface boundaries)
	 * @param maxH  max draw height (to ensure that character drawing takes place within surface boundaries)
	 */
	virtual void drawChar(void *dst, uint16 ch, int pitch, int bpp, uint32 c1, uint32 c2, int maxW = -1, int maxH = -1) const = 0;
};

/**
 * A base class to render 16x16 (2 byte chars), 8x16 (1 byte chars) monochrome SJIS fonts.
 */
class FontSJISBase : public FontSJIS {
public:
	FontSJISBase() : _drawMode(kDefaultMode), _flippedMode(false) {}

	void setDrawingMode(DrawingMode mode) { _drawMode = mode; }

	void toggleFlippedMode(bool enable) { _flippedMode = enable; }

	uint getFontHeight() const { return (_drawMode == kOutlineMode) ? 18 : (_drawMode == kDefaultMode ? 16 : 17); }

	uint getMaxFontWidth() const { return (_drawMode == kOutlineMode) ? 18 : (_drawMode == kDefaultMode ? 16 : 17); }

	uint getCharWidth(uint16 ch) const;

	void drawChar(void *dst, uint16 ch, int pitch, int bpp, uint32 c1, uint32 c2, int maxW = -1, int maxH = -1) const;
private:
	template<typename Color>
	void blitCharacter(const uint8 *glyph, const int w, const int h, uint8 *dst, int pitch, Color c) const;
	void createOutline(uint8 *outline, const uint8 *glyph, const int w, const int h) const;

#ifndef DISABLE_FLIPPED_MODE
	// This is used in the FM-Towns version of Monkey Island 1
	// when Guybrush gets shot out of the cannon in the circus tent.
	const uint8 *flipCharacter(const uint8 *glyph, const int w) const;
	mutable uint8 _tempGlyph[32];
#endif
protected:
	DrawingMode _drawMode;
	bool _flippedMode;

	bool is8x16(uint16 ch) const;

	virtual const uint8 *getCharData(uint16 c) const = 0;
	virtual const uint8 *getCharData8x16(uint16 c) const = 0;
};

/**
 * FM-TOWNS ROM based SJIS compatible font.
 *
 * This is used in KYRA and SCI.
 */
class FontTowns : public FontSJISBase {
public:
	/**
	 * Loads the ROM data from "FMT_FNT.ROM".
	 */
	bool loadData();
private:
	enum {
		kFont16x16Chars = 7808,
		kFont8x16Chars = 256
	};

	uint8 _fontData16x16[kFont16x16Chars * 32];
	uint8 _fontData8x16[kFont8x16Chars * 32];

	const uint8 *getCharData(uint16 c) const;
	const uint8 *getCharData8x16(uint16 c) const;
};

/**
 * Our custom SJIS FNT.
 */
class FontSjisSVM : public FontSJISBase {
public:
	FontSjisSVM() : _fontData16x16(0), _fontData16x16Size(0), _fontData8x16(0), _fontData8x16Size(0) {}
	~FontSjisSVM() { delete[] _fontData16x16; delete[] _fontData8x16; }

	/**
	 * Load the font data from "SJIS.FNT".
	 */
	bool loadData();
private:
	uint8 *_fontData16x16;
	uint _fontData16x16Size;

	uint8 *_fontData8x16;
	uint _fontData8x16Size;

	const uint8 *getCharData(uint16 c) const;
	const uint8 *getCharData8x16(uint16 c) const;
};

// TODO: Consider adding support for PC98 ROM

} // End of namespace Graphics

#endif

#endif // engine and dynamic plugins guard

