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

#ifndef MATH_MATRIX3_H
#define MATH_MATRIX3_H

#include "math/rotation3d.h"
#include "math/squarematrix.h"
#include "math/vector3d.h"

namespace Math {

template<>
class Matrix<3, 3> : public MatrixType<3, 3>, public Rotation3D<Matrix<3, 3> > {
public:
	Matrix();
	Matrix(const MatrixBase<3, 3> &m);

    void transpose();

	
	/**
	 * Builds a matrix that maps the given local space forward direction vector to point towards the given
	 * target direction, and the given local up direction towards the given target world up direction.
	 *
	 * @param modelForward The forward direction in the local space of the object.
	 * @param targetDirection The desired world space direction the object should look at.
	 * @param modelUp The up direction in the local space of the object. This vector must be
	 *                perpendicular to the vector localForward.
	 * @param worldUp The global up direction of the scene in world space. The worldUp and targetDirection
	 *                vectors cannot be collinear, but they do not need to be perpendicular either.
	 * All the parameters MUST be normalized.
	 */
	void buildFromTargetDir(const Math::Vector3d &modelForward, const Math::Vector3d &targetDirection, 
						   const Math::Vector3d &modelUp, const Math::Vector3d &worldUp);

	inline Matrix<3, 3> operator*(const Matrix<3, 3> &m2) const {
		Matrix<3, 3> result;
		const float *d1 = getData();
		const float *d2 = m2.getData();
		float *r = result.getData();

		for (int i = 0; i < 9; i += 3) {
			for (int j = 0; j < 3; ++j) {
				r[i + j] = (d1[i + 0] * d2[j + 0])
					+ (d1[i + 1] * d2[j + 3])
					+ (d1[i + 2] * d2[j + 6]);
			}
		}

		return result;
	}
};

typedef Matrix<3, 3> Matrix3;

} // end of namespace Math

#endif

