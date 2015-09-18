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

#ifndef BLADERUNNER_VECTOR_H
#define BLADERUNNER_VECTOR_H

#include "common/types.h"

namespace BladeRunner {

class Vector2
{
public:
	float x;
	float y;

	Vector2()
		: x(0.0), y(0.0)
	{}

	Vector2(float ax, float ay)
		: x(ax), y(ay)
	{}
};

class Vector3
{
public:
	float x;
	float y;
	float z;

	Vector3()
		: x(0.0), y(0.0), z(0.0)
	{}

	Vector3(float ax, float ay, float az)
		: x(ax), y(ay), z(az)
	{}

	float length() { return sqrtf(x * x + y * y + z * z); }
};

inline
Vector3 operator+(Vector3 a, Vector3 b)
{
	return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline
Vector3 operator-(Vector3 a, Vector3 b)
{
	return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline
Vector3 operator*(float f, Vector3 v)
{
	return Vector3(f * v.x, f * v.y, f * v.z);
}

class Vector4
{
public:
	float x;
	float y;
	float z;
	float w;

	Vector4()
		: x(0.0), y(0.0), z(0.0), w(0.0)
	{}

	Vector4(float ax, float ay, float az, float aw)
		: x(ax), y(ay), z(az), w(aw)
	{}
};

inline
Vector4 operator+(Vector4 a, Vector4 b)
{
	return Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

inline
Vector4 operator-(Vector4 a, Vector4 b)
{
	return Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

inline
Vector4 operator*(float f, Vector4 v)
{
	return Vector4(f * v.x, f * v.y, f * v.z, f * v.w);
}

inline
Vector4 operator*(Vector4 v, float f)
{
	return Vector4(f * v.x, f * v.y, f * v.z, f * v.w);
}

inline
Vector4 operator/(Vector4 a, Vector4 b)
{
	return Vector4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

} // End of namespace BladeRunner

#endif
