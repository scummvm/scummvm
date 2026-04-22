/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/algorithm.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/loader.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/color.h"

namespace MADS {
namespace MADSV2 {

constexpr int OFFSETS_OFFSET = 2 + FONT_SIZE;
constexpr int HEADER_SIZE = 2 + FONT_SIZE + (FONT_SIZE * 2);

FontPtr font_inter = NULL;
FontPtr font_main = NULL;              /* Interface & main font handles */
FontPtr font_conv = NULL;
FontPtr font_menu = NULL;
FontPtr font_misc = NULL;

byte font_colors[4] = { COLOR_TRANSPARENT, 15, 7, 8 };


FontPtr font_load(const char *name) {
	char temp_buf_2[80];
	char *mark;
	char block_name[20];
	long size, dataSize;
	FontPtr new_font = NULL;
	FontPtr result = NULL;
	Load load_handle;
	byte *buffer;
	int i;

	mem_last_alloc_loader = MODULE_FONT_LOADER;

	load_handle.open = false;

	Common::strcpy_s(temp_buf_2, name);
	mark = strchr(temp_buf_2, '.');
	if (mark == NULL) {
		Common::strcat_s(temp_buf_2, ".FF");
	}

	mark = temp_buf_2;
	if (*mark == '*') mark++;
	strncpy(block_name, mark, 8);
	block_name[8] = '\0';

	if (loader_open(&load_handle, temp_buf_2, "rb", true)) goto done;

	size = load_handle.pack.strategy[0].size;
	dataSize = size - HEADER_SIZE;	// Size for just the pixel data section

	new_font = (FontPtr)mem_get_name(sizeof(FontBuf) + dataSize, block_name);
	if (new_font == NULL)
		goto done;

	// Get the font resource into a temporary buffer
	buffer = (byte *)malloc(size);
	if (!buffer || !loader_read(buffer, size, 1, &load_handle)) {
		free(buffer);
		goto done;
	}

	// Copy data from the temporary buffer into the font
	new_font->max_y_size = buffer[0];
	new_font->max_x_size = buffer[1];
	Common::copy(buffer + 2, buffer + OFFSETS_OFFSET, &new_font->width[0]);
	Common::copy(buffer + HEADER_SIZE, buffer + size, (byte *)new_font + sizeof(FontBuf));

	// Set up the individual pointers to the start of each character's data
	for (i = 0; i < FONT_SIZE; ++i) {
		int offset = READ_LE_UINT16(buffer + OFFSETS_OFFSET + i * 2);
		new_font->data[i] = (byte *)new_font + sizeof(FontBuf) + (offset - HEADER_SIZE);
	}

	// Free the buffer
	free(buffer);

	result = new_font;

done:
	if (new_font != NULL) {
		if (result == NULL)
			mem_free(new_font);
	}
	if (load_handle.open)
		loader_close(&load_handle);

	return result;
}

int font_write(FontPtr font, Buffer *target, const char *out_string,
		int x, int y, int auto_spacing) {
	int target_wrap;
	int screen_loc;
	int skip_top = 0;
	int skip_bottom = 0;
	int bottom;
	int return_value = 0;
	char temp_buf[80];
	byte colors[4];
	char height;
	byte *target_ptr;

	*(uint32 *)&colors[0] = *(uint32 *)&font_colors[0];

	Common::strcpy_s(temp_buf, out_string);

	if (y < 0) {
		skip_top = 0 - y;
		y = 0;
	}

	height = (byte)MAX(0, (int)font->max_y_size - skip_top);

	bottom = y + height - 1;
	if (bottom > (target->y - 1)) {
		skip_bottom = MIN((int)height, (bottom - (target->y - 1)));
		height -= skip_bottom;
	}

	if (height <= 0) goto finish;

	target_ptr = (byte *)buffer_pointer(target, x, y);
	target_wrap = target->x;
	screen_loc = x;

	for (char *str = temp_buf; *str != '\0'; str++) {
		// char_loop: decrement character for pointer (original did dec dl / jns),
		// so '\0' (0) decrements to -1 (negative) and exits. Characters are
		// 1-based indices into the font table.
		byte ch_idx = (byte)(*str - 1);

		byte char_width = font->width[ch_idx];

		if (char_width == 0)
			continue;  // char_next with zero width: no spacing applied

		screen_loc += char_width;
		if (screen_loc >= target_wrap)
			break;  // Terminate

		// Locate the character's pixel data in the font, skipping clipped rows.
		// Each row of a character is packed 4 pixels per byte, so row stride
		// is ceil(char_width / 4) = (char_width - 1) / 4 + 1 = (char_width + 3) >> 2
		byte *glyph = font->data[ch_idx];

		if (skip_top > 0) {
			int row_stride = ((char_width - 1) >> 2) + 1;
			glyph += skip_top * row_stride;
		}

		// Draw the character glyph
		byte *row_ptr = target_ptr;

		for (int row = 0; row < height; row++) {
			byte *pixel_ptr = row_ptr;
			int   pixels_left = char_width;
			byte  data = *glyph++;
			int   pack_count = 4;  // 4 pixels packed per byte, 2 bits each

			while (pixels_left > 0) {
				// Shift next 2-bit color index into the high bits of AX,
				// then read into the low byte. Original: xor ah,ah / shl ax,2
				// with AL holding the packed byte - shifts top 2 bits of AL
				// into AH as the color index (0-3).
				byte color_idx = (data >> 6) & 0x03;
				data <<= 2;

				byte color = colors[color_idx];
				if (color != COLOR_TRANSPARENT)
					*pixel_ptr = color;

				pixel_ptr++;
				pixels_left--;

				if (pixels_left == 0)
					break;

				pack_count--;
				if (pack_count == 0) {
					data = *glyph++;
					pack_count = 4;
				}
			}

			row_ptr += target_wrap;
		}

		// char_next: advance target pointer by character width + autospacing
		target_ptr += char_width + auto_spacing;
		screen_loc += auto_spacing;
	}

	// Return value is the final X coordinate reached
	return_value = (int)(target_ptr - buffer_pointer(target, x, y)) + x;

finish:
	return return_value;
}

void font_set_colors(int zero, int one, int two, int three) {
	font_colors[0] = (byte)zero;
	font_colors[1] = (byte)one;
	font_colors[2] = (byte)two;
	font_colors[3] = (byte)three;
}

int font_string_width(FontPtr font, const char *out_string, int auto_spacing) {
	int width;
	int more_width;
	int idx;

	width = 0;

	for (; *out_string != 0;) {
		idx = *(out_string++) - 1;
		more_width = font->width[idx];
		if ((more_width > 0) && (*out_string)) more_width += auto_spacing;
		width += more_width;
	}

	return (width);
}

} // namespace MADSV2
} // namespace MADS
