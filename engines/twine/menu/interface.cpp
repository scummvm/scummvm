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

bool Interface::drawLine(int32 x0, int32 y0, int32 x1, int32 y1, uint8 color) {
	// always from left to right
	if (x0 > x1) {
		SWAP(x0, x1);
		SWAP(y0, y1);
	}

	const int32 cright = _clip.right;
	const int32 cleft = _clip.left;
	const int32 cbottom = _clip.bottom;
	const int32 ctop = _clip.top;

	uint16 clipFlags;
	do {
		if (x0 > cright || x1 < cleft) {
			return false;
		}

		int32 dx = x1 - x0;
		int32 dy = y1 - y0;

		clipFlags = 0;

		if (x0 < cleft) {
			clipFlags |= 0x100;
		}

		if (y0 < ctop) {
			clipFlags |= 0x800;
		} else if (y0 > cbottom) {
			clipFlags |= 0x400;
		}

		if (x1 > cright) {
			clipFlags |= 0x2;
		}

		if (y1 < ctop) {
			clipFlags |= 0x8;
		} else if (y1 > cbottom) {
			clipFlags |= 0x4;
		}

		if (clipFlags & (clipFlags >> 8)) {
			return false;
		}

		if (clipFlags) {
			if (clipFlags & 0x100) {
				y0 += ((cleft - x0) * dy) / dx;
				x0 = cleft;
			} else if (clipFlags & 0x800) {
				x0 += ((ctop - y0) * dx) / dy;
				y0 = ctop;
			} else if (clipFlags & 0x400) {
				x0 += ((cbottom - y0) * dx) / dy;
				y0 = cbottom;
			} else if (clipFlags & 0x2) {
				y1 = (((cright - x0) * dy) / dx) + y0;
				x1 = cright;
			} else if (clipFlags & 0x8) {
				x1 = (((ctop - y0) * dx) / dy) + x0;
				y1 = ctop;
			} else if (clipFlags & 0x4) {
				x1 = (((cbottom - y0) * dx) / dy) + x0;
				y1 = cbottom;
			}
		}
	} while (clipFlags);

	int16 lineOffset = (int16)_engine->width();
	x1 -= x0;
	y1 -= y0;
	if (y1 < 0) {
		lineOffset = -lineOffset;
		y1 = -y1;
	}

	byte *out = (byte *)_engine->_frontVideoBuffer.getBasePtr(x0, y0);

	if (x1 < y1) {
		// vertical
		SWAP(x1, y1);

		int32 dy = x1 << 1;
		y0 = x1;
		y1 <<= 1;
		x1++;

		do {
			*out = color;
			y0 -= y1;
			out += lineOffset;
			if (y0 < 0) {
				y0 += dy;
				out++;
			}
		} while (--x1);
	} else {
		// horizontal
		int32 dy = x1 << 1;
		y0 = x1;
		y1 <<= 1;
		x1++;

		do {
			*out++ = color;
			y0 -= y1;
			if (y0 < 0) {
				y0 += dy;
				out += lineOffset;
			}
		} while (--x1);
	}

	_engine->_frontVideoBuffer.addDirtyRect(Common::Rect(MIN<int16>(x0, x1), MIN<int16>(y0, y1), MAX<int16>(x0, x1), MAX<int16>(y0, y1)));

	return true;
}

void Interface::blitBox(const Common::Rect &rect, const Graphics::ManagedSurface &source, Graphics::ManagedSurface &dest) {
	Common::Rect r(rect);
	r.right += 1;
	r.bottom += 1;
	dest.blitFrom(source, r, Common::Point(rect.left, rect.top));
}

void Interface::shadeBox(const Common::Rect &rect, int32 colorAdj) {
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

void Interface::box(const Common::Rect &rect, uint8 colorIndex) { // Box
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

void Interface::memoClip() {
	_memoClip = _clip;
}

void Interface::restoreClip() {
	_clip = _memoClip;
}

void Interface::unsetClip() {
	_clip = _engine->rect();
}

} // namespace TwinE
