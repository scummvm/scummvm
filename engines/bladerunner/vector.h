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

namespace BladeRunner {

class Vector2 {
public:
	float x;
	float y;

	Vector2();

	Vector2(float ax, float ay);
};

bool operator==(const Vector2 &a, const Vector2 &b);

bool operator!=(const Vector2 &a, const Vector2 &b);

class Vector3 {
public:
	float x;
	float y;
	float z;

	Vector3();

	Vector3(float ax, float ay, float az);

	float length();

	Vector3 normalize();

	static Vector3 cross(Vector3 a, Vector3 b);

	static float dot(Vector3 a, Vector3 b);

	Vector2 xz() const;
};

Vector3 operator+(Vector3 a, Vector3 b);

Vector3 operator-(Vector3 a, Vector3 b);

Vector3 operator*(float f, Vector3 v);

class Vector4 {
public:
	float x;
	float y;
	float z;
	float w;

	Vector4();

	Vector4(float ax, float ay, float az, float aw);
};

Vector4 operator+(Vector4 a, Vector4 b);

Vector4 operator-(Vector4 a, Vector4 b);

Vector4 operator*(float f, Vector4 v);

Vector4 operator*(Vector4 v, float f);

Vector4 operator/(Vector4 a, Vector4 b);

int angle_1024(float x1, float z1, float x2, float z2);

int angle_1024(const Vector3 &v1, const Vector3 &v2);

float distance(float x1, float z1, float x2, float z2);

float distance(const Vector2 &v1, const Vector2 &v2);

float distance(const Vector3 &v1, const Vector3 &v2);

bool lineIntersection(Vector2 a1, Vector2 a2, Vector2 b1, Vector2 b2, Vector2 *intersection);

} // End of namespace BladeRunner

#endif
