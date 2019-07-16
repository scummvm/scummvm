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

#ifndef ADL_GRAPHICS_H
#define ADL_GRAPHICS_H

#include "common/rect.h"
#include "common/stream.h"

#include "adl/display.h"

namespace Adl {

class GraphicsMan {
public:
	virtual ~GraphicsMan() { }

	// Applesoft BASIC HLINE
	virtual void drawLine(const Common::Point &p1, const Common::Point &p2, byte color) const = 0;
	// Applesoft BASIC DRAW
	virtual void drawShape(Common::ReadStream &shape, Common::Point &pos, byte rotation = 0, byte scaling = 1, byte color = 0x7f) const = 0;
	virtual void drawPic(Common::SeekableReadStream &pic, const Common::Point &pos) = 0;
	virtual void clearScreen() const = 0;
	void setBounds(const Common::Rect &r) { _bounds = r; }

protected:
	Common::Rect _bounds;
};

// Used in hires1
template <class T>
class GraphicsMan_v1 : public GraphicsMan {
public:
	GraphicsMan_v1<T>(T &display) : _display(display) { this->setBounds(Common::Rect(280, 160)); }

	void drawLine(const Common::Point &p1, const Common::Point &p2, byte color) const override;
	void drawShape(Common::ReadStream &shape, Common::Point &pos, byte rotation = 0, byte scaling = 1, byte color = 0x7f) const override;
	void drawPic(Common::SeekableReadStream &pic, const Common::Point &pos) override;
	void clearScreen() const override;

protected:
	T &_display;
	void putPixel(const Common::Point &p, byte color) const;

private:
	void drawShapePixel(Common::Point &p, byte color, byte bits, byte quadrant) const;
	virtual byte getClearColor() const { return 0x00; }
};

// Used in hires0 and hires2-hires4
template <class T>
class GraphicsMan_v2 : public GraphicsMan_v1<T> {
public:
	GraphicsMan_v2<T>(T &display) : GraphicsMan_v1<T>(display), _color(0) { }
	void drawPic(Common::SeekableReadStream &pic, const Common::Point &pos) override;

protected:
	bool canFillAt(const Common::Point &p, const bool stopBit = false);
	void fillRow(Common::Point p, const byte pattern, const bool stopBit = false);
	byte getPatternColor(const Common::Point &p, byte pattern);

private:
	static bool readByte(Common::SeekableReadStream &pic, byte &b);
	bool readPoint(Common::SeekableReadStream &pic, Common::Point &p);
	void drawCorners(Common::SeekableReadStream &pic, bool yFirst);
	void drawRelativeLines(Common::SeekableReadStream &pic);
	void drawAbsoluteLines(Common::SeekableReadStream &pic);
	void fill(Common::SeekableReadStream &pic);
	virtual void fillRowLeft(Common::Point p, const byte pattern, const bool stopBit);
	virtual void fillAt(Common::Point p, const byte pattern);
	byte getClearColor() const override { return 0xff; }

	byte _color;
	Common::Point _offset;
};

// Used in hires5, hires6 and gelfling (possibly others as well)
template <class T>
class GraphicsMan_v3 : public GraphicsMan_v2<T> {
public:
	GraphicsMan_v3<T>(T &display) : GraphicsMan_v2<T>(display) { }

private:
	void fillRowLeft(Common::Point p, const byte pattern, const bool stopBit) override;
	void fillAt(Common::Point p, const byte pattern) override;
};

template <class T>
void GraphicsMan_v1<T>::clearScreen() const {
	_display.setMode(Display::kModeMixed);
	_display.clear(getClearColor());
}

// Draws a four-connected line
template <class T>
void GraphicsMan_v1<T>::drawLine(const Common::Point &p1, const Common::Point &p2, byte color) const {
	int16 deltaX = p2.x - p1.x;
	int8 xStep = 1;

	if (deltaX < 0) {
		deltaX = -deltaX;
		xStep = -1;
	}

	int16 deltaY = p2.y - p1.y;
	int8 yStep = -1;

	if (deltaY > 0) {
		deltaY = -deltaY;
		yStep = 1;
	}

	Common::Point p(p1);
	int16 steps = deltaX - deltaY + 1;
	int16 err = deltaX + deltaY;

	while (true) {
		putPixel(p, color);

		if (--steps == 0)
			return;

		if (err < 0) {
			p.y += yStep;
			err += deltaX;
		} else {
			p.x += xStep;
			err += deltaY;
		}
	}
}

template <class T>
void GraphicsMan_v1<T>::putPixel(const Common::Point &p, byte color) const {
	if (this->_bounds.contains(p))
		_display.putPixel(p, color);
}

template <class T>
void GraphicsMan_v1<T>::drawShapePixel(Common::Point &p, byte color, byte bits, byte quadrant) const {
	if (bits & 4)
		putPixel(p, color);

	bits += quadrant;

	if (bits & 1)
		p.x += (bits & 2 ? -1 : 1);
	else
		p.y += (bits & 2 ? 1 : -1);
}

template <class T>
void GraphicsMan_v1<T>::drawShape(Common::ReadStream &corners, Common::Point &pos, byte rotation, byte scaling, byte color) const {
	const byte stepping[] = {
		0xff, 0xfe, 0xfa, 0xf4, 0xec, 0xe1, 0xd4, 0xc5,
		0xb4, 0xa1, 0x8d, 0x78, 0x61, 0x49, 0x31, 0x18,
		0xff
	};

	byte quadrant = rotation >> 4;
	rotation &= 0xf;
	byte xStep = stepping[rotation];
	byte yStep = stepping[(rotation ^ 0xf) + 1] + 1;

	while (true) {
		byte b = corners.readByte();

		if (corners.eos() || corners.err())
			error("Error reading corners");

		if (b == 0)
			return;

		do {
			byte xFrac = 0x80;
			byte yFrac = 0x80;
			for (uint j = 0; j < scaling; ++j) {
				if (xFrac + xStep + 1 > 255)
					drawShapePixel(pos, color, b, quadrant);
				xFrac += xStep + 1;
				if (yFrac + yStep > 255)
					drawShapePixel(pos, color, b, quadrant + 1);
				yFrac += yStep;
			}
			b >>= 3;
		} while (b != 0);
	}
}

template <class T>
void GraphicsMan_v1<T>::drawPic(Common::SeekableReadStream &pic, const Common::Point &pos) {
	byte x, y;
	bool bNewLine = false;
	byte oldX = 0, oldY = 0;
	while (1) {
		x = pic.readByte();
		y = pic.readByte();

		if (pic.err() || pic.eos())
			error("Error reading picture");

		if (x == 0xff && y == 0xff)
			return;

		if (x == 0 && y == 0) {
			bNewLine = true;
			continue;
		}

		x += pos.x;
		y += pos.y;

		if (y > 160)
			y = 160;

		if (bNewLine) {
			putPixel(Common::Point(x, y), 0x7f);
			bNewLine = false;
		} else {
			drawLine(Common::Point(oldX, oldY), Common::Point(x, y), 0x7f);
		}

		oldX = x;
		oldY = y;
	}
}

template <class T>
bool GraphicsMan_v2<T>::readByte(Common::SeekableReadStream &pic, byte &b) {
	b = pic.readByte();

	if (pic.eos() || pic.err())
		error("Error reading picture");

	if (b >= 0xe0) {
		pic.seek(-1, SEEK_CUR);
		return false;
	}

	return true;
}

template <class T>
bool GraphicsMan_v2<T>::readPoint(Common::SeekableReadStream &pic, Common::Point &p) {
	byte b;

	if (!readByte(pic, b))
		return false;

	p.x = b + _offset.x;
	p.x <<= 1;

	if (!readByte(pic, b))
		return false;

	p.y = b + _offset.y;

	return true;
}

template <class T>
byte GraphicsMan_v2<T>::getPatternColor(const Common::Point &p, byte pattern) {
	const byte fillPatterns[][4] = {
		{ 0x00, 0x00, 0x00, 0x00 },
		{ 0x80, 0x80, 0x80, 0x80 },
		{ 0xff, 0xff, 0xff, 0xff },
		{ 0x7f, 0x7f, 0x7f, 0x7f },
		{ 0x2a, 0x55, 0x2a, 0x55 },
		{ 0xaa, 0xd5, 0xaa, 0xd5 },
		{ 0x55, 0x2a, 0x55, 0x2a },
		{ 0xd5, 0xaa, 0xd5, 0xaa },
		{ 0x33, 0x66, 0x4c, 0x19 },
		{ 0xb3, 0xe6, 0xcc, 0x99 },
		{ 0x22, 0x44, 0x08, 0x11 },
		{ 0xa2, 0xc4, 0x88, 0x91 },
		{ 0x11, 0x22, 0x44, 0x08 },
		{ 0x91, 0xa2, 0xc4, 0x88 },
		{ 0x6e, 0x5d, 0x3b, 0x77 },
		{ 0xee, 0xdd, 0xbb, 0xf7 },
		{ 0x5d, 0x3b, 0x77, 0x6e },
		{ 0xdd, 0xbb, 0xf7, 0xee },
		{ 0x66, 0x4c, 0x19, 0x33 },
		{ 0xe6, 0xcc, 0x99, 0xb3 },
		{ 0x33, 0x66, 0x4c, 0x19 },
		{ 0xb3, 0xe6, 0xcc, 0x99 }
	};

	if (pattern >= ARRAYSIZE(fillPatterns))
		error("Invalid fill pattern %i encountered in picture", pattern);

	byte offset = (p.y & 1) << 1;
	offset += (p.x / 7) & 3;

	return fillPatterns[pattern][offset % sizeof(fillPatterns[0])];
}

template <class T>
void GraphicsMan_v2<T>::drawCorners(Common::SeekableReadStream &pic, bool yFirst) {
	Common::Point p;

	if (!readPoint(pic, p))
		return;

	if (yFirst)
		goto doYStep;

	while (true) {
		byte b;
		int16 n;

		if (!readByte(pic, b))
			return;

		n = b + _offset.x;

		this->putPixel(p, _color);

		n <<= 1;
		this->drawLine(p, Common::Point(n, p.y), _color);
		p.x = n;

doYStep:
		if (!readByte(pic, b))
			return;

		n = b + _offset.y;

		this->putPixel(p, _color);
		this->drawLine(p, Common::Point(p.x, n), _color);

		this->putPixel(Common::Point(p.x + 1, p.y), _color);
		this->drawLine(Common::Point(p.x + 1, p.y), Common::Point(p.x + 1, n), _color);

		p.y = n;
	}
}

template <class T>
void GraphicsMan_v2<T>::drawRelativeLines(Common::SeekableReadStream &pic) {
	Common::Point p1;

	if (!readPoint(pic, p1))
		return;

	this->putPixel(p1, _color);

	while (true) {
		Common::Point p2(p1);

		byte n;

		if (!readByte(pic, n))
			return;

		byte h = (n & 0x70) >> 4;
		byte l = n & 7;

		if (n & 0x80)
			p2.x -= (h << 1);
		else
			p2.x += (h << 1);

		if (n & 8)
			p2.y -= l;
		else
			p2.y += l;

		this->drawLine(p1, p2, _color);
		p1 = p2;
	}
}

template <class T>
void GraphicsMan_v2<T>::drawAbsoluteLines(Common::SeekableReadStream &pic) {
	Common::Point p1;

	if (!readPoint(pic, p1))
		return;

	this->putPixel(p1, _color);

	while (true) {
		Common::Point p2;

		if (!readPoint(pic, p2))
			return;

		this->drawLine(p1, p2, _color);
		p1 = p2;
	}
}

template <class T>
bool GraphicsMan_v2<T>::canFillAt(const Common::Point &p, const bool stopBit) {
	return this->_display.getPixelBit(p) != stopBit && this->_display.getPixelBit(Common::Point(p.x + 1, p.y)) != stopBit;
}

template <class T>
void GraphicsMan_v2<T>::fillRowLeft(Common::Point p, const byte pattern, const bool stopBit) {
	byte color = getPatternColor(p, pattern);

	while (--p.x >= this->_bounds.left) {
		if ((p.x % 7) == 6) {
			color = getPatternColor(p, pattern);
			this->_display.setPixelPalette(p, color);
		}
		if (this->_display.getPixelBit(p) == stopBit)
			break;
		this->_display.setPixelBit(p, color);
	}
}

template <class T>
void GraphicsMan_v2<T>::fillRow(Common::Point p, const byte pattern, const bool stopBit) {
	// Set pixel at p and palette
	byte color = getPatternColor(p, pattern);
	this->_display.setPixelPalette(p, color);
	this->_display.setPixelBit(p, color);

	// Fill left of p
	fillRowLeft(p, pattern, stopBit);

	// Fill right of p
	while (++p.x < this->_bounds.right) {
		if ((p.x % 7) == 0) {
			color = getPatternColor(p, pattern);
			// Palette is set before the first bit is tested
			this->_display.setPixelPalette(p, color);
		}
		if (this->_display.getPixelBit(p) == stopBit)
			break;
		this->_display.setPixelBit(p, color);
	}
}

template <class T>
void GraphicsMan_v2<T>::fillAt(Common::Point p, const byte pattern) {
	const bool stopBit = !this->_display.getPixelBit(p);

	// Move up into the open space above p
	while (--p.y >= this->_bounds.top && canFillAt(p, stopBit)) {}

	// Then fill by moving down
	while (++p.y < this->_bounds.bottom && canFillAt(p, stopBit))
		fillRow(p, pattern, stopBit);
}

template <class T>
void GraphicsMan_v2<T>::fill(Common::SeekableReadStream &pic) {
	byte pattern;

	if (!readByte(pic, pattern))
		return;

	while (true) {
		Common::Point p;

		if (!readPoint(pic, p))
			return;

		if (this->_bounds.contains(p))
			fillAt(p, pattern);
	}
}

template <class T>
void GraphicsMan_v2<T>::drawPic(Common::SeekableReadStream &pic, const Common::Point &pos) {
	// NOTE: The original engine only resets the color for overlays. As a result, room
	// pictures that draw without setting a color or clearing the screen, will use the
	// last color set by the previous picture. We assume this is unintentional and do
	// not copy this behavior.
	_color = 0;
	_offset = pos;

	while (true) {
		byte opcode = pic.readByte();

		if (pic.eos() || pic.err())
			error("Error reading picture");

		switch (opcode) {
		case 0xe0:
			drawCorners(pic, false);
			break;
		case 0xe1:
			drawCorners(pic, true);
			break;
		case 0xe2:
			drawRelativeLines(pic);
			break;
		case 0xe3:
			drawAbsoluteLines(pic);
			break;
		case 0xe4:
			fill(pic);
			break;
		case 0xe5:
			this->clearScreen();
			_color = 0x00;
			break;
		case 0xf0:
			_color = 0x00;
			break;
		case 0xf1:
			_color = 0x2a;
			break;
		case 0xf2:
			_color = 0x55;
			break;
		case 0xf3:
			_color = 0x7f;
			break;
		case 0xf4:
			_color = 0x80;
			break;
		case 0xf5:
			_color = 0xaa;
			break;
		case 0xf6:
			_color = 0xd5;
			break;
		case 0xf7:
			_color = 0xff;
			break;
		case 0xff:
			return;
		default:
			if (opcode >= 0xe0)
				error("Invalid pic opcode %02x", opcode);
			else
				warning("Expected pic opcode, but found data byte %02x", opcode);
		}
	}
}

template <class T>
void GraphicsMan_v3<T>::fillRowLeft(Common::Point p, const byte pattern, const bool stopBit) {
	byte color = this->getPatternColor(p, pattern);

	while (--p.x >= this->_bounds.left) {
		// In this version, when moving left, it no longer sets the palette first
		if (!this->_display.getPixelBit(p))
			return;
		if ((p.x % 7) == 6) {
			color = this->getPatternColor(p, pattern);
			this->_display.setPixelPalette(p, color);
		}
		this->_display.setPixelBit(p, color);
	}
}

template <class T>
void GraphicsMan_v3<T>::fillAt(Common::Point p, const byte pattern) {
	// If the row at p cannot be filled, we do nothing
	if (!this->canFillAt(p))
			return;

	this->fillRow(p, pattern);

	Common::Point q(p);

	// Fill up from p
	while (--q.y >= this->_bounds.top && this->canFillAt(q))
		this->fillRow(q, pattern);

	// Fill down from p
	while (++p.y < this->_bounds.bottom && this->canFillAt(p))
		this->fillRow(p, pattern);
}

} // End of namespace Adl

#endif
