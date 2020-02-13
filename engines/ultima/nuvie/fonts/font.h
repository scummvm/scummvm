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

#ifndef NUVIE_FONTS_FONT_H
#define NUVIE_FONTS_FONT_H

namespace Ultima {
namespace Nuvie {

#define FONT_COLOR_U6_NORMAL    0x48
#define FONT_COLOR_U6_HIGHLIGHT 0x0c
#define FONT_COLOR_WOU_NORMAL    0
#define FONT_COLOR_WOU_CONVERSE_INPUT 1

#define FONT_COLOR_WOU_HIGHLIGHT 4

#define FONT_UP_ARROW_CHAR   19
#define FONT_DOWN_ARROW_CHAR 20

class Configuration;
class Screen;
class U6Shape;

class Font {
protected:
	uint16 num_chars;
	uint16 offset;
	uint8 default_color, default_highlight_color;

private:


public:

	Font();
	virtual ~Font();
	uint8 getDefaultColor() {
		return default_color;
	}
	void setDefaultColor(uint8 color) {
		default_color = color;
	}
	void setDefaultHighlightColor(uint8 color) {
		default_highlight_color = color;
	}

//   bool drawString(Screen *screen, Std::string str, uint16 x, uint16 y);
	uint16 drawString(Screen *screen, const char *str, uint16 x, uint16 y);
	uint16 drawString(Screen *screen, const char *str, uint16 x, uint16 y, uint8 color, uint8 highlight_color);
	uint16 drawString(Screen *screen, const char *str, uint16 string_len, uint16 x, uint16 y, uint8 color, uint8 highlight_color);

	uint16 drawChar(Screen *screen, uint8 char_num, uint16 x, uint16 y);
	virtual uint16 drawChar(Screen *screen, uint8 char_num, uint16 x, uint16 y,
	                        uint8 color) = 0;

	uint16 drawStringToShape(U6Shape *shp, const char *str, uint16 x, uint16 y, uint8 color);
	uint8 drawCharToShape(U6Shape *shp, uint8 char_num, uint16 x, uint16 y, uint8 color);

	virtual uint16 getCharWidth(uint8 c) = 0;
	virtual uint16 getCharHeight() = 0;
	uint16 getStringWidth(const char *str);
	virtual uint16 getStringWidth(const char *str, uint16 string_len);

	void setOffset(uint16 off) {
		offset = off;
	}

protected:

	uint8 get_char_num(uint8 c);

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
