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

#include "common/streamdebug.h"

#include "math/angle.h"
#include "math/utils.h"

namespace Math {

Angle::Angle(float degrees, float low) :
	_degrees(degrees), _rangeLow(low) {
	crop();
}

Angle::Angle(const Angle &a) :
	_degrees(a._degrees), _rangeLow(a._rangeLow) {

}

void Angle::setDegrees(float degrees) {
	_degrees = degrees;
	crop();
}

void Angle::setRadians(float radians) {
	_degrees = radianToDegree(radians);
	crop();
}

float Angle::getRadians() const {
	return degreeToRadian(_degrees);
}

float Angle::getDegrees(float low) const {
	return _degrees - _rangeLow + low;
}

float Angle::getRadians(float low) const {
	float d = getDegrees(low);
	return degreeToRadian(d);
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
	_rangeLow = a._rangeLow;

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

Angle Angle::fromRadians(float radians, float low) {
	return Angle(radianToDegree(radians), low);
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

void Angle::setRange(float low) {
	_rangeLow = low;
}

void Angle::crop() {
	if (_degrees >= _rangeLow + 360.f) {
		int x = (int)(_degrees - _rangeLow) / 360.f;
		_degrees -= 360.f * x;
	}
	if (_degrees < _rangeLow) {
		int x = (int)(_degrees + _rangeLow) / 360.f;
		_degrees -= 360.f * x;
	}
}

}

Common::Debug &operator<<(Common::Debug dbg, const Math::Angle &a) {
	dbg.nospace() << "Angle(" << a.getDegrees() << ")";

	return dbg.space();
}
