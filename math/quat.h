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

// Quaternion-math borrowed from plib http://plib.sourceforge.net/index.html
// Which is covered by LGPL2
// And has this additional copyright note:
/*
 Quaternion routines are Copyright (C) 1999
 Kevin B. Thompson <kevinbthompson@yahoo.com>
 Modified by Sylvan W. Clebsch <sylvan@stanford.edu>
 Largely rewritten by "Negative0" <negative0@earthlink.net>
 */

#ifndef MATH_QUAT_H
#define MATH_QUAT_H

#include "common/scummsys.h"
#include "common/endian.h"

#include "math/vector.h"
#include "math/angle.h"
#include "math/vector4d.h"
#include "matrix4.h"

namespace Math {

class Quaternion : public Vector4d {
public:
	Quaternion() : Vector4d(0, 0, 0, 0) {}
	Quaternion(float lx, float ly, float lz, float lw) : Vector4d(lx, ly, lz, lw) {}
	Quaternion(const Quaternion &q) : Vector4d(q.x(), q.y(), q.z(), q.w()) {} 
	Quaternion(const Vector4d &vec) : Vector4d(vec.x(), vec.y(), vec.z(), vec.w()) {}
	
	Matrix4 toMatrix() const;
	void toMatrix(Matrix4 &dst) const;
	/**
	 * Slerps between this quaternion and to by factor t
	 * @param to	the quaternion to slerp between
	 * @param t		factor to slerp by.
	 * @return		the resulting quaternion.
	 */
	Quaternion slerpQuat(const Quaternion& to, const float t);
	static Quaternion fromEuler(const Angle &yaw, const Angle &pitch, const Angle &roll);
	
	inline static Quaternion get_quaternion(const char *data) {
		return Quaternion(get_float(data), get_float(data + 4), get_float(data + 8), get_float(data + 12));
	}
	Quaternion& operator=(Vector4d &vec);
};

} // end of namespace Math

#endif
