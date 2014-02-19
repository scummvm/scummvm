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

#ifndef MADS_FONT_H
#define MADS_FONT_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/endian.h"
#include "mads/msurface.h"

namespace MADS {

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

class MADSEngine;

class Font {
protected:
	MADSEngine *_vm;

	uint8 _maxWidth, _maxHeight;
	uint8 *_charWidths;
	uint16 *_charOffs;
	uint8 *_charData;
	bool _sysFont;
	Common::String _filename;
	uint8 _fontColors[4];

protected:
	Font(MADSEngine *vm);

	virtual void setFont(const Common::String &filename) = 0;
	virtual int getBpp(int charWidth) = 0;
public:
	static Font *init(MADSEngine *vm);
public:
	virtual ~Font();

	void setColor(uint8 color);
	void setColors(uint8 alt1, uint8 alt2, uint8 foreground);

	int getWidth(const Common::String &msg, int spaceWidth = -1);
	int getHeight() const { return _maxHeight; }
	int write(MSurface *surface, const Common::String &msg, const Common::Point &pt, int width, int spaceWidth, uint8 colors[]);
	int writeString(MSurface *surface, const Common::String &msg, const Common::Point &pt, int width = 0, int spaceWidth = -1) {
		return write(surface, msg, pt, width, spaceWidth, _fontColors);
	}
};

class FontMADS: public Font {
	friend class Font;
protected:
	virtual void setFont(const Common::String &filename);
	virtual int getBpp(int charWidth);

	FontMADS(MADSEngine *vm): Font(vm) {}
};

class FontM4: public Font {
	friend class Font;
protected:
	virtual void setFont(const Common::String &filename);
	virtual int getBpp(int charWidth);

	FontM4(MADSEngine *vm): Font(vm) {}
};

} // End of namespace MADS

#endif /* MADS_FONT_H */
