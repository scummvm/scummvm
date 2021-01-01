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

#ifndef MATH_VECTOR3D_H
#define MATH_VECTOR3D_H

#include "common/scummsys.h"
#include "common/endian.h"

#include "math/vector.h"
#include "math/angle.h"

namespace Math {

typedef Matrix<3, 1> Vector3d;

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
	Matrix(const float *data);

	/**
	 * Set the value of the vector using three floats
	 * @param lx	X Value
	 * @param ly	Y Value
	 * @param lz	Z Value
	 */
	void set(float lx, float ly, float lz);

	/**
	 * Get the angle of this vector around the unit circle
	 * This operation ignores the z-component
	 * @return	The computed angle
	 */
	Angle unitCircleAngle() const;

	/**
	 * Find the cross product between two vectors
	 * @param v1	The first vector
	 * @param v2	The second vector
	 * @return	The resulting cross product
	 */
	inline static Vector3d crossProduct(const Vector3d& v1, const Vector3d& v2) {
		return Vector3d(v1.y() * v2.z() - v1.z() * v2.y(),
				v1.z() * v2.x() - v1.x() * v2.z(),
				v1.x() * v2.y() - v1.y() * v2.x());
	}

	/**
	 * Find the angle between two vectors
	 * @param v1	The first vector
	 * @param v2	The second vector
	 * @return	The computed angle
	 */
	inline static Angle angle(const Vector3d& v1, const Vector3d& v2) {
		return Angle::arcCosine(fminf(fmaxf(dotProduct(v1, v2) / (v1.getMagnitude() * v2.getMagnitude()), -1.0f), 1.0f));
	}

};

} // end of namespace Math

#endif
