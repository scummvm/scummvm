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

#ifndef NUVIE_FONTS_FONT_MANAGER_H
#define NUVIE_FONTS_FONT_MANAGER_H

#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class Font;

#define NUVIE_FONT_NORMAL 0
#define NUVIE_FONT_GARG   1

class FontManager {
	Configuration *config;

	Std::vector<Font *> fonts;
	uint16 num_fonts;
	Font *conv_font;
	Font *conv_garg_font;
	unsigned char *conv_font_data;
	uint8 *conv_font_widths;
public:

	FontManager(Configuration *cfg);
	~FontManager();

	bool init(nuvie_game_t game_type);


	Font *get_font(uint16 font_number);
	Font *get_conv_font() {
		return conv_font;
	}
	Font *get_conv_garg_font() {
		return conv_garg_font;
	}

protected:

	bool initU6();
	bool initWOU(Std::string filename);
	bool initWOUSystemFont();
	bool initConvFonts(nuvie_game_t game_type);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
