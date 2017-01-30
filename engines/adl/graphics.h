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

#ifndef ADL_PICTURE_H
#define ADL_PICTURE_H

#include "common/rect.h"

namespace Common {
class SeekableReadStream;
}

namespace Adl {

class Display;

class GraphicsMan {
public:
	virtual ~GraphicsMan() { }
	virtual void drawPic(Common::SeekableReadStream &pic, const Common::Point &pos) = 0;
	void clearScreen() const;

protected:
	GraphicsMan(Display &display) : _display(display) { }
	void drawLine(const Common::Point &p1, const Common::Point &p2, byte color) const;

	Display &_display;

private:
	virtual byte getClearColor() const = 0;
};

// Used in hires1
class Graphics_v1 : public GraphicsMan {
public:
	Graphics_v1(Display &display) : GraphicsMan(display) { }
	void drawPic(Common::SeekableReadStream &pic, const Common::Point &pos);
	void drawCorners(Common::ReadStream &corners, const Common::Point &pos, byte rotation = 0, byte scaling = 1, byte color = 0x7f) const;

private:
	void drawCornerPixel(Common::Point &p, byte color, byte bits, byte quadrant) const;
	byte getClearColor() const { return 0x00; }
};

// Used in hires0 and hires2-hires4
class Graphics_v2 : public GraphicsMan {
public:
	Graphics_v2(Display &display) : GraphicsMan(display), _color(0) { }
	void drawPic(Common::SeekableReadStream &pic, const Common::Point &pos);

protected:
	bool canFillAt(const Common::Point &p, const bool stopBit = false);
	void fillRow(Common::Point p, const byte pattern, const bool stopBit = false);

private:
	void drawCorners(Common::SeekableReadStream &pic, bool yFirst);
	void drawRelativeLines(Common::SeekableReadStream &pic);
	void drawAbsoluteLines(Common::SeekableReadStream &pic);
	virtual void fillRowLeft(Common::Point p, const byte pattern, const bool stopBit);
	virtual void fillAt(Common::Point p, const byte pattern);
	void fill(Common::SeekableReadStream &pic);
	byte getClearColor() const { return 0xff; }

	byte _color;
	Common::Point _offset;
};

// Used in hires5, hires6 and gelfling (possibly others as well)
class Graphics_v3 : public Graphics_v2 {
public:
	Graphics_v3(Display &display) : Graphics_v2(display) { }

private:
	void fillRowLeft(Common::Point p, const byte pattern, const bool stopBit);
	void fillAt(Common::Point p, const byte pattern);
};

} // End of namespace Adl

#endif
