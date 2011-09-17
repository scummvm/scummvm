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

#include "math/matrix4.h"

namespace Math {

Matrix<4, 4>::Matrix() :
	MatrixType<4, 4>(), Rotation3D<Matrix4>() {

}

Matrix<4, 4>::Matrix(const MatrixBase<4, 4> &m) :
	MatrixType<4, 4>(m), Rotation3D<Matrix4>() {

}

void Matrix<4, 4>::transform(Vector3d *v, bool trans) const {
	Matrix<4, 1> m;
	m(0, 0) = v->x();
	m(1, 0) = v->y();
	m(2, 0) = v->z();
	m(3, 0) = (trans ? 1.f : 0.f);

	m = *this * m;

	v->set(m(0, 0), m(1, 0), m(2, 0));
}

Vector3d Matrix<4, 4>::getPosition() const {
	return Vector3d(getValue(3, 0), getValue(3, 1), getValue(3, 2));
}

void Matrix<4, 4>::setPosition(const Vector3d &v) {
	setValue(3, 0, v.x());
	setValue(3, 1, v.y());
	setValue(3, 2, v.z());
}

void Matrix<4, 4>::translate(const Vector3d &vec) {
	Vector3d v(vec);
	transform(&v, false);

	operator()(3, 0) += v.x();
	operator()(3, 1) += v.y();
	operator()(3, 2) += v.z();
}

} // end of namespace Math

