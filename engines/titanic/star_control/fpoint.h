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

#ifndef TITANIC_FPOINT_H
#define TITANIC_FPOINT_H

#include "common/rect.h"

namespace Titanic {

/**
 * Floating point Point class
 */
class FPoint {
public:
	float _x, _y;
public:
	FPoint() : _x(0), _y(0) {}
	FPoint(float x, float y) : _x(x), _y(y) {}
	FPoint(const Common::Point &pt) : _x(pt.x), _y(pt.y) {}

	bool operator==(const FPoint &p) const { return _x == p._x && _y == p._y; }
	bool operator!=(const FPoint &p) const { return _x != p._x || _y != p._y; }
	FPoint operator+(const FPoint &delta) const { return FPoint(_x + delta._x, _y + delta._y); }
	FPoint operator-(const FPoint &delta) const { return FPoint(_x - delta._x, _y - delta._y); }

	void operator+=(const FPoint &delta) {
		_x += delta._x;
		_y += delta._y;
	}

	void operator-=(const FPoint &delta) {
		_x -= delta._x;
		_y -= delta._y;
	}

	/**
	 * Normalises the X and Y coordinates as fractions relative to the
	 * value of the hypotenuse formed by a triangle from the origin (0,0)
	 */
	float normalize();
};

} // End of namespace Titanic

#endif /* TITANIC_FPOINT_H */
