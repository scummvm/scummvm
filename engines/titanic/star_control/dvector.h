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

#ifndef TITANIC_DVECTOR_H
#define TITANIC_DVECTOR_H

#include "titanic/star_control/fvector.h"

namespace Titanic {

const double Rad2Deg = 180.0 / M_PI;
const double Deg2Rad = 1.0 / Rad2Deg;

class DAffine;

/**
 * Double based vector class.
 * @remarks		TODO: See if it can be merged with FVector
 */
class DVector {
public:
	double _x, _y, _z;
public:
	DVector() : _x(0), _y(0), _z(0) {}
	DVector(double x, double y, double z) : _x(x), _y(y), _z(z) {}
	DVector(const FVector &v) : _x(v._x), _y(v._y), _z(v._z) {}

	/**
	 * Attempts to normalizes the vector so the length from origin equals 1.0
	 * Return value is whether or not it was successful in normalizing
	 * First argument is scale value that normalizes the vector
	 * TODO: split this function into 2. One that calculates the normalization
	 * and another that does the normalization. The 2nd would assert if a 
	 * normalization of one was requested. This is cleaner than the current 
	 * implementation.
	 */
	bool normalize(double &);

	/**
	 * Returns the distance between this vector and the passed one
	 */
	double getDistance(const DVector &src);

	/**
	 * Returns the matrix product with this vector and 
	 * also does a z translations. Doesn't change this vector
	 */
	DVector dAffMatrixProdVec(const DAffine &m);

	/**
	 * Rotate this vector about the Y axis
	 */
	void rotVectAxisY(double angleDeg);

	/**
	 * Returns a vector, v, that represents a magnitude, and two angles in radians
	 * 1. Scale this vector to be unit magnitude and store scale in x component of v
	 * 2. X rotation angle from +y axis of this vector is put in y component of v
	 * 3. z component output of v is the 4-quadrant angle that z makes with x (Y axis rotation)
	 */
	DVector getAnglesAsVect() const;

	/**
	 * Returns a matrix that contains the frame rotation based on this vector and 
	 * a vector rotation based on input vector v
	 */
	DAffine getFrameTransform(const DVector &v);

	/**
	 * Returns a affine matrix that does a x then a y axis frame rotation
	 * based on the orientation of this vector
	 */
	DAffine rotXY() const;

	/**
	 * Returns true if the passed vector equals this one
	 */
	bool operator==(const DVector &src) const {
		return _x == src._x && _y == src._y && _z == src._z;
	}

	/**
	 * Returns true if the passed vector does not equal this one
	 */
	bool operator!=(const DVector &src) const {
		return _x != src._x || _y != src._y || _z != src._z;
	}

	DVector operator+(const DVector &delta) const {
		return DVector(_x + delta._x, _y + delta._y, _z + delta._z);
	}

	DVector operator-(const DVector &delta) const {
		return DVector(_x - delta._x, _y - delta._y, _z - delta._z);
	}

	void operator+=(const DVector &delta) {
		_x += delta._x;
		_y += delta._y;
		_z += delta._z;
	}

	void operator-=(const DVector &delta) {
		_x -= delta._x;
		_y -= delta._y;
		_z -= delta._z;
	}

	const DVector operator*(double right) const {
		return DVector(_x * right, _y * right, _z * right);
	}

	const DVector operator*(const DVector &right) const {
		return DVector(_x * right._x, _y * right._y, _z * right._z);
	}
};

} // End of namespace Titanic

#endif /* TITANIC_DVECTOR_H */
