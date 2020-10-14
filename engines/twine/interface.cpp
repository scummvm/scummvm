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
	if (x < textWindowLeft)
		code |= LEFT;
	else if (x > textWindowRight)
		code |= RIGHT;
	if (y < textWindowTop)
		code |= TOP;
	else if (y > textWindowBottom)
		code |= BOTTOM;
	return code;
}

// TODO: check if Graphics::drawLine() works here
void Interface::drawLine(int32 startWidth, int32 startHeight, int32 endWidth, int32 endHeight, int32 lineColor) {
	int32 temp;
	int32 flag2;
	uint8 *out;
	int16 color;
	int16 var2;
	int16 xchg;
	int32 outcode0, outcode1;
	int32 x, y, outcodeOut;
	int32 currentLineColor = lineColor;

	// draw line from left to right
	if (startWidth > endWidth) {
		temp = endWidth;
		endWidth = startWidth;
		startWidth = temp;

		temp = endHeight;
		endHeight = startHeight;
		startHeight = temp;
	}

	// Perform proper clipping (CohenSutherland algorithm)
	outcode0 = checkClipping(startWidth, startHeight);
	outcode1 = checkClipping(endWidth, endHeight);

	while ((outcode0 | outcode1) != 0) {
		if (((outcode0 & outcode1) != 0) && (outcode0 != INSIDE))
			return; // Reject lines which are behind one clipping plane

		// At least one endpoint is outside the clip rectangle; pick it.
		outcodeOut = outcode0 ? outcode0 : outcode1;

		if (outcodeOut & TOP) { // point is above the clip rectangle
			x = startWidth + (int)((endWidth - startWidth) * (float)(textWindowTop - startHeight) / (float)(endHeight - startHeight));
			y = textWindowTop;
		} else if (outcodeOut & BOTTOM) { // point is below the clip rectangle
			x = startWidth + (int)((endWidth - startWidth) * (float)(textWindowBottom - startHeight) / (float)(endHeight - startHeight));
			y = textWindowBottom;
		} else if (outcodeOut & RIGHT) { // point is to the right of clip rectangle
			y = startHeight + (int)((endHeight - startHeight) * (float)(textWindowRight - startWidth) / (float)(endWidth - startWidth));
			x = textWindowRight;
		} else if (outcodeOut & LEFT) { // point is to the left of clip rectangle
			y = startHeight + (int)((endHeight - startHeight) * (float)(textWindowLeft - startWidth) / (float)(endWidth - startWidth));
			x = textWindowLeft;
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

	flag2 = 640; //SCREEN_WIDTH;
	endWidth -= startWidth;
	endHeight -= startHeight;
	if (endHeight < 0) {
		flag2 = -flag2;
		endHeight = -endHeight;
	}

	out = _engine->frontVideoBuffer + _engine->screenLookupTable[startHeight] + startWidth;

	color = currentLineColor;
	if (endWidth < endHeight) { // significant slope
		xchg = endWidth;
		endWidth = endHeight;
		endHeight = xchg;
		var2 = endWidth;
		var2 <<= 1;
		startHeight = endWidth;
		endHeight <<= 1;
		endWidth++;
		do {
			*out = (uint8)color;
			startHeight -= endHeight;
			if (startHeight > 0) {
				out += flag2;
			} else {
				startHeight += var2;
				out += flag2 + 1;
			}
		} while (--endWidth);
	} else { // reduced slope
		var2 = endWidth;
		var2 <<= 1;
		startHeight = endWidth;
		endHeight <<= 1;
		endWidth++;
		do {
			*out = (uint8)color;
			out++;
			startHeight -= endHeight;
			if (startHeight < 0) {
				startHeight += var2;
				out += flag2;
			}
		} while (--endWidth);
	}
}

void Interface::blitBox(int32 left, int32 top, int32 right, int32 bottom, int8 *source, int32 leftDest, int32 topDest, int8 *dest) {
	int32 width;
	int32 height;
	int8 *s;
	int8 *d;
	int32 insideLine;
	int32 temp3;
	int32 i;
	int32 j;

	s = _engine->screenLookupTable[top] + source + left;
	d = _engine->screenLookupTable[topDest] + dest + leftDest;

	width = right - left + 1;
	height = bottom - top + 1;

	insideLine = SCREEN_WIDTH - width;
	temp3 = left;

	left >>= 2;
	temp3 &= 3;

	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			*(d++) = *(s++);
		}

		d += insideLine;
		s += insideLine;
	}
}

void Interface::drawTransparentBox(int32 left, int32 top, int32 right, int32 bottom, int32 colorAdj) {
	uint8 *pos;
	int32 width;
	int32 height;
	int32 height2;
	int32 temp;
	int32 localMode;
	int32 var1;
	int8 color;
	int8 color2;

	if (left > SCREEN_TEXTLIMIT_RIGHT)
		return;
	if (right < SCREEN_TEXTLIMIT_LEFT)
		return;
	if (top > SCREEN_TEXTLIMIT_BOTTOM)
		return;
	if (bottom < SCREEN_TEXTLIMIT_TOP)
		return;

	if (left < SCREEN_TEXTLIMIT_LEFT)
		left = SCREEN_TEXTLIMIT_LEFT;
	if (right > SCREEN_TEXTLIMIT_RIGHT)
		right = SCREEN_TEXTLIMIT_RIGHT;
	if (top < SCREEN_TEXTLIMIT_TOP)
		top = SCREEN_TEXTLIMIT_TOP;
	if (bottom > SCREEN_TEXTLIMIT_BOTTOM)
		bottom = SCREEN_TEXTLIMIT_BOTTOM;

	pos = _engine->screenLookupTable[top] + _engine->frontVideoBuffer + left;
	height2 = height = bottom - top;
	height2++;

	width = right - left + 1;

	temp = 640 - width; // SCREEN_WIDTH
	localMode = colorAdj;

	do {
		var1 = width;
		do {
			color2 = color = *pos;
			color2 &= 0xF0;
			color &= 0x0F;
			color -= localMode;
			if (color < 0)
				color = color2;
			else
				color += color2;
			*pos++ = color;
			var1--;
		} while (var1 > 0);
		pos += temp;
		height2--;
	} while (height2 > 0);
}

void Interface::drawSplittedBox(int32 left, int32 top, int32 right, int32 bottom, uint8 e) { // Box
	uint8 *ptr;

	int32 offset;

	int32 x;
	int32 y;

	if (left > SCREEN_TEXTLIMIT_RIGHT)
		return;
	if (right < SCREEN_TEXTLIMIT_LEFT)
		return;
	if (top > SCREEN_TEXTLIMIT_BOTTOM)
		return;
	if (bottom < SCREEN_TEXTLIMIT_TOP)
		return;

	// cropping
	offset = -((right - left) - SCREEN_WIDTH);

	ptr = _engine->frontVideoBuffer + _engine->screenLookupTable[top] + left;

	for (x = top; x < bottom; x++) {
		for (y = left; y < right; y++) {
			*(ptr++) = e;
		}
		ptr += offset;
	}
}

void Interface::setClip(int32 left, int32 top, int32 right, int32 bottom) {
	if (left < 0)
		left = 0;
	textWindowLeft = left;

	if (top < 0)
		top = 0;
	textWindowTop = top;

	if (right >= SCREEN_WIDTH)
		right = SCREEN_TEXTLIMIT_RIGHT;
	textWindowRight = right;

	if (bottom >= SCREEN_HEIGHT)
		bottom = SCREEN_TEXTLIMIT_BOTTOM;
	textWindowBottom = bottom;
}

void Interface::saveClip() { // saveTextWindow
	textWindowLeftSave = textWindowLeft;
	textWindowTopSave = textWindowTop;
	textWindowRightSave = textWindowRight;
	textWindowBottomSave = textWindowBottom;
}

void Interface::loadClip() { // loadSavedTextWindow
	textWindowLeft = textWindowLeftSave;
	textWindowTop = textWindowTopSave;
	textWindowRight = textWindowRightSave;
	textWindowBottom = textWindowBottomSave;
}

void Interface::resetClip() {
	textWindowTop = textWindowLeft = SCREEN_TEXTLIMIT_TOP;
	textWindowRight = SCREEN_TEXTLIMIT_RIGHT;
	textWindowBottom = SCREEN_TEXTLIMIT_BOTTOM;
}

} // namespace TwinE
