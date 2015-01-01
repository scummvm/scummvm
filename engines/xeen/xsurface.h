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

#ifndef XEEN_XSURFACE_H
#define XEEN_XSURFACE_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Xeen {

#define FONT_WIDTH 8
#define FONT_HEIGHT 8
#define DEFAULT_BG_COLOR 0x99

enum Justify { JUSTIFY_NONE = 0, JUSTIFY_CENTER = 1, JUSTIFY_RIGHT = 2 };

class XSurface: public Graphics::Surface {
private:
	const char *_displayString;
	bool _msgWraps;

	char getNextChar();

	bool getNextCharWidth(int &total);

	bool newLine(const Common::Rect &bounds);

	int fontAtoi(int len = 3);

	void setTextColor(int idx);
public:
	static const byte *_fontData;
	Common::Point _writePos;
	byte _textColors[4];
	byte _bgColor;
	bool _fontReduced;
	Justify _fontJustify;
public:
	virtual void addDirtyRect(const Common::Rect &r) {}
public:
	XSurface();
	XSurface(int w, int h);
	virtual ~XSurface();

	void create(uint16 w, uint16 h);

	void transBlitTo(XSurface &dest) const;

	void transBlitTo(XSurface &dest, const Common::Point &destPos) const;

	void blitTo(XSurface &dest, const Common::Point &destPos) const;

	void blitTo(XSurface &dest) const;

	bool empty() const { return getPixels() == nullptr; }

	void writeSymbol(int symbolId);

	Common::String writeString(const Common::String &s, const Common::Rect &bounds);

	void writeChar(char c);
};

} // End of namespace Xeen

#endif /* XEEN_XSURFACE_H */
