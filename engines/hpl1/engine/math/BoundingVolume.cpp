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

#include "hpl1/engine/math/BoundingVolume.h"

#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/math/Math.h"

namespace hpl {

static constexpr cTriEdge kvBVEdges[12] = {
	cTriEdge(1, 0, 0, 2),
	cTriEdge(3, 1, 0, 5),
	cTriEdge(2, 3, 0, 3),
	cTriEdge(0, 2, 0, 4),

	cTriEdge(0, 4, 4, 2),
	cTriEdge(4, 6, 4, 1),
	cTriEdge(6, 2, 4, 3),
	cTriEdge(4, 5, 1, 2),

	cTriEdge(5, 7, 1, 5),
	cTriEdge(7, 6, 1, 3),
	cTriEdge(1, 5, 2, 5),
	cTriEdge(3, 7, 5, 3)};

static constexpr cVector3f globalNormals[6] = {
	cVector3f(1, 0, 0),
	cVector3f(-1, 0, 0),

	cVector3f(0, 1, 0),
	cVector3f(0, -1, 0),

	cVector3f(0, 0, 1),
	cVector3f(0, 0, -1)};

static bool globalfacingLight[6] = {false, false, false, false, false, false};

static const int kvFacePoints[6] = {0, 5, 5, 6, 4, 7};

//////////////////////////////////////////////////////////////////////////
// SHADOW VOLUME
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cShadowVolumeBV::CollideBoundingVolume(cBoundingVolume *aBV) {
	// Do a simple sphere collide test
	if (CollideBVSphere(aBV) == false)
		return false;

	return CollideBVAABB(aBV);
}

//-----------------------------------------------------------------------

bool cShadowVolumeBV::CollideBVSphere(cBoundingVolume *aBV) {
	for (int i = 0; i < mlPlaneCount; ++i) {
		float fDist = cMath::PlaneToPointDist(mvPlanes[i], aBV->GetWorldCenter());

		if (fDist < -aBV->GetRadius()) {
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------

bool cShadowVolumeBV::CollideBVAABB(cBoundingVolume *aBV) {
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

	// Go through all the planes
	for (int i = 0; i < mlPlaneCount; i++) {
		int lInCount = 9;
		// bool bIsIn = true;

		for (int j = 0; j < 9; j++) {
			float fDist = cMath::PlaneToPointDist(mvPlanes[i], vCorners[j]);
			if (fDist < 0) {
				lInCount--;
				// bIsIn = false;
			}
		}

		if (lInCount == 0)
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cBoundingVolume::cBoundingVolume() {
	m_mtxTransform = cMatrixf::Identity;

	mvLocalMax = 0;
	mvLocalMin = 0;

	mvPosition = 0;
	mvPivot = 0;
	mvSize = 0;
	mfRadius = 0;

	mbPositionUpdated = true;
	mbSizeUpdated = true;

	mShadowVolume.mvPoints.reserve(8 * 4);
	mbShadowPlanesNeedUpdate = true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cVector3f cBoundingVolume::GetMax() {
	UpdateSize();
	return mvWorldMax;
}

cVector3f cBoundingVolume::GetMin() {
	UpdateSize();
	return mvWorldMin;
}

//-----------------------------------------------------------------------

cVector3f cBoundingVolume::GetLocalMax() {
	return mvLocalMax;
}

cVector3f cBoundingVolume::GetLocalMin() {
	return mvLocalMin;
}

//-----------------------------------------------------------------------

void cBoundingVolume::SetLocalMinMax(const cVector3f &avMin, const cVector3f &avMax) {
	mvLocalMax = avMax;
	mvLocalMin = avMin;

	mbSizeUpdated = true;
}

//-----------------------------------------------------------------------

cVector3f cBoundingVolume::GetLocalCenter() {
	return mvPivot;
}

//-----------------------------------------------------------------------

cVector3f cBoundingVolume::GetWorldCenter() {
	UpdateSize();

	return m_mtxTransform.GetTranslation() + mvPivot;
}

//-----------------------------------------------------------------------

void cBoundingVolume::SetPosition(const cVector3f &avPos) {
	m_mtxTransform.SetTranslation(avPos);

	mbPositionUpdated = true;
}

//-----------------------------------------------------------------------

cVector3f cBoundingVolume::GetPosition() {
	return m_mtxTransform.GetTranslation();
}

//-----------------------------------------------------------------------

void cBoundingVolume::SetTransform(const cMatrixf &a_mtxTransform) {
	m_mtxTransform = a_mtxTransform;

	mbSizeUpdated = true;
}

const cMatrixf &cBoundingVolume::GetTransform() {
	return m_mtxTransform;
}

//-----------------------------------------------------------------------

void cBoundingVolume::SetSize(const cVector3f &avSize) {
	mvLocalMax = avSize * 0.5;
	mvLocalMin = avSize * -0.5;

	mbSizeUpdated = true;
}

//-----------------------------------------------------------------------

cVector3f cBoundingVolume::GetSize() {
	UpdateSize();

	return mvSize;
}

//-----------------------------------------------------------------------

float cBoundingVolume::GetRadius() {
	UpdateSize();

	return mfRadius;
}

//-----------------------------------------------------------------------

cShadowVolumeBV *cBoundingVolume::GetShadowVolume(const cVector3f &avLightPos,
												  float afLightRange, bool abForceUpdate) {
	if (cMath::PointBVCollision(avLightPos, *this))
		return NULL;

	if (!abForceUpdate && !mbShadowPlanesNeedUpdate)
		return &mShadowVolume;

	// Set size 0.
	mShadowVolume.mvPoints.resize(0);

	// Get the corners.
	cVector3f vMax = GetMax();
	cVector3f vMin = GetMin();
	cVector3f vCorners[8];
	vCorners[0] = cVector3f(vMax.x, vMax.y, vMax.z);
	vCorners[1] = cVector3f(vMax.x, vMax.y, vMin.z);
	vCorners[2] = cVector3f(vMax.x, vMin.y, vMax.z);
	vCorners[3] = cVector3f(vMax.x, vMin.y, vMin.z);

	vCorners[4] = cVector3f(vMin.x, vMax.y, vMax.z);
	vCorners[5] = cVector3f(vMin.x, vMax.y, vMin.z);
	vCorners[6] = cVector3f(vMin.x, vMin.y, vMax.z);
	vCorners[7] = cVector3f(vMin.x, vMin.y, vMin.z);

	/////////////////////////////////////////////////////////////////////
	// Iterate the faces and check which ones are facing the light.
	// int lNearPoint = -1;
	mShadowVolume.mlPlaneCount = 0;
	for (int face = 0; face < 6; face++) {
		globalfacingLight[face] = cMath::Vector3Dot(globalNormals[face],
													vCorners[kvFacePoints[face]] - avLightPos) < 0;

		// Get a point for the near plane. (any edge point will do)
		if (globalfacingLight[face]) {
			mShadowVolume.mvPlanes[mShadowVolume.mlPlaneCount] = cPlanef(
				globalNormals[face] * -1.0f, vCorners[kvFacePoints[face]]);
			mShadowVolume.mlPlaneCount++;
		}
	}

	mShadowVolume.mlCapPlanes = mShadowVolume.mlPlaneCount;

	// The direction a point is pushed away in
	cVector3f vDir;

	// The length to push the shadow points.
	float fPushLength = afLightRange * kSqrt2f;

	//////////////////////////////////////////////////////////
	// Iterate the edges and build quads from the silhouette
	for (int edge = 0; edge < 12; edge++) {
		const cTriEdge &Edge = kvBVEdges[edge];

		const bool facingLight1 = globalfacingLight[Edge.tri1];
		const bool facingLight2 = globalfacingLight[Edge.tri2];

		if ((facingLight1 && !facingLight2) || (facingLight2 && !facingLight1)) {
			if (facingLight1) {
				mShadowVolume.mvPoints.push_back(vCorners[Edge.point1]);
				mShadowVolume.mvPoints.push_back(vCorners[Edge.point2]);

				vDir = (vCorners[Edge.point2] - avLightPos);
				vDir.Normalise();
				mShadowVolume.mvPoints.push_back(vCorners[Edge.point2] + vDir * fPushLength);

				vDir = (vCorners[Edge.point1] - avLightPos);
				vDir.Normalise();
				mShadowVolume.mvPoints.push_back(vCorners[Edge.point1] + vDir * fPushLength);
			} else {
				mShadowVolume.mvPoints.push_back(vCorners[Edge.point2]);
				mShadowVolume.mvPoints.push_back(vCorners[Edge.point1]);

				vDir = (vCorners[Edge.point1] - avLightPos);
				vDir.Normalise();
				mShadowVolume.mvPoints.push_back(vCorners[Edge.point1] + vDir * fPushLength);

				vDir = (vCorners[Edge.point2] - avLightPos);
				vDir.Normalise();
				mShadowVolume.mvPoints.push_back(vCorners[Edge.point2] + vDir * fPushLength);
			}
		}
	}

	/////////////////////////////////////
	// Create the side planes:

	for (int i = 0; i < (int)mShadowVolume.mvPoints.size(); i += 4) {
		// Normal should point inwards
		cVector3f vNormal = cMath::Vector3Cross(
			mShadowVolume.mvPoints[i + 1] - mShadowVolume.mvPoints[i + 0],
			mShadowVolume.mvPoints[i + 2] - mShadowVolume.mvPoints[i + 0]);
		mShadowVolume.mvPlanes[mShadowVolume.mlPlaneCount].FromNormalPoint(vNormal,
																		   mShadowVolume.mvPoints[i + 0]);
		mShadowVolume.mvPlanes[mShadowVolume.mlPlaneCount].Normalise();

		mShadowVolume.mlPlaneCount++;
	}

	return &mShadowVolume;
}

//-----------------------------------------------------------------------

void cBoundingVolume::DrawEdges(const cVector3f &avLightPos, float afLightRange, iLowLevelGraphics *apLowLevelGraphics) {
	cShadowVolumeBV *pVolume = GetShadowVolume(avLightPos, afLightRange, false);

	apLowLevelGraphics->SetBlendActive(true);
	apLowLevelGraphics->SetBlendFunc(eBlendFunc_One, eBlendFunc_One);
	apLowLevelGraphics->SetDepthWriteActive(false);
	tVertexVec vVtx;
	vVtx.resize(4);

	for (int capplane = 0; capplane < mShadowVolume.mlCapPlanes; capplane++) {
		mShadowVolume.mvPlanes[capplane].CalcNormal();
		apLowLevelGraphics->DrawLine(GetWorldCenter(), GetWorldCenter() + mShadowVolume.mvPlanes[capplane].normal * -0.5f, cColor(1, 1, 1, 1));
	}

	int lPlane = mShadowVolume.mlCapPlanes;
	for (int quad = 0; quad < (int)pVolume->mvPoints.size(); quad += 4) {

		for (int i = 0; i < 4; i++)
			vVtx[i].pos = pVolume->mvPoints[quad + i];

		apLowLevelGraphics->DrawQuad(vVtx, cColor(0.2f, 0, 0.2f));

		cVector3f vCenter = (vVtx[1].pos + vVtx[0].pos) * 0.5f;
		mShadowVolume.mvPlanes[lPlane].CalcNormal();
		apLowLevelGraphics->DrawLine(vCenter, vCenter + mShadowVolume.mvPlanes[lPlane].normal * -0.5f, cColor(1, 1, 1, 1));
		lPlane++;
	}

	apLowLevelGraphics->SetBlendActive(false);
	apLowLevelGraphics->SetDepthWriteActive(true);
}

//-----------------------------------------------------------------------

void cBoundingVolume::AddArrayPoints(const float *apArray, int alNumOfVectors) {
	cBVTempArray temp;
	temp.mpArray = apArray;
	temp.mlSize = alNumOfVectors;

	mlstArrays.push_back(temp);
}

//-----------------------------------------------------------------------

void cBoundingVolume::CreateFromPoints(int alStride) {
	mvLocalMax = cVector3f(-100000, -100000, -100000);
	mvLocalMin = cVector3f(100000, 100000, 100000);

	for (tBVTempArrayListIt it = mlstArrays.begin(); it != mlstArrays.end(); it++) {
		// Loop through all the vectors and find min and max
		const float *apVec = it->mpArray;
		int lNumOfVectors = it->mlSize;
		while (lNumOfVectors) {
			// Min and max X
			if (apVec[0] < mvLocalMin.x)
				mvLocalMin.x = apVec[0];
			if (apVec[0] > mvLocalMax.x)
				mvLocalMax.x = apVec[0];

			// Min and max Y
			if (apVec[1] < mvLocalMin.y)
				mvLocalMin.y = apVec[1];
			if (apVec[1] > mvLocalMax.y)
				mvLocalMax.y = apVec[1];

			// Min and max Z
			if (apVec[2] < mvLocalMin.z)
				mvLocalMin.z = apVec[2];
			if (apVec[2] > mvLocalMax.z)
				mvLocalMax.z = apVec[2];

			apVec += alStride;
			lNumOfVectors--;
		}
	}
	mlstArrays.clear();

	// Update the used size
	mbPositionUpdated = true;
	mbSizeUpdated = true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cBoundingVolume::UpdateSize() {
	if (mbSizeUpdated) {
		cMatrixf mtxRot = m_mtxTransform.GetRotation();

		// Transform the local corners
		cVector3f vCorners[8];
		vCorners[0] = cMath::MatrixMul(mtxRot, cVector3f(mvLocalMax.x, mvLocalMax.y, mvLocalMax.z));
		vCorners[1] = cMath::MatrixMul(mtxRot, cVector3f(mvLocalMax.x, mvLocalMax.y, mvLocalMin.z));
		vCorners[2] = cMath::MatrixMul(mtxRot, cVector3f(mvLocalMax.x, mvLocalMin.y, mvLocalMax.z));
		vCorners[3] = cMath::MatrixMul(mtxRot, cVector3f(mvLocalMax.x, mvLocalMin.y, mvLocalMin.z));

		vCorners[4] = cMath::MatrixMul(mtxRot, cVector3f(mvLocalMin.x, mvLocalMax.y, mvLocalMax.z));
		vCorners[5] = cMath::MatrixMul(mtxRot, cVector3f(mvLocalMin.x, mvLocalMax.y, mvLocalMin.z));
		vCorners[6] = cMath::MatrixMul(mtxRot, cVector3f(mvLocalMin.x, mvLocalMin.y, mvLocalMax.z));
		vCorners[7] = cMath::MatrixMul(mtxRot, cVector3f(mvLocalMin.x, mvLocalMin.y, mvLocalMin.z));

		mvMax = vCorners[0];
		mvMin = vCorners[0];

		// Calculate the transformed min and max
		for (int i = 1; i < 8; i++) {
			// X
			if (vCorners[i].x < mvMin.x)
				mvMin.x = vCorners[i].x;
			else if (vCorners[i].x > mvMax.x)
				mvMax.x = vCorners[i].x;

			// Y
			if (vCorners[i].y < mvMin.y)
				mvMin.y = vCorners[i].y;
			else if (vCorners[i].y > mvMax.y)
				mvMax.y = vCorners[i].y;

			// Z
			if (vCorners[i].z < mvMin.z)
				mvMin.z = vCorners[i].z;
			else if (vCorners[i].z > mvMax.z)
				mvMax.z = vCorners[i].z;
		}

		// Get the transformed size.
		mvSize = mvMax - mvMin;

		// Get the local pivot (or offset from origo).
		mvPivot = mvMax - (mvSize * 0.5f);

		// Get radius as pivot to localmax
		mfRadius = cMath::Vector3Dist(mvPivot, mvMax);

		mbSizeUpdated = false;
		mbPositionUpdated = true;
	}

	if (mbPositionUpdated) {
		mvWorldMax = m_mtxTransform.GetTranslation() + mvMax;
		mvWorldMin = m_mtxTransform.GetTranslation() + mvMin;

		mbPositionUpdated = false;

		mbShadowPlanesNeedUpdate = true;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SERIALIZE CLASS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerializeBase(cBoundingVolume)
	kSerializeVar(m_mtxTransform, eSerializeType_Matrixf)
		kSerializeVar(mvLocalMax, eSerializeType_Vector3f)
			kSerializeVar(mvLocalMin, eSerializeType_Vector3f)
				kSerializeVar(mvMax, eSerializeType_Vector3f)
					kSerializeVar(mvMin, eSerializeType_Vector3f)
						kSerializeVar(mvPivot, eSerializeType_Vector3f)
							kSerializeVar(mvWorldMax, eSerializeType_Vector3f)
								kSerializeVar(mvWorldMin, eSerializeType_Vector3f)
									kSerializeVar(mvPosition, eSerializeType_Vector3f)
										kSerializeVar(mvSize, eSerializeType_Vector3f)
											kSerializeVar(mfRadius, eSerializeType_Float32)
												kEndSerialize()

//-----------------------------------------------------------------------

/* OLD "NEAR PLANE CODE"
int lNearPoint =-1;
for(int face=0; face< 6; face++)
{
gvFaces[face].facingLight = cMath::Vector3Dot(gvFaces[face].normal,
vCorners[kvFacePoints[face]] - avLightPos)<0;

//Get a point for the near plane. (any edge point will do)
if(gvFaces[face].facingLight && lNearPoint<0)
{
lNearPoint = kvFacePoints[face];
}
}

//Build the near plane for the shadow.
cVector3f vLightNormal = GetWorldCenter() - avLightPos;
mShadowVolume.mvPlanes[0] = cPlanef(vLightNormal,vCorners[lNearPoint]);
mShadowVolume.mvPlanes[0].Normalise();


//The direction a point is pushed away in
cVector3f vDir;

float fPushLength = afLightRange*kSqrt2f;

//The number of planes created.
mShadowVolume.mlPlaneCount =1;*/
} // namespace hpl
