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

#include "math/rotation3d.h"
#include "math/vector.h"
#include "math/angle.h"
#include "math/vector4d.h"
#include "math/matrix4.h"

namespace Math {

class Quaternion : public Vector4d {
public:
	/**
	 * Default Constructor, creates an identity Quaternion with no rotation.
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
	 * Constructor from a rotation matrix
	 * @param m		The rotation matrix
	 * @return		The new Quaternion
	 */
	Quaternion(const Matrix3 &m);

	/**
	 * Constructor from a rotation matrix
	 * @param m		The rotation matrix
	 * @return		The new Quaternion
	 */
	Quaternion(const Matrix4 &m);

	/** Set the Quaternion from a rotation matrix
	 * @param m		The matrix used to set the Quaternion
	 */
	void fromMatrix(const Matrix3 &m);

	/**
	 * Constructor from an axis vector and the angle to rotate on that axis
	 * @param axis		The axis to perform the rotation around
	 * @param angle		The angle amount to rotate
	 * @return		The new Quaternion
	 */
	Quaternion(const Vector3d &axis, const Angle &angle);

	/**
	 * Constructs a Quaternion from Euler Coordinates
	 * @param first		The Euler Angle for the first Axis
	 * @param second	The Euler Angle for the second Axis
	 * @param third		The Euler Angle for the third Axis
	 * @param order		The Euler Order, specified in Rotation3D
	 * @return		The new Quaternion
	 */
	static Quaternion fromEuler(const Angle &first, const Angle &second, const Angle &third, EulerOrder order);

	/**
	 * Returns Euler Angles based on the Euler Order
	 * @param first		The Euler Angle for the first Axis
	 * @param second	The Euler Angle for the second Axis
	 * @param third		The Euler Angle for the third Axis
	 * @param order		The Euler Order, specified in Rotation3D
	 * @return		The new Quaternion
	 */
	void getEuler(Angle *first, Angle *second, Angle *third, EulerOrder order) const;

	/**
	 * Create a Quaternion from a rotation around the X Axis
	 * @param angle		The Euler Angle for rotation
	 * @return		The resulting Quaternion
	 */
	static Quaternion xAxis(const Angle &angle);

	/**
	 * Create a Quaternion from a rotation around the Y Axis
	 * @param angle		The Euler Angle for rotation
	 * @return		The resulting Quaternion
	 */
	static Quaternion yAxis(const Angle &angle);

	/**
	 * Create a Quaternion from a rotation around the Z Axis
	 * @param angle		The Euler Angle for rotation
	 * @return		The resulting Quaternion
	 */
	static Quaternion zAxis(const Angle &angle);

	/**
	 * Normalize the Quaternion
	 * @return		A reference to this quaternion
	 */
	Quaternion &normalize();

	/**
	 * Rotate a vector by a Quaternion
	 * @param v The Vector to be rotated
	 */
	void transform(Vector3d &v) const;

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
	Quaternion slerpQuat(const Quaternion& to, const float t) const;

	/**
	 * Get the direction vector specified by col
	 * @param col		Column in the rotation matrix to get the direction vector from
	 * @return		The resulting Vector3d
	 */
	Vector3d directionVector(const int col) const;

	/**
	 * Get the angle between two quaternions
	 * @param to		The quaternion we're comparing against
	 * @return		The angle between the two
	 */
	Angle getAngleBetween(const Quaternion &to);

	/**
	 * Assignment operator
	 * @param vec           The source quaternion
	 * @return              A reference to this Quaternion
	 */
	Quaternion& operator=(const Quaternion& quat);

	/**
	 * Multiply two Quaternions
	 * @param quat          The Quaternion multiplicand
	 * @return              The result of the multiplication
	 */
	Quaternion operator*(const Quaternion &quat) const;
	Quaternion& operator*=(const Quaternion &quat);

	/**
	 * Multiply this Quaternion by a constant
	 * @param quat 		The Quaternion multiplicand
	 * @return		The result of the multiplication
	 */
	Quaternion operator*(const float c) const;

	/**
	 * Sum two quaternions
	 * @param quat 		The Quaternion to be added
	 * @return		The result of the addition
	 */
	Quaternion operator+(const Quaternion &o) const;
	Quaternion& operator+=(const Quaternion &o);

	/**
	 * Compare quaternions
	 * @param quat 		The Quaternion to be compared
	 * @return		The result of the comparison
	 */
	bool operator==(const Quaternion &o) const;
	bool operator!=(const Quaternion &o) const;
};

} // end of namespace Math

#endif
