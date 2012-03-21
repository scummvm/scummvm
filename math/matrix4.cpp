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

#include "math/matrix4.h"
#include "math/vector4d.h"

namespace Math {

Matrix<4, 4>::Matrix() :
	MatrixType<4, 4>(), Rotation3D<Matrix4>() {

}

Matrix<4, 4>::Matrix(const MatrixBase<4, 4> &m) :
	MatrixType<4, 4>(m), Rotation3D<Matrix4>() {

}

void Matrix<4, 4>::transform(Vector3d *v, bool trans) const {
	Vector4d m;
	m(0, 0) = v->x();
	m(1, 0) = v->y();
	m(2, 0) = v->z();
	m(3, 0) = (trans ? 1.f : 0.f);

	m = *this * m;

	v->set(m(0, 0), m(1, 0), m(2, 0));
}

Vector3d Matrix<4, 4>::getPosition() const {
	return Vector3d(getValue(0, 3), getValue(1, 3), getValue(2, 3));
}

void Matrix<4, 4>::setPosition(const Vector3d &v) {
	setValue(0, 3, v.x());
	setValue(1, 3, v.y());
	setValue(2, 3, v.z());
}

Matrix3 Matrix<4, 4>::getRotation() const{
    Matrix3 m2;
    
    m2.setValue(0, 0, getValue(0, 0));
	m2.setValue(0, 1, getValue(0, 1));
	m2.setValue(0, 2, getValue(0, 2));
	m2.setValue(1, 0, getValue(1, 0));
	m2.setValue(1, 1, getValue(1, 1));
	m2.setValue(1, 2, getValue(1, 2));
	m2.setValue(2, 0, getValue(2, 0));
	m2.setValue(2, 1, getValue(2, 1));
	m2.setValue(2, 2, getValue(2, 2));
	
	return m2;
}

void Matrix<4, 4>::setRotation(const Matrix3 &m) {
    setValue(0, 0, m.getValue(0, 0));
	setValue(0, 1, m.getValue(0, 1));
	setValue(0, 2, m.getValue(0, 2));
	setValue(1, 0, m.getValue(1, 0));
	setValue(1, 1, m.getValue(1, 1));
	setValue(1, 2, m.getValue(1, 2));
	setValue(2, 0, m.getValue(2, 0));
	setValue(2, 1, m.getValue(2, 1));
	setValue(2, 2, m.getValue(2, 2));
}

void Matrix<4, 4>::translate(const Vector3d &vec) {
	Vector3d v(vec);
	transform(&v, false);

	operator()(0, 3) += v.x();
	operator()(1, 3) += v.y();
	operator()(2, 3) += v.z();
}

/** 
 * Generates a lookat matrix. For reference, see 
 * http://clb.demon.fi/MathGeoLib/docs/float3x3_LookAt.php 
 */
void Matrix<4, 4>::buildFromTargetDir(const Math::Vector3d &modelForward, const Math::Vector3d &targetDirection, 
						   const Math::Vector3d &modelUp, const Math::Vector3d &worldUp)
{
    Matrix3 rotation;
    rotation.buildFromTargetDir(modelForward, targetDirection, modelUp, worldUp);
    this->setRotation(rotation);
}

void Matrix<4, 4>::invertAffineOrthonormal() {
    Matrix3 rotation(getRotation());
    rotation.transpose();
    
    Vector3d position(getPosition().getNegative());
    
    rotation.transformVector(&position);
    setRotation(rotation);
    setPosition(position);
}

// The following functions are adapted from Portalib3d, which no longer is 
// available on the net, but was used in the iconoclast-project:
// http://code.google.com/p/iconoclast/
// Original copyright notice (license.txt was not supplied in iconoclast,
// but should match our LGPL-license):
// Author:	Brett Porter
// Email: brettporter@yahoo.com
// Website: http://rsn.gamedev.net/pl3d
// Copyright (C)2000, 2001, Brett Porter. All Rights Reserved.
// This source code is released under the LGPL. See license.txt for details.

void Matrix<4, 4>::inverseTranslate(Vector3d *v) {
	v->x() = v->x() - getValue(0, 3);
	v->y() = v->y() - getValue(1, 3);
	v->z() = v->z() - getValue(2, 3);
}

void Matrix<4, 4>::inverseRotate(Vector3d *v) {
	Vector3d temp;
	
	temp.x() = v->x() * getValue(0, 0) + v->y() * getValue(1, 0) + v->z() * getValue(2, 0);
	temp.y() = v->x() * getValue(0, 1) + v->y() * getValue(1, 1) + v->z() * getValue(2, 1);
	temp.z() = v->x() * getValue(0, 2) + v->y() * getValue(1, 2) + v->z() * getValue(2, 2);
	
	*v = temp;
}
	
} // end of namespace Math

