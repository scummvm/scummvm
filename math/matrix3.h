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
	
	void buildFromTargetDir(const Math::Vector3d &modelForward, const Math::Vector3d &targetDirection, 
						   const Math::Vector3d &modelUp, const Math::Vector3d &worldUp);

};

typedef Matrix<3, 3> Matrix3;

} // end of namespace Math

#endif

