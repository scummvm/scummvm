
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BAGEL_BOFLIB_Vector_H
#define BAGEL_BOFLIB_Vector_H

#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/object.h"

namespace Bagel {

#define PI (double)3.141592653
#define RADCNVT ((double)180 / PI) // PI is 180 degrees

#define Deg2Rad(d) ((d) / RADCNVT) // Converts degrees to radians
#define Rad2Deg(r) ((r) * RADCNVT) // Converts radians to degrees

class CVector : public CBofObject, public Vector {
public:
	/**
	 * Default constructor
	 */
	CVector();

	/**
	 * Copy constructor
	 */
	CVector(const Vector &stVector);

	/**
	 * Constructor based on passed figures
	 */
	CVector(double xx, double yy, double zz = 0);

	/**
	 * Calculates the dot-product of the 2 specified vectors
	 * @param vector        Second vector
	 * @return              Dot-product
	 */
	double dotProduct(const Vector &vector);

	/**
	 * Rotates this vector the specified number of degrees
	 * @param fAngle        Rotation angle
	 */
	void rotate(double fAngle);

	/**
	 * Gets the angle between this vector and specified vector
	 * @param vector    Second vector
	 * @return          Angle
	 */
	double angleBetween(const Vector &vector);

	/**
	 * Calculates the positive or negative angle between 2 vectors
	 * @param vector        Second vector
	 * @return              Angle
	 */
	double realAngle(const Vector &vector);

	/**
	 * Gets the length of this vector
	 * @return      Vector length
	 */
	double length();

	// Generic operations
	CVector operator+(Vector);
	CVector operator+(double);
	CVector operator-(Vector);
	CVector operator-(double);
	CVector operator*(double);
	CVector operator/(double);
	void operator+=(Vector);
	void operator-=(Vector);
	void operator*=(double);
	void operator/=(double);
	bool operator==(Vector);
};

} // namespace Bagel

#endif
