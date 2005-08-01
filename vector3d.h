// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef VECTOR3D_HH
#define VECTOR3D_HH

#include <cmath>

class Vector3d {
public:
	float _coords[3];		// Make sure this stays as an array so

	float& x() { return _coords[0]; }
	float x() const { return _coords[0]; }
	float& y() { return _coords[1]; }
	float y() const { return _coords[1]; }
	float& z() { return _coords[2]; }
	float z() const { return _coords[2]; }

	Vector3d() { this->x() = 0; this->y() = 0; this->z() = 0; }

	Vector3d(float x, float y, float z) {
		this->x() = x; this->y() = y; this->z() = z;
	}

	Vector3d(const Vector3d &v) {
		x() = v.x(); y() = v.y(); z() = v.z();
	}

	void set(float x, float y, float z) {
		this->x() = x; this->y() = y; this->z() = z;
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
		return std::sqrt(x() * x() + y() * y() + z() * z());
	}

	// Get the angle a vector is around the unit circle
	// (ignores z-component)
	float unitCircleAngle() const {
		float a = x() / magnitude();
		float b = y() / magnitude();
		float yaw;
		
		// find the angle on the upper half of the unit circle
		yaw = std::acos(a) * (180.0f / M_PI);
		if (b < 0.0f)
			// adjust for the lower half of the unit circle
			return 360.0f - yaw;
		else
			// no adjustment, angle is on the upper half
			return yaw;
	}

	float dotProduct( float sx, float sy, float sz ) {
		return x() * sx + y() * sy + z()*sz;
	}

	bool isZero() {
		if(x() == 0.f && y() == 0.f && z() == 0.f)
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
	return std::acos(dot(v1, v2) / (v1.magnitude() * v2.magnitude()));
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

#endif
