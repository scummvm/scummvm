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

#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/system/low_level_system.h"
#include "hpl1/hpl1.h"

namespace hpl {

static char mpTempChar[1024];

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

int cMath::RandRectl(int alMin, int alMax) {
	return Hpl1::g_engine->getRandomNumber(alMax - alMin) + alMin;
}

//-----------------------------------------------------------------------

float cMath::RandRectf(float afMin, float afMax) {
	float fRand = static_cast<float>(Hpl1::g_engine->getRandomNumber(UINT_MAX)) / static_cast<float>(UINT_MAX);

	return afMin + fRand * (afMax - afMin);
}

//-----------------------------------------------------------------------

cVector2f cMath::RandRectVector2f(const cVector3f &avMin, const cVector3f &avMax) {
	return cVector2f(RandRectf(avMin.x, avMax.x),
					 RandRectf(avMin.y, avMax.y));
}

//-----------------------------------------------------------------------

cVector3f cMath::RandRectVector3f(const cVector3f &avMin, const cVector3f &avMax) {
	return cVector3f(RandRectf(avMin.x, avMax.x),
					 RandRectf(avMin.y, avMax.y),
					 RandRectf(avMin.z, avMax.z));
}

//-----------------------------------------------------------------------

cColor cMath::RandRectColor(const cColor &aMin, const cColor &aMax) {
	return cColor(RandRectf(aMin.r, aMax.r),
				  RandRectf(aMin.g, aMax.g),
				  RandRectf(aMin.b, aMax.b),
				  RandRectf(aMin.a, aMax.a));
}

//-----------------------------------------------------------------------

void cMath::Randomize(int alSeed) {
#if 0
	if (alSeed == -1) {
		srand((unsigned)time(NULL));
	} else {
		srand(alSeed);
	}
#endif
}
//-----------------------------------------------------------------------

float cMath::Dist2D(const cVector2f &avPosA, const cVector2f &avPosB) {
	float fDx = avPosA.x - avPosB.x;
	float fDy = avPosA.y - avPosB.y;

	return sqrt(fDx * fDx + fDy * fDy);
}

//-----------------------------------------------------------------------

float cMath::Dist2D(const cVector3f &avPosA, const cVector3f &avPosB) {
	float fDx = avPosA.x - avPosB.x;
	float fDy = avPosA.y - avPosB.y;

	return sqrt(fDx * fDx + fDy * fDy);
}

//-----------------------------------------------------------------------

float cMath::SqrDist2D(const cVector2f &avPosA, const cVector2f &avPosB) {
	float fDx = avPosA.x - avPosB.x;
	float fDy = avPosA.y - avPosB.y;

	return fDx * fDx + fDy * fDy;
}

//-----------------------------------------------------------------------

float cMath::SqrDist2D(const cVector3f &avPosA, const cVector3f &avPosB) {
	float fDx = avPosA.x - avPosB.x;
	float fDy = avPosA.y - avPosB.y;

	return fDx * fDx + fDy * fDy;
}

//-----------------------------------------------------------------------

bool cMath::BoxCollision(cRect2l aRect1, cRect2l aRect2) {
	return (aRect1.x > aRect2.x + (aRect2.w - 1) || aRect2.x > aRect1.x + (aRect1.w - 1) ||
			aRect1.y > aRect2.y + (aRect2.h - 1) || aRect2.y > aRect1.y + (aRect1.h - 1)) == false;
}

//-----------------------------------------------------------------------

bool cMath::BoxCollision(cRect2f aRect1, cRect2f aRect2) {
	return (aRect1.x > aRect2.x + (aRect2.w) || aRect2.x > aRect1.x + (aRect1.w) ||
			aRect1.y > aRect2.y + (aRect2.h) || aRect2.y > aRect1.y + (aRect1.h)) == false;
}

//-----------------------------------------------------------------------

bool cMath::PointBoxCollision(cVector2f avPoint, cRect2f aRect) {
	if (avPoint.x < aRect.x || avPoint.x > aRect.x + aRect.w || avPoint.y < aRect.y || avPoint.y > aRect.y + aRect.h)
		return false;
	else
		return true;
}

//-----------------------------------------------------------------------

bool cMath::BoxFit(cRect2l aRectSrc, cRect2l aRectDest) {
	// check is size is smaller and doesn't overlap
	if (aRectSrc.w > aRectDest.w || aRectSrc.h > aRectDest.h ||
		aRectSrc.x + aRectSrc.w > aRectDest.x + aRectDest.w ||
		aRectSrc.y + aRectSrc.h > aRectDest.y + aRectDest.h)
		return false;

	// check if x,y is in borders
	if (aRectSrc.x < aRectDest.x || aRectSrc.y < aRectDest.y ||
		aRectSrc.x > aRectDest.x + aRectDest.w || aRectSrc.y > aRectDest.y + aRectDest.h)
		return false;

	return true;
}

//-----------------------------------------------------------------------

bool cMath::BoxFit(cRect2f aRectSrc, cRect2f aRectDest) {
	// check is size is smaller and doesn't overlap
	if (aRectSrc.w > aRectDest.w || aRectSrc.h > aRectDest.h ||
		aRectSrc.x + aRectSrc.w > aRectDest.x + aRectDest.w ||
		aRectSrc.y + aRectSrc.h > aRectDest.y + aRectDest.h)
		return false;

	// check if x,y is in borders
	if (aRectSrc.x < aRectDest.x || aRectSrc.y < aRectDest.y ||
		aRectSrc.x > aRectDest.x + aRectDest.w || aRectSrc.y > aRectDest.y + aRectDest.h)
		return false;

	return true;
}

//-----------------------------------------------------------------------

cRect2f &cMath::ClipRect(cRect2f &aRectSrc, const cRect2f &aRectDest) {
	if (aRectSrc.x < aRectDest.x) {
		aRectSrc.w -= aRectDest.x - aRectSrc.x;
		aRectSrc.x = aRectDest.x;
	}
	if (aRectSrc.y < aRectDest.y) {
		aRectSrc.h -= aRectDest.y - aRectSrc.y;
		aRectSrc.y = aRectDest.y;
	}

	if (aRectSrc.x + aRectSrc.w > aRectDest.x + aRectDest.w) {
		aRectSrc.w -= (aRectSrc.x + aRectSrc.w) - (aRectDest.x + aRectDest.w);
	}
	if (aRectSrc.y + aRectSrc.h > aRectDest.y + aRectDest.h) {
		aRectSrc.h -= (aRectSrc.y + aRectSrc.h) - (aRectDest.y + aRectDest.h);
	}

	return aRectSrc;
}

//-----------------------------------------------------------------------

bool cMath::CheckCollisionBV(cBoundingVolume &aBV1, cBoundingVolume &aBV2) {
	//////////////////////////////////////////
	// Check Sphere collision.
	// Not needed I think.
	/*float fRadiusSum = aBV1->GetRadius() + aBV2->GetRadius();
	cVector3f vSepAxis = aBV1->GetWorldCenter() - aBV2->GetWorldCenter();
	if(vSepAxis.SqrLength() > (fRadiusSum * fRadiusSum))
	{
		return eBVCollision_Outside;
	}*/

	///////////////////////////////////////////
	// Check box collision
	aBV1.UpdateSize();
	aBV2.UpdateSize();

	const cVector3f &vMin1 = aBV1.mvWorldMin;
	const cVector3f &vMin2 = aBV2.mvWorldMin;

	const cVector3f &vMax1 = aBV1.mvWorldMax;
	const cVector3f &vMax2 = aBV2.mvWorldMax;

	// Log("Min1: %s Max1: %s  vs  Min2: %s Max2: %s\n", vMin1.ToString().c_str(),vMax1.ToString().c_str(),
	//												vMin2.ToString().c_str(), vMax2.ToString().c_str());

	if (vMax1.x < vMin2.x || vMax1.y < vMin2.y || vMax1.z < vMin2.z ||
		vMax2.x < vMin1.x || vMax2.y < vMin1.y || vMax2.z < vMin1.z) {
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------

bool cMath::PointBVCollision(const cVector3f &avPoint, cBoundingVolume &aBV) {
	cVector3f vMax = aBV.GetMax();
	cVector3f vMin = aBV.GetMin();

	if (avPoint.x > vMax.x || avPoint.y > vMax.y || avPoint.z > vMax.z ||
		avPoint.x < vMin.x || avPoint.y < vMin.y || avPoint.z < vMin.z) {
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------

bool cMath::GetClipRectFromBV(cRect2l &aDestRect, cBoundingVolume &aBV,
							  const cMatrixf &a_mtxView, const cMatrixf &a_mtxProj,
							  float afNearClipPlane, const cVector2l &avScreenSize) {
	cVector3f vMax = aBV.GetMax();
	cVector3f vMin = aBV.GetMin();
	cVector3f vCorners[8];
	vCorners[0] = cVector3f(vMax.x, vMax.y, vMax.z);
	vCorners[1] = cVector3f(vMax.x, vMax.y, vMin.z);
	vCorners[2] = cVector3f(vMax.x, vMin.y, vMax.z);
	vCorners[3] = cVector3f(vMax.x, vMin.y, vMin.z);

	vCorners[4] = cVector3f(vMin.x, vMax.y, vMax.z);
	vCorners[5] = cVector3f(vMin.x, vMax.y, vMin.z);
	vCorners[6] = cVector3f(vMin.x, vMin.y, vMax.z);
	vCorners[7] = cVector3f(vMin.x, vMin.y, vMin.z);

	bool bVisible = true;

	float fMaxZ = -afNearClipPlane;
	for (int i = 0; i < 8; i++) {
		vCorners[i] = MatrixMul(a_mtxView, vCorners[i]);

		if (vCorners[i].z > fMaxZ) {
			vCorners[i].z = 0;
			bVisible = false; // we are inside the light
		}

		vCorners[i] = MatrixMulDivideW(a_mtxProj, vCorners[i]);
	}

	if (bVisible == false)
		return false;

	vMax = vCorners[0];
	vMin = vCorners[0];
	for (int i = 1; i < 8; i++) {
		if (vCorners[i].x < vMin.x)
			vMin.x = vCorners[i].x;
		if (vCorners[i].x > vMax.x)
			vMax.x = vCorners[i].x;

		// Y
		if (vCorners[i].y < vMin.y)
			vMin.y = vCorners[i].y;
		if (vCorners[i].y > vMax.y)
			vMax.y = vCorners[i].y;
	}

	// Clip min and max
	if (vMin.x < -1)
		vMin.x = -1;
	if (vMin.y < -1)
		vMin.y = -1;
	if (vMax.x > 1)
		vMax.x = 1;
	if (vMax.y > 1)
		vMax.y = 1;

	// Get the screen coordinates
	cVector2f vHalfScreenSize = cVector2f((float)avScreenSize.x, (float)avScreenSize.y) * 0.5f;
	cVector2l vTopLeft;
	cVector2l vBottomRight;
	vTopLeft.x = (int)(vHalfScreenSize.x + vMin.x * vHalfScreenSize.x);
	vTopLeft.y = (int)(vHalfScreenSize.y + vMax.y * -vHalfScreenSize.y);
	vBottomRight.x = (int)(vHalfScreenSize.x + vMax.x * vHalfScreenSize.x);
	vBottomRight.y = (int)(vHalfScreenSize.y + vMin.y * -vHalfScreenSize.y);

	// Clip the screen coordinates
	if (vTopLeft.x < 0)
		vTopLeft.x = 0;
	if (vTopLeft.y < 0)
		vTopLeft.y = 0;
	if (vBottomRight.x > avScreenSize.x - 1)
		vBottomRight.x = avScreenSize.x - 1;
	if (vBottomRight.y > avScreenSize.y - 1)
		vBottomRight.y = avScreenSize.y - 1;

	aDestRect.x = vTopLeft.x;
	aDestRect.y = vTopLeft.y;
	aDestRect.w = (vBottomRight.x - vTopLeft.x) + 1;
	aDestRect.h = (vBottomRight.y - vTopLeft.y) + 1;

	return true;
}

//-----------------------------------------------------------------------

bool cMath::CheckSphereInPlanes(const cVector3f &avCenter, float afRadius,
								const cPlanef *apPlanes, int alPlaneCount) {
	for (int i = 0; i < alPlaneCount; i++) {
		float fDist = cMath::PlaneToPointDist(apPlanes[i], avCenter);

		if (fDist < -afRadius) {
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------

float cMath::GetFraction(float afVal) {
	return afVal - floor(afVal);
}

//-----------------------------------------------------------------------

float cMath::Modulus(float afDividend, float afDivisor) {
	float fNum = floor(ABS(afDividend / afDivisor));

	float fRemain = ABS(afDividend) - ABS(afDivisor) * fNum;

	return fRemain;
}

//-----------------------------------------------------------------------

float cMath::Wrap(float afX, float afMin, float afMax) {
	// Quick check if value is okay. If so just return.
	if (afX >= afMin && afX <= afMax)
		return afX;

	// Change setup so that min is 0
	afMax = afMax - afMin;
	float fOffSet = afMin;
	afMin = 0;
	afX = afX - fOffSet;

	float fNumOfMax = floor(ABS(afX / afMax));

	if (afX >= afMax)
		afX = afX - fNumOfMax * afMax;
	else if (afX < afMin)
		afX = ((fNumOfMax + 1.0f) * afMax) + afX;

	return afX + fOffSet;
}

//-----------------------------------------------------------------------

float cMath::Clamp(float afX, float afMin, float afMax) {
	if (afX > afMax)
		afX = afMax;
	else if (afX < afMin)
		afX = afMin;

	return afX;
}

//-----------------------------------------------------------------------

float cMath::GetAngleDistanceRad(float afAngle1, float afAngle2) {
	return GetAngleDistance(afAngle1, afAngle2, k2Pif);
}

float cMath::GetAngleDistanceDeg(float afAngle1, float afAngle2) {
	return GetAngleDistance(afAngle1, afAngle2, 360.0f);
}

float cMath::GetAngleDistance(float afAngle1, float afAngle2, float afMaxAngle) {
	afAngle1 = Wrap(afAngle1, 0, afMaxAngle);
	afAngle2 = Wrap(afAngle2, 0, afMaxAngle);

	if (afAngle1 == afAngle2) {
		return 0;
	} else {
		float fDist1 = afAngle2 - afAngle1;
		float fDist2 = afMaxAngle - ABS(fDist1);

		if (fDist1 > 0)
			fDist2 = -fDist2;

		if (ABS(fDist1) < ABS(fDist2))
			return fDist1;
		else
			return fDist2;
	}
}

//-----------------------------------------------------------------------

float cMath::TurnAngle(float afAngle, float afFinalAngle, float afSpeed, float afMaxAngle) {
	if (afAngle != afFinalAngle) {
		float fAngleDist = afFinalAngle - afAngle;
		if ((afFinalAngle > afAngle && fAngleDist < afMaxAngle) ||
			(afFinalAngle < afAngle && fAngleDist < (-afMaxAngle)))
			afAngle = afAngle + afSpeed;
		else
			afAngle = afAngle + (-afSpeed);
	}
	if (Abs(GetAngleDistance(afAngle, afFinalAngle, afMaxAngle * 2)) <= (afSpeed * 1.5))
		afAngle = afFinalAngle;

	return afAngle;
}

float cMath::TurnAngleRad(float afAngle, float afFinalAngle, float afSpeed) {
	return TurnAngle(afAngle, afFinalAngle, afSpeed, kPif);
}

float cMath::TurnAngleDeg(float afAngle, float afFinalAngle, float afSpeed) {
	return TurnAngle(afAngle, afFinalAngle, afSpeed, 180.0f);
}

//-----------------------------------------------------------------------

float cMath::GetAngleFromPoints2D(const cVector2f &avStartPos, const cVector2f &avGoalPos) {
	float fDx;
	float fDy;
	float fAns = 0.0;

	fDx = avGoalPos.x - avStartPos.x;
	fDy = avGoalPos.y - avStartPos.y;
	if (fDx == 0)
		fDx = 0.00001f;
	if (fDy == 0)
		fDy = 0.00001f;

	if (fDx >= 0 && fDy < 0) {
		fAns = atan(fDx / (-fDy));
		// Log("1\n");
	} else if (fDx >= 0 && fDy >= 0) {
		fAns = atan(fDy / fDx) + kPi2f;
		// Log("2\n");
	} else if (fDx < 0 && fDy >= 0) {
		fAns = atan((-fDx) / fDy) + kPif;
		// Log("3\n");
	} else if (fDx < 0 && fDy < 0) {
		fAns = atan((-fDy) / (-fDx)) + kPi2f + kPif;
		// Log("4\n");
	}
	return fAns;
}

//-----------------------------------------------------------------------

cVector2f cMath::GetVectorFromAngle2D(float afAngle, float afLength) {
	cVector2f vVec;

	vVec.x = sin(afAngle) * afLength;
	vVec.y = -cos(afAngle) * afLength;

	return vVec;
}

//-----------------------------------------------------------------------

void cMath::GetAngleFromVector(const cVector2f &avVec, float *apAngle, float *apLength) {
	*apLength = sqrt(avVec.x * avVec.x + avVec.y * avVec.y);

	*apAngle = GetAngleFromPoints2D(0, avVec);
}

//-----------------------------------------------------------------------

cVector2f cMath::ProjectVector2D(const cVector2f &avSrcVec, const cVector2f &avDestVec) {
	float fTemp = (avSrcVec.x * avDestVec.x + avSrcVec.y * avDestVec.y) /
				  (avDestVec.x * avDestVec.x + avDestVec.y * avDestVec.y);

	return avDestVec * fTemp;
}

//-----------------------------------------------------------------------

float cMath::ToRad(float afAngle) {
	return (afAngle / 360.0f) * k2Pif;
}

//-----------------------------------------------------------------------

float cMath::ToDeg(float afAngle) {
	return (afAngle / k2Pif) * 360.0f;
}

//-----------------------------------------------------------------------

int cMath::Log2ToInt(int alX) {
	switch (alX) {
	case 1:
		return 0;
	case 2:
		return 1;
	case 4:
		return 2;
	case 8:
		return 3;
	case 16:
		return 4;
	case 32:
		return 5;
	case 64:
		return 6;
	case 128:
		return 7;
	case 256:
		return 8;
	case 512:
		return 9;
	default:
		return (int)floor((log((float)alX) / log(2.0f)) + 0.5f);
	}
	// return (int)floor((log((float)aX) / log(2.0f)) + 0.5f);
}

//-----------------------------------------------------------------------

bool cMath::IsPow2(int alX) {
	switch (alX) {
	case 0:
		return true;
	case 1:
		return true;
	case 2:
		return true;
	case 4:
		return true;
	case 8:
		return true;
	case 16:
		return true;
	case 32:
		return true;
	case 64:
		return true;
	case 128:
		return true;
	case 256:
		return true;
	case 512:
		return true;
	case 1024:
		return true;
	case 2048:
		return true;
	case 4096:
		return true;
	case 8192:
		return true;
	default:
		return false;
	}
}

//-----------------------------------------------------------------------

float cMath::InterpolateFloat(float afA, float afB, float afT) {
	return afA * (1 - afT) + afB * afT;
}

//-----------------------------------------------------------------------

cVector3f cMath::Vector3Cross(const cVector3f &avVecA, const cVector3f &avVecB) {
	cVector3f vResult;

	vResult.x = avVecA.y * avVecB.z - avVecA.z * avVecB.y;
	vResult.y = avVecA.z * avVecB.x - avVecA.x * avVecB.z;
	vResult.z = avVecA.x * avVecB.y - avVecA.y * avVecB.x;

	return vResult;
}

//-----------------------------------------------------------------------

float cMath::Vector3Dot(const cVector3f &avVecA, const cVector3f &avVecB) {
	return avVecA.x * avVecB.x + avVecA.y * avVecB.y + avVecA.z * avVecB.z;
}

//-----------------------------------------------------------------------

cVector3f cMath::ProjectVector3D(const cVector3f &avSrcVec, const cVector3f &avDestVec) {
	float fTemp = (avSrcVec.x * avDestVec.x + avSrcVec.y * avDestVec.y + avSrcVec.z * avDestVec.z) /
				  (avDestVec.x * avDestVec.x + avDestVec.y * avDestVec.y + avDestVec.z * avDestVec.z);

	return avDestVec * fTemp;
}

//-----------------------------------------------------------------------

float cMath::Vector3Angle(const cVector3f &avVecA, const cVector3f &avVecB) {
	float fCos = Vector3Dot(avVecA, avVecB);

	if (ABS(fCos - 1) <= kEpsilonf)
		return 0;

	return acos(fCos);
}

//-----------------------------------------------------------------------

cVector3f cMath::Vector3UnProject(const cVector3f &avVec, const cRect2f &aScreenRect,
								  cMatrixf a_mtxViewProj) {
	cMatrixf mtxInvViewProj = MatrixInverse(a_mtxViewProj);

	cVector3f vNormalized;
	vNormalized.x = ((avVec.x - aScreenRect.x) * 2.0f / aScreenRect.w) - 1.0f;
	vNormalized.y = -(((avVec.y - aScreenRect.y) * 2.0f / aScreenRect.h) - 1.0f);
	vNormalized.z = 2.0f * avVec.z - 1.0f;

	// Log("Normalized: %s\n",vNormalized.ToString().c_str());

	// Object coordinates.
	vNormalized = MatrixMulDivideW(mtxInvViewProj, vNormalized);
	// vNormalized = MatrixMul(mtxInvViewProj,vNormalized);

	// Log("Normalized After: %s\n",vNormalized.ToString().c_str());

	return vNormalized * -1;
}

//-----------------------------------------------------------------------

// Helper function for GetAnglesFromPoints3D
static float GetAngleFromPoints2DSimple(const cVector3f &avStartPos, const cVector3f &avGoalPos) {
	cVector3f vDelta = avGoalPos - avStartPos;

	if (vDelta.x == 0)
		vDelta.x = -kEpsilonf;
	if (vDelta.y == 0)
		vDelta.y = kEpsilonf;

	if (vDelta.y >= 0 && vDelta.x <= 0) {
		return -atan(vDelta.y / vDelta.x);
	} else if (vDelta.y < 0 && vDelta.x <= 0) {
		return -atan(vDelta.y / vDelta.x);
	} else {
		Error("Error in GetAngle 3D code! ARGHHH run in terror\n");
		return -1000;
	}
}

cVector3f cMath::GetAngleFromPoints3D(const cVector3f &avStartPos, const cVector3f &avGoalPos) {
	cVector3f vAngle = cVector3f(0, 0, 0);

	vAngle.y = -GetAngleFromPoints2D(cVector2f(avStartPos.x, avStartPos.z),
									 cVector2f(avGoalPos.x, avGoalPos.z));

	// Log("Y Angle: %f\n",vAngle.y);

	// vAngle.y = Wrap(vAngle.y,0.0f, k2Pif);

	cMatrixf mtxRot = MatrixRotateY(-vAngle.y);
	cVector3f vDelta = avGoalPos - avStartPos;

	// Log("vDelta: %s\n",vDelta.ToString().c_str());

	cVector3f vGoal = MatrixMul(mtxRot, vDelta);

	// Log("vGoal: %s\n",vGoal.ToString().c_str());

	vAngle.x = GetAngleFromPoints2DSimple(cVector3f(0, 0, 0), cVector2f(vGoal.z, vGoal.y));

	// Log("X Angle: %f\n",vAngle.x);

	vAngle.x = Wrap(vAngle.x, 0.0f, k2Pif);

	return vAngle;
}

//-----------------------------------------------------------------------

float cMath::PlaneToPointDist(const cPlanef &aPlane, const cVector3f &avVec) {
	return (aPlane.a * avVec.x) + (aPlane.b * avVec.y) + (aPlane.c * avVec.z) + aPlane.d;
}

//-----------------------------------------------------------------------

// TODO: This only works when there is no 0 in the planes.
void cMath::PlaneIntersectionLine(const cPlanef &aPA, const cPlanef &aPB,
								  cVector3f &avDir, cVector3f &avPoint) {
	avDir = Vector3Cross(cVector3f(aPA.a, aPA.b, aPA.c), cVector3f(aPB.a, aPB.b, aPB.c));

	// Set x to 0 so the calculation can be solved
	avPoint.x = 0;

	// find a value that sets b + b to 0.
	float fVal = (-aPB.b) / aPA.b;

	// Get z
	avPoint.z = ((aPA.d * fVal) + aPB.d) / ((aPA.c * fVal) + aPB.c);

	// Get y
	avPoint.y = (aPA.d - (aPA.c * avPoint.z)) / aPA.b;
}

//-----------------------------------------------------------------------

static inline bool IntersectsPlanePair(const cPlanef &aPlane1, const cPlanef &aPlane2,
									   const cVector3f &avPoint1, const cVector3f &avPoint2) {
	bool bPair[2][2];

	// 1 to 1
	if (cMath::PlaneToPointDist(aPlane1, avPoint1) >= 0)
		bPair[0][0] = true;
	else
		bPair[0][0] = false;
	// 1 to 2
	if (cMath::PlaneToPointDist(aPlane1, avPoint2) >= 0)
		bPair[0][1] = true;
	else
		bPair[0][1] = false;
	// 2 to 1
	if (cMath::PlaneToPointDist(aPlane2, avPoint1) >= 0)
		bPair[1][0] = true;
	else
		bPair[1][0] = false;
	// 2 to 2
	if (cMath::PlaneToPointDist(aPlane2, avPoint2) >= 0)
		bPair[1][1] = true;
	else
		bPair[1][1] = false;

	/*Log("-------------\n");
	Log("Pair 0 - 0: %d Pair 0 - 1: %d\n",bPair[0][0]?1:0,bPair[0][1]?1:0);
	Log("Pair 1 - 0: %d Pair 1 - 1: %d\n",bPair[1][0]?1:0,bPair[1][1]?1:0);*/

	// If either point is inside the plane pair we have an intersection.
	if ((bPair[0][0] && bPair[1][0]) || (bPair[0][1] && bPair[1][1]))
		return true;

	// If they are on different sides of the plane, it is intersected as well.
	if (((bPair[0][0] && !bPair[0][1]) && (!bPair[1][0] && bPair[1][1])) ||
		((!bPair[0][0] && bPair[0][1]) && (bPair[1][0] && !bPair[1][1]))) {
		return true;
	}

	return false;
}

bool cMath::CheckFrustumLineIntersection(const cPlanef *apPlanePairs, const cVector3f &avPoint1,
										 const cVector3f &avPoint2, int alPairNum) {
	for (int i = 0; i < alPairNum; ++i) {
		int lStart = i * 2;
		if (IntersectsPlanePair(apPlanePairs[lStart], apPlanePairs[lStart + 1], avPoint1, avPoint2) == false) {
			return false;
		}
	}

	return true;

	/*if( IntersectsPlanePair(apPlanePairs[0],apPlanePairs[1],avPoint1,avPoint2)
		&&
		IntersectsPlanePair(apPlanePairs[2],apPlanePairs[3],avPoint1,avPoint2)
		&&
		IntersectsPlanePair(apPlanePairs[4],apPlanePairs[5],avPoint1,avPoint2)
		)
	{
		return true;
	}
	return false;
	*/
}

//-----------------------------------------------------------------------

static const int kvQuadPairs[4][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};

bool cMath::CheckFrustumQuadMeshIntersection(const cPlanef *apPlanePairs, tVector3fVec *apPoints,
											 int alPairNum) {
	int lPointNum = (int)apPoints->size();

	for (int quad = 0; quad < lPointNum; quad += 4) {
		cVector3f *pQuad = &(*apPoints)[quad];

		// Check the pairs
		for (int i = 0; i < 4; i++) {
			if (CheckFrustumLineIntersection(apPlanePairs, pQuad[kvQuadPairs[i][0]],
											 pQuad[kvQuadPairs[i][1]], alPairNum)) {
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------

float cMath::QuaternionDot(const cQuaternion &aqA, const cQuaternion &aqB) {
	return aqA.w * aqB.w + aqA.v.x * aqB.v.x + aqA.v.y * aqB.v.y + aqA.v.z * aqB.v.z;
}

//-----------------------------------------------------------------------

cQuaternion cMath::QuaternionSlerp(float afT, const cQuaternion &aqA, const cQuaternion &aqB,
								   bool abShortestPath) {
	float fCos = QuaternionDot(aqA, aqB);

	// If the rotations are the same, just return the first.
	if (ABS(fCos - 1) <= kEpsilonf) {
		return aqA;
	}

	float fAngle = acos(fCos);

	float fSin = sin(fAngle);
	float fInvSin = 1.0f / fSin;
	float fCoeff0 = sin((1.0f - afT) * fAngle) * fInvSin;
	float fCoeff1 = sin(afT * fAngle) * fInvSin;
	// Do we need to invert rotation?
	if (fCos < 0.0f && abShortestPath) {
		fCoeff0 = -fCoeff0;
		// taking the complement requires renormalisation
		cQuaternion qT(aqA * fCoeff0 + aqB * fCoeff1);
		qT.Normalise();
		return qT;
	} else {
		return aqA * fCoeff0 + aqB * fCoeff1;
	}
}

//-----------------------------------------------------------------------

cMatrixf cMath::MatrixSlerp(float afT, const cMatrixf &a_mtxA, const cMatrixf &a_mtxB,
							bool abShortestPath) {
	cVector3f vPos = a_mtxA.GetTranslation() * (1 - afT) + a_mtxB.GetTranslation() * afT;

	cQuaternion qA;
	qA.FromRotationMatrix(a_mtxA);
	cQuaternion qB;
	qB.FromRotationMatrix(a_mtxB);

	cQuaternion qFinal = cMath::QuaternionSlerp(afT, qA, qB, true);

	cMatrixf mtxFinal = cMath::MatrixQuaternion(qFinal);
	mtxFinal.SetTranslation(vPos);

	return mtxFinal;
}

//-----------------------------------------------------------------------

cMatrixf cMath::MatrixMul(const cMatrixf &a_mtxA, const cMatrixf &a_mtxB) {
	cMatrixf mtxC;

	mtxC.m[0][0] = a_mtxA.m[0][0] * a_mtxB.m[0][0] + a_mtxA.m[0][1] * a_mtxB.m[1][0] + a_mtxA.m[0][2] * a_mtxB.m[2][0] + a_mtxA.m[0][3] * a_mtxB.m[3][0];
	mtxC.m[0][1] = a_mtxA.m[0][0] * a_mtxB.m[0][1] + a_mtxA.m[0][1] * a_mtxB.m[1][1] + a_mtxA.m[0][2] * a_mtxB.m[2][1] + a_mtxA.m[0][3] * a_mtxB.m[3][1];
	mtxC.m[0][2] = a_mtxA.m[0][0] * a_mtxB.m[0][2] + a_mtxA.m[0][1] * a_mtxB.m[1][2] + a_mtxA.m[0][2] * a_mtxB.m[2][2] + a_mtxA.m[0][3] * a_mtxB.m[3][2];
	mtxC.m[0][3] = a_mtxA.m[0][0] * a_mtxB.m[0][3] + a_mtxA.m[0][1] * a_mtxB.m[1][3] + a_mtxA.m[0][2] * a_mtxB.m[2][3] + a_mtxA.m[0][3] * a_mtxB.m[3][3];

	mtxC.m[1][0] = a_mtxA.m[1][0] * a_mtxB.m[0][0] + a_mtxA.m[1][1] * a_mtxB.m[1][0] + a_mtxA.m[1][2] * a_mtxB.m[2][0] + a_mtxA.m[1][3] * a_mtxB.m[3][0];
	mtxC.m[1][1] = a_mtxA.m[1][0] * a_mtxB.m[0][1] + a_mtxA.m[1][1] * a_mtxB.m[1][1] + a_mtxA.m[1][2] * a_mtxB.m[2][1] + a_mtxA.m[1][3] * a_mtxB.m[3][1];
	mtxC.m[1][2] = a_mtxA.m[1][0] * a_mtxB.m[0][2] + a_mtxA.m[1][1] * a_mtxB.m[1][2] + a_mtxA.m[1][2] * a_mtxB.m[2][2] + a_mtxA.m[1][3] * a_mtxB.m[3][2];
	mtxC.m[1][3] = a_mtxA.m[1][0] * a_mtxB.m[0][3] + a_mtxA.m[1][1] * a_mtxB.m[1][3] + a_mtxA.m[1][2] * a_mtxB.m[2][3] + a_mtxA.m[1][3] * a_mtxB.m[3][3];

	mtxC.m[2][0] = a_mtxA.m[2][0] * a_mtxB.m[0][0] + a_mtxA.m[2][1] * a_mtxB.m[1][0] + a_mtxA.m[2][2] * a_mtxB.m[2][0] + a_mtxA.m[2][3] * a_mtxB.m[3][0];
	mtxC.m[2][1] = a_mtxA.m[2][0] * a_mtxB.m[0][1] + a_mtxA.m[2][1] * a_mtxB.m[1][1] + a_mtxA.m[2][2] * a_mtxB.m[2][1] + a_mtxA.m[2][3] * a_mtxB.m[3][1];
	mtxC.m[2][2] = a_mtxA.m[2][0] * a_mtxB.m[0][2] + a_mtxA.m[2][1] * a_mtxB.m[1][2] + a_mtxA.m[2][2] * a_mtxB.m[2][2] + a_mtxA.m[2][3] * a_mtxB.m[3][2];
	mtxC.m[2][3] = a_mtxA.m[2][0] * a_mtxB.m[0][3] + a_mtxA.m[2][1] * a_mtxB.m[1][3] + a_mtxA.m[2][2] * a_mtxB.m[2][3] + a_mtxA.m[2][3] * a_mtxB.m[3][3];

	mtxC.m[3][0] = a_mtxA.m[3][0] * a_mtxB.m[0][0] + a_mtxA.m[3][1] * a_mtxB.m[1][0] + a_mtxA.m[3][2] * a_mtxB.m[2][0] + a_mtxA.m[3][3] * a_mtxB.m[3][0];
	mtxC.m[3][1] = a_mtxA.m[3][0] * a_mtxB.m[0][1] + a_mtxA.m[3][1] * a_mtxB.m[1][1] + a_mtxA.m[3][2] * a_mtxB.m[2][1] + a_mtxA.m[3][3] * a_mtxB.m[3][1];
	mtxC.m[3][2] = a_mtxA.m[3][0] * a_mtxB.m[0][2] + a_mtxA.m[3][1] * a_mtxB.m[1][2] + a_mtxA.m[3][2] * a_mtxB.m[2][2] + a_mtxA.m[3][3] * a_mtxB.m[3][2];
	mtxC.m[3][3] = a_mtxA.m[3][0] * a_mtxB.m[0][3] + a_mtxA.m[3][1] * a_mtxB.m[1][3] + a_mtxA.m[3][2] * a_mtxB.m[2][3] + a_mtxA.m[3][3] * a_mtxB.m[3][3];

	return mtxC;
}

//-----------------------------------------------------------------------

cVector3f cMath::MatrixMul(const cMatrixf &a_mtxA, const cVector3f &avB) {
	cVector3f vC;

	vC.x = (a_mtxA.m[0][0] * avB.x + a_mtxA.m[0][1] * avB.y + a_mtxA.m[0][2] * avB.z + a_mtxA.m[0][3]);
	vC.y = (a_mtxA.m[1][0] * avB.x + a_mtxA.m[1][1] * avB.y + a_mtxA.m[1][2] * avB.z + a_mtxA.m[1][3]);
	vC.z = (a_mtxA.m[2][0] * avB.x + a_mtxA.m[2][1] * avB.y + a_mtxA.m[2][2] * avB.z + a_mtxA.m[2][3]);

	return vC;
}

//-----------------------------------------------------------------------

cVector3f cMath::MatrixMulDivideW(const cMatrixf &a_mtxA, const cVector3f &avB) {
	cVector3f vC;

	float fInvW = 1.0f / (a_mtxA.m[3][0] * avB.x + a_mtxA.m[3][1] * avB.y + a_mtxA.m[3][2] * avB.z + a_mtxA.m[3][3]);

	vC.x = (a_mtxA.m[0][0] * avB.x + a_mtxA.m[0][1] * avB.y + a_mtxA.m[0][2] * avB.z + a_mtxA.m[0][3]) * fInvW;
	vC.y = (a_mtxA.m[1][0] * avB.x + a_mtxA.m[1][1] * avB.y + a_mtxA.m[1][2] * avB.z + a_mtxA.m[1][3]) * fInvW;
	vC.z = (a_mtxA.m[2][0] * avB.x + a_mtxA.m[2][1] * avB.y + a_mtxA.m[2][2] * avB.z + a_mtxA.m[2][3]) * fInvW;

	return vC;
}

//-----------------------------------------------------------------------

cMatrixf cMath::MatrixMulScalar(const cMatrixf &a_mtxA, float afB) {
	cMatrixf mtxC;

	mtxC.m[0][0] = a_mtxA.m[0][0] * afB;
	mtxC.m[0][1] = a_mtxA.m[0][1] * afB;
	mtxC.m[0][2] = a_mtxA.m[0][2] * afB;
	mtxC.m[0][3] = a_mtxA.m[0][3] * afB;
	mtxC.m[1][0] = a_mtxA.m[1][0] * afB;
	mtxC.m[1][1] = a_mtxA.m[1][1] * afB;
	mtxC.m[1][2] = a_mtxA.m[1][2] * afB;
	mtxC.m[1][3] = a_mtxA.m[1][3] * afB;
	mtxC.m[2][0] = a_mtxA.m[2][0] * afB;
	mtxC.m[2][1] = a_mtxA.m[2][1] * afB;
	mtxC.m[2][2] = a_mtxA.m[2][2] * afB;
	mtxC.m[2][3] = a_mtxA.m[2][3] * afB;
	mtxC.m[3][0] = a_mtxA.m[3][0] * afB;
	mtxC.m[3][1] = a_mtxA.m[3][1] * afB;
	mtxC.m[3][2] = a_mtxA.m[3][2] * afB;
	mtxC.m[3][3] = a_mtxA.m[3][3] * afB;

	return mtxC;
}

//-----------------------------------------------------------------------

cMatrixf cMath::MatrixRotate(cVector3f avRot, eEulerRotationOrder aOrder) {
	cMatrixf mtxRot = cMatrixf::Identity;
	switch (aOrder) {
	case eEulerRotationOrder_XYZ:
		mtxRot = MatrixMul(MatrixRotateX(avRot.x), mtxRot);
		mtxRot = MatrixMul(MatrixRotateY(avRot.y), mtxRot);
		mtxRot = MatrixMul(MatrixRotateZ(avRot.z), mtxRot);
		break;
	case eEulerRotationOrder_XZY:
		mtxRot = MatrixMul(MatrixRotateX(avRot.x), mtxRot);
		mtxRot = MatrixMul(MatrixRotateZ(avRot.z), mtxRot);
		mtxRot = MatrixMul(MatrixRotateY(avRot.y), mtxRot);
		break;
	case eEulerRotationOrder_YXZ:
		mtxRot = MatrixMul(MatrixRotateY(avRot.y), mtxRot);
		mtxRot = MatrixMul(MatrixRotateX(avRot.x), mtxRot);
		mtxRot = MatrixMul(MatrixRotateZ(avRot.z), mtxRot);
		break;
	case eEulerRotationOrder_YZX:
		mtxRot = MatrixMul(MatrixRotateY(avRot.y), mtxRot);
		mtxRot = MatrixMul(MatrixRotateZ(avRot.z), mtxRot);
		mtxRot = MatrixMul(MatrixRotateX(avRot.x), mtxRot);
		break;
	case eEulerRotationOrder_ZXY:
		mtxRot = MatrixMul(MatrixRotateZ(avRot.z), mtxRot);
		mtxRot = MatrixMul(MatrixRotateX(avRot.x), mtxRot);
		mtxRot = MatrixMul(MatrixRotateY(avRot.y), mtxRot);
		break;
	case eEulerRotationOrder_ZYX:
		mtxRot = MatrixMul(MatrixRotateZ(avRot.z), mtxRot);
		mtxRot = MatrixMul(MatrixRotateY(avRot.y), mtxRot);
		mtxRot = MatrixMul(MatrixRotateX(avRot.x), mtxRot);
		break;
	case eEulerRotationOrder_LastEnum:
		break;
	}

	return mtxRot;
}

//-----------------------------------------------------------------------

cMatrixf cMath::MatrixRotateX(float afAngle) {
	return cMatrixf(1, 0, 0, 0,
					0, cos(afAngle), -sin(afAngle), 0,
					0, sin(afAngle), cos(afAngle), 0,
					0, 0, 0, 1);
}

//-----------------------------------------------------------------------

cMatrixf cMath::MatrixRotateY(float afAngle) {
	return cMatrixf(cos(afAngle), 0, sin(afAngle), 0,
					0, 1, 0, 0,
					-sin(afAngle), 0, cos(afAngle), 0,
					0, 0, 0, 1);
}

//-----------------------------------------------------------------------

cMatrixf cMath::MatrixRotateZ(float afAngle) {
	return cMatrixf(cos(afAngle), -sin(afAngle), 0, 0,
					sin(afAngle), cos(afAngle), 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1);
}

//-----------------------------------------------------------------------

cMatrixf cMath::MatrixQuaternion(const cQuaternion &aqRot) {
	cMatrixf mtxOut;
	// Set the non rotation part.
	mtxOut.m[0][3] = 0;
	mtxOut.m[1][3] = 0;
	mtxOut.m[2][3] = 0;
	mtxOut.m[3][0] = 0;
	mtxOut.m[3][1] = 0;
	mtxOut.m[3][2] = 0;
	mtxOut.m[3][3] = 1;

	aqRot.ToRotationMatrix(mtxOut);

	return mtxOut;
}

//-----------------------------------------------------------------------

cMatrixf cMath::MatrixScale(cVector3f avScale) {
	return cMatrixf(avScale.x, 0, 0, 0,
					0, avScale.y, 0, 0,
					0, 0, avScale.z, 0,
					0, 0, 0, 1);
}

//-----------------------------------------------------------------------

cMatrixf cMath::MatrixTranslate(cVector3f avTrans) {
	return cMatrixf(1, 0, 0, avTrans.x,
					0, 1, 0, avTrans.y,
					0, 0, 1, avTrans.z,
					0, 0, 0, 1);
}
//-----------------------------------------------------------------------

float cMath::MatrixMinor(const cMatrixf &a_mtxA,
						 const size_t r0, const size_t r1, const size_t r2,
						 const size_t c0, const size_t c1, const size_t c2) {
	return a_mtxA.m[r0][c0] * (a_mtxA.m[r1][c1] * a_mtxA.m[r2][c2] - a_mtxA.m[r2][c1] * a_mtxA.m[r1][c2]) -
		   a_mtxA.m[r0][c1] * (a_mtxA.m[r1][c0] * a_mtxA.m[r2][c2] - a_mtxA.m[r2][c0] * a_mtxA.m[r1][c2]) +
		   a_mtxA.m[r0][c2] * (a_mtxA.m[r1][c0] * a_mtxA.m[r2][c1] - a_mtxA.m[r2][c0] * a_mtxA.m[r1][c1]);
}

//-----------------------------------------------------------------------

cMatrixf cMath::MatrixAdjoint(const cMatrixf &a_mtxA) {
	return cMatrixf(MatrixMinor(a_mtxA, 1, 2, 3, 1, 2, 3),
					-MatrixMinor(a_mtxA, 0, 2, 3, 1, 2, 3),
					MatrixMinor(a_mtxA, 0, 1, 3, 1, 2, 3),
					-MatrixMinor(a_mtxA, 0, 1, 2, 1, 2, 3),

					-MatrixMinor(a_mtxA, 1, 2, 3, 0, 2, 3),
					MatrixMinor(a_mtxA, 0, 2, 3, 0, 2, 3),
					-MatrixMinor(a_mtxA, 0, 1, 3, 0, 2, 3),
					MatrixMinor(a_mtxA, 0, 1, 2, 0, 2, 3),

					MatrixMinor(a_mtxA, 1, 2, 3, 0, 1, 3),
					-MatrixMinor(a_mtxA, 0, 2, 3, 0, 1, 3),
					MatrixMinor(a_mtxA, 0, 1, 3, 0, 1, 3),
					-MatrixMinor(a_mtxA, 0, 1, 2, 0, 1, 3),

					-MatrixMinor(a_mtxA, 1, 2, 3, 0, 1, 2),
					MatrixMinor(a_mtxA, 0, 2, 3, 0, 1, 2),
					-MatrixMinor(a_mtxA, 0, 1, 3, 0, 1, 2),
					MatrixMinor(a_mtxA, 0, 1, 2, 0, 1, 2));
}

//-----------------------------------------------------------------------

float cMath::MatrixDeterminant(const cMatrixf &a_mtxA) {
	return a_mtxA.m[0][0] * MatrixMinor(a_mtxA, 1, 2, 3, 1, 2, 3) -
		   a_mtxA.m[0][1] * MatrixMinor(a_mtxA, 1, 2, 3, 0, 2, 3) +
		   a_mtxA.m[0][2] * MatrixMinor(a_mtxA, 1, 2, 3, 0, 1, 3) -
		   a_mtxA.m[0][3] * MatrixMinor(a_mtxA, 1, 2, 3, 0, 1, 2);
}

//-----------------------------------------------------------------------

cMatrixf cMath::MatrixInverse(const cMatrixf &a_mtxA) {
	return MatrixMulScalar(MatrixAdjoint(a_mtxA), (1.0f / MatrixDeterminant(a_mtxA)));
}

//-----------------------------------------------------------------------

cVector3f cMath::MatrixToEulerAngles(const cMatrixf &a_mtxA, eEulerRotationOrder aOrder) {
	cVector3f vAngles;

	/*if (a_mtxA.m[2][0] > 0.998f) // singularity at north pole
	{
		vAngles.y = atan2(a_mtxA.m[0][2],a_mtxA.m[2][2]);
		vAngles.z = kPif/2;
		vAngles.x = 0;
		Log("Special case 1\n");
	}
	if (a_mtxA.m[2][0] < -0.998f) // singularity at south pole
	{
		vAngles.y = atan2(a_mtxA.m[0][2],a_mtxA.m[2][2]);
		vAngles.z = -kPif/2;
		vAngles.x = 0;
		Log("Special case 2\n");
	}
	else*/
	{
		vAngles.x = atan2(a_mtxA.m[2][1], a_mtxA.m[2][2]);
		vAngles.y = -asin(a_mtxA.m[2][0]);
		vAngles.z = atan2(a_mtxA.m[1][0], a_mtxA.m[0][0]);
	}
	return vAngles;
}

//-----------------------------------------------------------------------

const char *cMath::MatrixToChar(const cMatrixf &a_mtxA) {
	snprintf(mpTempChar, 1024, "[%.3f, %.3f, %.3f, %.3f] [%.3f, %.3f, %.3f, %.3f] [%.3f, %.3f, %.3f, %.3f] [%.3f, %.3f, %.3f, %.3f]",
			 a_mtxA.m[0][0], a_mtxA.m[0][1], a_mtxA.m[0][2], a_mtxA.m[0][3],
			 a_mtxA.m[1][0], a_mtxA.m[1][1], a_mtxA.m[1][2], a_mtxA.m[1][3],
			 a_mtxA.m[2][0], a_mtxA.m[2][1], a_mtxA.m[2][2], a_mtxA.m[2][3],
			 a_mtxA.m[3][0], a_mtxA.m[3][1], a_mtxA.m[3][2], a_mtxA.m[3][3]);

	return mpTempChar;
}
//-----------------------------------------------------------------------

static inline cVector3f GetVector3(const float *apVertexArray, int alIdx, int alStride) {
	const float *apVec = &apVertexArray[alIdx * alStride];

	return cVector3f(apVec[0], apVec[1], apVec[2]);
}

/*static inline void AddVector3(float *apArray, int alIdx, const cVector3f &avVec, int alStride) {
	float *apVec = &apArray[alIdx * alStride];
	apVec[0] += avVec.x;
	apVec[1] += avVec.y;
	apVec[2] += avVec.z;
}*/

static inline void SetVector4(const cVector3f &avVec, float afW, float *apArray, int alIdx) {
	float *apVec = &apArray[alIdx * 4];
	apVec[0] = avVec.x;
	apVec[1] = avVec.y;
	apVec[2] = avVec.z;
	apVec[3] = afW;
}

static inline bool Vector3Equal(const float *apArrayA, int alIdxA, const float *apArrayB, int alIdxB,
								int alStride) {
	if (apArrayA[alIdxA * alStride + 0] == apArrayB[alIdxB * alStride + 0] &&
		apArrayA[alIdxA * alStride + 1] == apArrayB[alIdxB * alStride + 1] &&
		apArrayA[alIdxA * alStride + 2] == apArrayB[alIdxB * alStride + 2]) {
		return true;
	}

	return false;
}

bool cMath::CreateTriTangentVectors(float *apDestArray,
									const unsigned int *apIndexArray, int alIndexNum,
									const float *apVertexArray, int alVtxStride,
									const float *apTexArray,
									const float *apNormalArray,
									int alVertexNum) {
	// Create two temp arrays and clear them.
	tVector3fVec vTempTangents1;
	tVector3fVec vTempTangents2;

	// Log("Creating tangents:\n");
	// Log("Num of indices: %d\n",alIndexNum);
	// Log("Num of vertrices: %d\n",alVertexNum);

	Hpl1::resizeAndFill(vTempTangents1, alVertexNum, cVector3f(0, 0, 0));
	Hpl1::resizeAndFill(vTempTangents2, alVertexNum, cVector3f(0, 0, 0));

	// Iterate through the triangles
	for (int triIdx = 0; triIdx < alIndexNum; triIdx += 3) {
		// Log("Triangle %d: ",triIdx/3);

		// Get the indices of the triangle
		int idx1 = apIndexArray[triIdx + 0];
		int idx2 = apIndexArray[triIdx + 1];
		int idx3 = apIndexArray[triIdx + 2];

		// Log("1: '%d' 2: '%d'  3: '%d' ", idx1, idx2, idx3);

		// Get the 3 points making up the triangle
		cVector3f vPos1 = GetVector3(apVertexArray, idx1, alVtxStride);
		cVector3f vPos2 = GetVector3(apVertexArray, idx2, alVtxStride);
		cVector3f vPos3 = GetVector3(apVertexArray, idx3, alVtxStride);

		// Get the 3 texture coords in the triangle.
		cVector3f vTex1 = GetVector3(apTexArray, idx1, 3);
		cVector3f vTex2 = GetVector3(apTexArray, idx2, 3);
		cVector3f vTex3 = GetVector3(apTexArray, idx3, 3);

		// Get the vectors between the positions.
		cVector3f vPos1To2 = vPos2 - vPos1;
		cVector3f vPos1To3 = vPos3 - vPos1;

		// Get the vectors between the tex coords
		cVector3f vTex1To2 = vTex2 - vTex1;
		cVector3f vTex1To3 = vTex3 - vTex1;

		// Get the direction of the S and T tangents
		float fR = 1.0f / (vTex1To2.x * vTex1To3.y - vTex1To2.y * vTex1To3.x);

		cVector3f vSDir((vTex1To3.y * vPos1To2.x - vTex1To2.y * vPos1To3.x) * fR,
						(vTex1To3.y * vPos1To2.y - vTex1To2.y * vPos1To3.y) * fR,
						(vTex1To3.y * vPos1To2.z - vTex1To2.y * vPos1To3.z) * fR);

		cVector3f vTDir((vTex1To2.x * vPos1To3.x - vTex1To3.x * vPos1To2.x) * fR,
						(vTex1To2.x * vPos1To3.y - vTex1To3.x * vPos1To2.y) * fR,
						(vTex1To2.x * vPos1To3.z - vTex1To3.x * vPos1To2.z) * fR);

		// Add the temp arrays with the values:
		vTempTangents1[idx1] += vSDir;
		vTempTangents1[idx2] += vSDir;
		vTempTangents1[idx3] += vSDir;

		vTempTangents2[idx1] += vTDir;
		vTempTangents2[idx2] += vTDir;
		vTempTangents2[idx3] += vTDir;

		// Log("\n");
	}

	// Log("Looking for duplicates: \n");
	// Go through the vertrices and find normal and vertex copies. Smooth the tangents on these
	float fMaxCosAngle = -1.0f;
	for (int i = 0; i < alVertexNum; i++) {
		for (int j = i + 1; j < alVertexNum; j++) {
			// Log("(%.1f, %.1f, %.1f)", apVertexArray[i+0],apVertexArray[i+1],apVertexArray[i+2]);
			// Log(" vs ");
			// Log("(%.1f, %.1f, %.1f)\n", apVertexArray[j+0],apVertexArray[j+1],apVertexArray[j+2]);

			if (Vector3Equal(apVertexArray, i, apVertexArray, j, alVtxStride) &&
				Vector3Equal(apNormalArray, i, apNormalArray, j, 3)) {
				// Log("Found at %d and %d!\n", i, j);

				cVector3f vAT1 = vTempTangents1[i];
				cVector3f vAT2 = vTempTangents2[i];

				cVector3f vBT1 = vTempTangents1[j];
				cVector3f vBT2 = vTempTangents2[j];

				if (Vector3Dot(vAT1, vBT1) >= fMaxCosAngle) {
					vTempTangents1[j] += vAT1;
					vTempTangents1[i] += vBT1;
				}

				if (Vector3Dot(vAT2, vBT2) >= fMaxCosAngle) {
					vTempTangents2[j] += vAT2;
					vTempTangents2[i] += vBT2;
				}
			}
		}
	}

	// Iterate through the dest array and set tangent values
	for (int vtxIdx = 0; vtxIdx < alVertexNum; vtxIdx++) {

		cVector3f vNormal = GetVector3(apNormalArray, vtxIdx, 3);
		cVector3f &vTempTan1 = vTempTangents1[vtxIdx];
		cVector3f &vTempTan2 = vTempTangents2[vtxIdx];

		// Gram-Schmidt orthogonalize
		cVector3f vTan = vTempTan1 - (vNormal * cMath::Vector3Dot(vNormal, vTempTan1));
		vTan.Normalise();

		// Log("Add tangent %d: ",vtxIdx);
		// Log(" %.1f, %.1f, %.1f ",vTan.x, vTan.y, vTan.z);
		// Log("\n");

		// Calculate if left or right handed.
		float fW = (cMath::Vector3Dot(cMath::Vector3Cross(vNormal, vTempTan1), vTempTan2) < 0.0f) ? -1.0f : 1.0f;

		SetVector4(vTan, fW, apDestArray, vtxIdx);
	}

	return true;
}

//-----------------------------------------------------------------------

bool cMath::CreateTriangleData(tTriangleDataVec &avTriangles,
							   const unsigned int *apIndexArray, int alIndexNum,
							   const float *apVertexArray, int alVtxStride, int alVertexNum) {
	int lNumOfTri = alIndexNum / 3;
	if ((int)avTriangles.size() < lNumOfTri)
		avTriangles.resize(lNumOfTri);

	// Log("Creating triangle data:\n");
	for (int tri = 0, idx = 0; tri < lNumOfTri; tri++, idx += 3) {
		// Log("checking: tri %d idx %d\n",tri, idx);
		// Calculate normal
		const float *pVtx0 = &apVertexArray[apIndexArray[idx] * alVtxStride];
		const float *pVtx1 = &apVertexArray[apIndexArray[idx + 1] * alVtxStride];
		const float *pVtx2 = &apVertexArray[apIndexArray[idx + 2] * alVtxStride];

		cVector3f vEdge1(pVtx1[0] - pVtx0[0], pVtx1[1] - pVtx0[1], pVtx1[2] - pVtx0[2]);
		cVector3f vEdge2(pVtx2[0] - pVtx0[0], pVtx2[1] - pVtx0[1], pVtx2[2] - pVtx0[2]);

		avTriangles[tri].normal = Vector3Cross(vEdge2, vEdge1);
	}

	return true;
}

//-----------------------------------------------------------------------
/*static bool EdgePointEqual(const float *apVertexArray,
						   const cTriEdge &edge1, const cTriEdge &edge2, int alStride) {
	if (Vector3Equal(apVertexArray, edge1.point1, apVertexArray, edge2.point1, alStride) &&
		Vector3Equal(apVertexArray, edge1.point2, apVertexArray, edge2.point2, alStride)) {
		return true;
	}

	if (Vector3Equal(apVertexArray, edge1.point1, apVertexArray, edge2.point2, alStride) &&
		Vector3Equal(apVertexArray, edge1.point2, apVertexArray, edge2.point1, alStride)) {
		return true;
	}

	return false;
}

/////////////////////////

static bool EdgeTriEqual(const cTriEdge &edge1, const cTriEdge &edge2) {
	if (edge1.tri1 == edge2.tri1 && edge1.tri2 == edge2.tri2)
		return true;
	if (edge1.tri1 == edge1.tri1 && edge1.tri2 == edge2.tri1)
		return true;
	return false;
}

/////////////////////////

static bool EdgeEqual(const float *apVertexArray, const cTriEdge &edge1, const cTriEdge &edge2, int alStride) {
	if (EdgePointEqual(apVertexArray, edge1, edge2, alStride) && EdgeTriEqual(edge1, edge2))
		return true;

	return false;
}*/

/////////////////////////

static const float *gpVertexArray;
static const unsigned int *gpIndexArray;
static int glVertexStride;

//////////////////////////////////////////////////////

class cVertexIndices {
public:
	cVertexIndices(unsigned int alIdx) {
		mlstIndices.push_back(alIdx);
	}
	tUIntList mlstIndices;
};

typedef Common::StableMap<cVector3f, cVertexIndices> tVtxIdxMap;
typedef tVtxIdxMap::iterator tVtxIdxMapIt;

//////////////////////////////////////////////////////

class cEdgeCompare {
public:
	// The point1 must be > than point2 for this to work!
	bool operator()(const cTriEdge &Edge1, const cTriEdge &Edge2) const {
		cVector3f vPoint1_1 = GetVector3(gpVertexArray, Edge1.point1, glVertexStride);
		cVector3f vPoint1_2 = GetVector3(gpVertexArray, Edge1.point2, glVertexStride);
		cVector3f vPoint2_1 = GetVector3(gpVertexArray, Edge2.point1, glVertexStride);
		cVector3f vPoint2_2 = GetVector3(gpVertexArray, Edge2.point2, glVertexStride);

		// 1 - 1
		if (vPoint1_1.x != vPoint2_1.x)
			return vPoint1_1.x > vPoint2_1.x;
		if (vPoint1_1.y != vPoint2_1.y)
			return vPoint1_1.y > vPoint2_1.y;
		if (vPoint1_1.z != vPoint2_1.z)
			return vPoint1_1.z > vPoint2_1.z;
		// 2 - 2
		if (vPoint1_2.x != vPoint2_2.x)
			return vPoint1_2.x > vPoint2_2.x;
		if (vPoint1_2.y != vPoint2_2.y)
			return vPoint1_2.y > vPoint2_2.y;
		if (vPoint1_2.z != vPoint2_2.z)
			return vPoint1_2.z > vPoint2_2.z;

		return false;
	}
};

typedef Hpl1::Std::set<cTriEdge, cEdgeCompare> tTriEdgeListMap;
typedef tTriEdgeListMap::iterator tTriEdgeListMapIt;

//////////////////////////////////////////////////////

static void CheckEdgeSwitch(cTriEdge *apEdge) {
	cVector3f vPoint1 = GetVector3(gpVertexArray, apEdge->point1, glVertexStride);
	cVector3f vPoint2 = GetVector3(gpVertexArray, apEdge->point2, glVertexStride);
	if (vPoint1 < vPoint2) {
		unsigned int lTemp = apEdge->point1;
		apEdge->point1 = apEdge->point2;
		apEdge->point2 = lTemp;
	}
}

//////////////////////////////////////////////////////

void AddEdgeToMap(cTriEdge &aEdge, tTriEdgeListMap &aMap) {
	tTriEdgeListMapIt it = aMap.find(aEdge);
	// if edge is not added, create a new element.
	if (it == aMap.end()) {
		aEdge.tri2 = -1;
		aMap.insert(aEdge);
	}
	// if added check if there already exist an edge with the triangles
	else {
		if (it->tri2 != -1)
			return;
		if (it->tri1 == aEdge.tri1)
			return;

		// Set the other triangle! voila our edge is done!
		it->tri2 = aEdge.tri1;
	}
}

//////////////////////////////////////////////////////

bool cMath::CreateEdges(tTriEdgeVec &avEdges,
						const unsigned int *apIndexArray, int alIndexNum,
						const float *apVertexArray, int alVtxStride, int alVertexNum,
						bool *apIsDoubleSided) {
	const bool bLog = false;

	// Initial setup
	*apIsDoubleSided = false;

	tVtxIdxMap mapVtxIndices;
	tTriEdgeListMap mapTriEdgeLists;

	gpIndexArray = apIndexArray;
	gpVertexArray = apVertexArray;
	glVertexStride = alVtxStride;

	////////////////////////////////////////////////
	// Iterate indices and add all that reference the same vertex to
	//  the same element in a map
	for (int idx = 0; idx < alIndexNum; idx++) {
		cVector3f vVtx = GetVector3(apVertexArray, apIndexArray[idx], alVtxStride);
		if (bLog)
			Log("Checking idx: %d with vec: %s, ", idx, vVtx.ToString().c_str());

		tVtxIdxMapIt it = mapVtxIndices.find(vVtx);
		// If vertex already exist just add the index
		if (it != mapVtxIndices.end()) {
			if (bLog)
				Log("Allready added, appending.!\n");
			it->second.mlstIndices.push_back(idx);
		}
		// if vertex is not added create new vertex and add.
		else {
			if (bLog)
				Log("Adding as new!\n");
			mapVtxIndices.insert(tVtxIdxMap::value_type(vVtx, cVertexIndices(idx)));
		}
	}
	if (bLog) {
		Log("FINAL VTX MAP:\n");
		tVtxIdxMapIt VtxIt = mapVtxIndices.begin();
		for (; VtxIt != mapVtxIndices.end(); ++VtxIt) {
			const cVector3f &vVtx = VtxIt->first;
			const cVertexIndices &Data = VtxIt->second;
			Log("Vtx: %s Idx: ", vVtx.ToString().c_str());
			for (tUIntList::const_iterator it = Data.mlstIndices.begin(); it != Data.mlstIndices.end(); ++it) {
				Log("%d, ", *it);
			}
			Log("\n");
		}
	}

	////////////////////////////////////////////////
	// Iterate vertices and check what indices belong to it.
	// create a edges from these.
	// TODO: iterate the amp here instead.
	tVtxIdxMapIt VtxIt = mapVtxIndices.begin();
	for (; VtxIt != mapVtxIndices.end(); ++VtxIt) {
		cVertexIndices &Data = VtxIt->second;

		// Iterate the indices and create edges.
		tUIntListIt it = Data.mlstIndices.begin();
		for (; it != Data.mlstIndices.end(); ++it) {
			int lTriIdx = ((*it) / 3) * 3;
			unsigned int lVtx = apIndexArray[*it]; // The num of the vertex this index reference to.

			// Create edges
			cTriEdge edge1, edge2;
			edge1.point1 = lVtx;
			edge2.point1 = lVtx;
			edge1.tri1 = lTriIdx / 3;
			edge2.tri1 = lTriIdx / 3;

			// Get the index the vertex has in the tri (0 -2)
			int lIdxInTri = (*it) % 3;
			// Log("Idx in tri: %d\n",lIdxInTri);

			// Get the end points of the edge.
			int lPoint1 = lIdxInTri + 1;
			if (lPoint1 > 2)
				lPoint1 = 0;
			int lPoint2 = lIdxInTri - 1;
			if (lPoint2 < 0)
				lPoint2 = 2;

			// Set the end points.
			edge1.point2 = apIndexArray[lTriIdx + lPoint1];
			edge2.point2 = apIndexArray[lTriIdx + lPoint2];

			// Switch points if they 1 < 2
			CheckEdgeSwitch(&edge1);
			CheckEdgeSwitch(&edge2);

			AddEdgeToMap(edge1, mapTriEdgeLists);
			AddEdgeToMap(edge2, mapTriEdgeLists);
		}
	}
	if (bLog) {
		Log("FINAL EDGE MAP:\n");
		tTriEdgeListMapIt EdgeIt = mapTriEdgeLists.begin();
		for (; EdgeIt != mapTriEdgeLists.end(); ++EdgeIt) {
			cTriEdge &Edge = const_cast<cTriEdge &>(*EdgeIt);

			Log("P1: %d P2: %d Tri1: %d Tri2: %d\n", Edge.point1, Edge.point2, Edge.tri1, Edge.tri2);
		}
	}

	////////////////////////////////////////////////
	// Iterate edge map and check triangles it has, create edges from this.
	avEdges.reserve(mapTriEdgeLists.size());
	tTriEdgeListMapIt EdgeIt = mapTriEdgeLists.begin();
	for (; EdgeIt != mapTriEdgeLists.end(); ++EdgeIt) {
		cTriEdge &Edge = const_cast<cTriEdge &>(*EdgeIt);
		const unsigned int *pTri1 = &apIndexArray[Edge.tri1 * 3];

		if (Edge.tri2 == -1) {
			Edge.invert_tri2 = true;
			*apIsDoubleSided = true;
		} else {
			Edge.invert_tri2 = false;
		}

		// Get position of point1 in triangle
		int lPoint1InTri = 0;
		for (int i = 0; i < 3; i++) {
			if (Vector3Equal(apVertexArray, pTri1[i], apVertexArray, Edge.point1, alVtxStride)) {
				lPoint1InTri = i;
				break;
			}
		}
		// The next position in the triangle.
		int lNextInTri = lPoint1InTri + 1;
		if (lNextInTri >= 3)
			lNextInTri = 0;

		// Log("Point in: %d Next: %d\n",lPoint1InTri,lNextInTri);

		// If next point is NOT point 2, then the edge
		// must be switched.
		if (Vector3Equal(apVertexArray, pTri1[lNextInTri], apVertexArray, Edge.point2, alVtxStride)) {
			unsigned int lTemp = Edge.point1;
			Edge.point1 = Edge.point2;
			Edge.point2 = lTemp;
			// if(bLog)Log("Switching points\n");
		}

		// Add the final edge.
		avEdges.push_back(Edge);
	}

	return true;
}

/*bool cMath::CreateEdges(tTriEdgeVec &avEdges,
						const unsigned int* apIndexArray,int alIndexNum,
						const float* apVertexArray, int alVtxStride,int alVertexNum,
						bool *apIsDoubleSided)
{
	const bool bLog= false;

	//Setup
	*apIsDoubleSided = false;

	////////////////////////////////////////////////////////////////////////
	//For each vertex check what triangles reference it and make edges to the
	//vertices in that triangle.
	for(int vtx=0; vtx < alVertexNum; vtx++)
	{
		if(bLog)Log("Checking vtx: %d\n",vtx);
		int lCount=0; //The number of times the vertex is referred to.
		tUIntList mlstIndices;
		for(int idx=0; idx < alIndexNum; idx++)
		{
			if(Vector3Equal(apVertexArray,vtx, apVertexArray, apIndexArray[idx],alVtxStride))
			{
				lCount++;
				mlstIndices.push_back(idx);
			}
		}
		if(lCount==0){
			Warning("Found unreferenced vertex when building edges!\n");
		}
		else
		{
			///////////////////////////////////////////
			//Create the edges
			//Log("Vertex has %d references!\n",lCount);
			tUIntListIt it = mlstIndices.begin();
			for(; it != mlstIndices.end(); ++it)
			{
				//Get the triangle start index.
				int lTriIdx = ((*it)/3)*3;

				//if(bLog)Log("Tri index: %d!\n",lTriIdx);
				//Create edges
				cTriEdge edge1,edge2;
				edge1.point1 = vtx;
				edge2.point1 = vtx;
				edge1.tri1 = lTriIdx/3;
				edge2.tri1 = lTriIdx/3;

				//Get the index the vertex has in the tri (0 -2)
				int lIdxInTri = (*it) % 3;
				//Log("Idx in tri: %d\n",lIdxInTri);

				//Get the end points of the edge.
				int lPoint1 = lIdxInTri+1;
				if(lPoint1>2) lPoint1 =0;
				int lPoint2 = lIdxInTri-1;
				if(lPoint2<0) lPoint2 =2;

				//if(bLog)Log("P1: %d P2: %d!\n",apIndexArray[lTriIdx +lPoint1],
				//							apIndexArray[lTriIdx +lPoint2]);

				//Set the end points.
				edge1.point2 = apIndexArray[lTriIdx + lPoint1];
				edge2.point2 = apIndexArray[lTriIdx + lPoint2];

				avEdges.push_back(edge1);
				avEdges.push_back(edge2);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////
	//Check for triangles that share the same edge. Pair these.
	if(bLog)Log("Looking for shared edges on triangles.\n");
	tTriEdgeVec vTempEdges;

	//Check for the last element as well incase it has no pair..
	for(int e1 =0; e1 < (int)avEdges.size(); e1++)
	{
		bool bFound = false;
		cTriEdge edge1 = avEdges[e1];

		if(bLog)Log("Checking p(%d)-p(%d)|(%d)\n",
			edge1.point1,edge1.point2,
			edge1.tri1);

		for(int e2 =e1+1; e2 < (int)avEdges.size(); e2++)
		{
			cTriEdge edge2 = avEdges[e2];

			//if(bLog)Log("Checking %d to %d\n",e1,e2);

			//Check if the edges has different triangles but the same points.
			if(edge1.tri1 != edge2.tri1 && EdgePointEqual(apVertexArray,edge1,edge2,alVtxStride))
			{
				bFound = true;

				if(bLog)Log("Found a pair: p(%d)-p(%d)|(%d)\n",edge2.point1,edge2.point2,
													edge2.tri1);

				edge1.tri2 = edge2.tri1;
				edge1.invert_tri2 = false;

				break;
			}
		}

		//If no edge was found it is at a hole.
		if(bFound == false)
		{
			edge1.tri2 = edge1.tri1;
			edge1.invert_tri2 = true;

			if(bLog) Log("No pair found, at a hole\n");
		}

		bFound = false;

		//SLOW AS HELL THIS IS...
		for(size_t i=0; i< vTempEdges.size(); i++)
		{
			if(EdgeEqual(apVertexArray,vTempEdges[i],edge1,alVtxStride)){
				bFound = true;
				break;
			}

			//if it is the last edge, don't add it if there is another edge with
			//the same points and different triangle sides.
			if(EdgePointEqual(apVertexArray,vTempEdges[i],edge1,alVtxStride) &&
				vTempEdges[i].tri1 != vTempEdges[i].tri2)
			{
				bFound = true;
				break;
			}
		}

		//If not already added, add
		if(bFound== false){

			//If a face with inverted tri was added, the mesh is double sided.
			if(edge1.invert_tri2){
				*apIsDoubleSided = true;
			}

			vTempEdges.push_back(edge1);
			if(bLog)Log("Added!\n");
		}
		if(bLog)Log("------------\n");
	}

	////////////////////////////////////////////////////////////////////////
	//Clear the old list and add the new nice pairs.
	avEdges.clear();
	avEdges.reserve(vTempEdges.size());

	//Log("FINAL EDGES:\n");
	//Create the final edges. This means making sure so that
	//edge point 1 is before point 2 in triangle 1 and 2 point 2 before
	//1 in triangle 2.
	for(size_t edge=0; edge< vTempEdges.size(); edge++)
	{
		cTriEdge Edge = vTempEdges[edge];
		const unsigned int *pTri1 = &apIndexArray[Edge.tri1 * 3];
		const unsigned int *pTri2 = &apIndexArray[Edge.tri2 * 3];

		if(bLog)Log("Edge %d:\n",edge);

		if(bLog)Log("Tri1: 0:(%.2f %.2f %.2f) 1:(%.2f %.2f %.2f) 2:(%.2f %.2f %.2f)\n",
					apVertexArray[pTri1[0]*alVtxStride+0],apVertexArray[pTri1[0]*alVtxStride+1],apVertexArray[pTri1[0]*alVtxStride+2],
					apVertexArray[pTri1[1]*alVtxStride+0],apVertexArray[pTri1[1]*alVtxStride+1],apVertexArray[pTri1[1]*alVtxStride+2],
					apVertexArray[pTri1[2]*alVtxStride+0],apVertexArray[pTri1[2]*alVtxStride+1],apVertexArray[pTri1[2]*alVtxStride+2]);

		if(bLog)Log("Tri2: 0:(%.2f %.2f %.2f) 1:(%.2f %.2f %.2f) 2:(%.2f %.2f %.2f)\n",
					apVertexArray[pTri2[0]*alVtxStride+0],apVertexArray[pTri2[0]*alVtxStride+1],apVertexArray[pTri2[0]*alVtxStride+2],
					apVertexArray[pTri2[1]*alVtxStride+0],apVertexArray[pTri2[1]*alVtxStride+1],apVertexArray[pTri2[1]*alVtxStride+2],
					apVertexArray[pTri2[2]*alVtxStride+0],apVertexArray[pTri2[2]*alVtxStride+1],apVertexArray[pTri2[2]*alVtxStride+2]);

		if(bLog)Log("Point1: (%.2f %.2f %.2f) Point2: (%.2f %.2f %.2f)\n",
					apVertexArray[Edge.point1*alVtxStride+0],apVertexArray[Edge.point1*alVtxStride+1],apVertexArray[Edge.point1*alVtxStride+2],
					apVertexArray[Edge.point2*alVtxStride+0],apVertexArray[Edge.point2*alVtxStride+1],apVertexArray[Edge.point2*alVtxStride+2]);

		//Get position of point1 in triangle
		int lPoint1InTri=0;
		for(int i=0; i < 3; i++){
			if(Vector3Equal(apVertexArray,pTri1[i],apVertexArray, Edge.point1,alVtxStride))
			{
				lPoint1InTri = i;
				break;
			}
		}
		//The next position in the triangle.
		int lNextInTri = lPoint1InTri +1;
		if(lNextInTri >=3 ) lNextInTri =0;

		if(bLog)Log("Point in: %d Next: %d\n",lPoint1InTri,lNextInTri);

		//If next point is NOT point 2, then the edge
		//must be switched.
		if(Vector3Equal(apVertexArray,pTri1[ lNextInTri],apVertexArray,Edge.point2,alVtxStride))
		{
			unsigned int lTemp = Edge.point1;
			Edge.point1 = Edge.point2;
			Edge.point2 = lTemp;
			if(bLog)Log("Switching points\n");
		}

		if(bLog)Log("%d: p(%d)-p(%d)|(%d)-(%d)\n",edge,
			Edge.point1,Edge.point2,
			Edge.tri1, Edge.tri2);

		//Log("-----------\n");

		avEdges.push_back(Edge);
	}

	return true;
}*/

//-----------------------------------------------------------------------

} // namespace hpl
