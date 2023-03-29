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

#ifndef HPL_BOUNDING_VOLUME_H
#define HPL_BOUNDING_VOLUME_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SerializeClass.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class iLowLevelGraphics;
class cBoundingVolume;

class cBVTempArray {
public:
	const float *mpArray;
	int mlSize;
};

enum eBVCollision {
	eBVCollision_Inside,
	eBVCollision_Outside,
	eBVCollision_Intersect,
	eBVCollision_LastEnum
};

typedef Common::List<cBVTempArray> tBVTempArrayList;
typedef tBVTempArrayList::iterator tBVTempArrayListIt;

class cShadowVolumeBV {
public:
	cPlanef mvPlanes[12];
	int mlPlaneCount;
	tVector3fVec mvPoints;
	int mlCapPlanes;

	bool CollideBoundingVolume(cBoundingVolume *aBV);

private:
	bool CollideBVSphere(cBoundingVolume *aBV);
	bool CollideBVAABB(cBoundingVolume *aBV);
};

class cBoundingVolume : public iSerializable {
	friend class cMath;
	kSerializableClassInit(cBoundingVolume) public : cBoundingVolume();

	cVector3f GetMax();
	cVector3f GetMin();

	cVector3f GetLocalMax();
	cVector3f GetLocalMin();

	void SetLocalMinMax(const cVector3f &mvMin, const cVector3f &mvMax);

	void SetPosition(const cVector3f &avPos);
	cVector3f GetPosition();

	void SetSize(const cVector3f &avSize);
	cVector3f GetSize();

	void SetTransform(const cMatrixf &a_mtxTransform);
	const cMatrixf &GetTransform();

	cVector3f GetLocalCenter();
	cVector3f GetWorldCenter();

	float GetRadius();

	void AddArrayPoints(const float *apArray, int alNumOfVectors);
	void CreateFromPoints(int alStride);

	cShadowVolumeBV *GetShadowVolume(const cVector3f &avLightPos, float afLightRange, bool abForceUpdate);

	// Debug:
	void DrawEdges(const cVector3f &avLightPos, float afLightRange, iLowLevelGraphics *apLowLevelGraphics);
	void UpdateSize();

	cMatrixf m_mtxTransform;

	cVector3f mvLocalMax;
	cVector3f mvLocalMin;

	cVector3f mvMax;
	cVector3f mvMin;

	cVector3f mvPivot;

	cVector3f mvWorldMax;
	cVector3f mvWorldMin;

	cVector3f mvPosition;
	cVector3f mvSize;
	float mfRadius;

private:
	bool mbPositionUpdated;
	bool mbSizeUpdated;

	tBVTempArrayList mlstArrays;

	cShadowVolumeBV mShadowVolume;
	bool mbShadowPlanesNeedUpdate;
};

} // namespace hpl

#endif // HPL_BOUNDING_VOLUME_H
