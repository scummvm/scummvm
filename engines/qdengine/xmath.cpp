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

////////////////////////////////////////////////////////////////////////////////
//	XMath 1.0
//	Vector - Matrix library for 3D visualisation and
//	physics modeling.
//
//	(C) 1999 KDV Games
//	Author: Alexandre Kotliar
//
//
////////////////////////////////////////////////////////////////////////////////
#include "qdengine/xmath.h"


namespace QDEngine {


///////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
///////////////////////////////////////////////////////////////////////////////

const Vect3f Vect3f::ZERO(0, 0, 0);

///////////////////////////////////////////////////////////////////////////////
//
//  class Vect2f
//
///////////////////////////////////////////////////////////////////////////////

float Vect2f::angle(const Vect2f &other) const {
	return acosf(dot(other) * invSqrtFast(norm2() * other.norm2() * 1.0000001f));
}

///////////////////////////////////////////////////////////////////////////////
//
//  class Vect3f
//
///////////////////////////////////////////////////////////////////////////////

float Vect3f::angle(const Vect3f &other) const {
	return acosf(dot(other) * invSqrtFast(norm2() * other.norm2() * 1.0000001f));
}

} // namespace QDEngine
