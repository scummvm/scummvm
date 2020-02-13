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

#ifndef NUVIE_FONTS_WOU_FONT_H
#define NUVIE_FONTS_WOU_FONT_H

#include "ultima/nuvie/fonts/font.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class Screen;
class U6Shape;

class WOUFont : public Font {
private:
	unsigned char *font_data;
	unsigned char *char_buf;
	uint16 height;
	uint8 pixel_char;

public:

	WOUFont();
	~WOUFont() override;

	bool init(const char *filename);
	bool initWithBuffer(unsigned char *buffer, uint32 buffer_len);


	uint16 drawChar(Screen *screen, uint8 char_num, uint16 x, uint16 y,
	                        uint8 color) override;

	uint16 drawStringToShape(U6Shape *shp, const char *str, uint16 x, uint16 y, uint8 color);
	uint8 drawCharToShape(U6Shape *shp, uint8 char_num, uint16 x, uint16 y, uint8 color);

	uint16 getCharWidth(uint8 c) override;
	uint16 getCharHeight() override {
		return height;
	}
private:
	bool initCharBuf();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
