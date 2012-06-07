/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 */

#ifndef MATH_MATRIX4_H
#define MATH_MATRIX4_H

#include "math/rotation3d.h"
#include "math/squarematrix.h"
#include "math/vector3d.h"
#include "math/matrix3.h"

namespace Math {

// matrix 4 is a rotation matrix + position
template<>
class Matrix<4, 4> : public MatrixType<4, 4>, public Rotation3D<Matrix<4, 4> > {
public:
	Matrix();
	Matrix(const MatrixBase<4, 4> &m);

	void transform(Vector3d *v, bool translate) const;
	void inverseTranslate(Vector3d *v);
	void inverseRotate(Vector3d *v);
	
	Vector3d getPosition() const;
	void setPosition(const Vector3d &v);
	
	Matrix3 getRotation() const;
	void setRotation(const Matrix3 &m);

	void translate(const Vector3d &v);
	
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

};

typedef Matrix<4, 4> Matrix4;

} // end of namespace Math

#endif
