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
class DVector;

/**
 * Floating point vector class.
 * @remarks		TODO: See if it can be merged with DVector
 */
class FVector {
public:
	float _x, _y, _z;
public:
	FVector() : _x(0), _y(0), _z(0) {}
	FVector(float x, float y, float z) : _x(x), _y(y), _z(z) {}
	FVector(const DVector &src);

	/**
	 * Clears the vector
	 */
	void clear() {
		_x = _y = _z = 0.0;
	}

	FVector fn1() const;

	/**
	 * Calculates the cross-product between this matrix and a passed one
	 */
	FVector crossProduct(const FVector &src) const;

	/**
	 * Normalizes the vector so the length from origin equals 1.0
	 */
	float normalize();

	/**
	 * Adds the current vector and a passed one together, normalizes them,
	 * and then returns the resulting vector
	 */
	FVector addAndNormalize(const FVector &v) const;

	/**
	 * Returns the distance between a specified point and this one
	 */
	float getDistance(const FVector &src) const;

	FVector fn5(const FPose &pose) const;

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
