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

#ifndef BAGEL_HODJNPODJ_LIBS_VECTOR_H
#define BAGEL_HODJNPODJ_LIBS_VECTOR_H

#include "bagel/boflib/stdinc.h"

namespace Bagel {
namespace HodjNPodj {

#define PI      (double)3.141592653
#define RADCNVT ((double)180/PI)               // PI is 180 degrees

#define Deg2Rad(d) (d/RADCNVT)                  // converts degrees to radians
#define Rad2Deg(r) (r*RADCNVT)                  // converts radians to degrees

typedef Bagel::Vector VECTOR;

class CVector : public VECTOR {
public:
	CVector();
	CVector(const VECTOR &src);
	CVector(double xx, double yy, double zz = 0);

	// vector operations
	void    Unitize();
	void    Normalize();
	void    SetVector(double xx, double yy, double zz = 0);
	double  DotProduct(const VECTOR &rhs) const;
	void    Rotate(double angle);
	void    Reflect(const VECTOR &rhs);
	double  AngleBetween(const VECTOR &rhs);
	double  RealAngle(const VECTOR &rhs);
	double  Length() const;

	// Generic operations
	CVector operator +(const VECTOR &rhs) const;
	CVector operator +(double) const;
	CVector operator -(const VECTOR &rhs) const;
	CVector operator -(double) const;
	CVector operator *(double) const;
	CVector operator /(double) const;
	void    operator +=(const VECTOR &rhs);
	void    operator -=(const VECTOR &rhs);
	void    operator *=(double);
	void    operator /=(double);
	bool operator ==(const VECTOR &rhs) const;

protected:

private:
};

// misc Vector add-ons
extern double distanceBetweenPoints(const VECTOR &v1, const VECTOR &v2);

} // namespace HodjNPodj
} // namespace Bagel

#endif
