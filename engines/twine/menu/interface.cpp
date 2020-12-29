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

#include "twine/menu/interface.h"
#include "graphics/managed_surface.h"
#include "twine/twine.h"

namespace TwinE {

Interface::Interface(TwinEEngine *engine) : _engine(engine) {}

const int32 INSIDE = 0; // 0000
const int32 LEFT = 1;   // 0001
const int32 RIGHT = 2;  // 0010
const int32 TOP = 4;    // 0100
const int32 BOTTOM = 8; // 1000

int32 Interface::checkClipping(int32 x, int32 y) const {
	int32 code = INSIDE;
	if (x < textWindow.left) {
		code |= LEFT;
	} else if (x > textWindow.right) {
		code |= RIGHT;
	}
	if (y < textWindow.top) {
		code |= TOP;
	} else if (y > textWindow.bottom) {
		code |= BOTTOM;
	}
	return code;
}

// TODO: check if Graphics::drawLine() works here
void Interface::drawLine(int32 startWidth, int32 startHeight, int32 endWidth, int32 endHeight, uint8 lineColor) {
	// draw line from left to right
	if (startWidth > endWidth) {
		SWAP(endWidth, startWidth);
		SWAP(endHeight, startHeight);
	}

	// Perform proper clipping (CohenSutherland algorithm)
	int32 outcode0 = checkClipping(startWidth, startHeight);
	int32 outcode1 = checkClipping(endWidth, endHeight);

	while ((outcode0 | outcode1) != INSIDE) {
		if ((outcode0 & outcode1) != INSIDE && outcode0 != INSIDE) {
			return; // Reject lines which are behind one clipping plane
		}

		// At least one endpoint is outside the clip rectangle; pick it.
		const int32 outcodeOut = outcode0 ? outcode0 : outcode1;

		int32 x = 0;
		int32 y = 0;
		if (outcodeOut & TOP) { // point is above the clip rectangle
			x = startWidth + (int)((endWidth - startWidth) * (float)(textWindow.top - startHeight) / (float)(endHeight - startHeight));
			y = textWindow.top;
		} else if (outcodeOut & BOTTOM) { // point is below the clip rectangle
			x = startWidth + (int)((endWidth - startWidth) * (float)(textWindow.bottom - startHeight) / (float)(endHeight - startHeight));
			y = textWindow.bottom;
		} else if (outcodeOut & RIGHT) { // point is to the right of clip rectangle
			y = startHeight + (int)((endHeight - startHeight) * (float)(textWindow.right - startWidth) / (float)(endWidth - startWidth));
			x = textWindow.right;
		} else if (outcodeOut & LEFT) { // point is to the left of clip rectangle
			y = startHeight + (int)((endHeight - startHeight) * (float)(textWindow.left - startWidth) / (float)(endWidth - startWidth));
			x = textWindow.left;
		}

		// Clip the point
		if (outcodeOut == outcode0) {
			startWidth = x;
			startHeight = y;
			outcode0 = checkClipping(startWidth, startHeight);
		} else {
			endWidth = x;
			endHeight = y;
			outcode1 = checkClipping(endWidth, endHeight);
		}
	}

	int32 pitch = SCREEN_WIDTH;
	endWidth -= startWidth;
	endHeight -= startHeight;
	if (endHeight < 0) {
		pitch = -pitch;
		endHeight = -endHeight;
	}

	uint8 *out = (uint8*)_engine->frontVideoBuffer.getBasePtr(startWidth, startHeight);

	if (endWidth < endHeight) { // significant slope
		SWAP(endWidth, endHeight);
		const int16 var2 = endWidth << 1;
		startHeight = endWidth;
		endHeight <<= 1;
		endWidth++;
		do {
			*out = lineColor;
			startHeight -= endHeight;
			if (startHeight > 0) {
				out += pitch;
			} else {
				startHeight += var2;
				out += pitch + 1;
			}
		} while (--endWidth);
	} else { // reduced slope
		const int16 var2 = endWidth << 1;
		startHeight = endWidth;
		endHeight <<= 1;
		endWidth++;
		do {
			*out++ = lineColor;
			startHeight -= endHeight;
			if (startHeight < 0) {
				startHeight += var2;
				out += pitch;
			}
		} while (--endWidth);
	}
}

void Interface::blitBox(const Common::Rect &rect, const Graphics::ManagedSurface &source, Graphics::ManagedSurface &dest) {
	dest.blitFrom(source, rect, Common::Point(rect.left, rect.top));
}

void Interface::drawTransparentBox(const Common::Rect &rect, int32 colorAdj) {
	const int32 left = MAX((int32)SCREEN_TEXTLIMIT_LEFT, (int32)rect.left);
	const int32 top = MAX((int32)SCREEN_TEXTLIMIT_TOP, (int32)rect.top);
	const int32 right = MIN((int32)SCREEN_TEXTLIMIT_RIGHT, (int32)rect.right);
	const int32 bottom = MIN((int32)SCREEN_TEXTLIMIT_BOTTOM, (int32)rect.bottom);

	if (left > SCREEN_TEXTLIMIT_RIGHT) {
		return;
	}
	if (right < SCREEN_TEXTLIMIT_LEFT) {
		return;
	}
	if (top > SCREEN_TEXTLIMIT_BOTTOM) {
		return;
	}
	if (bottom < SCREEN_TEXTLIMIT_TOP) {
		return;
	}

	uint8 *pos = (uint8*)_engine->frontVideoBuffer.getBasePtr(0, top);

	for (int32 y = top; y < bottom; ++y) {
		for (int32 x = left; x < right; ++x) {
			const int8 color = (pos[x] & 0x0F) - colorAdj;
			const int8 color2 = pos[x] & 0xF0;
			if (color < 0) {
				pos[x] = color2;
			} else {
				pos[x] = color + color2;
			}
		}
		pos += _engine->frontVideoBuffer.pitch;
	}
}

void Interface::drawSplittedBox(const Common::Rect &rect, uint8 colorIndex) {
	_engine->frontVideoBuffer.fillRect(rect, colorIndex);
}

void Interface::setClip(const Common::Rect &rect) {
	textWindow.left = MAX((int32)SCREEN_TEXTLIMIT_LEFT, (int32)rect.left);
	textWindow.top = MAX((int32)SCREEN_TEXTLIMIT_TOP, (int32)rect.top);
	textWindow.right = MIN((int32)SCREEN_TEXTLIMIT_RIGHT, (int32)rect.right);
	textWindow.bottom = MIN((int32)SCREEN_TEXTLIMIT_BOTTOM, (int32)rect.bottom);
}

void Interface::saveClip() {
	textWindowSave = textWindow;
}

void Interface::loadClip() {
	textWindow = textWindowSave;
}

void Interface::resetClip() {
	textWindow.top = SCREEN_TEXTLIMIT_TOP;
	textWindow.left = SCREEN_TEXTLIMIT_LEFT;
	textWindow.right = SCREEN_TEXTLIMIT_RIGHT;
	textWindow.bottom = SCREEN_TEXTLIMIT_BOTTOM;
}

} // namespace TwinE
