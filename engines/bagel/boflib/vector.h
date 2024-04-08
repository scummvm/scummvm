
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

#ifndef BAGEL_BOFLIB_VECTOR_H
#define BAGEL_BOFLIB_VECTOR_H

#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/object.h"

namespace Bagel {

#define PI (double)3.141592653
#define RADCNVT ((double)180 / PI) // PI is 180 degrees

#define Deg2Rad(d) (d / RADCNVT) // converts degrees to radians
#define Rad2Deg(r) (r * RADCNVT) // converts radians to degrees

class CVector : public CBofObject, public VECTOR {
public:
	/**
	 * Default constructor
	 */
	CVector();

	/**
	 * Copy constructor
	 */
	CVector(const VECTOR &stVector);

	/**
	 * Constructor based on passed figures
	 */
	CVector(double xx, double yy, double zz = 0);

#if BOF_DEBUG
	/**
	 * Destructor
	 */
	virtual ~CVector();
#endif

	// Vector operations
	//
	/**
	 * Forces this vector to be a unit vector
	 */
	void Unitize();

	/**
	 * Forces this vector to be a notmal vector
	 */
	void Normalize();

	/**
	 * Assigns new specified values to this vector
	 * @param xx        X value
	 * @param yy        Y value
	 * @param zz        Z value
	 */
	void SetVector(double, double, double zz = 0);

	/**
	 * Calculates the dot-product of the 2 specified vectors
	 * @param vector        Second vector
	 * @return              Dot-product
	 */
	double DotProduct(const VECTOR &vector);

	/**
	 * Calculates the cross-product of the 2 specified vectors
	 * @param vector        Second vector
	 * @return              Cross-product
	 */
	CVector CrossProduct(const VECTOR &vector);

	/**
	 * Rotates this vector the specified number of degrees
	 * @param fAngle        Rotation angle
	 */
	void Rotate(double fAngle);

	/**
	 * Reflects this vector around the specified vector
	 * @param vMirror       Mirro vector
	 */
	void Reflect(const VECTOR &vMirror);

	/**
	 * Gets the angle between this vector and specified vector
	 * @param vector    Second vector
	 * @return          Angle
	 */
	double AngleBetween(const VECTOR &vector);

	/**
	 * Calculates the positive or negative angle between 2 vectors
	 * @param vector        Second vector
	 * @return              Angle
	 */
	double RealAngle(const VECTOR &vector);

	/**
	 * Calculates the relative angle between 2 vectors
	 * @return              Angle
	 */
	double Angle();

	/**
	 * Gets the length of this vector
	 * @return      Vector length
	 */
	double Length();

	// Generic operations
	CVector operator+(VECTOR);
	CVector operator+(double);
	CVector operator-(VECTOR);
	CVector operator-(double);
	CVector operator*(double);
	CVector operator/(double);
	void operator+=(VECTOR);
	void operator-=(VECTOR);
	void operator*=(double);
	void operator/=(double);
	BOOL operator==(VECTOR);
};

// Misc Vector add-ons
double DistanceBetweenPoints(VECTOR, VECTOR);

} // namespace Bagel

#endif
