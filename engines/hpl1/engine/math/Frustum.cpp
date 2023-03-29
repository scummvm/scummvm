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

#include "hpl1/engine/math/Frustum.h"

#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cFrustum::cFrustum() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cFrustum::SetViewProjMatrix(const cMatrixf &a_mtxProj, const cMatrixf &a_mtxView,
								 float afFarPlane, float afNearPlane, float afFOV, float afAspect,
								 const cVector3f &avOrigin, bool abInfFarPlane) {
	m_mtxViewProj = cMath::MatrixMul(a_mtxProj, a_mtxView);
	m_mtxModelView = a_mtxView;

	mfFarPlane = afFarPlane;
	mfNearPlane = afNearPlane;
	mfFOV = afFOV;
	mfAspect = afAspect;

	mvOrigin = avOrigin;

	mbInfFarPlane = abInfFarPlane;

	// This could be made more accurate.
	mOriginBV.SetSize(afNearPlane * 2);
	mOriginBV.SetPosition(mvOrigin);

	UpdatePlanes();
	UpdateSphere();
	UpdateEndPoints();
	UpdateBV();
}

//-----------------------------------------------------------------------

cPlanef cFrustum::GetPlane(eFrustumPlane aType) {
	return mPlane[aType];
}

//-----------------------------------------------------------------------

eFrustumCollision cFrustum::CollideBoundingVolume(cBoundingVolume *aBV) {
	// Check if the BV is in the Frustum sphere.
	if (CollideFustrumSphere(aBV) == eFrustumCollision_Outside) {
		return eFrustumCollision_Outside;
	}

	// Do a simple sphere collide test
	eFrustumCollision ret = CollideBVSphere(aBV);

	// If there was an intersection, collide with the AABB
	if (ret == eFrustumCollision_Intersect) {
		return CollideBVAABB(aBV);
	}

	return ret;
}

//-----------------------------------------------------------------------

bool cFrustum::CheckLineIntersection(const cVector3f &avPoint1, const cVector3f &avPoint2) {
	return cMath::CheckFrustumLineIntersection(&mPlane[0], avPoint1, avPoint2, 3);
}

//-----------------------------------------------------------------------

bool cFrustum::CheckQuadMeshIntersection(tVector3fVec *apPoints) {
	return cMath::CheckFrustumQuadMeshIntersection(&mPlane[0], apPoints, 3);
}

//-----------------------------------------------------------------------

bool cFrustum::CheckVolumeIntersection(cShadowVolumeBV *apVolume) {
	if (CheckQuadMeshIntersection(&apVolume->mvPoints)) {
		return true;
	}

	// The first shadow plane is the near plane which we can skip.
	int lPairNum = (apVolume->mlPlaneCount - 1) / 2;

	// Check with volume planes.
	for (int i = 0; i < 4; ++i) {
		if (cMath::CheckFrustumLineIntersection(&apVolume->mvPlanes[1], mvOrigin, mvEndPoints[i],
												lPairNum)) {
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

eFrustumCollision cFrustum::CollideFustrumSphere(cBoundingVolume *aBV) {
	float fRadiusSum = mBoundingSphere.r + aBV->GetRadius();
	cVector3f vSepAxis = mBoundingSphere.center - aBV->GetWorldCenter();
	if (vSepAxis.SqrLength() > (fRadiusSum * fRadiusSum)) {
		return eFrustumCollision_Outside;
	}

	return eFrustumCollision_Intersect;
}

//-----------------------------------------------------------------------

eFrustumCollision cFrustum::CollideBVSphere(cBoundingVolume *aBV) {
	int lPlanes = 6;
	if (mbInfFarPlane)
		lPlanes = 5;

	for (int i = 0; i < lPlanes; i++) {
		float fDist = cMath::PlaneToPointDist(mPlane[i], aBV->GetWorldCenter());

		if (fDist < -aBV->GetRadius()) {
			return eFrustumCollision_Outside;
		}

		if (ABS(fDist) < aBV->GetRadius()) {
			return eFrustumCollision_Intersect;
		}
	}

	return eFrustumCollision_Inside;
}
//-----------------------------------------------------------------------

eFrustumCollision cFrustum::CollideBVAABB(cBoundingVolume *aBV) {
	cVector3f vMax = aBV->GetMax();
	cVector3f vMin = aBV->GetMin();

	// Get the corners from the AAB
	cVector3f vCorners[9] = {
		cVector3f(vMax.x, vMax.y, vMax.z),
		cVector3f(vMax.x, vMax.y, vMin.z),
		cVector3f(vMax.x, vMin.y, vMax.z),
		cVector3f(vMax.x, vMin.y, vMin.z),

		cVector3f(vMin.x, vMax.y, vMax.z),
		cVector3f(vMin.x, vMax.y, vMin.z),
		cVector3f(vMin.x, vMin.y, vMax.z),
		cVector3f(vMin.x, vMin.y, vMin.z),

		// The "fuling", add center as well...
		aBV->GetPosition()};

	int lTotalIn = 0;

	int lPlanes = 6;
	if (mbInfFarPlane)
		lPlanes = 5;

	// Go through all the planes
	for (int i = 0; i < lPlanes; i++) {
		int lInCount = 9;
		bool bIsIn = true;

		for (int j = 0; j < 9; j++) {
			float fDist = cMath::PlaneToPointDist(mPlane[i], vCorners[j]);
			if (fDist < 0) {
				lInCount--;
				bIsIn = false;
			}
		}

		if (lInCount == 0)
			return eFrustumCollision_Outside;
		if (bIsIn)
			lTotalIn++;
	}

	if (lTotalIn == lPlanes)
		return eFrustumCollision_Inside;

	return eFrustumCollision_Intersect;
}

//-----------------------------------------------------------------------

void cFrustum::UpdateSphere() {
	// calculate the radius of the frustum sphere
	float fViewLen = mfFarPlane - mfNearPlane;

	float fHeight = fViewLen * tan(mfFOV * 0.5f);
	float fWidth = fHeight * mfAspect;

	// halfway point between near/far planes starting at the origin and extending along the z axis
	cVector3f P(0.0f, 0.0f, mfNearPlane + fViewLen * 0.5f);

	// the calculate far corner of the frustum
	cVector3f Q(fWidth, fHeight, fViewLen);

	// the vector between P and Q
	cVector3f vDiff = P - Q;

	// the radius becomes the length of this vector
	float fRadius = vDiff.Length();

	// get the look vector of the camera from the view matrix
	cVector3f vLookVector = m_mtxModelView.GetForward() * -1;

	// calculate the center of the sphere
	cVector3f vCenter = (mvOrigin) + (vLookVector * (fViewLen * 0.5f + mfNearPlane));

	mBoundingSphere = cSpheref(vCenter, fRadius);
}

//-----------------------------------------------------------------------

void cFrustum::UpdatePlanes() {
	// Left
	mPlane[eFrustumPlane_Left] = cPlanef(m_mtxViewProj.m[3][0] + m_mtxViewProj.m[0][0],
										 m_mtxViewProj.m[3][1] + m_mtxViewProj.m[0][1],
										 m_mtxViewProj.m[3][2] + m_mtxViewProj.m[0][2],
										 m_mtxViewProj.m[3][3] + m_mtxViewProj.m[0][3]);

	// Right
	mPlane[eFrustumPlane_Right] = cPlanef(m_mtxViewProj.m[3][0] - m_mtxViewProj.m[0][0],
										  m_mtxViewProj.m[3][1] - m_mtxViewProj.m[0][1],
										  m_mtxViewProj.m[3][2] - m_mtxViewProj.m[0][2],
										  m_mtxViewProj.m[3][3] - m_mtxViewProj.m[0][3]);

	// Bottom
	mPlane[eFrustumPlane_Bottom] = cPlanef(m_mtxViewProj.m[3][0] + m_mtxViewProj.m[1][0],
										   m_mtxViewProj.m[3][1] + m_mtxViewProj.m[1][1],
										   m_mtxViewProj.m[3][2] + m_mtxViewProj.m[1][2],
										   m_mtxViewProj.m[3][3] + m_mtxViewProj.m[1][3]);

	// Top
	mPlane[eFrustumPlane_Top] = cPlanef(m_mtxViewProj.m[3][0] - m_mtxViewProj.m[1][0],
										m_mtxViewProj.m[3][1] - m_mtxViewProj.m[1][1],
										m_mtxViewProj.m[3][2] - m_mtxViewProj.m[1][2],
										m_mtxViewProj.m[3][3] - m_mtxViewProj.m[1][3]);

	// Near
	mPlane[eFrustumPlane_Near] = cPlanef(m_mtxViewProj.m[3][0] + m_mtxViewProj.m[2][0],
										 m_mtxViewProj.m[3][1] + m_mtxViewProj.m[2][1],
										 m_mtxViewProj.m[3][2] + m_mtxViewProj.m[2][2],
										 m_mtxViewProj.m[3][3] + m_mtxViewProj.m[2][3]);
	// Far
	mPlane[eFrustumPlane_Far] = cPlanef(m_mtxViewProj.m[3][0] - m_mtxViewProj.m[2][0],
										m_mtxViewProj.m[3][1] - m_mtxViewProj.m[2][1],
										m_mtxViewProj.m[3][2] - m_mtxViewProj.m[2][2],
										m_mtxViewProj.m[3][3] - m_mtxViewProj.m[2][3]);

	for (int i = 0; i < 6; i++) {
		mPlane[i].Normalise();
		mPlane[i].CalcNormal();
	}
}

//-----------------------------------------------------------------------

void cFrustum::UpdateEndPoints() {
	float fXAngle = mfFOV * 0.5f;
	float fYAngle = mfFOV * 0.5f * mfAspect;

	cVector3f vForward = m_mtxModelView.GetForward();
	// cVector3f vUp = m_mtxModelView.GetUp();
	cVector3f vRight = m_mtxModelView.GetRight();

	// Point the forward vec in different dirs.
	cVector3f vUpDir = cMath::MatrixMul(cMath::MatrixQuaternion(cQuaternion(fXAngle, vRight)), vForward);
	cVector3f vDownDir = cMath::MatrixMul(cMath::MatrixQuaternion(cQuaternion(-fXAngle, vRight)), vForward);

	// cVector3f vRightDir = cMath::MatrixMul(cMath::MatrixQuaternion(cQuaternion(fYAngle,vUp)), vForward);
	// cVector3f vLeftDir = cMath::MatrixMul(cMath::MatrixQuaternion(cQuaternion(-fYAngle,vUp)), vForward);

	vForward = vForward * -1;

	float fRightAdd = sin(fYAngle);
	// float fUpAdd = sin(fXAngle);

	cVector3f vVec0 = vUpDir + vRight * fRightAdd;
	cVector3f vVec2 = vDownDir + vRight * fRightAdd;
	cVector3f vVec3 = vDownDir + vRight * -fRightAdd;
	cVector3f vVec1 = vUpDir + vRight * -fRightAdd;

	/*cVector3f vVec0 = vUpDir;//cMath::Vector3Normalize(vUpDir+vRightDir);
	cVector3f vVec1 = vRightDir;//cMath::Vector3Normalize(vUpDir+vLeftDir);
	cVector3f vVec2 = vDownDir;//cMath::Vector3Normalize(vDownDir+vLeftDir);
	cVector3f vVec3 = vLeftDir;//cMath::Vector3Normalize(vDownDir+vRightDir);*/

	// angles between forward and the vectors
	float fAngle0 = cMath::Vector3Angle(vVec0, vForward);
	float fAngle1 = cMath::Vector3Angle(vVec1, vForward);
	float fAngle2 = cMath::Vector3Angle(vVec2, vForward);
	float fAngle3 = cMath::Vector3Angle(vVec3, vForward);

	// create end points.
	mvEndPoints[0] = mvOrigin + vVec0 * (mfFarPlane / cos(fAngle0));
	mvEndPoints[1] = mvOrigin + vVec1 * (mfFarPlane / cos(fAngle1));
	mvEndPoints[2] = mvOrigin + vVec2 * (mfFarPlane / cos(fAngle2));
	mvEndPoints[3] = mvOrigin + vVec3 * (mfFarPlane / cos(fAngle3));

	/*mvEndPoints[0] = mvOrigin + (vUpDir+vRightDir)	 * mfFarPlane *-1;
	mvEndPoints[1] = mvOrigin + (vUpDir+vLeftDir)	 * mfFarPlane*-1;
	mvEndPoints[2] = mvOrigin + (vDownDir+vRightDir)* mfFarPlane*-1;
	mvEndPoints[3] = mvOrigin + (vDownDir+vLeftDir) * mfFarPlane*-1;*/
}

//-----------------------------------------------------------------------

void cFrustum::UpdateBV() {
	cVector3f vMin = mvOrigin;
	cVector3f vMax = mvOrigin;

	for (int i = 0; i < 4; i++) {
		if (vMax.x < mvEndPoints[i].x)
			vMax.x = mvEndPoints[i].x;
		else if (vMin.x > mvEndPoints[i].x)
			vMin.x = mvEndPoints[i].x;

		if (vMax.y < mvEndPoints[i].y)
			vMax.y = mvEndPoints[i].y;
		else if (vMin.y > mvEndPoints[i].y)
			vMin.y = mvEndPoints[i].y;

		if (vMax.z < mvEndPoints[i].z)
			vMax.z = mvEndPoints[i].z;
		else if (vMin.z > mvEndPoints[i].z)
			vMin.z = mvEndPoints[i].z;
	}

	mBoundingVolume.SetLocalMinMax(vMin, vMax);
}

//-----------------------------------------------------------------------

const cVector3f &cFrustum::GetOrigin() {
	return mvOrigin;
}

//-----------------------------------------------------------------------

cBoundingVolume *cFrustum::GetOriginBV() {
	return &mOriginBV;
}

//-----------------------------------------------------------------------

cVector3f cFrustum::GetForward() {
	return m_mtxModelView.GetForward();
}
//-----------------------------------------------------------------------

void cFrustum::Draw(iLowLevelGraphics *apLowLevelGraphics) {
	apLowLevelGraphics->DrawLine(mvOrigin, mvEndPoints[0], cColor(1, 1, 1, 1));
	apLowLevelGraphics->DrawLine(mvOrigin, mvEndPoints[1], cColor(1, 1, 1, 1));
	apLowLevelGraphics->DrawLine(mvOrigin, mvEndPoints[2], cColor(1, 1, 1, 1));
	apLowLevelGraphics->DrawLine(mvOrigin, mvEndPoints[3], cColor(1, 1, 1, 1));

	apLowLevelGraphics->DrawLine(mvEndPoints[0], mvEndPoints[1], cColor(1, 1, 1, 1));
	apLowLevelGraphics->DrawLine(mvEndPoints[1], mvEndPoints[2], cColor(1, 1, 1, 1));
	apLowLevelGraphics->DrawLine(mvEndPoints[2], mvEndPoints[3], cColor(1, 1, 1, 1));
	apLowLevelGraphics->DrawLine(mvEndPoints[3], mvEndPoints[0], cColor(1, 1, 1, 1));

	apLowLevelGraphics->DrawLine(mvEndPoints[0], mvEndPoints[2], cColor(1, 1, 1, 1));
	apLowLevelGraphics->DrawLine(mvEndPoints[1], mvEndPoints[3], cColor(1, 1, 1, 1));
}

//-----------------------------------------------------------------------
} // namespace hpl
