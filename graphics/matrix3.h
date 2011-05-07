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
 */

#ifndef GRAPHICS_MATRIX3_H
#define GRAPHICS_MATRIX3_H

#include "graphics/vector3d.h"

namespace Graphics {

// matrix 3 is a rotation matrix
class Matrix3 {
public:
	Vector3d _right;
	Vector3d _up;
	Vector3d _at;

	void buildFromPitchYawRoll(float pitch, float yaw, float roll);
	void setAsIdentity();

	void constructAroundPitch(float pitch);
	void constructAroundYaw(float yaw);
	void constructAroundRoll(float roll);

	void getPitchYawRoll(float* pPitch, float* pYaw, float* pRoll) const;

	float getPitch() const;
	float getYaw() const;
	float getRoll() const;

	void transform(Vector3d* v) const;

	// operators
	Matrix3& operator *=(const Matrix3& s) {
		float rx = s._right.dotProduct(_right.x(), _up.x(), _at.x());
		float ry = s._right.dotProduct(_right.y(), _up.y(), _at.y());
		float rz = s._right.dotProduct(_right.z(), _up.z(), _at.z());

		float ux = s._up.dotProduct(_right.x(), _up.x(), _at.x());
		float uy = s._up.dotProduct(_right.y(), _up.y(), _at.y());
		float uz = s._up.dotProduct(_right.z(), _up.z(), _at.z());

		float ax = s._at.dotProduct(_right.x(), _up.x(), _at.x());
		float ay = s._at.dotProduct(_right.y(), _up.y(), _at.y());
		float az = s._at.dotProduct(_right.z(), _up.z(), _at.z());

		_right.set(rx,ry,rz);
		_up.set(ux,uy,uz);
		_at.set(ax,ay,az);

		return *this;
	}
	Matrix3& operator =(const Matrix3& s) {
		_right = s._right;
		_up = s._up;
		_at = s._at;

		return *this;
	}

private:
};

} // end of namespace Graphics

#endif

