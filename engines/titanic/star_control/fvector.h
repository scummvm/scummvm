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

namespace Titanic {

class CStarControlSub6;

/**
 * Floating point vector class.
 * @remarks		TODO: See if it can be merged with DVector
 */
class FVector {
public:
	double _x, _y, _z;
public:
	FVector() : _x(0), _y(0), _z(0) {}
	FVector(double x, double y, double z) : _x(x), _y(y), _z(z) {}

	void fn1(FVector *v);
	void multiply(FVector *dest, const FVector *src);
	void fn3();

	/**
	 * Returns the distance between a specified point and this one
	 */
	double getDistance(const FVector *src) const;

	static void fn4(FVector *dest, const FVector *v1, const FVector *v2);
	void fn5(FVector *dest, const CStarControlSub6 *sub6) const;

	/**
	 * Returns true if the passed vector equals this one
	 */
	bool operator==(const FVector &src) const {
		return _x != src._x || _y != src._y || _z != src._z;
	}

	/**
	 * Returns true if the passed vector does not equal this one
	 */
	bool operator!=(const FVector &src) const {
		return !operator==(src);
	}
};

} // End of namespace Titanic

#endif /* TITANIC_FVECTOR_H */
