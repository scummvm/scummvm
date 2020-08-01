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

#ifndef ULTIMA8_MISC_POINT3_H
#define ULTIMA8_MISC_POINT3_H

namespace Ultima {
namespace Ultima8 {

struct Point3 {
	int32 x, y, z;

	Point3() : x(0), y(0), z(0) {}
	Point3(int32 nx, int32 ny, int32 nz) : x(nx), y(ny), z(nz) {}

	int maxDistXYZ(const Point3 &other) const {
		int xdiff = abs(x - other.x);
		int ydiff = abs(y - other.y);
		int zdiff = abs(z - other.z);
		return MAX(xdiff, MAX(ydiff, zdiff));
	}

	void set(int32 nx, int32 ny, int32 nz) {
		x = nx;
		y = ny;
		z = nz;
	}

	void move(int32 dx, int32 dy, int32 dz) {
		x += dx;
		y += dy;
		z += dz;
	}

	bool loadData(Common::ReadStream *rs, uint32 version) {
		x = rs->readSint32LE();
		y = rs->readSint32LE();
		z = rs->readSint32LE();
		return true;
	}

	void saveData(Common::WriteStream *ws) {
		ws->writeSint32LE(x);
		ws->writeSint32LE(y);
		ws->writeSint32LE(z);
	}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
