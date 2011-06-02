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
	Font(MadsM4Engine *vm, const char *filename);
	~Font();

	void setColor(uint8 color);
	void setColors(uint8 col1, uint8 col2, uint8 col3);

	int32 getWidth(const char *text, int spaceWidth = -1);
	int32 getHeight() const { return _maxHeight; }
	int32 getMaxWidth() const { return _maxWidth; }
	int32 write(M4Surface *surface, const char *text, int x, int y, int width, int spaceWidth, uint8 colors[]);
	int32 writeString(M4Surface *surface, const char *text, int x, int y, int width = 0, int spaceWidth = -1) {
		return write(surface, text, x, y, width, spaceWidth, _fontColors);
	}
public:
	char _filename[20];
private:
	void setFontM4(const char *filename);
	void setFontMads(const char *filename);

	MadsM4Engine *_vm;
	uint8 _maxWidth, _maxHeight;
	uint8 *_charWidths;
	uint16 *_charOffs;
	uint8 *_charData;
	bool _sysFont;
	uint8 _fontColors[4];
};

class FontEntry {
public:
	Font *_font;

	FontEntry() {
		_font = NULL;
	}
	~FontEntry() {
		delete _font;
	}
};

class FontManager {
private:
	MadsM4Engine *_vm;
	Common::Array<Font *> _entries;
	Font *_currentFont;
public:
	FontManager(MadsM4Engine *vm): _vm(vm) { _currentFont = NULL; }
	~FontManager();

	Font *getFont(const char *filename);
	void setFont(const char *filename);

	Font *current() {
		assert(_currentFont);
		return _currentFont;
	}
};

} // End of namespace M4

#endif
