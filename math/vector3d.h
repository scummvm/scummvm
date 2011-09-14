/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 */

#ifndef MATH_VECTOR3D_H
#define MATH_VECTOR3D_H

#include "common/scummsys.h"
#include "common/endian.h"

#include "math/vector.h"

namespace Math {

template<>
class Matrix<3, 1> : public MatrixType<3, 1> {
public:
	float& x() { return value(0); }
	float x() const { return value(0); }
	float& y() { return value(1); }
	float y() const { return value(1); }
	float& z() { return value(2); }
	float z() const { return value(2); }

	Matrix();
	Matrix(float lx, float ly, float lz);
	Matrix(const MatrixBase<3, 1> &m);

	void set(float lx, float ly, float lz);

	// Get the angle a vector is around the unit circle
	// (ignores z-component)
	float unitCircleAngle() const;
};

typedef Matrix<3, 1> Vector3d;

inline Vector3d cross(const Vector3d& v1, const Vector3d& v2) {
	return Vector3d(v1.y() * v2.z() - v1.z() * v2.y(),
					v1.z() * v2.x() - v1.x() * v2.z(),
					v1.x() * v2.y() - v1.y() * v2.x());
}

inline float angle(const Vector3d& v1, const Vector3d& v2) {
	return acos(dot(v1, v2) / (v1.getMagnitude() * v2.getMagnitude()));
}

inline Vector3d get_vector3d(const char *data) {
	return Vector3d(get_float(data), get_float(data + 4), get_float(data + 8));
}

} // end of namespace Math

#endif
