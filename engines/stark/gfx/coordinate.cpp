/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/stark/gfx/coordinate.h"
#include "math/vector3d.h"

namespace Stark {
namespace Gfx {

/*
 * Quaternion to rotation matrix
 */
#define _Q_MAT(w, x, y, z) {1 - 2 * y * y - 2 * z * z,    2 * x * y - 2 * z * w,        2 * x * z + 2 * y * w, \
							   2 * x * y + 2 * w * z,    1 - 2 * x * x - 2 * z * z,      2 * y * z - 2 * w * x, \
							   2 * x * z - 2 * w * y,      2 * y * z + 2 * x * w,      1 - 2 * x * x - 2 * y * y, \
						};

Coordinate::Coordinate(const Math::Vector3d &left) {
	_coords[0] = left.x();
	_coords[1] = left.y();
	_coords[2] = left.z();

	_coords[3] = 1.f;
	_coords[4] = 0.f;
	_coords[5] = 0.f;
	_coords[6] = 0.f;
}

Coordinate::Coordinate() {
	for (int i = 0; i < 7; ++i)
		_coords[i] = 0;

	_coords[3] = 1.f;
}

Coordinate::Coordinate(float x, float y, float z) {
	_coords[0] = x;
	_coords[1] = y;
	_coords[2] = z;

	_coords[3] = 1.f;
	_coords[4] = 0.f;
	_coords[5] = 0.f;
	_coords[6] = 0.f;
}

Coordinate::~Coordinate() {
	
}

void Coordinate::translate(float x, float y, float z) {
	_coords[0] += x;
	_coords[1] += y;
	_coords[2] += z;
}

void Coordinate::translate(const Coordinate &coord) {
	_coords[0] += coord._coords[0];
	_coords[1] += coord._coords[1];
	_coords[2] += coord._coords[2];
}

void Coordinate::rotate(float w, float x, float y, float z) {
	float r[] = _Q_MAT(w, x, y, z);
	x = _coords[0] * r[0] + _coords[1] * r[1] + _coords[2] * r[2];
	y = _coords[0] * r[3] + _coords[1] * r[4] + _coords[2] * r[5];
	z = _coords[0] * r[6] + _coords[1] * r[7] + _coords[2] * r[8];
	_coords[0] = x;
	_coords[1] = y;
	_coords[2] = z;
}

void Coordinate::rotate(const Coordinate &coord) {
	rotate(coord._coords[3], coord._coords[4], coord._coords[5], coord._coords[6]);
}

void Coordinate::setTranslation(float x, float y, float z) {
	_coords[0] = x;
	_coords[1] = y;
	_coords[2] = z;
}

void Coordinate::setRotation(float w, float x, float y, float z) {
	_coords[3] = w;
	_coords[4] = x;
	_coords[5] = y;
	_coords[6] = z;
}

} // End of namespace Gfx
} // End of namespace Stark
