// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
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
	Vector3d right_;
    Vector3d up_;
    Vector3d at_;

	void buildFromPitchYawRoll( float pitch, float yaw, float roll );
	void setAsIdentity(void);

	void constructAroundPitch( float pitch );
	void constructAroundYaw( float pitch );
	void constructAroundRoll( float pitch );

	void getPitchYawRoll( float* pPitch, float* pYaw, float* pRoll );

	float getPitch();
	float getYaw();
	float getRoll();

	void transform( Vector3d v );

	// operators
	Matrix3& operator *=(const Matrix3& s)
	{
		float x, y, z;

		x = right_.dotProduct( s.right_.x(), s.up_.x(), s.at_.x() );
		y = right_.dotProduct( s.right_.y(), s.up_.y(), s.at_.y() );
		z = right_.dotProduct( s.right_.z(), s.up_.z(), s.at_.z() );

		right_.set( x, y, z );

		x = up_.dotProduct( s.right_.x(), s.up_.x(), s.at_.x() );
		y = up_.dotProduct( s.right_.y(), s.up_.y(), s.at_.y() );
		z = up_.dotProduct( s.right_.z(), s.up_.z(), s.at_.z() );

		up_.set( x, y, z );

		x = at_.dotProduct( s.right_.x(), s.up_.x(), s.at_.x() );
		y = at_.dotProduct( s.right_.y(), s.up_.y(), s.at_.y() );
		z = at_.dotProduct( s.right_.z(), s.up_.z(), s.at_.z() );

		at_.set( x, y, z );

		return *this;
	}
	Matrix3& operator =(const Matrix3& s)
	{
		right_ = s.right_;
		up_ = s.up_;
		at_ = s.at_;

		return *this;
	}

private:
};

#endif // MATRIX_HH

