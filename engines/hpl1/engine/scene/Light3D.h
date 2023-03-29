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

#ifndef HPL_LIGHT3D_H
#define HPL_LIGHT3D_H

#include "common/list.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/Renderable.h"
#include "hpl1/engine/scene/Entity3D.h"
#include "hpl1/engine/scene/Light.h"
#include "hpl1/std/set.h"

class TiXmlElement;

namespace hpl {

enum eLight3DType {
	eLight3DType_Point,
	eLight3DType_Spot,
	eLight3DType_LastEnum
};

enum eShadowVolumeType {
	eShadowVolumeType_None,
	eShadowVolumeType_ZPass,
	eShadowVolumeType_ZFail,
	eShadowVolumeType_LastEnum,
};

class iLowLevelGraphics;
class cRenderSettings;
class cCamera3D;
class cFrustum;
class iGpuProgram;
class iTexture;
class cTextureManager;
class cResources;
class cFileSearcher;
class cBillboard;
class cSectorVisibilityContainer;

typedef Hpl1::Std::set<iRenderable *> tCasterCacheSet;
typedef tCasterCacheSet::iterator tCasterCacheSetIt;

//------------------------------------------

kSaveData_ChildClass(iRenderable, iLight3D) {
	kSaveData_ClassInit(iLight3D) public : tString msFalloffMap;
	cContainerList<int> mlstBillboardIds;

	cColor mDiffuseColor;
	cColor mSpecularColor;
	float mfIntensity;
	float mfFarAttenuation;
	float mfNearAttenuation;
	float mfSourceRadius;

	bool mbCastShadows;
	bool mbAffectMaterial;

	cColor mColAdd;
	float mfRadiusAdd;
	cColor mDestCol;
	float mfDestRadius;
	float mfFadeTime;

	bool mbFlickering;
	tString msFlickerOffSound;
	tString msFlickerOnSound;
	tString msFlickerOffPS;
	tString msFlickerOnPS;
	float mfFlickerOnMinLength;
	float mfFlickerOffMinLength;
	float mfFlickerOnMaxLength;
	float mfFlickerOffMaxLength;
	cColor mFlickerOffColor;
	float mfFlickerOffRadius;
	bool mbFlickerFade;
	float mfFlickerOnFadeLength;
	float mfFlickerOffFadeLength;

	cColor mFlickerOnColor;
	float mfFlickerOnRadius;

	bool mbFlickerOn;
	float mfFlickerTime;
	float mfFlickerStateLength;
};

//------------------------------------------

class iLight3D : public iLight, public iRenderable {
	typedef iRenderable super;

public:
	iLight3D(tString asName, cResources *apResources);
	virtual ~iLight3D();

	void UpdateLogic(float afTimeStep);

	virtual void SetFarAttenuation(float afX);
	virtual void SetNearAttenuation(float afX);

	cVector3f GetLightPosition();

	virtual bool BeginDraw(cRenderSettings *apRenderSettings, iLowLevelGraphics *apLowLevelGraphics);
	virtual void EndDraw(cRenderSettings *apRenderSettings, iLowLevelGraphics *apLowLevelGraphics);

	bool CheckObjectIntersection(iRenderable *apObject);

	void AddShadowCaster(iRenderable *apObject, cFrustum *apFrustum, bool abStatic, cRenderList *apRenderList);
	bool HasStaticCasters();
	void ClearCasters(bool abClearStatic);

	void SetAllStaticCastersAdded(bool abX) { mbStaticCasterAdded = abX; }
	bool AllStaticCastersAdded() { return mbStaticCasterAdded; }

	eLight3DType GetLightType() { return mLightType; }

	// iEntity implementation
	tString GetEntityType() { return "iLight3D"; }

	bool IsVisible();
	void SetVisible(bool abVisible);

	iTexture *GetFalloffMap();
	void SetFalloffMap(iTexture *apTexture);

	// Renderable implementation:
	iMaterial *GetMaterial() { return NULL; }
	iVertexBuffer *GetVertexBuffer() { return NULL; }

	bool IsShadowCaster() { return false; }
	eRenderableType GetRenderType() { return eRenderableType_Light; }

	cBoundingVolume *GetBoundingVolume();

	int GetMatrixUpdateCount() { return GetTransformUpdateCount(); }

	cMatrixf *GetModelMatrix(cCamera3D *apCamera);

	inline void RenderShadow(iRenderable *apObject, cRenderSettings *apRenderSettings, iLowLevelGraphics *apLowLevelGraphics);

	void LoadXMLProperties(const tString asFile);

	void AttachBillboard(cBillboard *apBillboard);
	void RemoveBillboard(cBillboard *apBillboard);
	Common::Array<cBillboard *> *GetBillboardVec() { return &mvBillboards; }

	inline iTexture *GetTempTexture(size_t alIdx) { return mvTempTextures[alIdx]; }
	inline void SetTempTexture(size_t alIdx, iTexture *apTex) { mvTempTextures[alIdx] = apTex; }

	void SetOnlyAffectInSector(bool abX) { mbOnlyAffectInInSector = abX; }
	bool GetOnlyAffectInSector() { return mbOnlyAffectInInSector; }

	// SaveObject implementation
	virtual iSaveData *CreateSaveData();
	virtual void SaveToSaveData(iSaveData *apSaveData);
	virtual void LoadFromSaveData(iSaveData *apSaveData);
	virtual void SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame);

protected:
	void OnFlickerOff();
	void OnFlickerOn();
	void OnSetDiffuse();

	virtual cSectorVisibilityContainer *CreateSectorVisibility() = 0;
	virtual void ExtraXMLProperties(TiXmlElement *apMainElem) {}
	virtual void UpdateBoundingVolume() = 0;
	virtual bool CreateClipRect(cRect2l &aCliprect, cRenderSettings *apRenderSettings, iLowLevelGraphics *apLowLevelGraphics) = 0;

	eLight3DType mLightType;

	cTextureManager *mpTextureManager;
	cFileSearcher *mpFileSearcher;

	iTexture *mpFalloffMap;

	iTexture *mvTempTextures[3];

	Common::Array<cBillboard *> mvBillboards;

	cMatrixf mtxTemp;

	tCasterCacheSet m_setStaticCasters;
	tCasterCacheSet m_setDynamicCasters;

	bool mbStaticCasterAdded;

	bool mbOnlyAffectInInSector;

	int mlSectorVisibilityCount;
	cSectorVisibilityContainer *mpVisSectorCont;

	unsigned int *mpIndexArray;
};

typedef Common::List<iLight3D *> tLight3DList;
typedef tLight3DList::iterator tLight3DListIt;

} // namespace hpl

#endif // HPL_LIGHT3D_H
