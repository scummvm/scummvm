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

#ifndef GRAPHICS_FONTS_BDF_H
#define GRAPHICS_FONTS_BDF_H

#include "common/system.h"

#include "graphics/font.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {

typedef uint16 bitmap_t; /* bitmap image unit size*/

struct BBX {
	int8 w;
	int8 h;
	int8 x;
	int8 y;
};

/* builtin C-based proportional/fixed font structure */
/* based on The Microwindows Project http://microwindows.org */
struct BdfFontDesc {
	const char          *name;                  /* font name */
	int                 maxwidth;               /* max width in pixels */
	int                 height;                 /* height in pixels */
	int                 fbbw, fbbh, fbbx, fbby; /* max bounding box */
	int                 ascent;                 /* ascent (baseline) height */
	int                 firstchar;              /* first character in bitmap */
	int                 size;                   /* font size in glyphs */
	const bitmap_t      *bits;                  /* 16-bit right-padded bitmap data */
	const unsigned long *offset;                /* offsets into bitmap data */
	const unsigned char *width;                 /* character widths or NULL if fixed */
	const BBX           *bbx;                   /* character bounding box or NULL if fixed */
	int                 defaultchar;            /* default char (not glyph index) */
	long                bits_size;              /* # words of bitmap_t bits */
};

struct BdfFontData;

class BdfFont : public Font {
protected:
	BdfFontDesc _desc;
	BdfFontData *_font;

public:
	BdfFont(const BdfFontDesc &desc, BdfFontData *font = 0) : _desc(desc), _font(font) {}
	~BdfFont();

	virtual int getFontHeight() const;
	virtual int getMaxCharWidth() const;

	virtual int getCharWidth(byte chr) const;
	virtual void drawChar(Surface *dst, byte chr, int x, int y, uint32 color) const;

	static BdfFont *loadFont(Common::SeekableReadStream &stream);
	static bool cacheFontData(const BdfFont &font, const Common::String &filename);
	static BdfFont *loadFromCache(Common::SeekableReadStream &stream);
};

#define DEFINE_FONT(n) \
	const BdfFont *n = 0;   \
	void create_##n() { \
		n = new BdfFont(desc);  \
	}

#define FORWARD_DECLARE_FONT(n) \
	extern const BdfFont *n; \
	extern void create_##n()

#define INIT_FONT(n) \
	create_##n()

} // End of namespace Graphics

#endif

