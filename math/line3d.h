/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef MATH_LINE3D_H
#define MATH_LINE3D_H

#include "common/scummsys.h"
#include "common/endian.h"

#include "math/vector3d.h"

namespace Math {

class Line3d {
public:
	Line3d();
	Line3d(const Vector3d &begin, const Vector3d &end);
	Line3d(const Line3d &other);

	Math::Vector3d begin() const;
	Math::Vector3d end() const;
	Math::Vector3d middle() const;

	/**
	 * Check if this line segment intersects with another line segment
	 *
	 * The check is done in either a XY or a XZ 2D plane.
	 */
	bool intersectLine2d(const Line3d &other, Math::Vector3d *pos, bool useXZ);

	Line3d& operator=(const Line3d &other);

private:
	Math::Vector3d _begin, _end;

};

} // end of namespace Math

#endif
