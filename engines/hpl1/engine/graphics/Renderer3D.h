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

#ifndef HPL_RENDERER3D_H
#define HPL_RENDERER3D_H

#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/math/Frustum.h"
#include "hpl1/engine/math/MathTypes.h"

namespace hpl {

#define eRendererDebugFlag_DrawTangents (0x00000001)
#define eRendererDebugFlag_DrawNormals (0x00000002)
#define eRendererDebugFlag_DisableLighting (0x00000004)
#define eRendererDebugFlag_LogRendering (0x00000008)
#define eRendererDebugFlag_DrawBoundingBox (0x00000010)
#define eRendererDebugFlag_DrawBoundingSphere (0x00000020)
#define eRendererDebugFlag_DrawLightBoundingBox (0x00000040)
#define eRendererDebugFlag_RenderLines (0x00000080)

typedef tFlag tRendererDebugFlag;

class cResources;
class iLowLevelGraphics;
class LowLevelResources;
class iTexture;
class cCamera3D;
class cWorld3D;
class iVertexBuffer;
class cMeshCreator;
class cRenderList;
class iRenderable;
class iLight3D;
class cRendererPostEffects;
class cSector;

class cResources;

//---------------------------------------------

enum eRendererShowShadows {
	eRendererShowShadows_All,
	eRendererShowShadows_Static,
	eRendererShowShadows_None,
	eRendererShowShadows_LastEnum
};

//---------------------------------------------

class cRenderSettings {
public:
	cRenderSettings();

	void Clear();
	void Reset(iLowLevelGraphics *apLowLevel);

	// Setings that doesn't change:
	iGpuProgram *extrudeProgram;
	iLowLevelGraphics *mpLowLevel;

	unsigned int *mpTempIndexArray;

	// Setting that changes
	iLight3D *mpLight;
	cCamera3D *mpCamera;
	cFrustum *mpFrustum;

	cSector *mpSector;

	bool mbFogActive;
	bool mbFogCulling;
	float mfFogStart;
	float mfFogEnd;
	cColor mFogColor;

	eRendererShowShadows mShowShadows;
	bool mbLog;
	tRendererDebugFlag mDebugFlags;

	// State settings
	int mlLastShadowAlgo;

	bool mbDepthTest;

	eMaterialAlphaMode mAlphaMode;
	eMaterialBlendMode mBlendMode;
	eMaterialChannelMode mChannelMode;

	iGpuProgram *gpuProgram;
	iMaterialProgramSetup *gpuProgramSetup;
	iGpuProgram *mpVertexProgram;
	bool mbVertexUseLight;
	iMaterialProgramSetup *mpVtxProgramSetup;

	iGpuProgram *mpFragmentProgram;

	bool mbMatrixWasNULL;

	bool mbUsesLight;
	bool mbUsesEye;

	cColor mAmbientColor;

	iTexture *mpTexture[MAX_TEXTUREUNITS];
	eMaterialBlendMode mTextureBlend[MAX_TEXTUREUNITS];

	iVertexBuffer *mpVtxBuffer;
};

//---------------------------------------------

class cRenderer3D {
public:
	cRenderer3D(iLowLevelGraphics *apLowLevelGraphics, cResources *apResources,
				cMeshCreator *apMeshCreator, cRenderList *apRenderList);
	~cRenderer3D();

	void UpdateRenderList(cWorld3D *apWorld, cCamera3D *apCamera, float afFrameTime);

	void RenderWorld(cWorld3D *apWorld, cCamera3D *apCamera, float afFrameTime);

	void SetSkyBox(iTexture *apTexture, bool abAutoDestroy);
	void SetSkyBoxActive(bool abX);
	void SetSkyBoxColor(const cColor &aColor);

	iTexture *GetSkyBox() { return mpSkyBoxTexture; }
	bool GetSkyBoxActive() { return mbSkyBoxActive; }
	cColor GetSkyBoxColor() { return mSkyBoxColor; }

	void SetAmbientColor(const cColor &aColor) { mRenderSettings.mAmbientColor = aColor; }
	cColor GetAmbientColor() { return mRenderSettings.mAmbientColor; }

	iTexture *GetFogSolidTexture() { return mpFogLinearSolidTexture; }
	iTexture *GetFogAddTexture() { return mpFogLinearAddTexture; }
	iTexture *GetFogAlphaTexture() { return mpFogLinearAlphaTexture; }

	void SetRefractionUsed(bool abX) { mbRefractionUsed = abX; }
	bool GetRefractionUsed() { return mbRefractionUsed; }

	// Fog properties
	void SetFogActive(bool abX);
	void SetFogStart(float afX);
	void SetFogEnd(float afX);
	void SetFogColor(const cColor &aCol) { mRenderSettings.mFogColor = aCol; }
	void SetFogCulling(bool abX) { mRenderSettings.mbFogCulling = abX; }

	bool GetFogActive() { return mRenderSettings.mbFogActive; }
	float GetFogStart() { return mRenderSettings.mfFogStart; }
	float GetFogEnd() { return mRenderSettings.mfFogEnd; }
	cColor GetFogColor() { return mRenderSettings.mFogColor; }
	bool GetFogCulling() { return mRenderSettings.mbFogCulling; }

	cBoundingVolume *GetFogBV() { return &mFogBV; }

	void SetPostEffects(cRendererPostEffects *apPostEffects) { mpPostEffects = apPostEffects; }

	// Debug setup
	void SetDebugFlags(tRendererDebugFlag aFlags) { mDebugFlags = aFlags; }
	tRendererDebugFlag GetDebugFlags() { return mDebugFlags; }

	cRenderList *GetRenderList() { return mpRenderList; }
	cRenderSettings *GetRenderSettings() { return &mRenderSettings; }

	eRendererShowShadows GetShowShadows();
	void SetShowShadows(eRendererShowShadows aState);

	void FetchOcclusionQueries();

private:
	inline void BeginRendering(cCamera3D *apCamera);

	void InitSkyBox();

	// Render steps
	void RenderFog(cCamera3D *apCamera);

	void RenderSkyBox(cCamera3D *apCamera);

	void RenderZ(cCamera3D *apCamera);

	void RenderOcclusionQueries(cCamera3D *apCamera);

	void RenderLight(cCamera3D *apCamera);

	void RenderDiffuse(cCamera3D *apCamera);

	void RenderTrans(cCamera3D *apCamera);

	void RenderDebug(cCamera3D *apCamera);

	inline void RenderDebugObject(cCamera3D *apCamera, iRenderable *&apObject, iMaterial *apPrevMat,
								  int alPrevMatId, iVertexBuffer *apPrevVtxBuff,
								  eMaterialRenderType aRenderType, iLight3D *apLight);

	iLowLevelGraphics *mpLowLevelGraphics;
	LowLevelResources *mpLowLevelResources;

	cRendererPostEffects *mpPostEffects;

	bool mbLog;

	float mfRenderTime;

	iGpuProgram *_diffuseProgram;
	iGpuProgram *_solidFogProgram;

	iTexture *mpFogLinearSolidTexture;

	iTexture *mpFogLinearAddTexture;
	iTexture *mpFogLinearAlphaTexture;

	iGpuProgram *_refractProgram;
	iGpuProgram *_refractSpecProgram;
	bool mbRefractionAvailable;
	bool mbRefractionUsed;

	cRenderSettings mRenderSettings;

	cBoundingVolume mFogBV;

	tVertexVec mvVtxRect;

	cMeshCreator *mpMeshCreator;
	cRenderList *mpRenderList;

	iVertexBuffer *mpSkyBox;
	iTexture *mpSkyBoxTexture;
	bool mbAutoDestroySkybox;
	bool mbSkyBoxActive;
	cColor mSkyBoxColor;

	cResources *mpResources;

	tRendererDebugFlag mDebugFlags;
};

} // namespace hpl

#endif // HPL_RENDERER3D_H
