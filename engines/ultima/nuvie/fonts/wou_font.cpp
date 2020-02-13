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
 */

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/screen/screen.h"
#include "ultima/nuvie/files/u6_lzw.h"
#include "ultima/nuvie/files/u6_shape.h"
#include "ultima/nuvie/fonts/wou_font.h"
#include "ultima/nuvie/core/game.h"

namespace Ultima {
namespace Nuvie {

WOUFont::WOUFont() {
	font_data = NULL;
	char_buf = NULL;
	num_chars = 0;
	offset = 0;
	height = 0;
	pixel_char = 0;
	default_color = FONT_COLOR_U6_NORMAL;
	default_highlight_color = FONT_COLOR_U6_HIGHLIGHT;
}

WOUFont::~WOUFont() {
	if (font_data != NULL)
		free(font_data);

	if (char_buf != NULL)
		free(char_buf);
}

bool WOUFont::init(const char *filename) {

	U6Lzw lzw;
	uint32 decomp_size;
	font_data = lzw.decompress_file(filename, decomp_size);

	height = font_data[0];
	pixel_char = font_data[2];

	num_chars = 256;
	if (Game::get_game()->get_game_type() != NUVIE_GAME_U6) {
		default_color = FONT_COLOR_WOU_NORMAL;
		default_highlight_color = FONT_COLOR_WOU_HIGHLIGHT;
	}
	return initCharBuf();
}

bool WOUFont::initWithBuffer(unsigned char *buffer, uint32 buffer_len) {
	font_data = buffer;

	height = font_data[0];
	pixel_char = font_data[2];

	num_chars = 256;
	if (Game::get_game()->get_game_type() != NUVIE_GAME_U6) {
		default_color = FONT_COLOR_WOU_NORMAL;
		default_highlight_color = FONT_COLOR_WOU_HIGHLIGHT;
	}

	return initCharBuf();
}

bool WOUFont::initCharBuf() {
	uint8 max_width = 0;
	for (uint16 i = 0; i < num_chars; i++) {
		uint8 width = font_data[0x4 + i];
		if (width > max_width) {
			max_width = width;
		}
	}
	char_buf = (unsigned char *)malloc(max_width * height);
	if (char_buf == NULL)
		return false;

	return true;
}

uint16 WOUFont::getCharWidth(uint8 c) {
	if (font_data == NULL)
		return 0;

	return font_data[0x4 + get_char_num(c)];
}


uint16 WOUFont::drawChar(Screen *screen, uint8 char_num, uint16 x, uint16 y,
                         uint8 color) {
	unsigned char *pixels;
	uint16 width;

	if (font_data == NULL)
		return false;

	pixels = font_data + font_data[0x204 + char_num] * 256 + font_data[0x104 + char_num];
	width = font_data[0x4 + char_num];

	memset(char_buf, 0xff, width * height);

	//pixels += y * pitch + x;
	for (uint8 i = 0; i < (width * height); i++) {
		if (pixels[i] == pixel_char)
			char_buf[i] = color;
	}

	screen->blit(x, y, char_buf, 8, width, height, width, true, NULL);
	return width;
}

uint16 WOUFont::drawStringToShape(U6Shape *shp, const char *str, uint16 x, uint16 y, uint8 color) {
	uint16 i;
	uint16 string_len = strlen(str);

	if (font_data == NULL)
		return x;

	for (i = 0; i < string_len; i++) {
		x += drawCharToShape(shp, get_char_num(str[i]), x, y, color);
	}

	return x;
}

uint8 WOUFont::drawCharToShape(U6Shape *shp, uint8 char_num, uint16 x, uint16 y,
                               uint8 color) {
	unsigned char *pixels;
	uint16 i, j;
	unsigned char *font;
	uint16 pitch;
	uint16 dst_w, dst_h;

	pixels = shp->get_data();
	shp->get_size(&dst_w, &dst_h);
	pitch = dst_w;

	pixels += y * pitch + x;

	uint16 width;

	font = font_data + font_data[0x204 + char_num] * 256 + font_data[0x104 + char_num];
	width = font_data[0x4 + char_num];

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			if (font[j] == pixel_char) {
				pixels[j] = color;
			}
		}

		font += width;
		pixels += pitch;
	}

	return width;
}

} // End of namespace Nuvie
} // End of namespace Ultima
