
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
#include "bagel/boflib/bof_object.h"

namespace Bagel {

#define PI (DOUBLE)3.141592653
#define RADCNVT ((DOUBLE)180 / PI) // PI is 180 degrees

#define Deg2Rad(d) (d / RADCNVT) // converts degrees to radians
#define Rad2Deg(r) (r * RADCNVT) // converts radians to degrees

class CVector : public CBofObject, public bofVECTOR {
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
	CVector(DOUBLE xx, DOUBLE yy, DOUBLE zz = 0);

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
	VOID Unitize();

	/**
	 * Forces this vector to be a notmal vector
	 */
	VOID Normalize();

	/**
	 * Assigns new specified values to this vector
	 * @param xx		X value
	 * @param yy		Y value
	 * @param zz		Z value
	 */
	VOID SetVector(DOUBLE, DOUBLE, DOUBLE zz = 0);

	/**
	 * Calculates the dot-product of the 2 specified vectors
	 * @param vector		Second vector
	 * @return				Dot-product
	 */
	DOUBLE DotProduct(const VECTOR &vector);

	/**
	 * Calculates the cross-product of the 2 specified vectors
	 * @param vector		Second vector
	 * @return				Cross-product
	 */
	CVector CrossProduct(const VECTOR &vector);
	
	/**
	 * Rotates this vector the specified number of degrees
	 * @param fAngle		Rotation angle
	 */
	VOID Rotate(DOUBLE fAngle);

	/**
	 * Reflects this vector around the specified vector
	 * @param vMirror		Mirro vector
	 */
	VOID Reflect(const VECTOR &vMirror);

	/**
	 * Gets the angle between this vector and specified vector
	 * @param vector	Second vector
	 * @return			Angle
	 */
	DOUBLE AngleBetween(const VECTOR &vector);

	/**
	 * Calculates the positive or negative angle between 2 vectors
	 * @param vector		Second vector
	 * @return				Angle
	 */
	DOUBLE RealAngle(const VECTOR &vector);

	/**
	 * Calculates the relative angle between 2 vectors
	 * @return				Angle
	 */
	DOUBLE Angle();

	/**
	 * Gets the length of this vector
	 * @return		Vector length
	 */
	DOUBLE Length();

	// Generic operations
	CVector operator+(VECTOR);
	CVector operator+(DOUBLE);
	CVector operator-(VECTOR);
	CVector operator-(DOUBLE);
	CVector operator*(DOUBLE);
	CVector operator/(DOUBLE);
	VOID operator+=(VECTOR);
	VOID operator-=(VECTOR);
	VOID operator*=(DOUBLE);
	VOID operator/=(DOUBLE);
	BOOL operator==(VECTOR);
};

// Misc Vector add-ons
DOUBLE DistanceBetweenPoints(VECTOR, VECTOR);

} // namespace Bagel

#endif
