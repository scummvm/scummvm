/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

namespace GUI {

typedef unsigned short bitmap_t; /* bitmap image unit size*/

/* builtin C-based proportional/fixed font structure */
/* based on The Microwindows Project http://microwindows.org */
struct Font {
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

extern const Font g_sysfont;

} // End of namespace GUI

#endif
