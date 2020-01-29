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

#ifndef ULTIMA6_FONTS_BMP_FONT_H
#define ULTIMA6_FONTS_BMP_FONT_H

#include "ultima/ultima6/fonts/font.h"

namespace Ultima {
namespace Ultima6 {

class Configuration;
class Screen;

class BMPFont : public Font {
	Graphics::ManagedSurface *sdl_font_data;
	uint8 *font_width_data;

	uint16 char_w, char_h;

	bool dual_font_mode;
	bool rune_mode;

public:

	BMPFont();
	~BMPFont();

	bool init(Std::string bmp_filename, bool dual_fontmap = false);

	uint16 getCharWidth(uint8 c);
	uint16 getCharHeight() {
		return 16;
	}
	uint16 drawChar(Screen *screen, uint8 char_num, uint16 x, uint16 y,
	                uint8 color);
	uint16 getStringWidth(const char *str, uint16 string_len);
protected:

};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
