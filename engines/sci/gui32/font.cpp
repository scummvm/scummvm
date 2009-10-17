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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"
#include "sci/gui32/font.h"

namespace Sci {

int font_counter = 0;

void gfxr_free_font(gfx_bitmap_font_t *font) {
	free(font->widths);
	free(font->data);

	--font_counter;

	free(font);
}

bool gfxr_font_calculate_size(Common::Array<TextFragment> &fragments, gfx_bitmap_font_t *font, int max_width,
	const char *text, int *width, int *height, int *line_height_p, int *last_offset_p, int flags) {

	int maxheight = font->line_height;
	int last_breakpoint = 0;
	int last_break_width = 0;
	int maxwidth = 0, localmaxwidth = 0;
	const char *breakpoint_ptr = NULL;
	unsigned char curChar;

	if (line_height_p)
		*line_height_p = font->line_height;


	fragments.push_back(TextFragment(text));

	while ((curChar = *text++)) {
		if (curChar >= font->chars_nr) {
			error("Invalid char 0x%02x (max. 0x%02x) encountered in text string '%s', font %04x",
			        curChar, font->chars_nr, text, font->ID);
			if (font->chars_nr > ' ')
				curChar = ' ';
			else {
				return false;
			}
		}

		if (((curChar == '\n') || (curChar == 0x0d)) && !(flags & kFontNoNewlines)) {
			fragments.back().length = text - 1 - fragments.back().offset;

			if (*text)
				maxheight += font->line_height;

			if (curChar == 0x0d && *text == '\n')
				text++; // Interpret DOS-style CR LF as single NL

			fragments.push_back(TextFragment(text));

			if (localmaxwidth > maxwidth)
				maxwidth = localmaxwidth;

			localmaxwidth = 0;

		} else { // curChar != '\n'
			localmaxwidth += font->widths[curChar];

			if (localmaxwidth > max_width) {
				int blank_break = 1; // break is at a blank char, i.e. not within a word

				maxheight += font->line_height;

				if (last_breakpoint == 0) { // Text block too long and without whitespace?
					last_breakpoint = localmaxwidth - font->widths[curChar];
					last_break_width = 0;
					--text;
					blank_break = 0; // non-blank break
				} else {
					text = breakpoint_ptr + 1;
					assert(breakpoint_ptr);
				}

				if (last_breakpoint == 0) {
					warning("[GFX] maxsize %d too small for '%s'", max_width, text);
				}

				if (last_breakpoint > maxwidth)
					maxwidth = last_breakpoint;

				fragments.back().length = text - blank_break - fragments.back().offset;
				fragments.push_back(TextFragment(text));

				localmaxwidth = localmaxwidth - last_breakpoint;
				if (!(flags & kFontCountWhitespace))
					localmaxwidth -= last_break_width;
				last_breakpoint = localmaxwidth = 0;

			} else if (*text == ' ') {
				last_breakpoint = localmaxwidth;
				last_break_width = font->widths[curChar];
				breakpoint_ptr = text;
			}

		}
	}

	if (localmaxwidth > maxwidth)
		*width = localmaxwidth;
	else
		*width = maxwidth;

	if (last_offset_p)
		*last_offset_p = localmaxwidth;

	if (height)
		*height = maxheight;

	fragments.back().length = text - fragments.back().offset - 1;

	return true;
}

static void render_char(byte *dest, byte *src, int width, int line_width, int lines, int bytes_per_src_line, int fg0, int fg1, int bg) {
	int x, y;

	for (y = 0; y < lines; y++) {
		int dat = 0;
		byte *vdest = dest;
		byte *vsrc = src;
		int xc = 0;

		for (x = 0; x < width; x++) {
			if (!xc) {
				dat = *vsrc++;
				xc = 8;
			}
			xc--;

			if (dat & 0x80)
				*vdest++ = ((xc ^ y) & 1) ? fg0 : fg1; /* dither */
			else
				*vdest++ = bg;

			dat <<= 1;
		}
		src += bytes_per_src_line;
		dest += line_width;
	}
}

gfx_pixmap_t *gfxr_draw_font(gfx_bitmap_font_t *font, const char *stext, int characters, PaletteEntry *fg0, PaletteEntry *fg1, PaletteEntry *bg) {
	const byte *text = (const byte *)stext;
	int height = font->height;
	int width = 0;
	gfx_pixmap_t *pxm;
	int fore_0, fore_1, back;
	int i;
	int hack = 0;
	PaletteEntry dummy(0,0,0); // black
	byte *offset;

	for (i = 0; i < characters; i++) {
		int ch = (int) text[i];

		if (ch >= font->chars_nr) {
			error("Invalid character 0x%02x encountered", text[i]);
			return NULL;
		}

		width += font->widths[ch];
	}

	pxm = gfx_pixmap_alloc_index_data(gfx_new_pixmap(width, height, GFX_RESID_NONE, 0, 0));

	int colors_nr = !!fg0 + !!fg1 + !!bg;
	if (colors_nr == 0) {
		warning("[GFX] Pixmap would have zero colors, resetting");
		colors_nr = 3;
		hack = 1;
		fg0 = fg1 = bg = &dummy;
	}
	pxm->palette = new Palette(colors_nr);
	pxm->palette->name = "font";

	i = 0;

	if (fg0 || hack) {
		pxm->palette->setColor(i, fg0->r, fg0->g, fg0->b);
		fore_0 = i++;
	} else
		fore_0 = pxm->color_key;

	if (fg1 || hack) {
		pxm->palette->setColor(i, fg1->r, fg1->g, fg1->b);
		fore_1 = i++;
	} else
		fore_1 = pxm->color_key;

	if (bg || hack) {
		pxm->palette->setColor(i, bg->r, bg->g, bg->b);
		back = i++;
	} else
		back = pxm->color_key;

	offset = pxm->index_data;

	memset(pxm->index_data, back, pxm->index_width * pxm->index_height);
	for (i = 0; i < characters; i++) {
		unsigned char ch = text[i];
		width = font->widths[ch];

		render_char(offset, font->data + (ch * font->char_size), width,
		            pxm->index_width, pxm->index_height, font->row_size, fore_0, fore_1, back);

		offset += width;
	}

	return pxm;
}

} // End of namespace Sci
