// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
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
  float coords_[3];		// Make sure this stays as an array so
				// it can be passed to GL functions

  float& x() { return coords_[0]; }
  float x() const { return coords_[0]; }
  float& y() { return coords_[1]; }
  float y() const { return coords_[1]; }
  float& z() { return coords_[2]; }
  float z() const { return coords_[2]; }

  Vector3d() { }
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
