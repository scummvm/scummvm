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

#include "math/matrix3.h"

namespace Math {

Matrix<3, 3>::Matrix() :
	MatrixType<3, 3>(), Rotation3D<Matrix<3, 3> >() {

}

Matrix<3, 3>::Matrix(const MatrixBase<3, 3> &m) :
	MatrixType<3, 3>(m), Rotation3D<Matrix<3, 3> >() {

}

void swap (float &a, float &b) {
    float c=a; a=b; b=c;
}

void Matrix<3, 3>::transpose() {
    swap(operator()(0,1), operator()(1,0));
    swap(operator()(0,2), operator()(2,0));
    swap(operator()(1,2), operator()(2,1));
}

} // end of namespace Math
