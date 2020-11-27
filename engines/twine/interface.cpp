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

#include "twine/interface.h"
#include "graphics/managed_surface.h"
#include "twine/twine.h"

namespace TwinE {

Interface::Interface(TwinEEngine *engine) : _engine(engine) {}

const int32 INSIDE = 0; // 0000
const int32 LEFT = 1;   // 0001
const int32 RIGHT = 2;  // 0010
const int32 TOP = 4;    // 0100
const int32 BOTTOM = 8; // 1000

int32 Interface::checkClipping(int32 x, int32 y) {
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
	uint8 currentLineColor = lineColor;

	// draw line from left to right
	if (startWidth > endWidth) {
		int32 temp = endWidth;
		endWidth = startWidth;
		startWidth = temp;

		temp = endHeight;
		endHeight = startHeight;
		startHeight = temp;
	}

	// Perform proper clipping (CohenSutherland algorithm)
	int32 outcode0 = checkClipping(startWidth, startHeight);
	int32 outcode1 = checkClipping(endWidth, endHeight);

	while ((outcode0 | outcode1) != INSIDE) {
		if ((outcode0 & outcode1) != INSIDE && outcode0 != INSIDE) {
			return; // Reject lines which are behind one clipping plane
		}

		// At least one endpoint is outside the clip rectangle; pick it.
		int32 outcodeOut = outcode0 ? outcode0 : outcode1;

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

	int32 flag2 = SCREEN_WIDTH;
	endWidth -= startWidth;
	endHeight -= startHeight;
	if (endHeight < 0) {
		flag2 = -flag2;
		endHeight = -endHeight;
	}

	uint8 *out = (uint8*)_engine->frontVideoBuffer.getBasePtr(startWidth, startHeight);

	uint8 color = currentLineColor;
	if (endWidth < endHeight) { // significant slope
		int16 xchg = endWidth;
		endWidth = endHeight;
		endHeight = xchg;
		int16 var2 = endWidth;
		var2 <<= 1;
		startHeight = endWidth;
		endHeight <<= 1;
		endWidth++;
		do {
			*out = color;
			startHeight -= endHeight;
			if (startHeight > 0) {
				out += flag2;
			} else {
				startHeight += var2;
				out += flag2 + 1;
			}
		} while (--endWidth);
	} else { // reduced slope
		int16 var2 = endWidth;
		var2 <<= 1;
		startHeight = endWidth;
		endHeight <<= 1;
		endWidth++;
		do {
			*out = color;
			out++;
			startHeight -= endHeight;
			if (startHeight < 0) {
				startHeight += var2;
				out += flag2;
			}
		} while (--endWidth);
	}
}

void Interface::blitBox(const Common::Rect &rect, const Graphics::ManagedSurface &source, Graphics::ManagedSurface &dest) {
	int32 left = rect.left;
	const int32 top = rect.top;
	const int32 right = rect.right;
	const int32 bottom = rect.bottom;

	const int8 *s = (const int8 *)source.getBasePtr(left, top);
	int8 *d = (int8 *)dest.getBasePtr(left, top);

	int32 width = right - left + 1;
	int32 height = bottom - top + 1;

	int32 insideLine = SCREEN_WIDTH - width;
	int32 temp3 = left;

	left >>= 2;
	temp3 &= 3;

	for (int32 j = 0; j < height; j++) {
		for (int32 i = 0; i < width; i++) {
			*(d++) = *(s++);
		}

		d += insideLine;
		s += insideLine;
	}
}

void Interface::drawTransparentBox(const Common::Rect &rect, int32 colorAdj) {
	int32 left = rect.left;
	int32 top = rect.top;
	int32 right = rect.right;
	int32 bottom = rect.bottom;

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

	if (left < SCREEN_TEXTLIMIT_LEFT) {
		left = SCREEN_TEXTLIMIT_LEFT;
	}
	if (right > SCREEN_TEXTLIMIT_RIGHT) {
		right = SCREEN_TEXTLIMIT_RIGHT;
	}
	if (top < SCREEN_TEXTLIMIT_TOP) {
		top = SCREEN_TEXTLIMIT_TOP;
	}
	if (bottom > SCREEN_TEXTLIMIT_BOTTOM) {
		bottom = SCREEN_TEXTLIMIT_BOTTOM;
	}

	uint8 *pos = (uint8*)_engine->frontVideoBuffer.getBasePtr(left, top);
	const int32 height = bottom - top;
	int32 height2 = height + 1;
	const int32 width = right - left + 1;
	const int32 pitch = SCREEN_WIDTH - width;
	const int32 localMode = colorAdj;

	do {
		int32 var1 = width;
		do {
			int8 color = *pos & 0x0F;
			const int8 color2 = *pos & 0xF0;
			color -= localMode;
			if (color < 0) {
				color = color2;
			} else {
				color += color2;
			}
			*pos++ = color;
			var1--;
		} while (var1 > 0);
		pos += pitch;
		height2--;
	} while (height2 > 0);
}

void Interface::drawSplittedBox(const Common::Rect &rect, uint8 colorIndex) {
	const int32 left = rect.left;
	const int32 top = rect.top;
	const int32 right = rect.right;
	const int32 bottom = rect.bottom;

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

	// cropping
	int32 offset = -((right - left) - SCREEN_WIDTH);

	uint8 *ptr = (uint8*)_engine->frontVideoBuffer.getBasePtr(left, top);

	for (int32 x = top; x < bottom; x++) {
		for (int32 y = left; y < right; y++) {
			*(ptr++) = colorIndex;
		}
		ptr += offset;
	}
}

void Interface::setClip(const Common::Rect &rect) {
	int32 left = rect.left;
	int32 top = rect.top;
	int32 right = rect.right;
	int32 bottom = rect.bottom;

	if (left < 0) {
		left = 0;
	}
	textWindow.left = left;

	if (top < 0) {
		top = 0;
	}
	textWindow.top = top;

	if (right >= SCREEN_WIDTH) {
		right = SCREEN_TEXTLIMIT_RIGHT;
	}
	textWindow.right = right;

	if (bottom >= SCREEN_HEIGHT) {
		bottom = SCREEN_TEXTLIMIT_BOTTOM;
	}
	textWindow.bottom = bottom;
}

void Interface::saveClip() { // saveTextWindow
	textWindowLeftSave = textWindow.left;
	textWindowTopSave = textWindow.top;
	textWindowRightSave = textWindow.right;
	textWindowBottomSave = textWindow.bottom;
}

void Interface::loadClip() { // loadSavedTextWindow
	textWindow.left = textWindowLeftSave;
	textWindow.top = textWindowTopSave;
	textWindow.right = textWindowRightSave;
	textWindow.bottom = textWindowBottomSave;
}

void Interface::resetClip() {
	textWindow.top = textWindow.left = SCREEN_TEXTLIMIT_TOP;
	textWindow.right = SCREEN_TEXTLIMIT_RIGHT;
	textWindow.bottom = SCREEN_TEXTLIMIT_BOTTOM;
}

} // namespace TwinE
