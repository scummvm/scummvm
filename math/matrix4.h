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

#ifndef MATH_MATRIX4_H
#define MATH_MATRIX4_H

#include "math/rotation3d.h"
#include "math/squarematrix.h"
#include "math/vector3d.h"
#include "math/vector4d.h"
#include "math/matrix3.h"

namespace Math {

// matrix 4 is a rotation matrix + position
template<>
class Matrix<4, 4> : public MatrixType<4, 4>, public Rotation3D<Matrix<4, 4> > {
public:
	Matrix();
	Matrix(const MatrixBase<4, 4> &m);
	Matrix(const Angle &first, const Angle &second, const Angle &third, EulerOrder order) { buildFromEuler(first, second, third, order); }

	void transform(Vector3d *v, bool translate) const;
	void inverseTranslate(Vector3d *v) const;
	void inverseRotate(Vector3d *v) const;
	
	Vector3d getPosition() const;
	void setPosition(const Vector3d &v);
	
	Matrix3 getRotation() const;
	void setRotation(const Matrix3 &m);

	void translate(const Vector3d &v);

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
	
	/** 
	* Inverts a matrix in place.
	*	This function avoid having to do generic Gaussian elimination on the matrix
	*	by assuming that the top-left 3x3 part of the matrix is orthonormal
	*	(columns and rows 0, 1 and 2 orthogonal and unit length).
	*	See e.g. Eric Lengyel's Mathematics for 3D Game Programming and Computer Graphics, p. 82.
	*/
	void invertAffineOrthonormal();

	void transpose();

	inline Matrix<4, 4> operator*(const Matrix<4, 4> &m2) const {
		Matrix<4, 4> result;
		const float *d1 = getData();
		const float *d2 = m2.getData();
		float *r = result.getData();

		for (int i = 0; i < 16; i += 4) {
			for (int j = 0; j < 4; ++j) {
				r[i + j] = (d1[i + 0] * d2[j + 0])
					+ (d1[i + 1] * d2[j + 4])
					+ (d1[i + 2] * d2[j + 8])
					+ (d1[i + 3] * d2[j + 12]);
			}
		}

		return result;
	}

	inline Vector4d transform(const Vector4d &v) const {
		Vector4d result;
		const float *d1 = getData();
		const float *d2 = v.getData();
		float *r = result.getData();

		for (int i = 0; i < 4; i++) {
			r[i] = d2[0] * d1[0 * 4 + i] +
			       d2[1] * d1[1 * 4 + i] +
			       d2[2] * d1[2 * 4 + i] +
			       d2[3] * d1[3 * 4 + i];
		}

		return result;
	}

	inline bool inverse() {
		Matrix<4, 4> invMatrix;
		float *inv = invMatrix.getData();
		float *m = getData();

		inv[0] = m[5]  * m[10] * m[15] - 
			m[5]  * m[11] * m[14] - 
			m[9]  * m[6]  * m[15] + 
			m[9]  * m[7]  * m[14] +
			m[13] * m[6]  * m[11] - 
			m[13] * m[7]  * m[10];

		inv[4] = -m[4]  * m[10] * m[15] + 
			m[4]  * m[11] * m[14] + 
			m[8]  * m[6]  * m[15] - 
			m[8]  * m[7]  * m[14] - 
			m[12] * m[6]  * m[11] + 
			m[12] * m[7]  * m[10];

		inv[8] = m[4]  * m[9] * m[15] - 
			m[4]  * m[11] * m[13] - 
			m[8]  * m[5] * m[15] + 
			m[8]  * m[7] * m[13] + 
			m[12] * m[5] * m[11] - 
			m[12] * m[7] * m[9];

		inv[12] = -m[4]  * m[9] * m[14] + 
			m[4]  * m[10] * m[13] +
			m[8]  * m[5] * m[14] - 
			m[8]  * m[6] * m[13] - 
			m[12] * m[5] * m[10] + 
			m[12] * m[6] * m[9];

		inv[1] = -m[1]  * m[10] * m[15] + 
			m[1]  * m[11] * m[14] + 
			m[9]  * m[2] * m[15] - 
			m[9]  * m[3] * m[14] - 
			m[13] * m[2] * m[11] + 
			m[13] * m[3] * m[10];

		inv[5] = m[0]  * m[10] * m[15] - 
			m[0]  * m[11] * m[14] - 
			m[8]  * m[2] * m[15] + 
			m[8]  * m[3] * m[14] + 
			m[12] * m[2] * m[11] - 
			m[12] * m[3] * m[10];

		inv[9] = -m[0]  * m[9] * m[15] + 
			m[0]  * m[11] * m[13] + 
			m[8]  * m[1] * m[15] - 
			m[8]  * m[3] * m[13] - 
			m[12] * m[1] * m[11] + 
			m[12] * m[3] * m[9];

		inv[13] = m[0]  * m[9] * m[14] - 
			m[0]  * m[10] * m[13] - 
			m[8]  * m[1] * m[14] + 
			m[8]  * m[2] * m[13] + 
			m[12] * m[1] * m[10] - 
			m[12] * m[2] * m[9];

		inv[2] = m[1]  * m[6] * m[15] - 
			m[1]  * m[7] * m[14] - 
			m[5]  * m[2] * m[15] + 
			m[5]  * m[3] * m[14] + 
			m[13] * m[2] * m[7] - 
			m[13] * m[3] * m[6];

		inv[6] = -m[0]  * m[6] * m[15] + 
			m[0]  * m[7] * m[14] + 
			m[4]  * m[2] * m[15] - 
			m[4]  * m[3] * m[14] - 
			m[12] * m[2] * m[7] + 
			m[12] * m[3] * m[6];

		inv[10] = m[0]  * m[5] * m[15] - 
			m[0]  * m[7] * m[13] - 
			m[4]  * m[1] * m[15] + 
			m[4]  * m[3] * m[13] + 
			m[12] * m[1] * m[7] - 
			m[12] * m[3] * m[5];

		inv[14] = -m[0]  * m[5] * m[14] + 
			m[0]  * m[6] * m[13] + 
			m[4]  * m[1] * m[14] - 
			m[4]  * m[2] * m[13] - 
			m[12] * m[1] * m[6] + 
			m[12] * m[2] * m[5];

		inv[3] = -m[1] * m[6] * m[11] + 
			m[1] * m[7] * m[10] + 
			m[5] * m[2] * m[11] - 
			m[5] * m[3] * m[10] - 
			m[9] * m[2] * m[7] + 
			m[9] * m[3] * m[6];

		inv[7] = m[0] * m[6] * m[11] - 
			m[0] * m[7] * m[10] - 
			m[4] * m[2] * m[11] + 
			m[4] * m[3] * m[10] + 
			m[8] * m[2] * m[7] - 
			m[8] * m[3] * m[6];

		inv[11] = -m[0] * m[5] * m[11] + 
			m[0] * m[7] * m[9] + 
			m[4] * m[1] * m[11] - 
			m[4] * m[3] * m[9] - 
			m[8] * m[1] * m[7] + 
			m[8] * m[3] * m[5];

		inv[15] = m[0] * m[5] * m[10] - 
			m[0] * m[6] * m[9] - 
			m[4] * m[1] * m[10] + 
			m[4] * m[2] * m[9] + 
			m[8] * m[1] * m[6] - 
			m[8] * m[2] * m[5];

		float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

		if (det == 0)
			return false;

		det = 1.0 / det;

		for (int i = 0; i < 16; i++) {
			m[i] = inv[i] * det;
		}

		return true;
	}
};

typedef Matrix<4, 4> Matrix4;

} // end of namespace Math

#endif
