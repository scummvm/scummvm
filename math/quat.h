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
	/**
	 * Default Constructor
	 */
	Quaternion() : Vector4d(0, 0, 0, 1.0) {}

	/**
	 * Constructor from four floats in the order X,Y,Z,W
	 * The initial values should be normalized, otherwise call normalize() after creation
	 * @param lx		The X value of the Quaternion
	 * @param ly		The Y value of the Quaternion
	 * @param lz		The Z value of the Quaternion
	 * @param lw		The W value of the Quaternion
	 */
	Quaternion(float lx, float ly, float lz, float lw) : Vector4d(lx, ly, lz, lw) {}

	/**
	 * Constructor from an existing Quaternion
	 * @param q		The existing quaternion
	 * @return		The new Quaternion
	 */
	Quaternion(const Quaternion &q) : Vector4d(q.x(), q.y(), q.z(), q.w()) {} 

	/**
	 * Constructor from a vector of four floats in the order X,Y,Z,W
	 * The initial values should be normalized, otherwise call normalize() after creation
	 * @param vec		The vector of floats comprising the quaternion
	 * @return		The new Quaternion
	 */
	Quaternion(const Vector4d &vec) : Vector4d(vec.x(), vec.y(), vec.z(), vec.w()) {}

	/**
	 * Build the saved Quaternion from the array of floats
	 * @param data          The array holding the four floats that comprise the Quaternion
	 * @return              The new Quaternion
	 */
	inline static Quaternion getQuaternion(const char *data) {
		return Quaternion(get_float(data), get_float(data + 4), get_float(data + 8), get_float(data + 12));
	}

	/**
	 * Constructs a Quaternion from Euler Coordinates
	 * @param pitch         The Euler Angle for pitch
	 * @param yaw           The Euler Angle for yaw
	 * @param roll          The Euler Angle for roll
	 * @return              The new Quaternion
	 */
	static Quaternion fromEuler(const Angle &yaw, const Angle &pitch, const Angle &roll);

	/**
	 * Converts from this Quaternion to a Matrix4 representation
	 * @return              The resulting matrix
	 */
	Matrix4 toMatrix() const;

	/**
	 * Converts from this Quaternion to a Matrix4 representation
	 * @param dst           The resulting matrix
	 */
	void toMatrix(Matrix4 &dst) const;

	/**
	 * Make a new Quaternion that's the inverse of this Quaternion
	 * @return		The resulting Quaternion
	 */
	Quaternion inverse() const;

	/**
	 * Slerps between this quaternion and to by factor t
	 * @param to		the quaternion to slerp between
	 * @param t		factor to slerp by.
	 * @return		the resulting quaternion.
	 */
	Quaternion slerpQuat(const Quaternion& to, const float t);
	
	/**
	 * Assignment operator for assigning a vector of values (X,Y,Z,W) to a Quaternion
	 * @param vec           The source vector
	 * @return              A reference to this Quaternion
	 */
	Quaternion& operator=(Vector4d &vec);

	/**
	 * Multiply two Quaternions
	 * @param quat          The Quaternion multiplicand
	 * @return              The result of the multiplication
	 */
	Quaternion operator*(const Quaternion &quat) const;
};

} // end of namespace Math

#endif
