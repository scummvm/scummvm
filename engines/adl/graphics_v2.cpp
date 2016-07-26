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
#include "adl/adl.h"

namespace Adl {

// FIXME: Add clipping

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

void Graphics_v2::clear() {
	_display.clear(0xff);
	_color = 0;
}

void Graphics_v2::drawCorners(Common::SeekableReadStream &pic, bool yFirst) {
	Common::Point p;

	READ_POINT(p);

	if (yFirst)
		goto doYStep;

	while (true) {
		int16 n;

		READ_BYTE(n);
		n += _offset.x;

		_display.putPixel(p, _color);

		n <<= 1;
		drawLine(p, Common::Point(n, p.y), _color);
		p.x = n;

doYStep:
		READ_BYTE(n);
		n += _offset.y;

		_display.putPixel(p, _color);
		drawLine(p, Common::Point(p.x, n), _color);

		_display.putPixel(Common::Point(p.x + 1, p.y), _color);
		drawLine(Common::Point(p.x + 1, p.y), Common::Point(p.x + 1, n), _color);

		p.y = n;
	}
}

void Graphics_v2::drawRelativeLines(Common::SeekableReadStream &pic) {
	Common::Point p1;

	READ_POINT(p1);
	_display.putPixel(p1, _color);

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

void Graphics_v2::drawAbsoluteLines(Common::SeekableReadStream &pic) {
	Common::Point p1;

	READ_POINT(p1);
	_display.putPixel(p1, _color);

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

void Graphics_v2::fillRow(const Common::Point &p, bool stopBit, byte pattern) {
	const byte color = getPatternColor(p, pattern);
	_display.setPixelPalette(p, color);
	_display.setPixelBit(p, color);

	Common::Point q(p);
	byte c = color;

	while (++q.x < DISPLAY_WIDTH) {
		if ((q.x % 7) == 0) {
			c = getPatternColor(q, pattern);
			// Palette is set before the first bit is tested
			_display.setPixelPalette(q, c);
		}
		if (_display.getPixelBit(q) == stopBit)
			break;
		_display.setPixelBit(q, c);
	}

	q = p;
	c = color;
	while (--q.x >= 0) {
		if ((q.x % 7) == 6) {
			c = getPatternColor(q, pattern);
			_display.setPixelPalette(q, c);
		}
		if (_display.getPixelBit(q) == stopBit)
			break;
		_display.setPixelBit(q, c);
	}
}

// Basic flood fill
void Graphics_v2::fill(Common::SeekableReadStream &pic) {
	byte pattern;
	READ_BYTE(pattern);

	while (true) {
		Common::Point p;
		READ_POINT(p);

		bool stopBit = !_display.getPixelBit(p);

		while (--p.y >= 0) {
			if (_display.getPixelBit(p) == stopBit)
				break;
			if (_display.getPixelBit(Common::Point(p.x + 1, p.y)) == stopBit)
				break;
		}

		while (++p.y < DISPLAY_HEIGHT) {
			if (_display.getPixelBit(p) == stopBit)
				break;
			if (_display.getPixelBit(Common::Point(p.x + 1, p.y)) == stopBit)
				break;
			fillRow(p, stopBit, pattern);
		}
	}
}

void Graphics_v2::drawPic(Common::SeekableReadStream &pic, const Common::Point &pos) {
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
			clear();
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
			error("Invalid pic opcode %02x", opcode);
		}
	}
}

} // End of namespace Adl
