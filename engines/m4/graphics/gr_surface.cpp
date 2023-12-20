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

#include "common/algorithm.h"
#include "m4/graphics/gr_surface.h"
#include "m4/graphics/gr_sprite.h"

namespace M4 {

M4Surface::M4Surface(int sw, int sh) : Buffer() {
	this->w = sw;
	this->h = sh;
	this->stride = sw;
	this->encoding = NO_COMPRESS;
	this->data = new byte[sw * sh];
	Common::fill(this->data, this->data + sw * sh, 0);
	_disposeAfterUse = DisposeAfterUse::YES;
}

M4Surface::M4Surface(const byte *src, int sw, int sh) {
	this->w = sw;
	this->h = sh;
	this->stride = sw;
	this->encoding = NO_COMPRESS;
	this->data = new byte[sw * sh];
	Common::fill(this->data, this->data + sw * sh, 0);
	_disposeAfterUse = DisposeAfterUse::YES;

	rleDraw(src);
}

M4Surface::~M4Surface() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete[] data;
}

void M4Surface::rleDraw(const byte *src, int x, int y) {
	const byte *srcP = src;
	byte *destData = data + y * w + x;
	byte *destP = destData;
	int destWidth = w;
	byte count, val;
	int line = 0;

	assert(x >= 0 && y >= 0 && x < w && y < h);

	for (;;) {
		count = *srcP++;

		if (count) {
			// Basic run length
			val = *srcP++;

			// 0 pixels are transparent, and are skipped. Otherwise, draw pixels
			if (val != 0)
				Common::fill(destP, destP + count, val);
			destP += count;

		} else {
			count = *srcP++;

			if (count >= 3) {
				// Block of uncompressed pixels to copy
				for (; count > 0; --count, ++destP) {
					val = *srcP++;
					if (val != 0)
						*destP = val;
				}

			} else if (!(count & 3)) {
				// End of line code
				++line;
				destP = destData + line * destWidth;

			} else {
				// Stop drawing image. Seems weird that it doesn't handle the X/Y offset
				// form for count & 2, but the original explicitly doesn't implement it
				break;
			}
		}
	}

	assert(destP <= (data + h * stride));
}

void M4Surface::draw(const Buffer &src, int x, int y, bool forwards,
		const byte *depthCodes, int srcDepth) {
	if ((src.encoding & 0x7f) == RLE8) {
		// The standard case of RLE sprite drawing onto screen can directly
		// use RLE decompression for performance
		if (forwards && !depthCodes && !depthCodes && x >= 0 && y >= 0 &&
				(x + src.w) <= this->w && (y + src.h) <= this->h) {
			rleDraw(src.data, x, y);

		} else {
			// All other RLE drawing first decompresses the sprite, and then does
			// the various clipping, reverse, etc. on that
			M4Surface tmp(src.data, src.w, src.h);
			drawInner(tmp, depthCodes, x, y, forwards, srcDepth);
		}
	} else {
		// Uncompressed images get passed to inner drawing
		drawInner(src, depthCodes, x, y, forwards, srcDepth);
	}
}

void M4Surface::drawInner(const Buffer &src, const byte *depthCodes,
		int x, int y, bool forwards, int srcDepth) {
	assert((src.encoding & 0x7f) == NO_COMPRESS);

	for (int srcY = 0; srcY < src.h; ++srcY, ++y) {
		if (y >= h)
			// Below bottom of screen
			break;
		else if (y < 0)
			// Above top of screen
			continue;

		const byte *srcP = forwards ? src.getBasePtr(0, srcY) : src.getBasePtr(src.w - 1, srcY);
		byte *destP = getBasePtr(x, y);
		const byte *depthP = depthCodes ? depthCodes + y * w + x : nullptr;
		int deltaX = forwards ? 1 : -1;
		int destX = x;

		for (int srcX = 0; srcX < src.w; ++srcX, srcP += deltaX, ++destX) {
			if (destX >= w)
				// Beyond right of screen
				break;

			byte v = *srcP;
			if (destX >= 0 && v != 0 && (!depthP || *depthP == 0 || srcDepth < *depthP)) {
				*destP = v;
			}

			++destP;
			if (depthP)
				++depthP;
		}
	}
}

} // namespace M4
