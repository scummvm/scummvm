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

#ifndef HPL_RENDERABLE_H
#define HPL_RENDERABLE_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/scene/Entity3D.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class iMaterial;
class iVertexBuffer;

class cBoundingVolume;
class cFrustum;

class iLight3D;
class cCamera3D;

class cRenderList;

class iRenderable;

enum eRenderableType {
	eRenderableType_Normal,
	eRenderableType_ParticleSystem,
	eRenderableType_Mesh,
	eRenderableType_Light,
	eRenderableType_Dummy,
	eRenderableType_LastEnum
};

//------------------------------------------

kSaveData_ChildClass(iEntity3D, iRenderable) {
	kSerializableClassInit(cSaveData_iRenderable) public : bool mbStatic;
	bool mbRendered;
	float mfZ;
};

//------------------------------------------

class iRenderable : public iEntity3D {
	typedef iEntity3D super;

public:
	iRenderable(const tString &asName);

	bool IsRendered() { return mbRendered; }
	virtual void SetRendered(bool abX) { mbRendered = abX; }

	virtual iMaterial *GetMaterial() = 0;
	virtual iVertexBuffer *GetVertexBuffer() = 0;

	virtual bool IsShadowCaster() = 0;

	virtual bool CollidesWithBV(cBoundingVolume *apBV);
	virtual bool CollidesWithFrustum(cFrustum *apFrustum);

	virtual cMatrixf *GetModelMatrix(cCamera3D *apCamera) = 0;

	virtual eRenderableType GetRenderType() = 0;

	virtual void UpdateGraphics(cCamera3D *apCamera, float afFrameTime, cRenderList *apRenderList) {}

	/**
	 * Should return a different number each time the renderable model matrix is updated. never -1
	 * \return
	 */
	virtual int GetMatrixUpdateCount() = 0;

	float GetZ() { return mfZ; }
	void SetZ(float afZ) { mfZ = afZ; }

	cMatrixf *GetInvModelMatrix();

	inline void SetPrevMatrix(const cMatrixf &a_mtxPrev) { m_mtxPrevious = a_mtxPrev; }
	inline cMatrixf &GetPrevMatrix() { return m_mtxPrevious; }

	const cVector3f &GetCalcScale();

	void SetStatic(bool abX) { mbStatic = abX; }
	bool IsStatic() const { return mbStatic; }

	inline int GetRenderCount() const { return mlRenderCount; }
	inline void SetRenderCount(const int alCount) { mlRenderCount = alCount; }
	inline int GetPrevRenderCount() const { return mlPrevRenderCount; }
	inline void SetPrevRenderCount(const int alCount) { mlPrevRenderCount = alCount; }

	void SetForceShadow(bool abX) { mbForceShadow = abX; }
	bool GetForceShadow() { return mbForceShadow; }

	bool GetIsOneSided() { return mbIsOneSided; }
	const cVector3f &GetOneSidedNormal() { return mvOneSidedNormal; }

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

protected:
	cMatrixf m_mtxInvModel;
	cMatrixf m_mtxPrevious;

	bool mbIsOneSided;
	cVector3f mvOneSidedNormal;

	int mlLastMatrixCount;

	bool mbStatic;

	bool mbForceShadow;

	int mlRenderCount;
	int mlPrevRenderCount;

	int mlCalcScaleMatrixCount;
	cVector3f mvCalcScale;

	bool mbRendered;
	float mfZ;
};

} // namespace hpl

#endif // HPL_RENDERABLE_H
