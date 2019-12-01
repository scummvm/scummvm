/*
Copyright (C) 2005 The Pentagram Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef ULTIMA8_MISC_BOX_H
#define ULTIMA8_MISC_BOX_H

namespace Ultima8 {
namespace Pentagram {

struct Box {
	int32       x, y, z;
	int32       xd, yd, zd;

	Box() : x(0), y(0), z(0), xd(0), yd(0), zd(0) {}
	Box(int nx, int ny, int nz, int nxd, int nyd, int nzd)
		: x(nx), y(ny), z(nz), xd(nxd), yd(nyd), zd(nzd) {}
	Box(const Box &o) : x(o.x), y(o.y), z(o.z), xd(o.xd), yd(o.yd), zd(o.zd) {}

	void    Set(int nx, int ny, int nz, int nxd, int nyd, int nzd) {
		x = nx;
		y = ny;
		z = nz;
		xd = nxd;
		yd = nyd;
		zd = nzd;
	}
	void    Set(Box &o) {
		*this = o;
	}

	// Check to see if a Box is 'valid'
	bool    IsValid() const {
		return xd > 0 && yd > 0 && zd > 0;
	}

	// Check to see if a point is within the Box
	bool    InBox(int px, int py, int pz) const {
		return (px >= (x - xd) && py >= (y - yd) && pz >= z &&
		        px < x && py < y && pz < (z + zd));
	}

	// Move the Box (Relative)
	void    MoveRel(int32 dx, int32 dy, int32 dz) {
		x += dx;
		y += dy;
		z += dz;
	}

	// Move the Box (Absolute)
	void    MoveAbs(int32 nx, int32 ny, int32 nz) {
		x = nx;
		y = ny;
		z = nz;
	}

	// Resize the Box (Relative)
	void    ResizeRel(int32 dxd, int32 dyd, int32 dzd) {
		xd += dxd;
		yd += dyd;
		zd += dzd;
	}

	// Resize the Box (Absolute)
	void    ResizeAbs(int32 nxd, int32 nyd, int32 nzd) {
		xd = nxd;
		yd = nyd;
		zd = nzd;
	}

	bool    Overlaps(const Box &o) const {
		if (x <= o.x - o.xd || o.x <= x - xd) return false;
		if (y <= o.y - o.yd || o.y <= y - yd) return false;
		if (z + zd <= o.z || o.z + o.zd <= z) return false;
		return true;
	}

	bool operator == (const Box &o) const {
		return (x == o.x && y == o.y && z == o.z &&
		        xd == o.xd && yd == o.yd && zd == o.zd);
	}

};

} // End of namespace Pentagram
} // End of namespace Ultima8

#endif
