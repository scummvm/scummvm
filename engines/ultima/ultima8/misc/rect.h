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

#ifndef ULTIMA8_MISC_RECT_H
#define ULTIMA8_MISC_RECT_H

namespace Ultima {
namespace Ultima8 {

// TODO: Replace Ultima8::Rect with Common::Rect
// The key difference between Ultima8::Rect and Common::Rect is the use of int32 for variables.
// Attempts to change this may cause the game to be unstable.

struct Rect {
	int32 left, top;
	int32 right, bottom;

	Rect() : top(0), left(0), bottom(0), right(0) {}
	Rect(int x1, int y1, int x2, int y2) : top(y1), left(x1), bottom(y2), right(x2) {}

	bool operator==(const Rect &rhs) const { return equals(rhs); }
	bool operator!=(const Rect &rhs) const { return !equals(rhs); }

	int16 width() const { return right - left; }
	int16 height() const { return bottom - top; }

	void setWidth(int16 aWidth) {
		right = left + aWidth;
	}

	void setHeight(int16 aHeight) {
		bottom = top + aHeight;
	}

	void grow(int16 offset) {
		left -= offset;
		top -= offset;
		right += offset;
		bottom += offset;
	}

	// Check to see if a Rectangle is 'valid'
	bool isValidRect() const {
		return (left <= right && top <= bottom);
	}

	// Check to see if a point is within the Rectangle
	bool contains(int16 x, int16 y) const {
		return (left <= x) && (x < right) && (top <= y) && (y < bottom);
	}

	// Move the Rect (Relative)
	void translate(int32 dx, int32 dy) {
		left += dx;
		right += dx;
		top += dy;
		bottom += dy;
	}

	// Move the Rect (Absolute)
	void moveTo(int32 x, int32 y) {
		bottom += y - top;
		right += x - left;
		top = y;
		left = x;
	}

	void clip(const Rect &r) {
		if (top < r.top) top = r.top;
		else if (top > r.bottom) top = r.bottom;

		if (left < r.left) left = r.left;
		else if (left > r.right) left = r.right;

		if (bottom < r.top) bottom = r.bottom;
		else if (bottom > r.bottom) bottom = r.bottom;

		if (right < r.left) right = r.left;
		else if (right > r.right) right = r.right;
	}

	bool intersects(const Rect &r) const {
		return (left < r.right) && (r.left < right) && (top < r.bottom) && (r.top < bottom);
	}

	bool equals(const Rect &o) const {
		return left == o.left && top == o.top && right == o.right && bottom == o.bottom;
	}

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
