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
 * $URL$
 * $Id$
 */

// The code in this file is currently only used in KYRA and SCI.
// So if neither of those is enabled, we will skip compiling it.
// If you plan to use this code in another engine, you will have
// to add the proper define check here.
// Also please add the define check at the comment after the
// matching #endif further down this file.
#if defined(ENABLE_KYRA) || defined(ENABLE_SCI)

#ifndef GRAPHICS_SJIS_H
#define GRAPHICS_SJIS_H

#include "common/scummsys.h"
#include "common/stream.h"

#include "graphics/surface.h"

namespace Graphics {

/**
 * A font that is able to draw SJIS encoded characters.
 *
 * The font is always monospaced.
 */
class FontSJIS {
public:
	virtual ~FontSJIS() {}

	/**
	 * Creates the first SJIS font, which ROM/font file is present.
	 * It will also call loadData, so the user can just start
	 * using the font.
	 *
	 * It will prefer ScummVM's font.
	 *
	 * TODO: Consider adding some way to overwrite the first checked
	 * font, some games might want to prefer the original ROM over
	 * ScummVM's.
	 */
	static FontSJIS *createFont();

	/**
	 * Load the font data.
	 */
	virtual bool loadData() = 0;

	/**
	 * Enable outline drawing.
	 *
	 * After changing outline state, getFontHeight and getFontWidth might return
	 * different values!
	 */
	virtual void enableOutline(bool enable) {}

	/**
	 * Returns the height of the font.
	 */
	virtual uint getFontHeight() const = 0;

	/**
	 * Returns the width of the font.
	 */
	virtual uint getFontWidth() const = 0;

	/**
	 * Draws a SJIS encoded character on the given surface.
	 */
	void drawChar(Graphics::Surface &dst, uint16 ch, int x, int y, uint32 c1, uint32 c2) const {
		drawChar(dst.getBasePtr(x, y), ch, c1, c2, dst.pitch, dst.bytesPerPixel);
	}

	/**
	 * Draws a SJIS char on the given raw buffer.
	 *
	 * @param dst	pointer to the destination
	 * @param ch	character to draw (in little endian)
	 * @param pitch	pitch of the destination buffer (size in *bytes*)
	 * @param bpp	bytes per pixel of the destination buffer
	 * @param c1	forground color
	 * @param c2	outline color
	 */
	virtual void drawChar(void *dst, uint16 ch, int pitch, int bpp, uint32 c1, uint32 c2) const = 0;
};

/**
 * A base class to render 16x16 monochrome SJIS fonts.
 */
class FontSJIS16x16 : public FontSJIS {
public:
	FontSJIS16x16() : _outlineEnabled(false) {}

	void enableOutline(bool enable) { _outlineEnabled = enable; }

	uint getFontHeight() const { return _outlineEnabled ? 18 : 16; }
	uint getFontWidth() const { return _outlineEnabled ? 18 : 16; }

	void drawChar(void *dst, uint16 ch, int pitch, int bpp, uint32 c1, uint32 c2) const;

private:
	template<typename Color>
	void drawCharInternOutline(const uint16 *glyph, uint8 *dst, int pitch, Color c1, Color c2) const;

	template<typename Color>
	void drawCharIntern(const uint16 *glyph, uint8 *dst, int pitch, Color c1) const;

	bool _outlineEnabled;
protected:

	virtual const uint16 *getCharData(uint16 c) const = 0;
};

/**
 * FM-TOWNS ROM based SJIS compatible font.
 *
 * This is used in KYRA and SCI.
 */
class FontTowns : public FontSJIS16x16 {
public:
	/**
	 * Loads the ROM data from "FMT_FNT.ROM".
	 */
	bool loadData();

private:
	enum {
		kFontRomSize = 262144
	};

	uint16 _fontData[kFontRomSize / 2];

	const uint16 *getCharData(uint16 c) const;
};

/**
 * Our custom SJIS FNT.
 */
class FontSjisSVM : public FontSJIS16x16 {
public:
	FontSjisSVM() : _fontData(0) {}
	~FontSjisSVM() { delete[] _fontData; }

	/**
	 * Load the font data from "SJIS.FNT".
	 */
	bool loadData();

private:
	uint16 *_fontData;

	const uint16 *getCharData(uint16 c) const;
};

// TODO: Consider adding support for PC98 ROM

} // end of namespace Graphics

#endif

#endif // defined(ENABLE_KYRA) || defined(ENABLE_SCI)

