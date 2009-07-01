/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GRAPHICS_VECTOR3D_H
#define GRAPHICS_VECTOR3D_H

#include "common/sys.h"
#include "common/endian.h"

namespace Graphics {

class Vector3d {
public:
	float _coords[3];		// Make sure this stays as an array so

	float& x() { return _coords[0]; }
	float x() const { return _coords[0]; }
	float& y() { return _coords[1]; }
	float y() const { return _coords[1]; }
	float& z() { return _coords[2]; }
	float z() const { return _coords[2]; }

	Vector3d() { x() = 0; y() = 0; z() = 0; }

	Vector3d(float lx, float ly, float lz) {
		x() = lx; y() = ly; z() = lz;
	}

	Vector3d(const Vector3d &v) {
		x() = v.x(); y() = v.y(); z() = v.z();
	}

	void set(float lx, float ly, float lz) {
		x() = lx; y() = ly; z() = lz;
	}

	Vector3d& operator =(const Vector3d &v) {
		x() = v.x(); y() = v.y(); z() = v.z();
		return *this;
	}

	bool operator ==(const Vector3d &v) {
		return ( (x() == v.x()) && (y() == v.y()) && (z() == v.z()) );
	}

	bool operator !=(const Vector3d &v) {
		return ( (x() != v.x()) || (y() != v.y()) || (z() != v.z()) );
	}

	Vector3d& operator +=(const Vector3d &v) {
		x() += v.x(); y() += v.y(); z() += v.z();
		return *this;
	}

	Vector3d& operator -=(const Vector3d &v) {
		x() -= v.x(); y() -= v.y(); z() -= v.z();
		return *this;
	}

	Vector3d& operator *=(float s) {
		x() *= s; y() *= s; z() *= s;
		return *this;
	}

	Vector3d& operator /=(float s) {
		x() /= s; y() /= s; z() /= s;
		return *this;
	}

	float magnitude() const {
		return sqrt(x() * x() + y() * y() + z() * z());
	}

	// Get the angle a vector is around the unit circle
	// (ignores z-component)
	float unitCircleAngle() const {
		float a = x() / magnitude();
		float b = y() / magnitude();
		float yaw;

		// find the angle on the upper half of the unit circle
		yaw = acos(a) * (180.0f / LOCAL_PI);
		if (b < 0.0f)
			// adjust for the lower half of the unit circle
			return 360.0f - yaw;
		else
			// no adjustment, angle is on the upper half
			return yaw;
	}

	void normalize() {
		float len = sqrt(dotProduct(x(), y(), z()));
		if (len != 0.0f) {
			float t = 1.0f / len;
			set(x() * t, y() * t, y() * t);
		}
	}

	float dotProduct(float sx, float sy, float sz) {
		return x() * sx + y() * sy + z() * sz;
	}

	bool isZero() {
		if (x() == 0.f && y() == 0.f && z() == 0.f)
			return true;
		return false;
	}
};

inline float dot(const Vector3d& v1, const Vector3d& v2) {
	return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
}

inline Vector3d cross(const Vector3d& v1, const Vector3d& v2) {
	return Vector3d(v1.y() * v2.z() - v1.z() * v2.y(),
		v1.z() * v2.x() - v1.x() * v2.z(),
		v1.x() * v2.y() - v1.y() * v2.x());
}

inline float angle(const Vector3d& v1, const Vector3d& v2) {
	return acos(dot(v1, v2) / (v1.magnitude() * v2.magnitude()));
}

inline Vector3d operator +(const Vector3d& v1, const Vector3d& v2) {
	Vector3d result = v1;
	result += v2;
	return result;
}

inline Vector3d operator -(const Vector3d& v1, const Vector3d& v2) {
	Vector3d result = v1;
	result -= v2;
	return result;
}

inline Vector3d operator *(float s, const Vector3d& v) {
	Vector3d result = v;
	result *= s;
	return result;
}

inline Vector3d operator -(const Vector3d& v) {
	return (-1.0f) * v;
}

inline Vector3d operator *(const Vector3d& v, float s) {
	return s * v;
}

inline Vector3d operator /(const Vector3d& v, float s) {
	Vector3d result = v;
	result /= s;
	return result;
}

inline bool operator ==(const Vector3d& v1, const Vector3d& v2) {
	return v1.x() == v2.x() && v1.y() == v2.y() && v1.z() == v2.z();
}

inline Vector3d get_vector3d(const char *data) {
	return Vector3d(get_float(data), get_float(data + 4), get_float(data + 8));
}

} // end of namespace Graphics

#endif
