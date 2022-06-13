/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HPL_QUATERNION_H
#define HPL_QUATERNION_H

#include "hpl1/engine/math/Vector3.h"
#include "hpl1/engine/math/Matrix.h"

namespace hpl {

	class cQuaternion
	{
	public:
		cVector3<float> v;
		float w;

		cQuaternion();
		cQuaternion(float afAngle, const cVector3<float> &avAxis);
		cQuaternion(float afW, float afX, float afY, float afZ);

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
};
#endif // HPL_QUATERNION_H
