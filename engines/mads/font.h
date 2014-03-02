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

#define FONT_CONVERSATION "*FONTCONV.FF"
#define FONT_INTERFACE "*FONTINTR.FF"
#define FONT_MAIN "*FONTMAIN.FF"
#define FONT_MENU "*FONTMENU.FF"	// Not in Rex (uses bitmap files for menu strings)
#define FONT_MISC "*FONTMISC.FF"
#define FONT_TELE "*FONTTELE.FF"	// Not in Phantom
#define FONT_PHAN "*FONTPHAN.FF"	// Phantom only

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

	int getBpp(int charWidth);
public:
	Font(MADSEngine *vm);
	virtual ~Font();

	void setFont(const Common::String &filename);
	void setColor(uint8 color);
	void setColors(uint8 v1, uint8 v2, uint8 v3, uint8 v4);

	int maxWidth() const { return _maxWidth; }
	int getWidth(const Common::String &msg, int spaceWidth = -1);
	int getHeight() const { return _maxHeight; }
	int write(MSurface *surface, const Common::String &msg, const Common::Point &pt, int width, int spaceWidth, uint8 colors[]);
	int writeString(MSurface *surface, const Common::String &msg, const Common::Point &pt, int width = 0, int spaceWidth = -1) {
		return write(surface, msg, pt, width, spaceWidth, _fontColors);
	}

	/**
	 * Returns a new Font instance using the specified font name
	 */
	Font *getFont(const Common::String &fontName);
};

} // End of namespace MADS

#endif /* MADS_FONT_H */
