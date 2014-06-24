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

#ifndef MATH_ROTATION3D_H
#define MATH_ROTATION3D_H

#include "common/streamdebug.h"

#include "math/utils.h"
#include "math/transform.h"
#include "math/angle.h"
#include "common/textconsole.h"

namespace Math {

/**
 * Euler angle sequence constants
 */
enum EulerOrder {
	EO_XYX,
	EO_XYZ,
	EO_XZX,
	EO_XZY,
	EO_YXY,
	EO_YXZ,
	EO_YZX,
	EO_YZY,
	EO_ZXY,		// Original ResidualVM implmentation
	EO_ZXZ,
	EO_ZYX,
	EO_ZYZ
};

template<class T>
class Rotation3D : public Transform<T> {
public:
	Rotation3D();

	/**
	 * Constructor and assignment from buildFromXYZ
	 * @param rotX		Rotation on the X Axis angle in degrees
	 * @param rotY		Rotation on the Y Axis angle in degrees
	 * @param rotZ		Rotation on the Z Axis angle in degrees
	 * @param order		The Euler Order
	 */
	Rotation3D(const Angle &rotX, const Angle &rotY, const Angle &rotZ, EulerOrder order);

	/**
	 * Build a rotation matrix from Euler Angles
	 * @param rotX		Rotation on the X Axis angle in degrees
	 * @param rotY		Rotation on the Y Axis angle in degrees
	 * @param rotZ		Rotation on the Z Axis angle in degrees
	 * @param order		The Euler Order
	 */
	void buildFromXYZ(const Angle &rotX, const Angle &rotY, const Angle &rotZ, EulerOrder order);

	/**
	 * Build a rotation matrix on the X Axis from an angle
	 * @param rotX		Rotation on the X Axis angle in degrees
	 */
	void buildAroundX(const Angle &rotX);

	/**
	 * Build a rotation matrix on the Y Axis from an angle
	 * @param rotY		Rotation on the Y Axis angle in degrees
	 */
	void buildAroundY(const Angle &rotY);

	/**
	 * Build a rotation matrix on the Z Axis from an angle
	 * @param rotZ		Rotation on the Z Axis angle in degrees
	 */
	void buildAroundZ(const Angle &rotZ);

	/**
	 * Get Euler Angles from a rotation matrix
	 * @param pRotX		Pointer to the storage for the X axis angle
	 * @param pRotY		Pointer to the storage for the Y axis angle
	 * @param pRotZ		Pointer to the storage for the Z axis angle
	 * @param order		The Euler order
	 */
	void getXYZ(Angle *pRotX, Angle *pRotY, Angle *pRotZ, EulerOrder order) const;
};

template<class T>
Rotation3D<T>::Rotation3D() : Transform<T>() {}

template<class T>
void Rotation3D<T>::buildFromXYZ(const Angle &rotX, const Angle &rotY, const Angle &rotZ, EulerOrder order) {
	// Build a matrix around each rotation angle
	T m1, m2, m3;

	// Combine them in the order requested
	switch (order) {
		case EO_XYX:
			m1.buildAroundX(rotX);
			m2.buildAroundY(rotY);
			m3.buildAroundX(rotZ);
			break;
		case EO_XYZ:
			m1.buildAroundX(rotX);
			m2.buildAroundY(rotY);
			m3.buildAroundZ(rotZ);
			break;
		case EO_XZX:
			m1.buildAroundX(rotX);
			m2.buildAroundZ(rotY);
			m3.buildAroundX(rotZ);
			break;
		case EO_XZY:
			m1.buildAroundX(rotX);
			m2.buildAroundZ(rotY);
			m3.buildAroundY(rotZ);
			break;
		case EO_YXY:
			m1.buildAroundY(rotX);
			m2.buildAroundX(rotY);
			m3.buildAroundY(rotZ);
			break;
		case EO_YXZ:
			m1.buildAroundY(rotX);
			m2.buildAroundX(rotY);
			m3.buildAroundZ(rotZ);
			break;
		case EO_YZX:
			m1.buildAroundY(rotX);
			m2.buildAroundZ(rotY);
			m3.buildAroundX(rotZ);
			break;
		case EO_YZY:
			m1.buildAroundY(rotX);
			m2.buildAroundZ(rotY);
			m3.buildAroundY(rotZ);
			break;
		// Original ResidualVM Implementation
		case EO_ZXY:
			m1.buildAroundZ(rotX);
			m2.buildAroundX(rotY);
			m3.buildAroundY(rotZ);
			break;
		case EO_ZXZ:
			m1.buildAroundZ(rotX);
			m2.buildAroundX(rotY);
			m3.buildAroundZ(rotZ);
			break;
		case EO_ZYX:
			m1.buildAroundZ(rotX);
			m2.buildAroundY(rotY);
			m3.buildAroundX(rotZ);
			break;
		case EO_ZYZ:
			m1.buildAroundZ(rotX);
			m2.buildAroundY(rotY);
			m3.buildAroundZ(rotZ);
			break;
		default:
			error("Invalid Euler Order");
			break;
	}
	// Combine the rotations
	this->getMatrix() = m1 * m2 * m3;
}

// at. Rotates about the +X axis.
// Left Handed (DirectX) Coordinates
template<class T>
void Rotation3D<T>::buildAroundX(const Angle &rotX) {
	float cosa = rotX.getCosine();
	float sina = rotX.getSine();

	this->getMatrix().getRow(0) << 1.f << 0.f  << 0.f;
	this->getMatrix().getRow(1) << 0.f << cosa << -sina;
	this->getMatrix().getRow(2) << 0.f << sina << cosa;
}

// right. Rotates about the +Y axis.
// Left Handed (DirectX) Coordinates
template<class T>
void Rotation3D<T>::buildAroundY(const Angle &rotY) {
	float cosa = rotY.getCosine();
	float sina = rotY.getSine();

	this->getMatrix().getRow(0) << cosa  << 0.f << sina;
	this->getMatrix().getRow(1) << 0.f   << 1.f << 0.f;
	this->getMatrix().getRow(2) << -sina << 0.f << cosa;
}

// up. Rotates about the +Z axis.
// Left Handed (DirectX) Coordinates
template<class T>
void Rotation3D<T>::buildAroundZ(const Angle &rotZ) {
	float cosa = rotZ.getCosine();
	float sina = rotZ.getSine();

	this->getMatrix().getRow(0) << cosa << -sina << 0.f;
	this->getMatrix().getRow(1) << sina << cosa  << 0.f;
	this->getMatrix().getRow(2) << 0.f  << 0.f   << 1.f;
}

// FIXME: Gimbal Lock
template<class T>
void Rotation3D<T>::getXYZ(Angle *rotX, Angle *rotY, Angle *rotZ, EulerOrder order) const {
	// Cast as the matrix type so we can use getValue
	const T *m = &(this->getMatrix());

	float x, y, z;
	switch (order) {
		case EO_XYX:
			x = atan2(m->getValue(1, 0), -(m->getValue(2, 0)));
			y = acos(m->getValue(0, 0));
			z = atan2(m->getValue(0, 1), m->getValue(0, 2));
			break;
		case EO_XYZ:
			x = -atan2(m->getValue(1, 2), m->getValue(2, 2));
			y = asin(m->getValue(0, 2));
			z = -atan2(m->getValue(0, 1), m->getValue(0, 0));
			break;
		case EO_XZX:
			x = atan2(m->getValue(2, 0), m->getValue(1, 0));
			y = acos(m->getValue(0, 0));
			z = atan2(m->getValue(0, 2), -(m->getValue(0, 1)));
			break;
		case EO_XZY:
			x = atan2(m->getValue(2, 1), m->getValue(1, 1));
			y = -asin(m->getValue(0, 1));
			z = atan2(m->getValue(0, 2), m->getValue(0, 0));
			break;
		case EO_YXY:
			x = atan2(m->getValue(0, 1), m->getValue(2, 1));
			y = acos(m->getValue(1, 1));
			z = atan2(m->getValue(1, 0), -(m->getValue(1, 2)));
			break;
		case EO_YXZ:
			x = atan2(m->getValue(0, 2), m->getValue(2, 2));
			y = -asin(m->getValue(1, 2));
			z = atan2(m->getValue(1, 0), m->getValue(1, 1));
			break;
		case EO_YZX:
			x = -atan2(m->getValue(2, 0), m->getValue(0, 0));
			y = asin(m->getValue(1, 0));
			z = -atan2(m->getValue(1, 2), m->getValue(1, 1));
			break;
		case EO_YZY:
			x = atan2(m->getValue(2, 1), -(m->getValue(0, 1)));
			y = acos(m->getValue(1, 1));
			z = atan2(m->getValue(1, 2), m->getValue(1, 0));
			break;
		case EO_ZXY:		// Original ResidualVM implmentation
			if (m->getValue(2, 1) < 1.0f) {
				if (m->getValue(2, 1) > -1.0f) {
					x = -atan2(m->getValue(0, 1), m->getValue(1, 1));
					y = asin(m->getValue(2, 1));
					z = -atan2(m->getValue(2, 0), m->getValue(2, 2));
				// Not a unique solution, pick an arbitrary one
				} else {
					x = -atan2(-m->getValue(0, 2), m->getValue(0, 0));
					y = -M_PI/2.0f;
					z = 0.0f;
				}
			// Not a unique solution, pick an arbitrary one
			} else {
				x = atan2(m->getValue(0, 2), m->getValue(0, 0));
				y = M_PI/2.0f;
				z = 0.0f;
			}
			break;
		case EO_ZXZ:
			x = atan2(m->getValue(0, 2), -(m->getValue(1, 2)));
			y = acos(m->getValue(2, 2));
			z = atan2(m->getValue(2, 0), m->getValue(2, 1));
			break;
		case EO_ZYX:
			x = atan2(m->getValue(1, 0), m->getValue(0, 0));
			y = -asin(m->getValue(2, 0));
			z = atan2(m->getValue(2, 1), m->getValue(2, 2));
			break;
		case EO_ZYZ:
			x = atan2(m->getValue(1, 2), m->getValue(0, 2));
			y = acos(m->getValue(2, 2));
			z = atan2(m->getValue(2, 1), -(m->getValue(2, 0)));
			break;
		default:
			error("Invalid Euler Order");
			break;
	}

	if (rotX) {
		*rotX = Math::Angle::fromRadians(x);
	}
	if (rotY) {
		*rotY = Math::Angle::fromRadians(y);
	}
	if (rotZ) {
		*rotZ = Math::Angle::fromRadians(z);
	}
}

}

#endif
