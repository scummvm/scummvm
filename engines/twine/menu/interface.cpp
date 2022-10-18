/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "twine/menu/interface.h"
#include "graphics/managed_surface.h"
#include "graphics/primitives.h"
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
	if (x < _clip.left) {
		code |= LEFT;
	} else if (x > _clip.right) {
		code |= RIGHT;
	}
	if (y < _clip.top) {
		code |= TOP;
	} else if (y > _clip.bottom) {
		code |= BOTTOM;
	}
	return code;
}

bool Interface::drawLine(int32 startWidth, int32 startHeight, int32 endWidth, int32 endHeight, uint8 lineColor) {
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
			return false; // Reject lines which are behind one clipping plane
		}

		// At least one endpoint is outside the clip rectangle; pick it.
		const int32 outcodeOut = outcode0 ? outcode0 : outcode1;

		int32 x = 0;
		int32 y = 0;
		if (outcodeOut & TOP) { // point is above the clip rectangle
			x = startWidth + (int)((endWidth - startWidth) * (float)(_clip.top - startHeight) / (float)(endHeight - startHeight));
			y = _clip.top;
		} else if (outcodeOut & BOTTOM) { // point is below the clip rectangle
			x = startWidth + (int)((endWidth - startWidth) * (float)(_clip.bottom - startHeight) / (float)(endHeight - startHeight));
			y = _clip.bottom;
		} else if (outcodeOut & RIGHT) { // point is to the right of clip rectangle
			y = startHeight + (int)((endHeight - startHeight) * (float)(_clip.right - startWidth) / (float)(endWidth - startWidth));
			x = _clip.right;
		} else if (outcodeOut & LEFT) { // point is to the left of clip rectangle
			y = startHeight + (int)((endHeight - startHeight) * (float)(_clip.left - startWidth) / (float)(endWidth - startWidth));
			x = _clip.left;
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

	int32 pitch = _engine->width();
	endWidth -= startWidth;
	endHeight -= startHeight;
	if (endHeight < 0) {
		pitch = -pitch;
		endHeight = -endHeight;
	}

	uint8 *out = (uint8*)_engine->_frontVideoBuffer.getBasePtr(startWidth, startHeight);
	_engine->_frontVideoBuffer.addDirtyRect(Common::Rect(startWidth, startHeight, startWidth + endWidth, startHeight + endHeight));

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
	return true;
}

void Interface::blitBox(const Common::Rect &rect, const Graphics::ManagedSurface &source, Graphics::ManagedSurface &dest) {
	Common::Rect r(rect);
	r.right += 1;
	r.bottom += 1;
	dest.blitFrom(source, r, Common::Point(rect.left, rect.top));
}

void Interface::drawTransparentBox(const Common::Rect &rect, int32 colorAdj) {
	Common::Rect r = rect;
	r.clip(_engine->rect());
	if (r.isEmpty()) {
		return;
	}

	uint8 *pos = (uint8*)_engine->_frontVideoBuffer.getBasePtr(0, r.top);

	for (int32 y = r.top; y <= r.bottom; ++y) {
		for (int32 x = r.left; x <= r.right; ++x) {
			const int8 color = (pos[x] & 0x0F) - colorAdj;
			const int8 color2 = pos[x] & 0xF0;
			if (color < 0) {
				pos[x] = color2;
			} else {
				pos[x] = color + color2;
			}
		}
		pos += _engine->_frontVideoBuffer.pitch;
	}
	_engine->_frontVideoBuffer.addDirtyRect(r);
}

void Interface::drawFilledRect(const Common::Rect &rect, uint8 colorIndex) { // Box
	if (!rect.isValidRect()) {
		return;
	}
	Common::Rect clipped(rect.left, rect.top, rect.right + 1, rect.bottom + 1);
	if (_clip.isValidRect()) {
		clipped.clip(_clip);
	}
	_engine->_frontVideoBuffer.fillRect(clipped, colorIndex);
}

bool Interface::setClip(const Common::Rect &rect) {
	if (!_clip.isValidRect()) {
		return false;
	}
	_clip = rect;
	_clip.clip(_engine->rect());
	return true;
}

void Interface::saveClip() {
	_savedClip = _clip;
}

void Interface::loadClip() {
	_clip = _savedClip;
}

void Interface::resetClip() {
	_clip = _engine->rect();
}

} // namespace TwinE
