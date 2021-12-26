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

#ifndef MATH_VECTOR3D_H
#define MATH_VECTOR3D_H

#include "common/scummsys.h"
#include "common/endian.h"

#include "math/angle.h"
#include "math/squarematrix.h"
#include "math/vector.h"

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
	 * Multiply vector XYZ with Matrix 3x3
	 *
	 * @return	The result of multiplication
	 */
	inline Vector3d operator*(const MatrixType<3, 3> &m) const {
		const float *d = m.getData();
		return Vector3d(x() * d[0] + y() * d[3] + z() * d[6],
				x() * d[1] + y() * d[4] + z() * d[7],
				x() * d[2] + y() * d[5] + z() * d[8]);
	}

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

	/**
	 * Calculate vector length
	 * @return      The computed length
	 */
	inline static float length(const Vector3d& v) {
		return sqrtf(v.x() * v.x() + v.y() * v.y() + v.z() * v.z());
	}

	/**
	 * Calculate vector length
	 * @return      The computed length
	 */
	float length() {
		return sqrtf(x() * x() + y() * y() + z() * z());
	}

	/**
	 * Linearly interpolate between two vectors
	 * @param v1    The first vector
	 * @param v2    The second vector
	 * @param a     The value to use to interpolate between v1 and v2
	 * @return      The resulting calculation
	 */
	inline static Vector3d interpolate(const Vector3d& v1, const Vector3d& v2, const float a) {
		return Vector3d(v1 * (1.0f - a) + v2 * a);
	}
};

} // end of namespace Math

#endif
