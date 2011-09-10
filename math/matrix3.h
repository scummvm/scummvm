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

#ifndef MATH_MATRIX3_H
#define MATH_MATRIX3_H

#include "math/utils.h"
#include "math/matrix.h"
#include "math/transform.h"
#include "math/vector3d.h"

namespace Math {

template<class T>
class Rotation3D : public Transform<T> {
public:
	Rotation3D(T *matrix);

	void buildFromPitchYawRoll(float pitch, float yaw, float roll);

	void constructAroundPitch(float pitch);
	void constructAroundYaw(float yaw);
	void constructAroundRoll(float roll);

	void getPitchYawRoll(float* pPitch, float* pYaw, float* pRoll) const;

	float getPitch() const;
	float getYaw() const;
	float getRoll() const;

};

class Matrix3x3 : public Matrix<3, 3>, public Rotation3D<Matrix3x3> {
public:
	Matrix3x3();

	void transform(Vector3d* v) const;
};

typedef Matrix3x3 Matrix3;



template<class T>
Rotation3D<T>::Rotation3D(T *matrix) :
	Transform<T>(matrix) {

}

template<class T>
void Rotation3D<T>::buildFromPitchYawRoll(float pitch, float yaw, float roll) {
	T temp;

	constructAroundYaw(yaw);
	temp.constructAroundPitch(pitch);
	(*this->getMatrix()) *= temp;
	temp.constructAroundRoll(roll);
	(*this->getMatrix()) *= temp;
}

// at, around x-axis
template<class T>
void Rotation3D<T>::constructAroundRoll(float roll) {
	float cosa = (float)cos(degreeToRadian(roll));
	float sina = (float)sin(degreeToRadian(roll));

	this->getMatrix()->getRow(0) << 1.f << 0.f  << 0.f;
	this->getMatrix()->getRow(1) << 0.f << cosa << -sina;
	this->getMatrix()->getRow(2) << 0.f << sina << cosa;
}

// right
template<class T>
void Rotation3D<T>::constructAroundPitch(float pitch) {
	float cosa = (float)cos(degreeToRadian(pitch));
	float sina = (float)sin(degreeToRadian(pitch));

	this->getMatrix()->getRow(0) << cosa  << 0.f << sina;
	this->getMatrix()->getRow(1) << 0.f   << 1.f << 0.f;
	this->getMatrix()->getRow(2) << -sina << 0.f << cosa;
}

// up
template<class T>
void Rotation3D<T>::constructAroundYaw(float yaw) {
	float cosa = (float)cos(degreeToRadian(yaw));
	float sina = (float)sin(degreeToRadian(yaw));

	this->getMatrix()->getRow(0) << cosa << -sina << 0.f;
	this->getMatrix()->getRow(1) << sina << cosa  << 0.f;
	this->getMatrix()->getRow(2) << 0.f  << 0.f   << 1.f;
}

/*
 0 * 1 2 3
 4 5 6 7
 8 9 10 11
 */
template<class T>
void Rotation3D<T>::getPitchYawRoll(float *pPitch, float *pYaw, float *pRoll) const {
	// based on http://planning.cs.uiuc.edu/node103.html
	if (pYaw) {
		*pYaw = radianToDegree(atan2f(this->getMatrix()->getValue(1, 0),
									  this->getMatrix()->getValue(0, 0)));
	}

	if (pPitch) {
		float a = this->getMatrix()->getValue(2, 1);
		float b = this->getMatrix()->getValue(2, 2);
		float mag = sqrt(a * a + b * b);
		*pPitch = radianToDegree(atan2f(-this->getMatrix()->getValue(2, 0), mag));
	}

	if (pRoll) {
		*pRoll = radianToDegree(atan2f(this->getMatrix()->getValue(2, 1),
									   this->getMatrix()->getValue(2, 2)));
	}
}

template<class T>
float Rotation3D<T>::getPitch() const {
	float pitch;

	getPitchYawRoll(&pitch, 0, 0);

	return pitch;
}

template<class T>
float Rotation3D<T>::getYaw() const {
	float yaw;

	getPitchYawRoll(0, &yaw, 0);

	return yaw;
}

template<class T>
float Rotation3D<T>::getRoll() const {
	float roll;

	getPitchYawRoll(0, 0, &roll);

	return roll;
}


} // end of namespace Math

#endif

