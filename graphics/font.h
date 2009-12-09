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

#ifndef GRAPHICS_FONT_H
#define GRAPHICS_FONT_H

#include "common/str.h"
#include "graphics/surface.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {

/** Text alignment modes */
enum TextAlign {
	kTextAlignInvalid,
	kTextAlignLeft,		///< Text should be aligned to the left
	kTextAlignCenter,	///< Text should be centered
	kTextAlignRight		///< Text should be aligned to the right
};

/**
 * Instances of this class represent a distinct font, with a built-in renderer.
 * @todo Maybe move the high-level methods (drawString etc.) to a separate
 *       FontRenderer class? That way, we could have different variants... ?
 */
class Font {
public:
	Font() {}
	virtual ~Font() {}

	virtual int getFontHeight() const = 0;
	virtual int getMaxCharWidth() const = 0;

	virtual int getCharWidth(byte chr) const = 0;
	virtual void drawChar(Surface *dst, byte chr, int x, int y, uint32 color) const = 0;

	void drawString(Surface *dst, const Common::String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = true) const;

	/**
	 * Compute and return the width the string str has when rendered using this font.
	 */
	int getStringWidth(const Common::String &str) const;

	/**
	 * Take a text (which may contain newlines characters) and word wrap it so thata
	 * no text line is wider than maxWidth pixels. If necessary, additional line breaks
	 * are generated, preferably between words (i.e. were whitespaces are).
	 * The resulting lines are appended to the string list lines.
	 * It returns the maximal width of any of the new lines (i.e. a value which is less
	 * or equal to maxWidth).
	 *
	 * @param str	the string to word wrap
	 * @param maxWidth	the maximum width a line may have
	 * @param lines	the string list to which the text lines from str are appended
	 * @return the maximal width of any of the lines added to lines
	 */
	int wordWrapText(const Common::String &str, int maxWidth, Common::StringList &lines) const;
};


class ScummFont : public Font {
public:
	virtual int getFontHeight() const { return 8; }
	virtual int getMaxCharWidth() const { return 8; }

	virtual int getCharWidth(byte chr) const;
	virtual void drawChar(Surface *dst, byte chr, int x, int y, uint32 color) const;
};

typedef uint16 bitmap_t; /* bitmap image unit size*/

struct BBX {
	int8 w;
	int8 h;
	int8 x;
	int8 y;
};

/* builtin C-based proportional/fixed font structure */
/* based on The Microwindows Project http://microwindows.org */
struct FontDesc {
	const char *	name;		/* font name*/
	int		maxwidth;	/* max width in pixels*/
	int		height;		/* height in pixels*/
	int	fbbw, fbbh, fbbx, fbby;	/* max bounding box */
	int		ascent;		/* ascent (baseline) height*/
	int		firstchar;	/* first character in bitmap*/
	int		size;		/* font size in glyphs*/
	const bitmap_t*	bits;		/* 16-bit right-padded bitmap data*/
	const unsigned long* offset;	/* offsets into bitmap data*/
	const unsigned char* width;	/* character widths or NULL if fixed*/
	const BBX* bbx;			/* character bounding box or NULL if fixed */
	int		defaultchar;	/* default char (not glyph index)*/
	long	bits_size;	/* # words of bitmap_t bits*/
};

struct NewFontData;

class NewFont : public Font {
protected:
	FontDesc desc;
	NewFontData *font;

public:
	NewFont(const FontDesc &d, NewFontData *font_ = 0) : desc(d), font(font_) {}
	~NewFont();

	virtual int getFontHeight() const { return desc.height; }
	virtual int getMaxCharWidth() const { return desc.maxwidth; }

	virtual int getCharWidth(byte chr) const;
	virtual void drawChar(Surface *dst, byte chr, int x, int y, uint32 color) const;

	static NewFont *loadFont(Common::SeekableReadStream &stream);
	static bool cacheFontData(const NewFont &font, const Common::String &filename);
	static NewFont *loadFromCache(Common::SeekableReadStream &stream);
};

#define DEFINE_FONT(n) \
		const NewFont *n;	\
		void create_##n() {	\
			n = new NewFont(desc);	\
		}

#define INIT_FONT(n) \
		extern void create_##n(); \
		create_##n();

} // End of namespace Graphics

#endif
