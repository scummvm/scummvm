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

#ifndef HPL_MATHTYPES_H
#define HPL_MATHTYPES_H

#include "common/array.h"
#include "common/list.h"
#include "hpl1/engine/math/Vector2.h"
#include "hpl1/engine/math/Vector3.h"
#include "hpl1/engine/math/hplMatrix.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

#define kPif (3.141592654f)
#define kPi2f (1.570796327f)
#define kPi4f (0.7853981634f)
#define k2Pif (6.283185307f)

#define kEpsilonf (0.0001f)

#define kSqrt2f (1.414213562f)

enum eEulerRotationOrder {
	eEulerRotationOrder_XYZ,
	eEulerRotationOrder_XZY,
	eEulerRotationOrder_YXZ,
	eEulerRotationOrder_YZX,
	eEulerRotationOrder_ZXY,
	eEulerRotationOrder_ZYX,
	eEulerRotationOrder_LastEnum,
};

//-------------------------------------------

class cLinearOscillation {
public:
	float max, min, val, up_speed, down_speed;

	cLinearOscillation() {}
	cLinearOscillation(float afMin, float afMax, float afVal, float afUpSpeed, float afDownSpeed) {
		SetUp(afMin, afMax, afVal, afUpSpeed, afDownSpeed);
	}

	void SetUp(float afMin, float afMax, float afVal, float afUpSpeed, float afDownSpeed) {
		min = afMin;
		max = afMax;
		val = afVal;

		up_speed = ABS(afUpSpeed);
		down_speed = -ABS(afDownSpeed);

		add = up_speed;
	}

	void Update(float afTimeStep) {
		val += add * afTimeStep;
		if (add > 0) {
			if (val >= max) {
				val = max;
				add = down_speed;
			}
		} else {
			if (val <= min) {
				val = min;
				add = up_speed;
			}
		}
	}

private:
	float add;
};

//-------------------------------------------

template<class T>
class cPlane {
public:
	T a, b, c, d;
	cVector3<T> normal;

	/////////////////

	cPlane(T aA, T aB, T aC, T aD) {
		a = aA;
		b = aB;
		c = aC;
		d = aD;
	}

	cPlane() {
		a = 0;
		b = 0;
		c = 0;
		d = 0;
	}

	/////////////////

	cPlane(const cVector3<T> &avNormal, const cVector3<T> &avPoint) {
		FromNormalPoint(avNormal, avPoint);
	}

	/////////////////

	cPlane(const cVector3<T> &avPoint0, const cVector3<T> &avPoint1,
		   const cVector3<T> &avPoint2) {
		FromPoints(avPoint0, avPoint1, avPoint2);
	}

	/////////////////

	inline void FromNormalPoint(const cVector3<T> &avNormal, const cVector3<T> &avPoint) {
		a = avNormal.x;
		b = avNormal.y;
		c = avNormal.z;

		// Dot product
		d = -(avNormal.x * avPoint.x + avNormal.y * avPoint.y + avNormal.z * avPoint.z);
	}

	/////////////////

	inline void FromPoints(const cVector3<T> &avPoint0, const cVector3<T> &avPoint1,
						   const cVector3<T> &avPoint2) {
		cVector3<T> vEdge1 = avPoint1 - avPoint0;
		cVector3<T> vEdge2 = avPoint2 - avPoint0;

		// Cross product
		normal.x = vEdge1.y * vEdge2.z - vEdge1.z * vEdge2.y;
		normal.y = vEdge1.z * vEdge2.x - vEdge1.x * vEdge2.z;
		normal.z = vEdge1.x * vEdge2.y - vEdge1.y * vEdge2.x;

		normal.Normalise();

		a = normal.x;
		b = normal.y;
		c = normal.z;

		// Dot product
		d = -(normal.x * avPoint0.x + normal.y * avPoint0.y + normal.z * avPoint0.z);
	}

	/////////////////

	inline void Normalise() {
		T fMag = sqrt(a * a + b * b + c * c);
		a = a / fMag;
		b = b / fMag;
		c = c / fMag;
		d = d / fMag;
	}

	/////////////////

	inline void CalcNormal() {
		normal = cVector3<T>(a, b, c);
		normal.Normalise();
	}

	/////////////////

	void FromVec(T *apV) {
		a = apV[0];
		b = apV[1];
		c = apV[2];
		d = apV[3];

		CalcNormal();
	}
};

typedef cPlane<float> cPlanef;
typedef Common::Array<cPlanef> tPlanefVec;
typedef tPlanefVec tPlanefVecIt;

//-------------------------------------------

template<class T>
class cSphere {
public:
	T r;
	cVector3<T> center;

	cSphere() {
	}

	cSphere(cVector3<T> aV, T aR) {
		center = aV;
		r = aR;
	}
};

typedef cSphere<float> cSpheref;

//-------------------------------------------

template<class T>
class cRect2 {
public:
	T x, y, w, h;
	cRect2(T aX, T aY, T aW, T aH) {
		x = aX;
		y = aY;
		w = aW;
		h = aH;
	}
	cRect2(T aX, T aY) {
		x = aX;
		y = aY;
		w = 0;
		h = 0;
	}
	cRect2() {
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}
	cRect2(cVector2<T> aPos, cVector2<T> aSize) {
		x = aPos.x;
		y = aPos.y;
		w = aSize.x;
		h = aSize.y;
	}

	void FromVec(T *apV) {
		x = apV[0];
		y = apV[1];
		w = apV[2];
		h = apV[3];
	}
};

typedef cRect2<float> cRect2f;
typedef cRect2<int> cRect2l;
typedef Common::List<cRect2f> tRect2lList;
typedef tRect2lList tRect2lListIt;

//-------------------------------------------

typedef cVector2<float> cVector2f;
typedef cVector2<int> cVector2l;

typedef cMatrix<float> cMatrixf;

typedef Common::Array<cMatrixf> tMatrixfVec;
typedef tMatrixfVec::iterator tMatrixfVecIt;

typedef Common::List<cMatrixf> tMatrixfList;
typedef tMatrixfList::iterator tMatrixfListIt;

typedef Common::List<cVector2f> tVector2fList;
typedef tVector2fList::iterator tVector2fListIt;

typedef cVector3<float> cVector3f;
typedef cVector3<int> cVector3l;

typedef Common::List<cVector3f> tVector3fList;
typedef tVector3fList::iterator tVector3fListIt;

typedef Common::Array<cVector2f> tVector2fVec;
typedef tVector2fVec::iterator tVector2fVecIt;

typedef Common::Array<cVector3f> tVector3fVec;
typedef tVector3fList::iterator tVector3fVecIt;

//-------------------------------------------
} // namespace hpl

#include "hpl1/engine/math/Quaternion.h"

#endif // HPL_MATHTYPES_H
