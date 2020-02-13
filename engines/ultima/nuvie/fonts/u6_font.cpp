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
#include "ultima/nuvie/fonts/u6_font.h"

namespace Ultima {
namespace Nuvie {

U6Font::U6Font() {
	font_data = NULL;
	num_chars = 0;
	offset = 0;
}

U6Font::~U6Font() {
	if (font_data != NULL)
		free(font_data);
}

bool U6Font::init(unsigned char *data, uint16 num_c, uint16 char_offset) {
	offset = char_offset;
	num_chars = num_c;

	font_data = (unsigned char *)malloc(num_chars * 8);
	memcpy(font_data, data, num_chars * 8);

	return true;
}

uint16 U6Font::drawChar(Screen *screen, uint8 char_num, uint16 x, uint16 y,
                        uint8 color) {
	unsigned char buf[64];
	unsigned char *pixels;
	uint16 i, j;
	unsigned char *font;
	uint16 pitch;

	memset(buf, 0xff, 64);

//pixels = (unsigned char *)screen->get_pixels();
	pixels = buf;
	pitch = 8;//screen->get_pitch();

	font = &font_data[char_num * 8];

//pixels += y * pitch + x;

	for (i = 0; i < 8; i++) {
		for (j = 8; j > 0; j--) {
			if (font[i] & (1 << (j - 1)))
				pixels[8 - j] = color; // 0th palette entry should be black
		}

		pixels += pitch;
	}

	screen->blit(x, y, buf, 8, 8, 8, 8, true, NULL);
	return 8;
}

} // End of namespace Nuvie
} // End of namespace Ultima
