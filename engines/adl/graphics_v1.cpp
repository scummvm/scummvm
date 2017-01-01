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

void Graphics_v1::drawPic(Common::SeekableReadStream &pic, const Common::Point &pos) {
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
			_display.putPixel(Common::Point(x, y), 0x7f);
			bNewLine = false;
		} else {
			drawLine(Common::Point(oldX, oldY), Common::Point(x, y), 0x7f);
		}

		oldX = x;
		oldY = y;
	}
}

void Graphics_v1::drawCornerPixel(Common::Point &p, byte color, byte bits, byte quadrant) const {
	if (bits & 4)
		_display.putPixel(p, color);

	bits += quadrant;

	if (bits & 1)
		p.x += (bits & 2 ? -1 : 1);
	else
		p.y += (bits & 2 ? 1 : -1);
}

void Graphics_v1::drawCorners(Common::ReadStream &corners, const Common::Point &pos, byte rotation, byte scaling, byte color) const {
	const byte stepping[] = {
		0xff, 0xfe, 0xfa, 0xf4, 0xec, 0xe1, 0xd4, 0xc5,
		0xb4, 0xa1, 0x8d, 0x78, 0x61, 0x49, 0x31, 0x18,
		0xff
	};

	byte quadrant = rotation >> 4;
	rotation &= 0xf;
	byte xStep = stepping[rotation];
	byte yStep = stepping[(rotation ^ 0xf) + 1] + 1;

	Common::Point p(pos);

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
					drawCornerPixel(p, color, b, quadrant);
				xFrac += xStep + 1;
				if (yFrac + yStep > 255)
					drawCornerPixel(p, color, b, quadrant + 1);
				yFrac += yStep;
			}
			b >>= 3;
		} while (b != 0);
	}
}

} // End of namespace Adl
