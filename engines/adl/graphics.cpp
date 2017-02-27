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

#include "common/stream.h"
#include "common/rect.h"
#include "common/textconsole.h"

#include "adl/display.h"
#include "adl/graphics.h"

namespace Adl {

void GraphicsMan::clearScreen() const {
	_display.setMode(DISPLAY_MODE_MIXED);
	_display.clear(getClearColor());
}

// Draws a four-connected line
void GraphicsMan::drawLine(const Common::Point &p1, const Common::Point &p2, byte color) const {
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

void GraphicsMan::putPixel(const Common::Point &p, byte color) const {
	if (_bounds.contains(p))
		_display.putPixel(p, color);
}

void GraphicsMan::drawShapePixel(Common::Point &p, byte color, byte bits, byte quadrant) const {
	if (bits & 4)
		putPixel(p, color);

	bits += quadrant;

	if (bits & 1)
		p.x += (bits & 2 ? -1 : 1);
	else
		p.y += (bits & 2 ? 1 : -1);
}

void GraphicsMan::drawShape(Common::ReadStream &corners, Common::Point &pos, byte rotation, byte scaling, byte color) const {
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

void GraphicsMan::drawPic(Common::SeekableReadStream &pic, const Common::Point &pos) {
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

#define NUM_PATTERNS 22
#define PATTERN_LEN 4
static const byte fillPatterns[NUM_PATTERNS][PATTERN_LEN] = {
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

#define MIN_COMMAND 0xe0

#define CHECK_COMMAND(X) \
	do { \
		if ((X) >= MIN_COMMAND) { \
			pic.seek(-1, SEEK_CUR); \
			return; \
		} \
	} while (0)

#define READ_BYTE(b) \
	do { \
		b = pic.readByte(); \
		if (pic.eos() || pic.err()) \
			error("Error reading picture"); \
		CHECK_COMMAND(b); \
	} while (0)

#define READ_POINT(p) \
	do { \
		READ_BYTE(p.x); \
		p.x += _offset.x; \
		p.x <<= 1; \
		READ_BYTE(p.y); \
		p.y += _offset.y; \
	} while (0)

void GraphicsMan_v2::drawCorners(Common::SeekableReadStream &pic, bool yFirst) {
	Common::Point p;

	READ_POINT(p);

	if (yFirst)
		goto doYStep;

	while (true) {
		int16 n;

		READ_BYTE(n);
		n += _offset.x;

		putPixel(p, _color);

		n <<= 1;
		drawLine(p, Common::Point(n, p.y), _color);
		p.x = n;

doYStep:
		READ_BYTE(n);
		n += _offset.y;

		putPixel(p, _color);
		drawLine(p, Common::Point(p.x, n), _color);

		putPixel(Common::Point(p.x + 1, p.y), _color);
		drawLine(Common::Point(p.x + 1, p.y), Common::Point(p.x + 1, n), _color);

		p.y = n;
	}
}

void GraphicsMan_v2::drawRelativeLines(Common::SeekableReadStream &pic) {
	Common::Point p1;

	READ_POINT(p1);
	putPixel(p1, _color);

	while (true) {
		Common::Point p2(p1);

		byte n;
		READ_BYTE(n);

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

		drawLine(p1, p2, _color);
		p1 = p2;
	}
}

void GraphicsMan_v2::drawAbsoluteLines(Common::SeekableReadStream &pic) {
	Common::Point p1;

	READ_POINT(p1);
	putPixel(p1, _color);

	while (true) {
		Common::Point p2;

		READ_POINT(p2);
		drawLine(p1, p2, _color);
		p1 = p2;
	}
}

static byte getPatternColor(const Common::Point &p, byte pattern) {
	if (pattern >= NUM_PATTERNS)
		error("Invalid fill pattern %i encountered in picture", pattern);

	byte offset = (p.y & 1) << 1;
	offset += (p.x / 7) & 3;

	return fillPatterns[pattern][offset % PATTERN_LEN];
}

bool GraphicsMan_v2::canFillAt(const Common::Point &p, const bool stopBit) {
	return _display.getPixelBit(p) != stopBit && _display.getPixelBit(Common::Point(p.x + 1, p.y)) != stopBit;
}

void GraphicsMan_v2::fillRowLeft(Common::Point p, const byte pattern, const bool stopBit) {
	byte color = getPatternColor(p, pattern);

	while (--p.x >= _bounds.left) {
		if ((p.x % 7) == 6) {
			color = getPatternColor(p, pattern);
			_display.setPixelPalette(p, color);
		}
		if (_display.getPixelBit(p) == stopBit)
			break;
		_display.setPixelBit(p, color);
	}
}

void GraphicsMan_v2::fillRow(Common::Point p, const byte pattern, const bool stopBit) {
	// Set pixel at p and palette
	byte color = getPatternColor(p, pattern);
	_display.setPixelPalette(p, color);
	_display.setPixelBit(p, color);

	// Fill left of p
	fillRowLeft(p, pattern, stopBit);

	// Fill right of p
	while (++p.x < _bounds.right) {
		if ((p.x % 7) == 0) {
			color = getPatternColor(p, pattern);
			// Palette is set before the first bit is tested
			_display.setPixelPalette(p, color);
		}
		if (_display.getPixelBit(p) == stopBit)
			break;
		_display.setPixelBit(p, color);
	}
}

void GraphicsMan_v2::fillAt(Common::Point p, const byte pattern) {
	const bool stopBit = !_display.getPixelBit(p);

	// Move up into the open space above p
	while (--p.y >= _bounds.top && canFillAt(p, stopBit));

	// Then fill by moving down
	while (++p.y < _bounds.bottom && canFillAt(p, stopBit))
		fillRow(p, pattern, stopBit);
}

void GraphicsMan_v2::fill(Common::SeekableReadStream &pic) {
	byte pattern;
	READ_BYTE(pattern);

	while (true) {
		Common::Point p;
		READ_POINT(p);

		if (_bounds.contains(p))
			fillAt(p, pattern);
	}
}

void GraphicsMan_v2::drawPic(Common::SeekableReadStream &pic, const Common::Point &pos) {
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
			clearScreen();
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

void GraphicsMan_v3::fillRowLeft(Common::Point p, const byte pattern, const bool stopBit) {
	byte color = getPatternColor(p, pattern);

	while (--p.x >= _bounds.left) {
		// In this version, when moving left, it no longer sets the palette first
		if (!_display.getPixelBit(p))
			return;
		if ((p.x % 7) == 6) {
			color = getPatternColor(p, pattern);
			_display.setPixelPalette(p, color);
		}
		_display.setPixelBit(p, color);
	}
}

void GraphicsMan_v3::fillAt(Common::Point p, const byte pattern) {
	// If the row at p cannot be filled, we do nothing
	if (!canFillAt(p))
			return;

	fillRow(p, pattern);

	Common::Point q(p);

	// Fill up from p
	while (--q.y >= _bounds.top && canFillAt(q))
		fillRow(q, pattern);

	// Fill down from p
	while (++p.y < _bounds.bottom && canFillAt(p))
		fillRow(p, pattern);
}

} // End of namespace Adl
