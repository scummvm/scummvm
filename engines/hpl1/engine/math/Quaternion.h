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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_QUATERNION_H
#define HPL_QUATERNION_H

#include "hpl1/engine/math/Vector3.h"
#include "hpl1/engine/math/hplMatrix.h"

namespace hpl {

class cQuaternion {
public:
	cVector3<float> v;
	float w;

	cQuaternion();
	cQuaternion(float afAngle, const cVector3<float> &avAxis);
	constexpr cQuaternion(float afW, float afX, float afY, float afZ) : v(afX, afY, afZ), w(afW) {
	}

	void Normalise();
	void ToRotationMatrix(cMatrix<float> &a_mtxDest) const;
	void FromRotationMatrix(const cMatrix<float> &a_mtxRot);

	/**
	 * Set the quaternion from and angle and axis.
	 * \param afAngle
	 * \param &avAxis MUST be unit length!
	 */
	void FromAngleAxis(float afAngle, const cVector3<float> &avAxis);

	cQuaternion operator+(const cQuaternion &aqB) const;
	cQuaternion operator-(const cQuaternion &aqB) const;
	cQuaternion operator*(const cQuaternion &aqB) const;
	cQuaternion operator*(float afScalar) const;

	static const cQuaternion Identity;

private:
};

} // namespace hpl

#endif // HPL_QUATERNION_H
