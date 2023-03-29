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

#ifndef HPL_FRUSTUM_H
#define HPL_FRUSTUM_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/BoundingVolume.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class iLowLevelGraphics;

enum eFrustumPlane {
	eFrustumPlane_Left = 0,
	eFrustumPlane_Right = 1,
	eFrustumPlane_Bottom = 2,
	eFrustumPlane_Top = 3,
	eFrustumPlane_Near = 4,
	eFrustumPlane_Far = 5,
	eFrustumPlane_LastEnum = 7,
};

enum eFrustumCollision {
	eFrustumCollision_Inside,
	eFrustumCollision_Outside,
	eFrustumCollision_Intersect,
	eFrustumCollision_LastEnum
};

class cFrustum {
public:
	cFrustum();

	void SetViewProjMatrix(const cMatrixf &a_mtxProj, const cMatrixf &a_mtxView,
						   float afFarPlane, float afNearPlane, float afFOV, float afAspect,
						   const cVector3f &avOrigin, bool abInfFarPlane = false);

	cPlanef GetPlane(eFrustumPlane aType);

	eFrustumCollision CollideBoundingVolume(cBoundingVolume *aBV);

	eFrustumCollision CollideFustrumSphere(cBoundingVolume *aBV);

	eFrustumCollision CollideBVSphere(cBoundingVolume *aBV);
	eFrustumCollision CollideBVAABB(cBoundingVolume *aBV);

	/**
	 * Checks intersection with a line
	 */
	bool CheckLineIntersection(const cVector3f &avPoint1, const cVector3f &avPoint2);

	/**
	 * Checks intersection with a quad mesh.
	 * \param apPoints the points of the quad mesh, every 4 points is a face.
	 */
	bool CheckQuadMeshIntersection(tVector3fVec *apPoints);

	bool CheckVolumeIntersection(cShadowVolumeBV *apVolume);

	cSpheref GetBoundingSphere() { return mBoundingSphere; }
	const cBoundingVolume &GetBoundingVolume() { return mBoundingVolume; }

	const cVector3f &GetOrigin();
	cBoundingVolume *GetOriginBV();

	cVector3f GetForward();

	void Draw(iLowLevelGraphics *apLowLevelGraphics);

private:
	void UpdatePlanes();
	void UpdateSphere();
	void UpdateEndPoints();
	void UpdateBV();

	float mfFarPlane;
	float mfNearPlane;
	float mfAspect;
	float mfFOV;

	bool mbInfFarPlane;

	cVector3f mvOrigin;
	cBoundingVolume mOriginBV;

	cMatrixf m_mtxViewProj;
	cMatrixf m_mtxModelView;
	cPlanef mPlane[6];
	cSpheref mBoundingSphere;
	cBoundingVolume mBoundingVolume;

	cVector3f mvEndPoints[4];
	tVector3fVec mvPoints;
};

} // namespace hpl

#endif // HPL_FRUSTUM_H
