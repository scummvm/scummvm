/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#ifndef STARK_GFX_COORDINATE_H
#define STARK_GFX_COORDINATE_H

#include "math/vector3d.h"

namespace Stark {
namespace Gfx {

/**
 * Convenient 'joint' coordinate manipulation structure.
 *
 * This structure contains information on a bone+joint system - the best way to think
 * of this class is a vector to a location plus a quaterntion rotation to apply at
 * that position to any children (socket + ball style)
 */
class Coordinate {
public:
	Coordinate();
	Coordinate(const Math::Vector3d &left);
	Coordinate(float x, float y, float z);
	~Coordinate();

	float& getX() { return _coords[0]; }
	float getX() const { return _coords[0]; }
	float& getY() { return _coords[1]; }
	float getY() const { return _coords[1]; }
	float& getZ() { return _coords[2]; }
	float getZ() const { return _coords[2]; }

	/**
	 * Rotate the coordinate vector - socket rotation unaffected.
	 */
	void rotate(float w, float x, float y, float z);
	void rotate(const Coordinate &coord);
	/**
	 * Move coordinate vector - socket rotation unaffected.
	 */
	void translate(float x, float y, float z);
	void translate(const Coordinate &coord);

	/**
	 * Set the coordinate rotation to an explicit value
	 */
	void setRotation(float w, float x, float y, float z);
	/**
	 * Set the coordinate vector to an explicit value
	 */
	void setTranslation(float x, float y, float z);

	/**
	 * Add two coordinates - rotation affected as we're moving down the chain
	 */
	Coordinate& operator +=(const Coordinate &v) {
		_coords[0] += v._coords[0]; _coords[1] += v._coords[1]; _coords[2] += v._coords[2];
		
		float w, x, y, z;

		w = _coords[3]*v._coords[3] - _coords[4]*v._coords[4] - _coords[5]*v._coords[5] - _coords[6]*v._coords[6];
		x = _coords[3]*v._coords[4] + _coords[4]*v._coords[3] + _coords[5]*v._coords[6] - _coords[6]*v._coords[5];
		y = _coords[3]*v._coords[5] - _coords[4]*v._coords[6] + _coords[5]*v._coords[3] + _coords[6]*v._coords[4];
		z = _coords[3]*v._coords[6] + _coords[4]*v._coords[5] - _coords[5]*v._coords[4] + _coords[6]*v._coords[3];

		_coords[3] = w; _coords[4] = x; _coords[5] = y; _coords[6] = z;
		return *this;
	}

	/**
	 * Apply a scalar to the bone - vector change only
	 */
	Coordinate& operator *=(const float &f) {
		_coords[0] *= f; _coords[1] *= f; _coords[2] *= f;
		return *this;
	}

private:
	float _coords[7];

};

inline Coordinate operator +(const Coordinate& v1, const Coordinate& v2) {
	Coordinate result = v1;
	result += v2;
	return result;
}

inline Coordinate operator *(const Coordinate& c, const float f) {
	Coordinate result = c;
	result *= f;
	return result;
}

inline Coordinate operator -(const Coordinate& c) {
	return c * (-1.f);
}

} // End of namespace Gfx
} // End of namespace Stark

#endif
