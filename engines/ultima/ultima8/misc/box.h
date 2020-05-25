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

#ifndef ULTIMA8_MISC_BOX_H
#define ULTIMA8_MISC_BOX_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima8 {

struct Box {
	int32 _x, _y, _z;
	int32 _xd, _yd, _zd;

	Box() : _x(0), _y(0), _z(0), _xd(0), _yd(0), _zd(0) {}
	Box(int nx, int ny, int nz, int nxd, int nyd, int nzd)
		: _x(nx), _y(ny), _z(nz), _xd(nxd), _yd(nyd), _zd(nzd) {}

	void    Set(int nx, int ny, int nz, int nxd, int nyd, int nzd) {
		_x = nx;
		_y = ny;
		_z = nz;
		_xd = nxd;
		_yd = nyd;
		_zd = nzd;
	}
	void    Set(Box &o) {
		*this = o;
	}

	// Check to see if a Box is 'valid'
	bool    IsValid() const {
		return _xd > 0 && _yd > 0 && _zd > 0;
	}

	// Check to see if a point is within the Box
	bool    InBox(int px, int py, int pz) const {
		return (px > (_x - _xd) && py > (_y - _yd) && pz >= _z &&
		        px <= _x && py <= _y && pz < (_z + _zd));
	}

	// Move the Box (Relative)
	void    MoveRel(int32 dx, int32 dy, int32 dz) {
		_x += dx;
		_y += dy;
		_z += dz;
	}

	// Move the Box (Absolute)
	void    MoveAbs(int32 nx, int32 ny, int32 nz) {
		_x = nx;
		_y = ny;
		_z = nz;
	}

	// Resize the Box (Relative)
	void    ResizeRel(int32 dxd, int32 dyd, int32 dzd) {
		_xd += dxd;
		_yd += dyd;
		_zd += dzd;
	}

	// Resize the Box (Absolute)
	void    ResizeAbs(int32 nxd, int32 nyd, int32 nzd) {
		_xd = nxd;
		_yd = nyd;
		_zd = nzd;
	}

	bool    Overlaps(const Box &o) const {
		if (_x <= o._x - o._xd || o._x <= _x - _xd) return false;
		if (_y <= o._y - o._yd || o._y <= _y - _yd) return false;
		if (_z + _zd <= o._z || o._z + o._zd <= _z) return false;
		return true;
	}

	bool operator == (const Box &o) const {
		return (_x == o._x && _y == o._y && _z == o._z &&
		        _xd == o._xd && _yd == o._yd && _zd == o._zd);
	}

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
