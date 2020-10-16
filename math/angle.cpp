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

#include "common/streamdebug.h"
#include "common/math.h"

#include "math/angle.h"

namespace Math {

Angle::Angle(float degrees) :
	_degrees(degrees) {
}

Angle::Angle(const Angle &a) :
	_degrees(a._degrees) {

}

Angle &Angle::normalize(float low) {
	_degrees = getDegrees(low);

	return *this;
}

Angle &Angle::clampDegrees(float mag) {
    _degrees = getDegrees(-180.f);
    if (_degrees >= mag)
		setDegrees(mag);
	if (_degrees <= -mag)
		setDegrees(-mag);
    
    return *this;
}

Angle &Angle::clampDegrees(float min, float max) {
	_degrees = getDegrees(-180.f);
	if (_degrees >= max)
		setDegrees(max);
	if (_degrees <= min)
		setDegrees(min);

	return *this;
}

void Angle::setDegrees(float degrees) {
	_degrees = degrees;
}

void Angle::setRadians(float radians) {
	_degrees = Common::rad2deg(radians);
}

float Angle::getDegrees() const {
	return _degrees;
}

float Angle::getRadians() const {
	return Common::deg2rad(getDegrees());
}

float Angle::getDegrees(float low) const {
	float degrees = _degrees;
	if (degrees >= low + 360.f) {
		float x = floor((degrees - low) / 360.f);
		degrees -= 360.f * x;
	} else if (degrees < low) {
		float x = floor((degrees - low) / 360.f);
		degrees -= 360.f * x;
	}
	return degrees;
}

float Angle::getRadians(float low) const {
	float d = getDegrees(low);
	return Common::deg2rad(d);
}

float Angle::getCosine() const {
	return cosf(getRadians());
}

float Angle::getSine() const {
	return sinf(getRadians());
}

float Angle::getTangent() const {
	return tanf(getRadians());
}

Angle &Angle::operator=(const Angle &a) {
	_degrees = a._degrees;

	return *this;
}

Angle &Angle::operator=(float degrees) {
	setDegrees(degrees);

	return *this;
}

Angle &Angle::operator+=(const Angle &a) {
	setDegrees(_degrees + a._degrees);

	return *this;
}

Angle &Angle::operator+=(float degrees) {
	setDegrees(_degrees + degrees);

	return *this;
}

Angle &Angle::operator-=(const Angle &a) {
	setDegrees(_degrees - a._degrees);

	return *this;
}

Angle &Angle::operator-=(float degrees) {
	setDegrees(_degrees - degrees);

	return *this;
}

Angle Angle::fromRadians(float radians) {
	return Angle(Common::rad2deg(radians));
}

Angle Angle::arcCosine(float x) {
	Angle a;
	a.setRadians(acosf(x));
	return a;
}

Angle Angle::arcSine(float x) {
	Angle a;
	a.setRadians(asinf(x));
	return a;
}

Angle Angle::arcTangent(float x) {
	Angle a;
	a.setRadians(atanf(x));
	return a;
}

Angle Angle::arcTangent2(float y, float x) {
	Angle a;
	a.setRadians(atan2f(y, x));
	return a;
}

Common::StreamDebug &operator<<(Common::StreamDebug &dbg, const Math::Angle &a) {
	dbg.nospace() << "Angle(" << a.getDegrees(-180) << ")";

	return dbg.space();
}

}
