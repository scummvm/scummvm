/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef FONT_H
#define FONT_H

#include "common/str.h"
#include "graphics/surface.h"

namespace Graphics {

// Text alignment modes for drawString()
enum TextAlignment {
	kTextAlignLeft,
	kTextAlignCenter,
	kTextAlignRight
};

/**
 * Instances of this class represent a distinct font, with a built-in renderer.
 * @todo Maybe move the high-level methods (drawString etc.) to a separate 
 *       FontRenderer class? That way, we could have different variants... ?
 * @todo Add more parameters to drawString, or additional similar methods,
 *       featuring abilities like
 *       - rendering with wrap-around instead of inserting an ellipsis or
 *         cutting them; needs a 'height' parameter
 *       - rendering multi-line strings (essentially, invoke the regular
 *         drawString for each line, and advance one line)
 *       - combinations of the two above: honor line feeds, and also wrap
 *         overlong lines
 */
class Font {
public:
	virtual int getFontHeight() const = 0;
	virtual int getMaxCharWidth() const = 0;

	virtual int getCharWidth(byte chr) const = 0;
	virtual void drawChar(const Surface *dst, byte chr, int x, int y, uint32 color) const = 0;

	void drawString(const Surface *dst, const Common::String &str, int x, int y, int w, uint32 color, TextAlignment align = kTextAlignLeft, int deltax = 0, bool useEllipsis = true) const;
	int getStringWidth(const Common::String &str) const;
};


class ScummFont : public Font {
public:
	virtual int getFontHeight() const { return 8; }
	virtual int getMaxCharWidth() const { return 8; };

	virtual int getCharWidth(byte chr) const;
	virtual void drawChar(const Surface *dst, byte chr, int x, int y, uint32 color) const;
};

extern const ScummFont g_scummfont;



typedef unsigned short bitmap_t; /* bitmap image unit size*/

/* builtin C-based proportional/fixed font structure */
/* based on The Microwindows Project http://microwindows.org */
struct FontDesc {
	const char *	name;		/* font name*/
	int		maxwidth;	/* max width in pixels*/
	int 	height;		/* height in pixels*/
	int		ascent;		/* ascent (baseline) height*/
	int		firstchar;	/* first character in bitmap*/
	int		size;		/* font size in glyphs*/
	const bitmap_t*	bits;		/* 16-bit right-padded bitmap data*/
	const unsigned long* offset;	/* offsets into bitmap data*/
	const unsigned char* width;	/* character widths or NULL if fixed*/
	int		defaultchar;	/* default char (not glyph index)*/
	long	bits_size;	/* # words of bitmap_t bits*/
};

class NewFont : public Font {
protected:
	FontDesc desc;

public:
	NewFont(const FontDesc &d) : desc(d) {}
	
	virtual int getFontHeight() const { return desc.height; }
	virtual int getMaxCharWidth() const { return desc.maxwidth; };

	virtual int getCharWidth(byte chr) const;
	virtual void drawChar(const Surface *dst, byte chr, int x, int y, uint32 color) const;
};

extern const NewFont g_sysfont;

} // End of namespace Graphics

#endif
