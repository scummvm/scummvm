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
#include "sci/gfx/font.h"

namespace Sci {

int font_counter = 0;

void gfxr_free_font(gfx_bitmap_font_t *font) {
	free(font->widths);
	free(font->data);

	--font_counter;

	free(font);
}

text_fragment_t *gfxr_font_calculate_size(gfx_bitmap_font_t *font, int max_width,
	const char *text, int *width, int *height,
	int *lines, int *line_height_p, int *last_offset_p, int flags) {

	int est_char_width = font->widths[(font->chars_nr > 'M')? 'M' : font->chars_nr - 1];
	// 'M' is typically among the widest chars
	int fragments_nr;
	text_fragment_t *fragments;
	int lineheight = font->line_height;
	int maxheight = lineheight;
	int last_breakpoint = 0;
	int last_break_width = 0;
	int max_allowed_width = max_width;
	int maxwidth = 0, localmaxwidth = 0;
	int current_fragment = 1;
	const char *breakpoint_ptr = NULL;
	unsigned char foo;

	if (line_height_p)
		*line_height_p = lineheight;

	if (max_width > 1) fragments_nr = 3 + (strlen(text) * est_char_width) * 3 / (max_width << 1);
	else fragments_nr = 1;

	fragments = (text_fragment_t *)sci_calloc(sizeof(text_fragment_t), fragments_nr);

	fragments[0].offset = text;

	while ((foo = *text++)) {
		if (foo >= font->chars_nr) {
			GFXWARN("Invalid char 0x%02x (max. 0x%02x) encountered in text string '%s', font %04x\n",
			        foo, font->chars_nr, text, font->ID);
			if (font->chars_nr > ' ')
				foo = ' ';
			else {
				free(fragments);
				return NULL;
			}
		}

		if (((foo == '\n') || (foo == 0x0d)) && !(flags & GFXR_FONT_FLAG_NO_NEWLINES)) {
			fragments[current_fragment-1].length = text - 1 - fragments[current_fragment-1].offset;

			if (*text)
				maxheight += lineheight;

			if (foo == 0x0d && *text == '\n')
				text++; // Interpret DOS-style CR LF as single NL

			fragments[current_fragment++].offset = text;

			if (localmaxwidth > maxwidth)
				maxwidth = localmaxwidth;

			if (current_fragment == fragments_nr)
				fragments = (text_fragment_t*)sci_realloc(fragments, sizeof(text_fragment_t) * (fragments_nr <<= 1));

			localmaxwidth = 0;

		} else { // foo != '\n'
			localmaxwidth += font->widths[foo];

			if (localmaxwidth > max_allowed_width) {
				int blank_break = 1; // break is at a blank char, i.e. not within a word

				maxheight += lineheight;

				if (last_breakpoint == 0) { // Text block too long and without whitespace?
					last_breakpoint = localmaxwidth - font->widths[foo];
					last_break_width = 0;
					--text;
					blank_break = 0; // non-blank break
				} else {
					text = breakpoint_ptr + 1;
					assert(breakpoint_ptr);
				}

				if (last_breakpoint == 0) {
					GFXWARN("Warning: maxsize %d too small for '%s'\n", max_allowed_width, text);
				}

				if (last_breakpoint > maxwidth)
					maxwidth = last_breakpoint;

				fragments[current_fragment-1].length = text - blank_break - fragments[current_fragment-1].offset;
				fragments[current_fragment++].offset = text;

				if (current_fragment == fragments_nr)
					fragments = (text_fragment_t*)sci_realloc(fragments, sizeof(text_fragment_t *) * (fragments_nr <<= 1));

				localmaxwidth = localmaxwidth - last_breakpoint;
				if (!(flags & GFXR_FONT_FLAG_COUNT_WHITESPACE))
					localmaxwidth -= last_break_width;
				last_breakpoint = localmaxwidth = 0;

			} else if (*text == ' ') {
				last_breakpoint = localmaxwidth;
				last_break_width = font->widths[foo];
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
	if (lines)
		*lines = current_fragment;

	fragments[current_fragment-1].length = text - fragments[current_fragment-1].offset - 1;

	return fragments;
}

static inline void render_char(byte *dest, byte *src, int width, int line_width, int lines, int bytes_per_src_line, int fg0, int fg1, int bg) {
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

gfx_pixmap_t *gfxr_draw_font(gfx_bitmap_font_t *font, const char *stext, int characters,
	gfx_pixmap_color_t *fg0, gfx_pixmap_color_t *fg1, gfx_pixmap_color_t *bg) {
	unsigned char *text = (unsigned char *)stext;
	int height = font->height;
	int width = 0;
	gfx_pixmap_t *pxm;
	int fore_0, fore_1, back;
	int i;
	int hack = 0;
	gfx_pixmap_color_t dummy = {0, 0, 0, 0};
	byte *offset;

	for (i = 0; i < characters; i++) {
		int ch = (int) text[i];

		if (ch >= font->chars_nr) {
			GFXERROR("Invalid character 0x%02x encountered!\n", text[i]);
			return NULL;
		}

		width += font->widths[ch];
	}

	pxm = gfx_pixmap_alloc_index_data(gfx_new_pixmap(width, height, GFX_RESID_NONE, 0, 0));

	pxm->colors_nr = !!fg0 + !!fg1 + !!bg;
	if (pxm->colors_nr == 0) {
		GFXWARN("Pixmap would have zero colors, resetting!\n");
		pxm->colors_nr = 3;
		hack = 1;
		fg0 = fg1 = bg = &dummy;
	}
	pxm->colors = (gfx_pixmap_color_t *)sci_malloc(sizeof(gfx_pixmap_color_t) * pxm->colors_nr);
#ifdef SATISFY_PURIFY
	memset(pxm->colors, 0, sizeof(gfx_pixmap_color_t) * pxm->colors_nr);
#endif
	pxm->flags |= GFX_PIXMAP_FLAG_PALETTE_ALLOCATED | GFX_PIXMAP_FLAG_DONT_UNALLOCATE_PALETTE;

	i = 0;

	if (fg0 || hack) {
		memcpy(pxm->colors + i, fg0, sizeof(gfx_pixmap_color_t));
		fore_0 = i++;
	} else
		fore_0 = pxm->color_key;

	if (fg1 || hack) {
		memcpy(pxm->colors + i, fg1, sizeof(gfx_pixmap_color_t));
		fore_1 = i++;
	} else
		fore_1 = pxm->color_key;

	if (bg || hack) {
		memcpy(pxm->colors + i, bg, sizeof(gfx_pixmap_color_t));
		back = i++;
	} else
		back = pxm->color_key;

	offset = pxm->index_data;

	memset(pxm->index_data, back, pxm->index_xl * pxm->index_yl);
	for (i = 0; i < characters; i++) {
		unsigned char ch = text[i];
		width = font->widths[ch];

		render_char(offset, font->data + (ch * font->char_size), width,
		            pxm->index_xl, pxm->index_yl, font->row_size, fore_0, fore_1, back);

		offset += width;
	}

	return pxm;
}

} // End of namespace Sci
