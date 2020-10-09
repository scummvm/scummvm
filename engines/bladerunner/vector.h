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

class Vector2 {
public:
	float x;
	float y;

	Vector2() : x(0.0), y(0.0) {}

	Vector2(float ax, float ay) : x(ax), y(ay) {}
};

inline bool operator==(const Vector2 &a, const Vector2 &b) {
	return a.x == b.x && a.y == b.y;
}

inline bool operator!=(const Vector2 &a, const Vector2 &b) {
	return !(a == b);
}

class Vector3 {
public:
	float x;
	float y;
	float z;

	Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

	Vector3(float ax, float ay, float az) : x(ax), y(ay), z(az) {}

	inline float length() {
		return sqrt(x * x + y * y + z * z);
	}

	inline Vector3 normalize() {
		float len = length();
		if (len == 0) {
			return Vector3(0.0f, 0.0f, 0.0f);
		}
		return Vector3(x / len, y / len, z / len);
	}

	inline static Vector3 cross(Vector3 a, Vector3 b) {
		return Vector3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x);
	}

	inline static float dot(Vector3 a, Vector3 b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	Vector2 xz() const {
		return Vector2(x, z);
	}
};

inline Vector3 operator+(Vector3 a, Vector3 b) {
	return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vector3 operator-(Vector3 a, Vector3 b) {
	return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vector3 operator*(float f, Vector3 v) {
	return Vector3(f * v.x, f * v.y, f * v.z);
}

class Vector4 {
public:
	float x;
	float y;
	float z;
	float w;

	Vector4() : x(0.0), y(0.0), z(0.0), w(0.0) {}

	Vector4(float ax, float ay, float az, float aw) : x(ax), y(ay), z(az), w(aw) {}
};

inline Vector4 operator+(Vector4 a, Vector4 b) {
	return Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

inline Vector4 operator-(Vector4 a, Vector4 b) {
	return Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

inline Vector4 operator*(float f, Vector4 v) {
	return Vector4(f * v.x, f * v.y, f * v.z, f * v.w);
}

inline Vector4 operator*(Vector4 v, float f) {
	return Vector4(f * v.x, f * v.y, f * v.z, f * v.w);
}

inline Vector4 operator/(Vector4 a, Vector4 b) {
	return Vector4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

inline int angle_1024(float x1, float z1, float x2, float z2) {
	float angle_rad = atan2(x2 - x1, z1 - z2);
	int a = int(512.0 * angle_rad / M_PI);
	return (a + 1024) % 1024;
}

inline int angle_1024(const Vector3 &v1, const Vector3 &v2) {
	return angle_1024(v1.x, v1.z, v2.x, v2.z);
}

inline float distance(float x1, float z1, float x2, float z2) {
	float dx = x1 - x2;
	float dz = z1 - z2;
	float d = sqrt(dx * dx + dz * dz);

	float int_part = (int)d;
	float frac_part = d - int_part;

	if (frac_part < 0.001)
		frac_part = 0.0;

	return int_part + frac_part;
}

inline float distance(const Vector2 &v1, const Vector2 &v2) {
	return distance(v1.x, v1.y, v2.x, v2.y);
}

inline float distance(const Vector3 &v1, const Vector3 &v2) {
	return distance(v1.x, v1.z, v2.x, v2.z);
}

inline bool lineIntersection(Vector2 a1, Vector2 a2, Vector2 b1, Vector2 b2, Vector2 *intersection) {
	Vector2 s1(a2.x - a1.x, a2.y - a1.y);
	Vector2 s2(b2.x - b1.x, b2.y - b1.y);

	float s = (s1.x * (a1.y - b1.y) - s1.y * (a1.x - b1.x)) / (s1.x * s2.y - s2.x * s1.y);
	float t = (s2.x * (a1.y - b1.y) - s2.y * (a1.x - b1.x)) / (s1.x * s2.y - s2.x * s1.y);

	if (s >= 0.0f && s <= 1.0f && t >= 0.0f && t <= 1.0f) {
		intersection->x = a1.x + (t * s1.x);
		intersection->y = a1.y + (t * s1.y);
		return true;
	}

	return false; // No collision
}

} // End of namespace BladeRunner

#endif
