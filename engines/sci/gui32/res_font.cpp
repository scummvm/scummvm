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

#include "sci/sci.h"	// for INCLUDE_OLDGFX
#ifdef INCLUDE_OLDGFX

#include "common/endian.h"
#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"
#include "sci/gui32/font.h"

namespace Sci {

extern int font_counter;

#define FONT_HEIGHT_OFFSET 4
#define FONT_MAXCHAR_OFFSET 2

static void calc_char(byte *dest, int total_width, int total_height, byte *src, int size) {
	int width = src[0];
	int height = src[1];
	int byte_width = (width + 7) >> 3;
	int y;

	src += 2;

	if ((width >> 3) > total_width || height > total_height) {
		error("Weird character: width=%d/%d, height=%d/%d", width, total_width, height, total_height);
	}

	if (byte_width * height + 2 > size) {
		error("Character extends to %d of %d allowed bytes", byte_width * height + 2, size);
	}

	for (y = 0; y < height; y++) {
		memcpy(dest, src, byte_width);
		src += byte_width;
		dest += total_width;
	}
}

gfx_bitmap_font_t *gfxr_read_font(int id, byte *resource, int size) {
	gfx_bitmap_font_t *font = (gfx_bitmap_font_t*)calloc(sizeof(gfx_bitmap_font_t), 1);
	int chars_nr;
	int max_width = 0, max_height;
	int i;

	++font_counter;

	if (size < 6) {
		error("Font %04x size is %d", id, size);
		gfxr_free_font(font);
		return NULL;
	}

	font->chars_nr = chars_nr = READ_LE_UINT16(resource + FONT_MAXCHAR_OFFSET);
	font->line_height = max_height = READ_LE_UINT16(resource + FONT_HEIGHT_OFFSET);

	if (chars_nr < 0 || chars_nr > 256 || max_height < 0) {
		if (chars_nr < 0 || chars_nr > 256)
			error("Font %04x: Invalid number of characters: %d", id, chars_nr);
		if (max_height < 0)
			error("Font %04x: Invalid font height: %d", id, max_height);
		gfxr_free_font(font);
		return NULL;
	}

	if (size < 6 + chars_nr * 2) {
		error("Font %04x: Insufficient space for %d characters in font", id, chars_nr);
		gfxr_free_font(font);
		return NULL;
	}

	font->ID = id;
	font->widths = (int*)malloc(sizeof(int) * chars_nr);

	for (i = 0; i < chars_nr; i++) {
		int offset = READ_LE_UINT16(resource + (i << 1) + 6);

		if (offset >= size) {
			error("Font %04x: Error: Character 0x%02x is at offset 0x%04x (beyond 0x%04x)", id, i, offset, size);
			gfxr_free_font(font);
			return NULL;
		}

		if ((resource[offset]) > max_width)
			max_width = resource[offset];
		if ((resource[offset + 1]) > max_height)
			max_height = resource[offset + 1];

		font->widths[i] = resource[offset];
	}

	font->height = max_height;
	font->row_size = (max_width + 7) >> 3;

	if (font->row_size == 3)
		font->row_size = 4;

	if (font->row_size > 4)
		font->row_size = (font->row_size + 3) & ~3;

	font->char_size = font->row_size * max_height;
	font->data = (byte *)calloc(font->char_size, chars_nr);

	for (i = 0; i < chars_nr; i++) {
		int offset = READ_LE_UINT16(resource + (i << 1) + 6);

		calc_char(font->data + (font->char_size * i), font->row_size, max_height, resource + offset, size - offset);
	}

	return font;
}

} // End of namespace Sci

#endif
