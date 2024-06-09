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

#ifndef ULTIMA8_MISC_POINT3_H
#define ULTIMA8_MISC_POINT3_H

namespace Ultima {
namespace Ultima8 {

struct Point3 {
	int32 x, y, z;

	Point3() : x(0), y(0), z(0) {}
	Point3(int32 nx, int32 ny, int32 nz) : x(nx), y(ny), z(nz) {}

	bool operator==(const Point3 &rhs) const { return equals(rhs); }
	bool operator!=(const Point3 &rhs) const { return !equals(rhs); }

	bool equals(const Point3 &p) const {
		return (x == p.x && y == p.y && z == p.z);
	}

	Point3 operator+(const Point3 &delta) const { return Point3(x + delta.x, y + delta.y, z + delta.z); }
	Point3 operator-(const Point3 &delta) const { return Point3(x - delta.x, y - delta.y, z - delta.z); }

	void operator+=(const Point3 &delta) {
		x += delta.x;
		y += delta.y;
		z += delta.z;
	}

	void operator-=(const Point3 &delta) {
		x -= delta.x;
		y -= delta.y;
		z -= delta.z;
	}

	int maxDistXYZ(const Point3 &other) const {
		int xdiff = abs(x - other.x);
		int ydiff = abs(y - other.y);
		int zdiff = abs(z - other.z);
		return MAX(xdiff, MAX(ydiff, zdiff));
	}

	uint32 sqrDist(const Point3 &other) const {
		int xdiff = abs(x - other.x);
		int ydiff = abs(y - other.y);
		int zdiff = abs(z - other.z);
		return uint32(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);
	}

	void set(int32 nx, int32 ny, int32 nz) {
		x = nx;
		y = ny;
		z = nz;
	}

	void translate(int32 dx, int32 dy, int32 dz) {
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
