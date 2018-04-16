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

#ifndef MADS_FONT_H
#define MADS_FONT_H

#include "common/scummsys.h"
#include "common/hashmap.h"
#include "common/endian.h"
#include "common/util.h"
#include "mads/msurface.h"

namespace MADS {

#define FONT_CONVERSATION "*FONTCONV.FF"
#define FONT_INTERFACE "*FONTINTR.FF"
#define FONT_MAIN "*FONTMAIN.FF"
#define FONT_MENU "*FONTMENU.FF"	// Not in Rex (uses bitmap files for menu strings)
#define FONT_MISC "*FONTMISC.FF"
#define FONT_TELE "*FONTTELE.FF"	// Not in Phantom
#define FONT_PHAN "*FONTPHAN.FF"	// Phantom only

enum SelectionMode {
	SELMODE_UNSELECTED = 0, SELMODE_HIGHLIGHTED = 1, SELMODE_SELECTED = 2
};

class MADSEngine;

class Font {
private:
	static uint8 _fontColors[4];
	static MADSEngine *_vm;
	static Common::HashMap<Common::String, Font *> *_fonts;
public:
	/**
	 * Initialize the font system
	 */
	static void init(MADSEngine *vm);

	/**
	 * Free up the resources used by the font
	 */
	static void deinit();

	/**
	* Returns a new Font instance using the specified font name
	*/
	static Font *getFont(const Common::String &fontName);
private:
	uint8 _maxWidth, _maxHeight;
	uint8 *_charWidths;
	uint16 *_charOffs;
	uint8 *_charData;
	Common::String _filename;

	int getBpp(int charWidth);

	void setFont(const Common::String &filename);
public:
	Font();
	Font(const Common::String &filename);
	virtual ~Font();

	void setColors(uint8 v1, uint8 v2, uint8 v3, uint8 v4);
	void setColorMode(SelectionMode mode);

	int maxWidth() const { return _maxWidth; }
	int getWidth(const Common::String &msg, int spaceWidth = -1);
	int getHeight() const { return _maxHeight; }
	int writeString(BaseSurface *surface, const Common::String &msg, const Common::Point &pt,
		int spaceWidth = 0, int width = 0);
};

} // End of namespace MADS

#endif /* MADS_FONT_H */
