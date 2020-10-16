/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef MATH_ANGLE_H
#define MATH_ANGLE_H

#include "common/scummsys.h"

namespace Common {
class StreamDebug;
}

namespace Math {

class Angle {
public:
	/**
	 * Construct an angle object.
	 *
	 * \param degrees The angle, in degrees. Default is 0.
	 */
	Angle(float degrees = 0);
	/**
	 * Construct and angle object, copying an already existing one.
	 *
	 * \param a The angle to copy.
	 */
	Angle(const Angle &a);

	/**
	 * Normalize the angle in a [x; x + 360] range and return the object.
	 *
	 * \param low The lower bound of the range, in degrees.
	 */
	Angle &normalize(float low);
	
	/**
	 * Clamp the angle to range [-mag, mag]
	 *
	 * \param mag The maximum distance from 0, in degrees.
	 */
	Angle &clampDegrees(float mag);

	/**
	* Clamp the angle to range [-min, max]
	*
	* \param min The lower bound of the range, in degrees.
	* \param max The upper bound of the range, in degrees.
	*/
	Angle &clampDegrees(float min, float max);

	void setDegrees(float degrees);
	void setRadians(float radians);

	float getDegrees() const;
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
	 */
	static Angle fromRadians(float radians);

	static Angle arcCosine(float x);
	static Angle arcSine(float x);
	static Angle arcTangent(float x);
	static Angle arcTangent2(float y, float x);

private:
	float _degrees;

};

inline Angle operator-(const Angle &a) {
	return Angle(-a.getDegrees());
}

inline Angle operator+(const Angle &a1, const Angle &a2) {
	return Angle(a1.getDegrees() + a2.getDegrees());
}

inline Angle operator-(const Angle &a1, const Angle &a2) {
	return Angle(a1.getDegrees() - a2.getDegrees());
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

inline bool operator>(const Angle &a1, const Angle &a2) {
	return a1.getDegrees() > a2.getDegrees();
}

Common::StreamDebug &operator<<(Common::StreamDebug &dbg, const Math::Angle &a);

}

#endif
