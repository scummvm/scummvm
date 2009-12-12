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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef M4_FONT_H
#define M4_FONT_H

#include "common/util.h"
#include "common/endian.h"

#include "m4/graphics.h"

/*
  TODO:
	- make a FontSystem class that creates/manages the fonts
	  (similar to FileSystem)
*/

namespace M4 {

#define FONT_MENU "fontmenu.fnt"
#define FONT_INTERFACE "fontintr.fnt"
#define FONT_TINY "small.fnt"
#define FONT_SMALL "small.fnt"
#define FONT_MEDIUM "medium.fnt"
#define FONT_LINE "fontline.fnt"
#define FONT_CONVERSATION "fontconv.fnt"
#define FONT_4X6 "4x6pp.fnt"
#define FONT_5X6 "5x6pp.fnt"

#define FONT_CONVERSATION_MADS "fontconv.ff"
#define FONT_INTERFACE_MADS "fontintr.ff"
#define FONT_MAIN_MADS "fontmain.ff"
#define FONT_MENU_MADS "fontmenu.ff"	// Not in Rex (uses bitmap files for menu strings)
#define FONT_MISC_MADS "fontmisc.ff"
#define FONT_TELE_MADS "fonttele.ff"	// Not in Phantom
#define FONT_PHAN_MADS "fontphan.ff"	// Phantom only

class Font {
public:
	Font(M4Engine *vm);
	~Font();
	void setFont(const char *filename);
	void setColor(uint8 color);
	void setColors(uint8 alt1, uint8 alt2, uint8 foreground);
	void setColour(uint8 colour) { setColor(colour); }
	void setColours(uint8 alt1, uint8 alt2, uint8 foreground) { setColors(alt1, alt2, foreground); } 

	int32 getWidth(char *text, int spaceWidth = -1);
	int32 getHeight() const { return _maxHeight; }
	int32 getMaxWidth() const { return _maxWidth; }
	int32 write(M4Surface *surface, const char *text, int x, int y, int width, int spaceWidth, uint8 colors[]);
	int32 writeString(M4Surface *surface, const char *text, int x, int y, int width = 0, int spaceWidth = -1) {
		return write(surface, text, x, y, width, spaceWidth, _fontColors);
	}

private:
	void setFontM4(const char *filename);
	void setFontMads(const char *filename);

	M4Engine *_vm;
	uint8 _maxWidth, _maxHeight;
	uint8 *_charWidths;
	uint16 *_charOffs;
	uint8 *_charData;
	bool _sysFont;
	const char *_filename;
	uint8 _fontColors[4];
};

} // End of namespace M4

#endif
