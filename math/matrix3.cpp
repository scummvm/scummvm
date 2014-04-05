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

/** 
 * Generates a lookat matrix. For reference, see 
 * http://clb.demon.fi/MathGeoLib/docs/float3x3_LookAt.php 
 */
void Matrix<3, 3>::buildFromTargetDir(const Math::Vector3d &modelForward, const Math::Vector3d &targetDirection, 
						   const Math::Vector3d &modelUp, const Math::Vector3d &worldUp)
{
    Math::Vector3d modelRight = Math::Vector3d::crossProduct(modelUp, modelForward);
	modelRight.normalize();
	Math::Vector3d worldRight = Math::Vector3d::crossProduct(worldUp, targetDirection);
	worldRight.normalize();
	Math::Vector3d perpWorldUp = Math::Vector3d::crossProduct(targetDirection, worldRight);
	perpWorldUp.normalize();
	
	Math::Matrix3 m1;
	m1.getRow(0) << worldRight.x() << worldRight.y() << worldRight.z();
	m1.getRow(1) << perpWorldUp.x() << perpWorldUp.y() << perpWorldUp.z();
	m1.getRow(2) << targetDirection.x() << targetDirection.y() << targetDirection.z();
	m1.transpose();
	
	Math::Matrix3 m2;
	m2.getRow(0) << modelRight.x() << modelRight.y() << modelRight.z();
	m2.getRow(1) << modelUp.x() << modelUp.y() << modelUp.z();
	m2.getRow(2) << modelForward.x() << modelForward.y() << modelForward.z();
	
	this->operator=(m1 * m2);
}


} // end of namespace Math
