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

#ifndef HPL_BILLBOARD_H
#define HPL_BILLBOARD_H

#include "RenderList.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/Renderable.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/scene/Entity3D.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

enum eBillboardType {
	eBillboardType_Point,
	eBillboardType_Axis,
	eBillboardType_LastEnum
};

class cMaterialManager;
class cResources;
class cGraphics;
class cMeshCreator;
class cFileSearcher;
class iLowLevelGraphics;
class iMaterial;
class iVertexBuffer;
class iOcclusionQuery;

//------------------------------------------

kSaveData_ChildClass(iRenderable, cBillboard) {
	kSaveData_ClassInit(cBillboard) public : tString msMaterial;

	int mType;
	cVector2f mvSize;
	cVector3f mvAxis;
	float mfForwardOffset;
	cColor mColor;
	float mfHaloAlpha;

	bool mbIsHalo;
	cVector3f mvHaloSourceSize;
	bool mbHaloSourceIsParent;

	virtual iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame);
	virtual int GetSaveCreatePrio();
};

//------------------------------------------

class cBillboard : public iRenderable {
	typedef iRenderable super;

public:
	cBillboard(const tString asName, const cVector2f &avSize, cResources *apResources,
			   cGraphics *apGraphics);
	~cBillboard();

	void SetMaterial(iMaterial *apMaterial);

	void SetSize(const cVector2f &avSize);
	cVector2f GetSize() { return mvSize; }

	void SetType(eBillboardType aType);
	eBillboardType GetType() { return mType; }

	void SetAxis(const cVector3f &avAxis);
	cVector3f GetAxis() { return mvAxis; }

	void SetForwardOffset(float afOffset);
	float GetForwardOffset() { return mfForwardOffset; }

	void SetColor(const cColor &aColor);
	const cColor &GetColor() { return mColor; }

	void SetHaloAlpha(float afX);
	float GetHaloAlpha() { return mfHaloAlpha; }

	void LoadXMLProperties(const tString asFile);

	/////////////////////////////////
	// Halo stuff
	void SetIsHalo(bool abX);
	bool IsHalo() { return mbIsHalo; }

	void SetHaloSourceSize(const cVector3f &avSize);
	cVector3f GetHaloSourceSize() { return mvHaloSourceSize; }

	void SetHaloSourceIsParent(bool abX);
	bool GetHaloSourceIsParent() { return mbHaloSourceIsParent; }

	iOcclusionQuery *GetQuery() { return mQueryObject.mpQuery; }
	iOcclusionQuery *GetMaxQuery() { return mMaxQueryObject.mpQuery; }

	/////////////////////////////////
	// Entity implementation
	tString GetEntityType() { return "Billboard"; }

	bool IsVisible();
	void SetVisible(bool abVisible) { SetRendered(abVisible); }

	// Renderable implementations
	iMaterial *GetMaterial() { return mpMaterial; }
	iVertexBuffer *GetVertexBuffer() { return mpVtxBuffer; }

	void UpdateGraphics(cCamera3D *apCamera, float afFrameTime, cRenderList *apRenderList);

	bool IsShadowCaster() { return false; }

	cBoundingVolume *GetBoundingVolume();

	cMatrixf *GetModelMatrix(cCamera3D *apCamera);

	int GetMatrixUpdateCount();

	eRenderableType GetRenderType() { return eRenderableType_ParticleSystem; }

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

private:
	eBillboardType ToType(const char *apString);

	void UpdateSourceBufferSize();

	cMaterialManager *mpMaterialManager;
	cFileSearcher *mpFileSearcher;
	iLowLevelGraphics *mpLowLevelGraphics;
	cMeshCreator *mpMeshCreator;

	iMaterial *mpMaterial;
	iVertexBuffer *mpVtxBuffer;

	iVertexBuffer *mpHaloSourceBuffer;

	cMatrixf m_mtxTempTransform;

	eBillboardType mType;
	cVector2f mvSize;
	cVector3f mvAxis;

	int mlLastRenderCount;

	bool mbIsHalo;
	cVector3f mvHaloSourceSize;
	bool mbHaloSourceIsParent;
	cOcclusionQueryObject mQueryObject;
	cOcclusionQueryObject mMaxQueryObject;

	float mfForwardOffset;
	cColor mColor;
	float mfHaloAlpha;
};

} // namespace hpl

#endif // HPL_BILLBOARD_H
