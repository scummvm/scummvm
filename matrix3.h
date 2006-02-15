// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef MATRIX3_HH
#define MATRIX3_HH

#include "vector3d.h"

// matrix 3 is a rotation matrix
class Matrix3{
public:
	Vector3d _right;
	Vector3d _up;
	Vector3d _at;

	void buildFromPitchYawRoll(float pitch, float yaw, float roll);
	void setAsIdentity();

	void constructAroundPitch(float pitch);
	void constructAroundYaw(float pitch);
	void constructAroundRoll(float pitch);

	void getPitchYawRoll(float* pPitch, float* pYaw, float* pRoll);

	float getPitch();
	float getYaw();
	float getRoll();

	void transform(Vector3d* v);

	// operators
	Matrix3& operator *=(const Matrix3& s) {
		float x, y, z;

		x = _right.dotProduct(s._right.x(), s._up.x(), s._at.x());
		y = _right.dotProduct(s._right.y(), s._up.y(), s._at.y());
		z = _right.dotProduct(s._right.z(), s._up.z(), s._at.z());

		_right.set(x, y, z);

		x = _up.dotProduct(s._right.x(), s._up.x(), s._at.x());
		y = _up.dotProduct(s._right.y(), s._up.y(), s._at.y());
		z = _up.dotProduct(s._right.z(), s._up.z(), s._at.z());

		_up.set( x, y, z );

		x = _at.dotProduct(s._right.x(), s._up.x(), s._at.x());
		y = _at.dotProduct(s._right.y(), s._up.y(), s._at.y());
		z = _at.dotProduct(s._right.z(), s._up.z(), s._at.z());

		_at.set(x, y, z);

		return *this;
	}
	Matrix3& operator =(const Matrix3& s) {
		_right = s._right;
		_up = s._up;
		_at = s._at;

		return *this;
	}

private:
};

#endif // MATRIX_HH

