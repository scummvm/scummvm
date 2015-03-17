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

#include "sherlock/graphics.h"
#include "sherlock/sherlock.h"
#include "common/system.h"
#include "graphics/palette.h"

namespace Sherlock {

Surface::Surface(uint16 width, uint16 height) {
	create(width, height, Graphics::PixelFormat::createFormatCLUT8());
}

Surface::~Surface() {
    free();
}

/**
 * Draws a surface into another
 */
void Surface::copyFrom(const Graphics::Surface &src) {
	copyFrom(src, Common::Point());
}

/**
 * Draws a surface at a given position within this surface
 */
void Surface::copyFrom(const Graphics::Surface &src, const Common::Point &pt) {
	Common::Rect drawRect(0, 0, src.w, src.h);
	Common::Point destPt = pt;
	
	if (destPt.x < 0) {
		drawRect.left += -destPt.x;
		destPt.x = 0;
	}
	if (destPt.y < 0) {
		drawRect.top += -destPt.y;
		destPt.y = 0;
	}
	int right = destPt.x + src.w;
	if (right > this->w) {
		drawRect.right -= (right - this->w);
	}
	int bottom = destPt.y + src.h;
	if (bottom > this->h) {
		drawRect.bottom -= (bottom - this->h);
	}

	if (drawRect.isValidRect()) {
		addDirtyRect(Common::Rect(destPt.x, destPt.y, destPt.x + drawRect.width(),
			destPt.y + drawRect.height()));
		copyRectToSurface(src, destPt.x, destPt.y, drawRect);
	}
}

void Surface::fillRect(int x1, int y1, int x2, int y2, byte color) {
    Graphics::Surface::fillRect(Common::Rect(x1, y1, x2, y2), color);
}

} // End of namespace Sherlock
