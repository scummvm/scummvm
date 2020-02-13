/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#ifndef TITANIC_FVECTOR_H
#define TITANIC_FVECTOR_H

#include "titanic/star_control/fpoint.h"

namespace Titanic {

enum Axis { X_AXIS, Y_AXIS, Z_AXIS };

class FPose;

/**
 * Floating point vector class.
 */
class FVector {
public:
	float _x, _y, _z;
public:
	FVector() : _x(0), _y(0), _z(0) {}
	FVector(float x, float y, float z) : _x(x), _y(y), _z(z) {}

	/**
	 * Clears the vector
	 */
	void clear() {
		_x = _y = _z = 0.0;
	}

	/**
	 * Returns a vector with all components of this vector circularlly rotated up 1.
	 * this x being _y, this y being _z, and this z being _x. A sign change may also
	 * be done on x/_y based on some conditions.
	 */
	FVector swapComponents() const;

	/**
	 * Calculates the cross-product between this matrix and a passed one
	 */
	FVector crossProduct(const FVector &src) const;

	/**
	 * Rotate this vector about the Y axis
	 */
	void rotVectAxisY(float angleDeg);

	/**
	 * Attempts to normalizes the vector so the length from origin equals 1.0
	 * Return value is whether or not it was successful in normalizing
	 * First argument is scale value that normalizes the vector
	 */
	bool normalize(float &hyp);

	void normalize() {
		float hyp;
		bool result = normalize(hyp);
		assert(result);
	}

	/**
	 * Calculates a vector halfway between two given vectors
	 */
	FVector half(const FVector &v) const;

	/**
	 * Returns a vector, v, that represents a magnitude, and two angles in radians
	 * 1. Scale this vector to be unit magnitude and store scale in x component of v
	 * 2. X rotation angle from +y axis of this vector is put in y component of v
	 * 3. z component output of v is the 4-quadrant angle that z makes with x (Y axis rotation)
	 */
	FVector getPolarCoord() const;

	/**
	 * Returns the distance between a specified point and this one
	 */
	float getDistance(const FVector &src) const;

	/**
	 * Returns a vector that is this vector on the left as a row vector
	 * times the 3x4 affine matrix on the right.
	 */
	FVector matProdRowVect(const FPose &pose) const;

	/**
	 * Returns a matrix that contains the frame rotation based on this vector and 
	 * a vector rotation based on input vector v
	 */
	FPose getFrameTransform(const FVector &v);

	/**
	 * Constructs an affine matrix that does a x then a y axis frame rotation
	 * based on the orientation of this vector
	 */
	FPose formRotXY() const;

	/**
	 * Returns true if the passed vector equals this one
	 */
	bool operator==(const FVector &src) const {
		return _x == src._x && _y == src._y && _z == src._z;
	}

	/**
	 * Returns true if the passed vector does not equal this one
	 */
	bool operator!=(const FVector &src) const {
		return _x != src._x || _y != src._y || _z != src._z;
	}

	FVector operator+(const FVector &delta) const {
		return FVector(_x + delta._x, _y + delta._y, _z + delta._z);
	}

	FVector operator-(const FVector &delta) const {
		return FVector(_x - delta._x, _y - delta._y, _z - delta._z);
	}

	const FVector operator*(float right) const {
		return FVector(_x * right, _y * right, _z * right);
	}

	const FVector operator*(const FVector &right) const {
		return FVector(_x * right._x, _y * right._y, _z * right._z);
	}	

	void operator+=(const FVector &delta) {
		_x += delta._x;
		_y += delta._y;
		_z += delta._z;
	}

	void operator-=(const FVector &delta) {
		_x -= delta._x;
		_y -= delta._y;
		_z -= delta._z;
	}

	void operator+=(const FPoint &delta) {
		_x += delta._x;
		_y += delta._y;
	}

	void operator-=(const FPoint &delta) {
		_x -= delta._x;
		_y -= delta._y;
	}

	/**
	 * Converts the vector to a string
	 */
	Common::String toString() const;
};

} // End of namespace Titanic

#endif /* TITANIC_FVECTOR_H */
