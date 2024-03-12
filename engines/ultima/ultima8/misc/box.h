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

#ifndef ULTIMA8_MISC_BOX_H
#define ULTIMA8_MISC_BOX_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima8 {

/**
 * Represents a worldspace bounding box and manipulation of those bounds.
 * The box is built from a world point and positive dimensions
 * The box has reversed coordinates for x and y, meaning those dimensions are
 * subtracted from primary world point to calculate other points.
 */
struct Box {
	int32 _x, _y, _z;
	int32 _xd, _yd, _zd;

	Box() : _x(0), _y(0), _z(0), _xd(0), _yd(0), _zd(0) {}
	Box(int nx, int ny, int nz, int nxd, int nyd, int nzd)
		: _x(nx), _y(ny), _z(nz), _xd(nxd), _yd(nyd), _zd(nzd) {}

	// Check if the Box is empty (its width, height, or depth is 0) or invalid (its width, height, or depth are negative).
	bool isEmpty() const {
		return _xd <= 0 || _yd <= 0 || _zd <= 0;
	}

	// Check to see if a Box is 'valid'
	bool isValid() const {
		return _xd >= 0 && _yd >= 0 && _zd >= 0;
	}

	// Check to see if a point is within the Box
	bool contains(int32 px, int32 py, int32 pz) const {
		return px > _x - _xd && px <= _x &&
			   py > _y - _yd && py <= _y &&
			   pz >= _z && pz < _z + _zd;
	}

	// Check to see if a 2d point is within the XY of the Box
	bool containsXY(int32 px, int32 py) const {
		return px > _x - _xd && px <= _x &&
			   py > _y - _yd && py <= _y;
	}

	// Check to see if the box is below a point
	bool isBelow(int32 px, int32 py, int32 pz) const {
		return px > _x - _xd && px <= _x &&
			   py > _y - _yd && py <= _y &&
			   pz >= _z + _zd;
	}

	// Move the Box (Relative)
	void translate(int32 dx, int32 dy, int32 dz) {
		_x += dx;
		_y += dy;
		_z += dz;
	}

	// Move the Box (Absolute)
	void moveTo(int32 nx, int32 ny, int32 nz) {
		_x = nx;
		_y = ny;
		_z = nz;
	}

	// Resize the Box (Absolute)
	void resize(int32 nxd, int32 nyd, int32 nzd) {
		_xd = nxd;
		_yd = nyd;
		_zd = nzd;
	}

	bool overlaps(const Box &o) const {
		if (_x <= o._x - o._xd || o._x <= _x - _xd)
			return false;
		if (_y <= o._y - o._yd || o._y <= _y - _yd)
			return false;
		if (_z + _zd <= o._z || o._z + o._zd <= _z)
			return false;
		return true;
	}

	bool overlapsXY(const Box& o) const {
		if (_x <= o._x - o._xd || o._x <= _x - _xd)
			return false;
		if (_y <= o._y - o._yd || o._y <= _y - _yd)
			return false;
		return true;
	}

	void extend(const Box &o) {
		int32 x2 = MIN(_x - _xd, o._x - o._xd);
		int32 y2 = MIN(_y - _yd, o._y - o._yd);
		int32 z2 = MAX(_z + _zd, o._z + o._zd);

		_x = MAX(_x, o._x);
		_y = MAX(_y, o._y);
		_z = MIN(_z, o._z);
		_xd = _x - x2;
		_yd = _y - y2;
		_zd = z2 - _z;
	}

	bool operator==(const Box &rhs) const { return equals(rhs); }
	bool operator!=(const Box &rhs) const { return !equals(rhs); }

	bool equals(const Box &o) const {
		return (_x == o._x && _y == o._y && _z == o._z &&
		        _xd == o._xd && _yd == o._yd && _zd == o._zd);
	}

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
