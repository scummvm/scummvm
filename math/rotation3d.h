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
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef MATH_ROTATION3D_H
#define MATH_ROTATION3D_H

#include "common/streamdebug.h"

#include "math/utils.h"
#include "math/transform.h"
#include "math/angle.h"

namespace Math {

template<class T>
class Rotation3D : public Transform<T> {
public:
	Rotation3D();

	void buildFromPitchYawRoll(const Angle &pitch, const Angle &yaw, const Angle &roll);

	void buildAroundPitch(const Angle &pitch);
	void buildAroundYaw(const Angle &yaw);
	void buildAroundRoll(const Angle &roll);

	void getPitchYawRoll(Angle* pPitch, Angle* pYaw, Angle* pRoll) const;

	Angle getPitch() const;
	Angle getYaw() const;
	Angle getRoll() const;

};


template<class T>
Rotation3D<T>::Rotation3D() :
	Transform<T>() {

}

// NOTE: Builds a rotation matrix of form R_Yaw * R_Pitch * R_Roll (i.e. R_z * R_x * R_y).
// The order of rotations is of the form Matrix * Vector, so roll is applied first, then pitch, then yaw.
template<class T>
void Rotation3D<T>::buildFromPitchYawRoll(const Angle &pitch, const Angle &yaw, const Angle &roll) {
	T temp;

	buildAroundYaw(yaw); // Rotate about +Z
	temp.buildAroundPitch(pitch); // Rotate about +X
	this->getMatrix() = this->getMatrix() * temp;
	temp.buildAroundRoll(roll); // Rotate about +Y
	this->getMatrix() = this->getMatrix() * temp;
	// The created matrix has the Euler order ZXY. (M*v)
}

// at. Rotates about the +X axis.
template<class T>
void Rotation3D<T>::buildAroundPitch(const Angle &pitch) {
	float cosa = pitch.getCosine();
	float sina = pitch.getSine();

	this->getMatrix().getRow(0) << 1.f << 0.f  << 0.f;
	this->getMatrix().getRow(1) << 0.f << cosa << -sina;
	this->getMatrix().getRow(2) << 0.f << sina << cosa;
}

// right. Rotates about the +Y axis.
template<class T>
void Rotation3D<T>::buildAroundRoll(const Angle &roll) {
	float cosa = roll.getCosine();
	float sina = roll.getSine();

	this->getMatrix().getRow(0) << cosa  << 0.f << sina;
	this->getMatrix().getRow(1) << 0.f   << 1.f << 0.f;
	this->getMatrix().getRow(2) << -sina << 0.f << cosa;
}

// up. Rotates about the +Z axis.
template<class T>
void Rotation3D<T>::buildAroundYaw(const Angle &yaw) {
	float cosa = yaw.getCosine();
	float sina = yaw.getSine();

	this->getMatrix().getRow(0) << cosa << -sina << 0.f;
	this->getMatrix().getRow(1) << sina << cosa  << 0.f;
	this->getMatrix().getRow(2) << 0.f  << 0.f   << 1.f;
}


/** 
 * Decomposes the matrix M to form M = R_z * R_x * R_y (R_D being the cardinal rotation 
 * matrix about the axis +D), and outputs the angles of rotation in parameters pPitch, pYaw and pRoll.
 * In the convention of the coordinate system used in Grim Fandango characters:
 *	Pitch is rotation about the X axis (right)
 *  Yaw is rotation about the Z axis (up)
 *  Roll is rotation about the Y axix (out)
 * This function was adapted from http://www.geometrictools.com/Documentation/EulerAngles.pdf
 * The matrix M must be orthonormal. 
 */
template<class T>
void Rotation3D<T>::getPitchYawRoll(Angle *pPitch, Angle *pYaw, Angle *pRoll) const {
	const T *m = &(this->getMatrix());  // so dumb
	
	float x,y,z;
	if (m->getValue(2, 1) < 1.f) {
		if (m->getValue(2, 1) > -1.f) {
			x = asin(m->getValue(2, 1));
			z = atan2(-m->getValue(0, 1), m->getValue(1, 1));
			y = atan2(-m->getValue(2, 0), m->getValue(2, 2));
		}
		else {
			// Not a unique solution. Pick an arbitrary one.
			x = -3.141592654f/2.f;
			z = -atan2(-m->getValue(0, 2), m->getValue(0, 0));
			y = 0;
		}
	}
	else {
		// Not a unique solution. Pick an arbitrary one.
		x = 3.141592654f/2.f;
		z = atan2(m->getValue(0, 2), m->getValue(0, 0));
		y = 0;
	}
	*pPitch = Math::Angle::fromRadians(x);
	*pRoll = Math::Angle::fromRadians(y);
	*pYaw = Math::Angle::fromRadians(z);
}

template<class T>
Angle Rotation3D<T>::getPitch() const {
	Angle pitch;

	getPitchYawRoll(&pitch, 0, 0);

	return pitch;
}

template<class T>
Angle Rotation3D<T>::getYaw() const {
	Angle yaw;

	getPitchYawRoll(0, &yaw, 0);

	return yaw;
}

template<class T>
Angle Rotation3D<T>::getRoll() const {
	Angle roll;

	getPitchYawRoll(0, 0, &roll);

	return roll;
}

}

#endif
