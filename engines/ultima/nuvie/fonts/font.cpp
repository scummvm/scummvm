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
#include "ultima/nuvie/fonts/font.h"
#include "ultima/nuvie/core/game.h"

namespace Ultima {
namespace Nuvie {

Font::Font() {
	num_chars = 0;
	offset = 0;

	default_color = FONT_COLOR_U6_NORMAL;
	default_highlight_color = FONT_COLOR_U6_HIGHLIGHT;
}

Font::~Font() {

}

uint16 Font::drawString(Screen *screen, const char *str, uint16 x, uint16 y) {
	return drawString(screen, str, strlen(str), x, y, default_color, default_highlight_color);
}

uint16 Font::drawString(Screen *screen, const char *str, uint16 x, uint16 y, uint8 color, uint8 highlight_color) {
	return drawString(screen, str, strlen(str), x, y, color, highlight_color);
}

uint16 Font::drawString(Screen *screen, const char *str, uint16 string_len, uint16 x, uint16 y, uint8 color, uint8 highlight_color) {
	uint16 i;
	bool highlight = false;
	uint16 font_len = 0;

	for (i = 0; i < string_len; i++) {
		if (str[i] == '@')
			highlight = true;
		else {
			if (!Common::isAlpha(str[i]))
				highlight = false;
			font_len += drawChar(screen, get_char_num(str[i]), x + font_len, y,
			                     highlight ? highlight_color : color);
		}
	}
	highlight = false;
	return font_len;
}

uint8 Font::get_char_num(uint8 c) {
	if (c >= offset && c < offset + num_chars)
		c -= offset;
	else
		c = 0;

	return c;
}

uint16 Font::getStringWidth(const char *str) {
	return getStringWidth(str, strlen(str));
}

uint16 Font::getStringWidth(const char *str, uint16 string_len) {
	uint16 i;
	uint16 w = 0;

	for (i = 0; i < string_len; i++) {
		w += getCharWidth(str[i]);
	}

	return w;
}

uint16 Font::drawChar(Screen *screen, uint8 char_num, uint16 x, uint16 y) {
	return drawChar(screen, char_num, x, y, default_color);
}

} // End of namespace Nuvie
} // End of namespace Ultima
