// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
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

#ifndef MATRIX4_HH
#define MATRIX4_HH

#include "vector3d.h"
#include "matrix3.h"

// matrix 4 is a rotation matrix + position
class Matrix4 {
public:
	Matrix3 rot_;
	Vector3d pos_;

	Matrix4();

	Matrix4& operator =(const Matrix4& s) {
		pos_ = s.pos_;
		rot_ = s.rot_;

		return *this;
	}

	Matrix4& operator *=(const Matrix4& s) {
		Vector3d v;

		v = s.pos_;
		rot_.transform(&v);

		pos_+=v;

		rot_ *= s.rot_;

		return *this;
	}

	void translate(float x, float y, float z);

private:
};

#endif // MATRIX_HH

