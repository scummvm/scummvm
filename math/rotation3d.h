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
	EO_ZXY,		// Original ScummVM implmentation
	EO_ZXZ,
	EO_ZYX,
	EO_ZYZ
};

template<class T>
class Rotation3D : public Transform<T> {
public:
	Rotation3D();

	/**
	 * Constructor and assignment from buildFromEuler
	 * @param first		Rotation on the first Axis, angle in degrees
	 * @param second	Rotation on the second Axis, angle in degrees
	 * @param third		Rotation on the third Axis, angle in degrees
	 * @param order		The Euler Order (specifies axis order)
	 */
	Rotation3D(const Angle &first, const Angle &second, const Angle &third, EulerOrder order);

	/**
	 * Build a rotation matrix from Euler Angles
	 * @param first		Rotation on the first Axis, angle in degrees
	 * @param second	Rotation on the second Axis, angle in degrees
	 * @param third		Rotation on the third Axis, angle in degrees
	 * @param order		The Euler Order (specifies axis order)
	 */
	void buildFromEuler(const Angle &first, const Angle &second, const Angle &third, EulerOrder order);

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
	 * @param first		Pointer to the storage for the first axis angle
	 * @param second	Pointer to the storage for the second axis angle
	 * @param third		Pointer to the storage for the third axis angle
	 * @param order		The Euler order (specifies axis order)
	 */
	void getEuler(Angle *first, Angle *second, Angle *third, EulerOrder order) const;
};

template<class T>
Rotation3D<T>::Rotation3D() : Transform<T>() {}

template<class T>
void Rotation3D<T>::buildFromEuler(const Angle &first, const Angle &second, const Angle &third, EulerOrder order) {
	// Build a matrix around each rotation angle
	T m2, m3;

	// Combine them in the order requested
	switch (order) {
		case EO_XYX:
			this->buildAroundX(first);
			m2.buildAroundY(second);
			m3.buildAroundX(third);
			break;
		case EO_XYZ:
			this->buildAroundX(first);
			m2.buildAroundY(second);
			m3.buildAroundZ(third);
			break;
		case EO_XZX:
			this->buildAroundX(first);
			m2.buildAroundZ(second);
			m3.buildAroundX(third);
			break;
		case EO_XZY:
			this->buildAroundX(first);
			m2.buildAroundZ(second);
			m3.buildAroundY(third);
			break;
		case EO_YXY:
			this->buildAroundY(first);
			m2.buildAroundX(second);
			m3.buildAroundY(third);
			break;
		case EO_YXZ:
			this->buildAroundY(first);
			m2.buildAroundX(second);
			m3.buildAroundZ(third);
			break;
		case EO_YZX:
			this->buildAroundY(first);
			m2.buildAroundZ(second);
			m3.buildAroundX(third);
			break;
		case EO_YZY:
			this->buildAroundY(first);
			m2.buildAroundZ(second);
			m3.buildAroundY(third);
			break;
		// Original ScummVM Implementation
		case EO_ZXY:
			this->buildAroundZ(first);
			m2.buildAroundX(second);
			m3.buildAroundY(third);
			break;
		case EO_ZXZ:
			this->buildAroundZ(first);
			m2.buildAroundX(second);
			m3.buildAroundZ(third);
			break;
		case EO_ZYX:
			this->buildAroundZ(first);
			m2.buildAroundY(second);
			m3.buildAroundX(third);
			break;
		case EO_ZYZ:
			this->buildAroundZ(first);
			m2.buildAroundY(second);
			m3.buildAroundZ(third);
			break;
		default:
			error("Invalid Euler Order");
			break;
	}
	// Combine the rotations
	this->getMatrix() = this->getMatrix() * m2 * m3;
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

template<class T>
void Rotation3D<T>::getEuler(Angle *first, Angle *second, Angle *third, EulerOrder order) const {
	// Cast as the matrix type so we can use getValue
	const T *m = &(this->getMatrix());

	float f, s, t;
	switch (order) {
		case EO_XYX:
			if (m->getValue(0, 0) < 1.0f) {
				if (m->getValue(0, 0) > -1.0f) {
					f = atan2(m->getValue(1, 0), -(m->getValue(2, 0)));
					s = acos(m->getValue(0, 0));
					t = atan2(m->getValue(0, 1), m->getValue(0, 2));
				} else {
					f = -atan2(-m->getValue(1, 2), m->getValue(1, 1));
					s = (float) M_PI;
					t = 0.0f;
				}
			} else {
				f = atan2(-m->getValue(1, 2), m->getValue(1, 1));
				s = 0.0f;
				t = 0.0f;
			}
			break;
		case EO_XYZ:
			if (m->getValue(0, 2) < 1.0f) {
				if (m->getValue(0, 2) > -1.0f) {
					f = atan2(-(m->getValue(1, 2)), m->getValue(2, 2));
					s = asin(m->getValue(0, 2));
					t = atan2(-(m->getValue(0, 1)), m->getValue(0, 0));
				} else {
					f = -atan2(m->getValue(1, 0), m->getValue(1, 1));
					s = -(float) M_PI / 2.0f;
					t = 0.0f;
				}
			} else {
				f = atan2(m->getValue(1, 0), m->getValue(1, 1));
				s = (float) M_PI / 2.0f;
				t = 0.0f;
			}
			break;
		case EO_XZX:
			if (m->getValue(0, 0) < 1.0f) {
				if (m->getValue(0, 0) > -1.0f) {
					f = atan2(m->getValue(2, 0), m->getValue(1, 0));
					s = acos(m->getValue(0, 0));
					t = atan2(m->getValue(0, 2), -(m->getValue(0, 1)));
				} else {
					f = -atan2(m->getValue(2, 1), m->getValue(2, 2));
					s = (float) M_PI;
					t = 0.0f;
				}
			} else {
				f = atan2(m->getValue(2, 1), m->getValue(2, 2));
				s = 0.0f;
				t = 0.0f;
			}
			break;
		case EO_XZY:
			if (m->getValue(0, 1) < 1.0f) {
				if (m->getValue(0, 1) > -1.0f) {
					f = atan2(m->getValue(2, 1), m->getValue(1, 1));
					s = asin(-(m->getValue(0, 1)));
					t = atan2(m->getValue(0, 2), m->getValue(0, 0));
				} else {
					f = -atan2(-(m->getValue(2, 0)), m->getValue(2, 2));
					s = (float) M_PI / 2.0f;
					t = 0.0f;
				}
			} else {
				f = atan2(-(m->getValue(2, 0)), m->getValue(2, 2));
				s = -(float) M_PI / 2.0f;
				t = 0.0f;
			}
			break;
		case EO_YXY:
			if (m->getValue(1, 1) < 1.0f) {
				if (m->getValue(1, 1) > -1.0f) {
					f = atan2(m->getValue(0, 1), m->getValue(2, 1));
					s = acos(m->getValue(1, 1));
					t = atan2(m->getValue(1, 0), -(m->getValue(1, 2)));
				} else {
					f = -atan2(m->getValue(0, 2), m->getValue(0, 0));
					s = (float) M_PI;
					t = 0.0f;
				}
			} else {
				f = atan2(m->getValue(0, 2), m->getValue(0, 0));
				s = 0.0f;
				t = 0.0f;
			}
			break;
		case EO_YXZ:
			if (m->getValue(1, 2) < 1.0f) {
				if (m->getValue(1, 2) > -1.0f) {
					f = atan2(m->getValue(0, 2), m->getValue(2, 2));
					s = asin(-(m->getValue(1, 2)));
					t = atan2(m->getValue(1, 0), m->getValue(1, 1));
				} else {
					f = -atan2(-(m->getValue(0, 1)), m->getValue(0, 0));
					s = (float) M_PI / 2.0f;
					t = 0.0f;
				}
			} else {
				f = atan2(-(m->getValue(0, 1)), m->getValue(0, 0));
				s = -(float) M_PI / 2.0f;
				t = 0.0f;
			}
			break;
		case EO_YZX:
			if (m->getValue(1, 0) < 1.0f) {
				if (m->getValue(1, 0) > -1.0f) {
					f = atan2(-(m->getValue(2, 0)), m->getValue(0, 0));
					s = asin(m->getValue(1, 0));
					t = atan2(-(m->getValue(1, 2)), m->getValue(1, 1));
				} else {
					f = -atan2(m->getValue(2, 1), m->getValue(2, 2));
					s = -(float) M_PI / 2.0f;
					t = 0.0f;
				}
			} else {
				f = atan2(m->getValue(2, 1), m->getValue(2, 2));
				s = (float) M_PI / 2.0f;
				t = 0.0f;
			}
			break;
		case EO_YZY:
			if (m->getValue(1, 1) < 1.0f) {
				if (m->getValue(1, 1) > -1.0f) {
					f = atan2(m->getValue(2, 1), -(m->getValue(0, 1)));
					s = acos(m->getValue(1, 1));
					t = atan2(m->getValue(1, 2), m->getValue(1, 0));
				} else {
					f = -atan2(-(m->getValue(2, 0)), m->getValue(2, 2));
					s = (float) M_PI;
					t = 0.0f;
				}
			} else {
				f = atan2(-(m->getValue(2, 0)), m->getValue(2, 2));
				s = 0.0f;
				t = 0.0f;
			}
			break;
		case EO_ZXY:		// Original ScummVM implmentation
			if (m->getValue(2, 1) < 1.0f) {
				if (m->getValue(2, 1) > -1.0f) {
					f = -atan2(m->getValue(0, 1), m->getValue(1, 1));
					s = asin(m->getValue(2, 1));
					t = -atan2(m->getValue(2, 0), m->getValue(2, 2));
				} else {
					f = -atan2(-m->getValue(0, 2), m->getValue(0, 0));
					s = -(float) M_PI / 2.0f;
					t = 0.0f;
				}
			} else {
				f = atan2(m->getValue(0, 2), m->getValue(0, 0));
				s = (float) M_PI / 2.0f;
				t = 0.0f;
			}
			break;
		case EO_ZXZ:
			if (m->getValue(2, 2) < 1.0f) {
				if (m->getValue(2, 2) > -1.0f) {
					f = atan2(m->getValue(0, 2), -(m->getValue(1, 2)));
					s = acos(m->getValue(2, 2));
					t = atan2(m->getValue(2, 0), m->getValue(2, 1));
				} else {
					f = -atan2(-(m->getValue(0, 1)), m->getValue(0, 0));
					s = (float) M_PI;
					t = 0.0f;
				}
			} else {
				f = atan2(-(m->getValue(0, 1)), m->getValue(0, 0));
				s = 0.0f;
				t = 0.0f;
			}
			break;
		case EO_ZYX:
			if (m->getValue(2, 0) < 1.0f) {
				if (m->getValue(2, 0) > -1.0f) {
					f = atan2(m->getValue(1, 0), m->getValue(0, 0));
					s = asin(-(m->getValue(2, 0)));
					t = atan2(m->getValue(2, 1), m->getValue(2, 2));
				} else {
					f = -atan2(-m->getValue(1, 2), m->getValue(1, 1));
					s = (float) M_PI / 2.0f;
					t = 0.0f;
				}
			} else {
				f = atan2(-m->getValue(1, 2), m->getValue(1, 1));
				s = -(float) M_PI / 2.0f;
				t = 0.0f;
			}
			break;
		case EO_ZYZ:
			if (m->getValue(2, 2) < 1.0f) {
				if (m->getValue(2, 2) > -1.0f) {
					f = atan2(m->getValue(1, 2), m->getValue(0, 2));
					s = acos(m->getValue(2, 2));
					t = atan2(m->getValue(2, 1), -(m->getValue(2, 0)));
				} else {
					f = -atan2(m->getValue(1, 0), m->getValue(1, 1));
					s = (float) M_PI;
					t = 0.0f;
				}
			} else {
				f = atan2(m->getValue(1, 0), m->getValue(1, 1));
				s = 0.0f;
				t = 0.0f;
			}
			break;
		default:
			error("Invalid Euler Order");
			break;
	}

	if (first) {
		*first = Math::Angle::fromRadians(f);
	}
	if (second) {
		*second = Math::Angle::fromRadians(s);
	}
	if (third) {
		*third = Math::Angle::fromRadians(t);
	}
}

}

#endif
