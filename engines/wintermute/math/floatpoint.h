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

#ifndef WINTERMUTE_FLOATPOINT_H
#define WINTERMUTE_FLOATPOINT_H

namespace Wintermute {

struct FloatPoint {
	float x;
	float y;
	FloatPoint() : x(0), y(0) {}
	FloatPoint(float x1, float y1) : x(x1), y(y1) {}
	bool operator==(const FloatPoint &p) const { return x == p.x && y == p.y; }
	bool operator!=(const FloatPoint  &p) const { return x != p.x || y != p.y; }
	FloatPoint operator+(const FloatPoint &delta) const { return FloatPoint (x + delta.x, y + delta.y);	}
	FloatPoint operator-(const FloatPoint &delta) const { return FloatPoint (x - delta.x, y - delta.y);	}

	FloatPoint& operator+=(const FloatPoint &delta) {
		x += delta.x;
		y += delta.y;
		return *this;
	}
	FloatPoint& operator-=(const FloatPoint &delta) {
		x -= delta.x;
		y -= delta.y;
		return *this;
	}
};

} // End of namespace Wintermute

#endif
