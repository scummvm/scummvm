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

struct Rect {
	int32 left, top;
	int32 right, bottom;

	Rect() : left(0), top(0), right(0), bottom(0) {}
	Rect(int nx, int ny, int nw, int nh) : left(nx), top(ny), right(nx + nw), bottom(ny + nh) {}

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

	void    Set(int nx, int ny, int nw, int nh) {
		left = nx;
		top = ny;
		right = nx + nw;
		bottom = ny + nh;
	}
	void    Set(Rect &o) {
		*this = o;
	}

	// Check to see if a Rectangle is 'valid'
	bool    IsValid() const {
		return right > left && bottom > top;
	}

	// Check to see if a point is within the Rectangle
	bool    InRect(int px, int py) const {
		return px >= left && py >= top && px < right && py < bottom;
	}

	// Move the Rect (Relative)
	void    MoveRel(int32 dx, int32 dy) {
		left += dx;
		top += dy;
		right += dx;
		bottom += dy;
	}

	// Move the Rect (Absolute)
	void    MoveAbs(int32 nx, int32 ny) {
		right += nx - left;
		bottom += ny - top;
		left = nx;
		top = ny;
	}

	// Resize the Rect (Relative)
	void    ResizeRel(int32 dw, int32 dh) {
		right += dw;
		bottom += dh;
	}

	// Resize the Rect (Absolute)
	void    ResizeAbs(int32 nw, int32 nh) {
		right = left + nw;
		bottom = top + nh;
	}

	// Intersect/Clip this rect with another
	void    Intersect(int ox, int oy, int ow, int oh) {
		int x2 = right,     y2 = bottom;
		int ox2 = ox + ow,  oy2 = oy + oh;

		if (left < ox) left = ox;
		else if (left > ox2) left = ox2;

		if (x2 < ox) x2 = ox;
		else if (x2 > ox2) x2 = ox2;

		if (top < oy) top = oy;
		else if (top > oy2) top = oy2;

		if (y2 < oy) y2 = oy;
		else if (y2 > oy2) y2 = oy2;

		right = x2;
		bottom = y2;
	}

	// Intersect/Clip this another with this
	template<typename T>
	void IntersectOther(T &ox, T &oy, T &ow, T &oh) const {
		int x2 = right,     y2 = bottom;
		int ox2 = ox + ow,  oy2 = oy + oh;

		if (ox < left) ox = left;
		else if (ox > x2) ox = x2;

		if (ox2 < left) ox2 = left;
		else if (ox2 > x2) ox2 = x2;

		if (oy < top) oy = top;
		else if (oy > y2) oy = y2;

		if (oy2 < top) oy2 = top;
		else if (oy2 > y2) oy2 = y2;

		ow = ox2 - ox;
		oh = oy2 - oy;
	}

	// Intersect/Clip this rect with another
	void    Intersect(const Rect &o) {
		Intersect(o.left, o.top, o.width(), o.height());
	}

	// Union/Add this rect with another
	void    Union(int ox, int oy, int ow, int oh) {
		int x2 = right,     y2 = bottom;
		int ox2 = ox + ow,  oy2 = oy + oh;

		if (ox < left) left = ox;
		else if (ox2 > x2) x2 = ox2;

		if (oy < top) top = ox;
		else if (oy2 > y2) y2 = ox2;

		right = x2;
		bottom = y2;
	}

	// Union/Add this rect with another
	void    Union(const Rect &o) {
		Union(o.left, o.top, o.width(), o.height());
	}

	bool    Overlaps(const Rect &o) const {
		if (right <= o.left || o.right <= left) return false;
		if (bottom <= o.top || o.bottom <= top) return false;
		return true;
	}

	// Operator +=
	Rect &operator += (const Rect &o) {
		Union(o.left, o.top, o.width(), o.height());
		return *(this);
	}

	// Operator +
	Rect &operator + (const Rect &o) const {
		Rect result(*this);
		return (result += o);
	}

	bool equals(const Rect &o) const {
		return left == o.left && top == o.top && right == o.right && bottom == o.bottom;
	}

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
