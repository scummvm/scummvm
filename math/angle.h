/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 */

#ifndef MATH_ANGLE_H
#define MATH_ANGLE_H

#include "common/scummsys.h"

namespace Common {
class Debug;
}

namespace Math {

class Angle {
public:
	/**
	 * Construct an angle object.
	 *
	 * \param degrees The angle, in degrees. Default is 0.
	 * \param low The lower bound of the range, in degrees. Default is -180.
	 */
	Angle(float degrees = 0, float low = -180);
	/**
	 * Construct and angle object, copying an already existing one.
	 *
	 * \param a The angle to copy.
	 */
	Angle(const Angle &a);

	void setDegrees(float degrees);
	void setRadians(float radians);

	/**
	 * Sets the range of the angle.
	 *
	 * \param low The lower bound of the range, in degrees.
	 */
	void setRange(float low);

	inline float getDegrees() const { return _degrees; }
	float getRadians() const;

	/**
	 * Returns the degrees of the angle, in the defined range.
	 *
	 * \param low The lower bound of the range, in degrees.
	 */
	float getDegrees(float low) const;
	/**
	 * Returns the radianss of the angle, in the defined range.
	 *
	 * \param low The lower bound of the range, in degrees.
	 */
	float getRadians(float low) const;

	float getCosine() const;
	float getSine() const;
	float getTangent() const;

	Angle &operator=(const Angle &a);
	Angle &operator=(float degrees);
	Angle &operator+=(const Angle &a);
	Angle &operator+=(float degrees);
	Angle &operator-=(const Angle &a);
	Angle &operator-=(float degrees);

	/**
	 * Build an angle object.
	 *
	 * \param radians The angle, in radians.
	 * \param low The lower bound of the range, in degrees. Default is -180.
	 */
	static Angle fromRadians(float radians, float low = -180);

	static Angle arcCosine(float x);
	static Angle arcSine(float x);
	static Angle arcTangent(float x);
	static Angle arcTangent2(float y, float x);

private:
	void crop();

	float _degrees;
	float _rangeLow;

};

inline Angle operator-(const Angle &a) {
	return Angle(-a.getDegrees());
}

inline Angle operator+(const Angle &a1, const Angle &a2) {
	return Angle(a1.getDegrees() + a2.getDegrees());
}
inline Angle operator+(const Angle &a1, float degrees) {
	return Angle(a1.getDegrees() + degrees);
}
inline Angle operator+(float degrees, const Angle &a2) {
	return Angle(degrees + a2.getDegrees());
}


inline Angle operator-(const Angle &a1, const Angle &a2) {
	return Angle(a1.getDegrees() - a2.getDegrees());
}
inline Angle operator-(const Angle &a1, float degrees) {
	return Angle(a1.getDegrees() - degrees);
}
inline Angle operator-(float degrees, const Angle &a2) {
	return Angle(degrees - a2.getDegrees());
}

inline Angle operator*(const Angle &a1, float f) {
	return Angle(a1.getDegrees() * f);
}
inline Angle operator*(float f, const Angle &a2) {
	return Angle(a2.getDegrees() * f);
}

inline Angle operator/(const Angle &a1, float f) {
	return Angle(a1.getDegrees() / f);
}
inline Angle operator/(float f, const Angle &a2) {
	return Angle(a2.getDegrees() / f);
}


inline bool operator==(const Angle &a1, const Angle &a2) {
	return fabsf(a1.getDegrees() - a2.getDegrees()) < 0.001;
}

inline bool operator!=(const Angle &a1, const Angle &a2) {
	return !(a1 == a2);
}



inline bool operator<(const Angle &a1, const Angle &a2) {
	return a1.getDegrees() < a2.getDegrees();
}
inline bool operator<(const Angle &a1, float degrees) {
	return a1.getDegrees() < degrees;
}
inline bool operator<(float degrees, const Angle &a2) {
	return degrees < a2.getDegrees();
}


inline bool operator>(const Angle &a1, const Angle &a2) {
	return a1.getDegrees() > a2.getDegrees();
}
inline bool operator>(const Angle &a1, float degrees) {
	return a1.getDegrees() > degrees;
}
inline bool operator>(float degrees, const Angle &a2) {
	return degrees > a2.getDegrees();
}

}

Common::Debug &operator<<(Common::Debug dbg, const Math::Angle &a);

#endif
