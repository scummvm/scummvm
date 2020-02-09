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

#ifndef NUVIE_FONTS_U6_FONT_H
#define NUVIE_FONTS_U6_FONT_H

#define FONT_COLOR_U6_NORMAL    0x48
#define FONT_COLOR_U6_HIGHLIGHT 0x0c

#include "ultima/nuvie/fonts/font.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class Screen;

class U6Font : public Font {
private:
	unsigned char *font_data;

public:

	U6Font();
	~U6Font() override;

	bool init(unsigned char *data, uint16 num_chars, uint16 char_offset);

	uint16 getCharWidth(uint8 c) override {
		return 8;
	}
	uint16 getCharHeight() override {
		return 8;
	}
	uint16 drawChar(Screen *screen, uint8 char_num, uint16 x, uint16 y,
	                uint8 color) override;
protected:

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
