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

#ifndef ULTIMA6_GUI_GUI_FONT_H
#define ULTIMA6_GUI_GUI_FONT_H

//#include <string.h>



namespace Ultima {
namespace Ultima6 {

#define GUI_FONT_DEFAULT 0
#define GUI_FONT_6X8     1
#define GUI_FONT_GUMP    2

class GUI_Font {

public:
	/* use default 8x8 font */
	GUI_Font(uint8 fontType = GUI_FONT_DEFAULT);

	/* open named BMP file */
	GUI_Font(char *name);

	/* use given YxY surface */
	GUI_Font(Graphics::ManagedSurface *bitmap);

	/* copy constructor */
	GUI_Font(GUI_Font &font);

	virtual ~GUI_Font();

	/* determine drawing style */
	virtual void SetTransparency(int on);

	/* determine foreground and background color values RGB*/
	virtual void SetColoring(uint8 fr, uint8 fg, uint8 fb, uint8 br = 255, uint8 bg = 255, uint8 bb = 255);
	/* Two color font */
	virtual void SetColoring(uint8 fr, uint8 fg, uint8 fb, uint8 fr1, uint8 fg1, uint8 fb1, uint8 br, uint8 bg, uint8 bb);

	/* yields the pixel height of a single character */
	inline virtual int CharHeight() {
		return charh - 1;
	}

	/* yields the pixel width of a single character */
	inline virtual int CharWidth() {
		return charw;
	}

	/* put the text onto the given surface using the preset mode and colors */
	virtual void TextOut(Graphics::ManagedSurface *context, int x, int y, const char *text, int line_wrap = 0);

	/* yields pixel width and height of a string when printed with this font */
	void TextExtent(const char *text, int *w, int *h, int line_wrap = 0);

	/* yields the center (of width) where the text should be placed or 0 if larger than width */
	uint16 get_center(const char *text, uint16 width);

protected:
	/* the font source surface */
	Graphics::ManagedSurface *fontStore;

	/* flags */
	int transparent;
	int freefont;

	/* dimensions */
	int charh, charw;

	uint8 *w_data;
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
