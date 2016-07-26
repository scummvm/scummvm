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

#include "adl/display.h"
#include "adl/graphics.h"

namespace Adl {

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
		_display.putPixel(p, color);

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

} // End of namespace Adl
