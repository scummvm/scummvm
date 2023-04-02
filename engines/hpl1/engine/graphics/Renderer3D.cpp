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

#include "hpl1/engine/graphics/Renderer3D.h"

#include "hpl1/debug.h"
#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/MeshCreator.h"
#include "hpl1/engine/graphics/RenderList.h"
#include "hpl1/engine/graphics/Renderable.h"
#include "hpl1/engine/graphics/RendererPostEffects.h"
#include "hpl1/engine/graphics/Texture.h"
#include "hpl1/engine/graphics/VertexBuffer.h"
#include "hpl1/engine/math/BoundingVolume.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/GpuProgramManager.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/resources/low_level_resources.h"
#include "hpl1/engine/scene/Camera3D.h"
#include "hpl1/engine/scene/Entity3D.h"
#include "hpl1/engine/scene/Light3D.h"
#include "hpl1/engine/scene/RenderableContainer.h"
#include "hpl1/engine/scene/World3D.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// RENDER SETTINGS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cRenderSettings::cRenderSettings() {
	mbFogActive = false;
	mfFogStart = 5.0f;
	mfFogEnd = 5.0f;
	mFogColor = cColor(1, 1);
	mbFogCulling = false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cRenderer3D::cRenderer3D(iLowLevelGraphics *apLowLevelGraphics, cResources *apResources,
						 cMeshCreator *apMeshCreator, cRenderList *apRenderList) {
	Hpl1::logInfo(Hpl1::kDebugGraphics, "%s", "Creating Renderer3D\n");

	mpLowLevelGraphics = apLowLevelGraphics;
	mpLowLevelResources = apResources->GetLowLevel();
	mpResources = apResources;
	mpMeshCreator = apMeshCreator;

	mpSkyBoxTexture = NULL;
	mbAutoDestroySkybox = false;
	mbSkyBoxActive = false;
	mSkyBoxColor = cColor(1, 1);

	mRenderSettings.mAmbientColor = cColor(0, 1);

	mpRenderList = apRenderList;

	mDebugFlags = 0;

	mbLog = false;

	mfRenderTime = 0;

	mbRefractionUsed = true;

	mvVtxRect.resize(4);
	mvVtxRect[0] = cVertex(cVector3f(0, 0, 0), cVector2f(0, 1), cColor(1, 1));
	mvVtxRect[1] = cVertex(cVector3f(1, 0, 0), cVector2f(1, 1), cColor(1, 1));
	mvVtxRect[2] = cVertex(cVector3f(1, 1, 0), cVector2f(1, 0), cColor(1, 1));
	mvVtxRect[3] = cVertex(cVector3f(0, 1, 40), cVector2f(0, 0), cColor(1, 1));

	// Set up render settings.
	mRenderSettings.mpLowLevel = mpLowLevelGraphics;
	mRenderSettings.mbLog = false;
	mRenderSettings.mShowShadows = eRendererShowShadows_All;
	mRenderSettings.mpTempIndexArray = hplNewArray(unsigned int, 60000);

	Hpl1::logInfo(Hpl1::kDebugGraphics, "%s", "Load Renderer3D gpu programs:\n");
	cGpuProgramManager *pProgramManager = apResources->GetGpuProgramManager();

	mRenderSettings.extrudeProgram = pProgramManager->CreateProgram("hpl1_ShadowExtrude", "hpl1_ShadowExtrude");

	///////////////////////////////////
	// Load diffuse program, for stuff like query rendering
	_diffuseProgram = pProgramManager->CreateProgram("hpl1_Diffuse_Color", "hpl1_Diffuse_Color");

	///////////////////////////////////
	// Fog Vertex Program Init
	_solidFogProgram = pProgramManager->CreateProgram("hpl1_Fog_Solid", "hpl1_Fog_Solid");

	///////////////////////////////////
	// Fog Texture Init
	Log("Creating fog textures:");
	unsigned char *pFogArray = hplNewArray(unsigned char, 256 * 2);

	// Solid
	Log("Solid");
	iTexture *pTex = mpLowLevelGraphics->CreateTexture("FogLinearSolid",
													   false, eTextureType_Normal, eTextureTarget_1D);

	for (int i = 0; i < 256; ++i) {
		pFogArray[i * 2 + 0] = 255;
		// pFogArray[i*2 + 0] = 255 - (unsigned char)i;
		pFogArray[i * 2 + 1] = 255 - (unsigned char)i;
	}

	if (pTex) {
		pTex->CreateFromArray(pFogArray, 2, cVector3l(256, 1, 1));
		pTex->SetWrapR(eTextureWrap_ClampToEdge);
		pTex->SetWrapS(eTextureWrap_ClampToEdge);
		pTex->SetWrapT(eTextureWrap_ClampToEdge);
	}

	mpFogLinearSolidTexture = pTex;

	// Additive
	Log("Additive ");
	pTex = mpLowLevelGraphics->CreateTexture("FogLinearAdd",
											 false, eTextureType_Normal, eTextureTarget_1D);
	for (int i = 0; i < 256; ++i) {
		pFogArray[i * 2 + 0] = (unsigned char)i;
		pFogArray[i * 2 + 1] = (unsigned char)i;
	}

	if (pTex) {
		pTex->CreateFromArray(pFogArray, 2, cVector3l(256, 1, 1));
		pTex->SetWrapR(eTextureWrap_ClampToEdge);
		pTex->SetWrapS(eTextureWrap_ClampToEdge);
	}

	mpFogLinearAddTexture = pTex;

	// Alpha
	Log("Alpha ");
	pTex = mpLowLevelGraphics->CreateTexture("FogLinearAlpha",
											 false, eTextureType_Normal, eTextureTarget_1D);
	for (int i = 0; i < 256; ++i) {
		pFogArray[i * 2 + 0] = 255;
		pFogArray[i * 2 + 1] = (unsigned char)i;
	}

	if (pTex) {
		pTex->CreateFromArray(pFogArray, 2, cVector3l(256, 1, 1));
		pTex->SetWrapR(eTextureWrap_ClampToEdge);
		pTex->SetWrapS(eTextureWrap_ClampToEdge);
	}

	mpFogLinearAlphaTexture = pTex;

	hplDeleteArray(pFogArray);

	Log("\n");

	/////////////////////////////////////////////
	// Create Refraction programs
	mbRefractionAvailable = true;
	_refractProgram = pProgramManager->CreateProgram("hpl1_refract", "hpl1_refract");
	_refractSpecProgram = pProgramManager->CreateProgram("hpl1_refract", "hpl1_refract_special");
	if (!_refractProgram || !_refractSpecProgram) {
		mbRefractionAvailable = false;
		Hpl1::logInfo(Hpl1::kDebugGraphics, "%s", "refraction will not be supported");
	}

	/////////////////////////////////////////////
	// Create sky box graphics.

	Log("init sky box\n");
	InitSkyBox();

	Log("Renderer3D created\n");
}

//-----------------------------------------------------------------------

cRenderer3D::~cRenderer3D() {
	hplDeleteArray(mRenderSettings.mpTempIndexArray);

	if (mRenderSettings.extrudeProgram)
		mpResources->GetGpuProgramManager()->Destroy(mRenderSettings.extrudeProgram);

	if (_diffuseProgram)
		mpResources->GetGpuProgramManager()->Destroy(_diffuseProgram);

	if (_solidFogProgram)
		mpResources->GetGpuProgramManager()->Destroy(_solidFogProgram);

	if (_refractProgram)
		mpResources->GetGpuProgramManager()->Destroy(_refractProgram);
	if (_refractSpecProgram)
		mpResources->GetGpuProgramManager()->Destroy(_refractSpecProgram);

	if (mpSkyBox)
		hplDelete(mpSkyBox);
	if (mpSkyBoxTexture && mbAutoDestroySkybox) {
		mpResources->GetTextureManager()->Destroy(mpSkyBoxTexture);
	}

	if (mpFogLinearSolidTexture)
		hplDelete(mpFogLinearSolidTexture);
	if (mpFogLinearAddTexture)
		hplDelete(mpFogLinearAddTexture);
	if (mpFogLinearAlphaTexture)
		hplDelete(mpFogLinearAlphaTexture);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cRenderSettings::Clear() {
	mlLastShadowAlgo = 0;

	mbDepthTest = true;

	mAlphaMode = eMaterialAlphaMode_Solid;
	mBlendMode = eMaterialBlendMode_None;
	mChannelMode = eMaterialChannelMode_RGBA;

	gpuProgram = nullptr;
	mbVertexUseLight = false;

	mpSector = NULL;

	mbUsesLight = false;
	mbUsesEye = false;

	mbMatrixWasNULL = false;

	for (int i = 0; i < MAX_TEXTUREUNITS; i++) {
		mpTexture[i] = NULL;
		mTextureBlend[i] = eMaterialBlendMode_None;
	}

	mpVtxBuffer = NULL;
}

void cRenderSettings::Reset(iLowLevelGraphics *apLowLevel) {
	if (gpuProgram)
		gpuProgram->UnBind();
	if (mpVtxBuffer)
		mpVtxBuffer->UnBind();

	for (int i = 0; i < MAX_TEXTUREUNITS; i++) {
		if (mpTexture[i]) {
			apLowLevel->SetTexture(i, NULL);
			// mpLowLevelGraphics->SetTextureParam(eTextureParam_ColorOp1,eTextureOp_Color);
			// mpLowLevelGraphics->SetTextureParam(eTextureParam_ColorFunc, eTextureFunc_Modulate);
		}
	}

	Clear();
}

//-----------------------------------------------------------------------

eRendererShowShadows cRenderer3D::GetShowShadows() {
	return mRenderSettings.mShowShadows;
}
void cRenderer3D::SetShowShadows(eRendererShowShadows aState) {
	mRenderSettings.mShowShadows = aState;
}

//-----------------------------------------------------------------------

void cRenderer3D::UpdateRenderList(cWorld3D *apWorld, cCamera3D *apCamera, float afFrameTime) {
	// Clear all objects to be rendereded
	mpRenderList->Clear();

	// Set some variables
	mpRenderList->SetFrameTime(afFrameTime);
	mpRenderList->SetCamera(apCamera);

	// Set the frustum
	mRenderSettings.mpFrustum = apCamera->GetFrustum();

	// Setup fog BV
	if (mRenderSettings.mbFogActive && mRenderSettings.mbFogCulling) {
		// This is becuase the fog line is a stright line infront of the camera.
		float fCornerDist = (mRenderSettings.mfFogEnd * 2.0f) /
							cos(apCamera->GetFOV() * apCamera->GetAspect() * 0.5f);

		mFogBV.SetSize(fCornerDist);
		mFogBV.SetPosition(apCamera->GetPosition());
	}

	// Add all objects to be rendered
	apWorld->GetRenderContainer()->GetVisible(mRenderSettings.mpFrustum, mpRenderList);

	// Compile an optimized render list.
	mpRenderList->Compile();
}

//-----------------------------------------------------------------------

void cRenderer3D::RenderWorld(cWorld3D *apWorld, cCamera3D *apCamera, float afFrameTime) {
	mfRenderTime += afFrameTime;

	//////////////////////////////
	// Setup render settings and logging
	if (mDebugFlags & eRendererDebugFlag_LogRendering) {
		mbLog = true;
		mRenderSettings.mbLog = true;
	} else if (mbLog) {
		mbLog = false;
		mRenderSettings.mbLog = false;
	}
	mRenderSettings.mDebugFlags = mDebugFlags;

	/////////////////////////////////
	// Set up rendering
	BeginRendering(apCamera);

	mRenderSettings.Clear();

	////////////////////////////
	// Render Z
	// mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_E);
	mpLowLevelGraphics->SetColorWriteActive(true, true, true, true);
	mRenderSettings.mChannelMode = eMaterialChannelMode_RGBA;

	// mpLowLevelGraphics->SetColorWriteActive(false, false, false,false);
	// mRenderSettings.mChannelMode = eMaterialChannelMode_Z;

	// Set the ambient color
	mpLowLevelGraphics->SetActiveTextureUnit(0);
	mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorFunc, eTextureFunc_Modulate);
	mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorSource0, eTextureSource_Texture);
	mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorSource1, eTextureSource_Constant);

	// mpLowLevelGraphics->SetTextureEnv(eTextureParam_AlphaSource1,eTextureSource_Constant);

	mpLowLevelGraphics->SetTextureConstantColor(mRenderSettings.mAmbientColor);

	if (mbLog)
		Log("Rendering ZBuffer:\n");
	RenderZ(apCamera);

	// reset parameters.
	mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorSource1, eTextureSource_Previous);
	// mpLowLevelGraphics->SetTextureEnv(eTextureParam_AlphaSource1,eTextureSource_Previous);

	////////////////////////////
	// Render Occlusion Queries
	mpLowLevelGraphics->SetColorWriteActive(false, false, false, false);
	mRenderSettings.mChannelMode = eMaterialChannelMode_Z;

	if (mbLog)
		Log("Rendering Occlusion Queries:\n");
	mpLowLevelGraphics->SetDepthWriteActive(false);
	RenderOcclusionQueries(apCamera);

	////////////////////////////
	// Render lighting
	mRenderSettings.mChannelMode = eMaterialChannelMode_RGBA;
	mpLowLevelGraphics->SetColorWriteActive(true, true, true, true);

	mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_Equal);

	if (mbLog)
		Log("Rendering Lighting:\n");
	RenderLight(apCamera);

	////////////////////////////
	// Render Diffuse
	if (mbLog)
		Log("Rendering Diffuse:\n");
	RenderDiffuse(apCamera);

	////////////////////////////
	// Render fog
	if (mbLog)
		Log("Rendering fog:\n");
	RenderFog(apCamera);

	////////////////////////////
	// Render sky box
	mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_LessOrEqual);

	////////////////////////////
	if (mbLog)
		Log("Rendering Skybox:\n");
	RenderSkyBox(apCamera);

	// Render transparent
	if (mbLog)
		Log("Rendering Transperant:\n");
	RenderTrans(apCamera);

	mRenderSettings.Reset(mpLowLevelGraphics);

	////////////////////////////
	// Render debug
	RenderDebug(apCamera);

	mpLowLevelGraphics->SetDepthWriteActive(true);
}

//-----------------------------------------------------------------------

void cRenderer3D::SetSkyBox(iTexture *apTexture, bool abAutoDestroy) {
	if (mpSkyBoxTexture && mbAutoDestroySkybox) {
		mpResources->GetTextureManager()->Destroy(mpSkyBoxTexture);
	}

	mbAutoDestroySkybox = abAutoDestroy;
	mpSkyBoxTexture = apTexture;
	if (mpSkyBoxTexture) {
		mpSkyBoxTexture->SetWrapS(eTextureWrap_ClampToEdge);
		mpSkyBoxTexture->SetWrapT(eTextureWrap_ClampToEdge);
	}
}

void cRenderer3D::SetSkyBoxActive(bool abX) {
	mbSkyBoxActive = abX;
}

void cRenderer3D::SetSkyBoxColor(const cColor &aColor) {
	if (mSkyBoxColor == aColor)
		return;

	mSkyBoxColor = aColor;

	float *pColors = mpSkyBox->GetArray(eVertexFlag_Color0);

	int lNum = mpSkyBox->GetVertexNum();
	for (int i = 0; i < lNum; ++i) {
		pColors[0] = mSkyBoxColor.r;
		pColors[1] = mSkyBoxColor.g;
		pColors[2] = mSkyBoxColor.b;
		pColors[3] = mSkyBoxColor.a;
		pColors += 4;
	}

	mpSkyBox->UpdateData(eVertexFlag_Color0, false);
}

//-----------------------------------------------------------------------

void cRenderer3D::SetFogActive(bool abX) {
	if (_solidFogProgram)
		mRenderSettings.mbFogActive = abX;
}
void cRenderer3D::SetFogStart(float afX) {
	mRenderSettings.mfFogStart = afX;
}

void cRenderer3D::SetFogEnd(float afX) {
	mRenderSettings.mfFogEnd = afX;
}

//-----------------------------------------------------------------------

void cRenderer3D::FetchOcclusionQueries() {
	if (mbLog)
		Log("Fetching Occlusion Queries Result:\n");

	// With depth test
	cOcclusionQueryObjectIterator it = mpRenderList->GetQueryIterator();
	while (it.HasNext()) {
		cOcclusionQueryObject *pObject = it.Next();
		// LogUpdate("Query: %d!\n",pObject->mpQuery);

		while (pObject->mpQuery->FetchResults() == false)
			;

		if (mbLog)
			Log(" Query: %d SampleCount: %d\n", pObject->mpQuery,
				pObject->mpQuery->GetSampleCount());
	}

	if (mbLog)
		Log("Done fetching queries\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cRenderer3D::BeginRendering(cCamera3D *apCamera) {
	//////////////////////////////////////////////////
	/////Setup for clearing the screen
	// turn of scissor test.
	mpLowLevelGraphics->SetScissorActive(false);

	mpLowLevelGraphics->SetClearColor(cColor(0, 0));
	mpLowLevelGraphics->SetClearColorActive(true);
	mpLowLevelGraphics->SetClearDepthActive(true);
	mpLowLevelGraphics->SetClearDepth(1);

	mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_LessOrEqual);

	mpLowLevelGraphics->SetClearStencilActive(false);
	mpLowLevelGraphics->ClearScreen();

	//////////////////////////////////////////////////
	// Cull and depth mode.
	mpLowLevelGraphics->SetCullActive(true);

	mpLowLevelGraphics->SetCullMode(eCullMode_CounterClockwise);
	mpLowLevelGraphics->SetDepthTestActive(true);

	mpLowLevelGraphics->SetMatrix(eMatrix_Projection, apCamera->GetProjectionMatrix());

	mpLowLevelGraphics->SetColor(cColor(1, 1, 1, 1));

	mRenderSettings.mpCamera = apCamera;

	for (int i = 0; i < MAX_TEXTUREUNITS; ++i)
		mpLowLevelGraphics->SetTexture(i, NULL);
}

//-----------------------------------------------------------------------

void cRenderer3D::InitSkyBox() {
	mpSkyBox = mpMeshCreator->CreateSkyBoxVertexBuffer(1);
}

//-----------------------------------------------------------------------

static void clearTextures(uint start, cRenderSettings &rendererSettings, iLowLevelGraphics *lowLevelGfx) {
	for (uint i = start; i < MAX_TEXTUREUNITS; ++i) {
		if (rendererSettings.mpTexture[i]) {
			lowLevelGfx->SetTexture(i, nullptr);
			rendererSettings.mpTexture[i] = nullptr;
		}
	}
}

void cRenderer3D::RenderFog(cCamera3D *apCamera) {
	if (mRenderSettings.mbFogActive == false || !_solidFogProgram)
		return;

	clearTextures(0, mRenderSettings, mpLowLevelGraphics);

	//////////////////////////////////
	// Set fog program
	if (mRenderSettings.gpuProgram)
		mRenderSettings.gpuProgram->UnBind();
	mRenderSettings.gpuProgram = nullptr;
	_solidFogProgram->Bind();
	_solidFogProgram->SetColor3f("fogColor", mRenderSettings.mFogColor);
	_solidFogProgram->SetFloat("fogStart", mRenderSettings.mfFogStart);
	_solidFogProgram->SetFloat("fogEnd", mRenderSettings.mfFogEnd);

	//////////////////////////////////
	// Blend mode
	mpLowLevelGraphics->SetBlendActive(true);
	// mpLowLevelGraphics->SetBlendFunc(eBlendFunc_One,eBlendFunc_Zero);
	mpLowLevelGraphics->SetBlendFunc(eBlendFunc_SrcAlpha, eBlendFunc_OneMinusSrcAlpha);
	mRenderSettings.mBlendMode = eMaterialBlendMode_LastEnum;

	//////////////////////////////////////
	// Texture
	mpLowLevelGraphics->SetTexture(0, mpFogLinearSolidTexture);
	mRenderSettings.mpTexture[0] = mpFogLinearSolidTexture;

	//////////////////////////////////
	// Render objects
	cMotionBlurObjectIterator it = mpRenderList->GetMotionBlurIterator();

	while (it.HasNext()) {
		iRenderable *pObject = it.Next();
		cMatrixf *pMtx = pObject->GetModelMatrix(apCamera);

		//////////////////
		// Non static models
		if (pMtx) {
			mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, cMath::MatrixMul(apCamera->GetViewMatrix(),
																			  *pMtx));

			_solidFogProgram->SetMatrixf("worldViewProj", eGpuProgramMatrix_ViewProjection,
										 eGpuProgramMatrixOp_Identity);
		}
		//////////////////
		// NULL Model view matrix (static)
		else {
			mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, apCamera->GetViewMatrix());

			_solidFogProgram->SetMatrixf("worldViewProj", eGpuProgramMatrix_ViewProjection,
										 eGpuProgramMatrixOp_Identity);
		}

		pObject->GetVertexBuffer()->Bind();
		pObject->GetVertexBuffer()->Draw();
		pObject->GetVertexBuffer()->UnBind();
	}

	_solidFogProgram->UnBind();
}

//-----------------------------------------------------------------------

void cRenderer3D::RenderSkyBox(cCamera3D *apCamera) {
	if (mbSkyBoxActive == false)
		return;

	Hpl1::logInfo(Hpl1::kDebugRenderer, "%s\n", "Drawing skybox");

	if (mRenderSettings.gpuProgram) {
		mRenderSettings.gpuProgram->UnBind();
		mRenderSettings.gpuProgram = nullptr;
	}

	if (mRenderSettings.mpVtxBuffer) {
		mRenderSettings.mpVtxBuffer->UnBind();
		mRenderSettings.mpVtxBuffer = NULL;
		if (mbLog)
			Log(" Setting Vertex Buffer: NULL\n");
	}

	clearTextures(1, mRenderSettings, mpLowLevelGraphics);

	mRenderSettings.mbMatrixWasNULL = false;

	cMatrixf mtxSky = cMatrixf::Identity;

	// Calculate the size of the sky box need to just touch the far clip plane.
	float fFarClip = apCamera->GetFarClipPlane();
	float fSide = fFarClip / sqrt(3) * 0.95f;
	mtxSky.m[0][0] = fSide;
	mtxSky.m[1][1] = fSide;
	mtxSky.m[2][2] = fSide;

	mtxSky = cMath::MatrixMul(mtxSky, apCamera->GetViewMatrix());

	mtxSky.SetTranslation(0);

	mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, mtxSky);

	mpLowLevelGraphics->SetTexture(0, mpSkyBoxTexture);
	mRenderSettings.mpTexture[0] = mpSkyBoxTexture;

	mpLowLevelGraphics->SetBlendActive(false);
	mRenderSettings.mBlendMode = eMaterialBlendMode_Replace;

	mpSkyBox->Bind();
	mpSkyBox->Draw();
	mpSkyBox->UnBind();
}
//-----------------------------------------------------------------------

void cRenderer3D::RenderZ(cCamera3D *apCamera) {
	cRenderNode *pNode = mpRenderList->GetRootNode(eRenderListDrawType_Normal,
												   eMaterialRenderType_Z, 0);

	mRenderSettings.mpLight = NULL;
	pNode->Render(&mRenderSettings);
}

//-----------------------------------------------------------------------

void cRenderer3D::RenderOcclusionQueries(cCamera3D *apCamera) {
#if 0
	//////////////////////////////////////////////////
	// Reset any vertex buffers,fragment or vertex programs.
	if (mRenderSettings.mpFragmentProgram)
		mRenderSettings.mpFragmentProgram->UnBind();

	mRenderSettings.mpFragmentProgram = NULL;
#endif

	////////////////////////////
	// Vertex program
	if (mRenderSettings.gpuProgram != _diffuseProgram) {
		if (mRenderSettings.gpuProgram)
			mRenderSettings.gpuProgram->UnBind();
		mRenderSettings.gpuProgram = _diffuseProgram;
		_diffuseProgram->Bind();
		Hpl1::logInfo(Hpl1::kDebugGraphics, "%s", "binding Rendered3D::_diffuseProgram");
	}

	////////////////////////
	// Reset texture unit 0
	mpLowLevelGraphics->SetTexture(0, NULL);
	mRenderSettings.mpTexture[0] = NULL;

	////////////////////////
	// Keep track of what has been set
	iVertexBuffer *pPrevBuffer = mRenderSettings.mpVtxBuffer;
	cMatrixf *pPrevMatrix = NULL;
	bool bFirstRound = true;
	bool bPrevDepthTest = true;

	//////////////////////////////////
	// Iterate the query objects
	cOcclusionQueryObjectIterator it = mpRenderList->GetQueryIterator();
	while (it.HasNext()) {
		cOcclusionQueryObject *pObject = it.Next();

		/*if(pObject->mbDepthTest) {
			mpLowLevelGraphics->SetColorWriteActive(true, true,true,true);
			mRenderSettings.mChannelMode = eMaterialChannelMode_RGBA;
		}
		else {
			mpLowLevelGraphics->SetColorWriteActive(false, false, false, false);
			mRenderSettings.mChannelMode = eMaterialChannelMode_Z;
		}*/

		/////////////////////
		// Set depth test
		if (bPrevDepthTest != pObject->mbDepthTest) {
			// mpLowLevelGraphics->SetDepthTestActive(pObject->mbDepthTest);
			if (pObject->mbDepthTest)
				mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_LessOrEqual);
			else
				mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_Always);

			bPrevDepthTest = pObject->mbDepthTest;
			if (mbLog)
				Log(" Setting depth test %d\n", pObject->mbDepthTest ? 1 : 0);
		}

		/////////////////////
		// Set matrix
		if (pPrevMatrix != pObject->mpMatrix || bFirstRound) {
			if (pObject->mpMatrix) {
				mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, cMath::MatrixMul(apCamera->GetViewMatrix(), *pObject->mpMatrix));
			} else {
				mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, apCamera->GetViewMatrix());
			}
			pPrevMatrix = pObject->mpMatrix;
			// Set the vertex program matrix.
			if (_diffuseProgram)
				_diffuseProgram->SetMatrixf("worldViewProj", eGpuProgramMatrix_ViewProjection,
											eGpuProgramMatrixOp_Identity);

			if (mbLog)
				Log(" Setting matrix %d\n", pObject->mpMatrix);
		}
		/////////////////////
		// Set Vertex buffer and draw
		if (pPrevBuffer != pObject->mpVtxBuffer) {
			if (pPrevBuffer)
				pPrevBuffer->UnBind();
			pObject->mpVtxBuffer->Bind();
			pPrevBuffer = pObject->mpVtxBuffer;

			if (mbLog)
				Log(" Setting vtx buffer %d\n", pObject->mpVtxBuffer);
		}

		pObject->mpQuery->Begin();
		pObject->mpVtxBuffer->Draw();
		pObject->mpQuery->End();

		if (mbLog)
			Log(" Render with query: %d\n", pObject->mpQuery);

		bFirstRound = false;
	}

	mRenderSettings.mpVtxBuffer = pPrevBuffer;

	// mpLowLevelGraphics->FlushRendering();

	// if(bPrevDepthTest==false)
	//	mpLowLevelGraphics->SetDepthTestActive(true);
}

//-----------------------------------------------------------------------

void cRenderer3D::RenderLight(cCamera3D *apCamera) {
	if (mDebugFlags & eRendererDebugFlag_DisableLighting)
		return;

	cLight3DIterator lightIt = mpRenderList->GetLightIt();

	int lLightCount = 0;
	while (lightIt.HasNext()) {
		if (lLightCount >= MAX_NUM_OF_LIGHTS)
			break;

		iLight3D *pLight = lightIt.Next();

		if (mpRenderList->GetLightObjects(lLightCount) == 0) {
			lLightCount++;
			continue;
		}

		if (mbLog)
			Log("-----Light %s/%d ------\n", pLight->GetName().c_str(), (size_t)pLight);

		cRenderNode *pNode = mpRenderList->GetRootNode(eRenderListDrawType_Normal,
													   eMaterialRenderType_Light, lLightCount);

		if (pLight->BeginDraw(&mRenderSettings, mpLowLevelGraphics)) {
			pNode->Render(&mRenderSettings);
		}
		pLight->EndDraw(&mRenderSettings, mpLowLevelGraphics);

		lLightCount++;
	}
}

//-----------------------------------------------------------------------

void cRenderer3D::RenderDiffuse(cCamera3D *apCamera) {
	cRenderNode *pNode = mpRenderList->GetRootNode(eRenderListDrawType_Normal,
												   eMaterialRenderType_Diffuse, 0);
	mRenderSettings.mpLight = NULL;
	pNode->Render(&mRenderSettings);
}

//-----------------------------------------------------------------------

/*void cRenderer3D::RenderTrans(cCamera3D *apCamera)
{
	cRenderNode* pNode = mpRenderList->GetRootNode(eRenderListDrawType_Trans,
													eMaterialRenderType_Diffuse, 0);
	mRenderSettings.mpLight = NULL;
	pNode->Render(&mRenderSettings);
}*/

void cRenderer3D::RenderTrans(cCamera3D *apCamera) {
	mpLowLevelGraphics->SetColorWriteActive(true, true, true, true);
	mpLowLevelGraphics->SetDepthWriteActive(false);

	bool bLog = mRenderSettings.mbLog;

	iTexture *vTextures[MAX_TEXTUREUNITS];

	/*for(int i=0;i<MAX_TEXTUREUNITS;i++)
	{
		mpLowLevelGraphics->SetTexture(i,NULL);
		mRenderSettings.mpTexture[i] = NULL;
	}*/

	/*cVector3f vForward = */ apCamera->GetForward();

	//////////////////////////////////
	// Iterate the query objects
	cTransperantObjectIterator it = mpRenderList->GetTransperantIterator();
	while (it.HasNext()) {
		iRenderable *pObject = it.Next();

		if (bLog)
			Log(" Rendering '%s'\n", pObject->GetName().c_str());

		/*if(pObject->GetIsOneSided())
		{
			cVector3f vNormal;

			if(pObject->GetModelMatrix(apCamera))
			{
				vNormal = cMath::MatrixMul(pObject->GetWorldMatrix().GetRotation(), pObject->GetOneSidedNormal());
				vNormal = cMath::MatrixMul(apCamera->GetViewMatrix().GetRotation(), vNormal);
			}
			else
			{
				vNormal = cMath::MatrixMul(apCamera->GetViewMatrix().GetRotation(), pObject->GetOneSidedNormal());
			}

			if(cMath::Vector3Dot(vNormal, cVector3f(0,0,1)) < -0.5f)
			{
				continue;
			}
		}*/

		/////////////////////////////////////////
		// Get all data needed
		iMaterial *pMaterial = pObject->GetMaterial();

		bool bDepthTest = pMaterial->GetDepthTest();

		eMaterialAlphaMode alphaMode = pMaterial->GetAlphaMode(eMaterialRenderType_Diffuse, 0, NULL);

		eMaterialBlendMode blendMode = pMaterial->GetBlendMode(eMaterialRenderType_Diffuse, 0, NULL);

		for (int i = 0; i < MAX_TEXTUREUNITS; ++i)
			vTextures[i] = pMaterial->GetTexture(i, eMaterialRenderType_Diffuse, 0, NULL);

		iVertexBuffer *pVtxBuffer = pObject->GetVertexBuffer();

		cMatrixf *pModelMatrix = pObject->GetModelMatrix(apCamera);
		cMatrixf *pInvModelMatrix = pObject->GetInvModelMatrix();

		iTexture *pRefraction = pMaterial->GetTexture(eMaterialTexture_Refraction);

		if (mRenderSettings.mbDepthTest != bDepthTest) {
			mpLowLevelGraphics->SetDepthTestActive(bDepthTest);
			mRenderSettings.mbDepthTest = bDepthTest;

			if (bLog)
				Log("  Set depth test %d\n", bDepthTest);
		}

		/////////////////////////////////////////
		// Refraction Rendering
		if (mbRefractionAvailable && pRefraction && mbRefractionUsed) {
			if (bLog)
				Log("  Start render refraction\n");

			if (bLog)
				Log("   Unbind vtx buffer, vtx program and frag program\n");
			if (mRenderSettings.mpVtxBuffer)
				mRenderSettings.mpVtxBuffer->UnBind();
			if (mRenderSettings.gpuProgram)
				mRenderSettings.gpuProgram->UnBind();

			/////////////////////////////////////
			// Alpha and blend mode
			mRenderSettings.mAlphaMode = eMaterialAlphaMode_Solid;
			mpLowLevelGraphics->SetAlphaTestActive(false);

			mRenderSettings.mBlendMode = eMaterialBlendMode_None;
			mpLowLevelGraphics->SetBlendActive(false);

			//////////////////////////////
			// Get Screen Clip space
			cVector2f vScreenSizeFloat = mpLowLevelGraphics->GetScreenSize();
			cVector2l vScreenSize((int)mpLowLevelGraphics->GetScreenSize().x,
								  (int)mpLowLevelGraphics->GetScreenSize().y);
			iTexture *pScreen = mpPostEffects->GetFreeScreenTexture();

			// Get the cliprect objects bounding volume
			cRect2l clipRect;
			int lScale = cMath::Abs((int)pMaterial->GetValue()) + 1;
			bool bHasClipRect = cMath::GetClipRectFromBV(clipRect, *pObject->GetBoundingVolume(),
														 apCamera->GetViewMatrix(), apCamera->GetProjectionMatrix(),
														 apCamera->GetNearClipPlane(), vScreenSize);
			if (bHasClipRect) {
				clipRect.x -= lScale;
				if (clipRect.x < 0) {
					clipRect.w += clipRect.x;
					clipRect.x = 0;
				}
				clipRect.y -= lScale;
				if (clipRect.y < 0) {
					clipRect.h += clipRect.y;
					clipRect.y = 0;
				}

				clipRect.w += lScale * 2;
				if (clipRect.w + clipRect.x > vScreenSize.x)
					clipRect.w = vScreenSize.x - clipRect.x;

				clipRect.h += lScale * 2;
				if (clipRect.h + clipRect.y > vScreenSize.y)
					clipRect.h = vScreenSize.y - clipRect.y;
			}

			//////////////////////////////
			// Render to alpha if water, (shitty test this is...)
			if (pMaterial->GetRefractionSkipsStandardTrans()) {
				if (bLog)
					Log("   Render alpha to screen\n");

				// NULL all textuer
				for (int i = 0; i < MAX_TEXTUREUNITS; i++) {
					if (mRenderSettings.mpTexture[i] != NULL) {
						mpLowLevelGraphics->SetTexture(i, NULL);
						mRenderSettings.mpTexture[i] = NULL;

						if (bLog)
							Log("    Set texture[%d] NULL\n", i);
					}
				}

				// Only write to alpha
				mpLowLevelGraphics->SetColorWriteActive(false, false, false, true);

				////////////////////////////////////////////////
				// Clear alpha using 2D quad.
				mpLowLevelGraphics->SetOrthoProjection(mpLowLevelGraphics->GetScreenSize(), -1000, 1000);
				mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);

				if (bHasClipRect) {
					cVector2f vOffset = cVector2f((float)clipRect.x, (float)clipRect.y);
					cVector2f vSize = cVector2f((float)clipRect.w, (float)clipRect.h);

					mvVtxRect[0].pos = cVector3f(vOffset.x, vOffset.y, 0);
					mvVtxRect[1].pos = cVector3f(vOffset.x + vSize.x, vOffset.y, 0);
					mvVtxRect[2].pos = cVector3f(vOffset.x + vSize.x, vOffset.y + vSize.y, 0);
					mvVtxRect[3].pos = cVector3f(vOffset.x, vOffset.y + vSize.y, 0);
				} else {
					mvVtxRect[0].pos = cVector3f(0, 0, 0);
					mvVtxRect[1].pos = cVector3f(vScreenSizeFloat.x, 0, 0);
					mvVtxRect[2].pos = cVector3f(vScreenSizeFloat.x, vScreenSizeFloat.y, 0);
					mvVtxRect[3].pos = cVector3f(0, vScreenSizeFloat.y, 0);
				}

				for (int i = 0; i < 4; ++i)
					mvVtxRect[i].col.a = 0;
				mpLowLevelGraphics->DrawQuad(mvVtxRect);

				// Set back to ordinary projection...
				mpLowLevelGraphics->SetMatrix(eMatrix_Projection, apCamera->GetProjectionMatrix());

				// Set Model matrix
				if (pModelMatrix) {
					cMatrixf mtxModel = cMath::MatrixMul(apCamera->GetViewMatrix(), *pModelMatrix);
					mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, mtxModel);
					mRenderSettings.mbMatrixWasNULL = false;
				} else {
					mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, apCamera->GetViewMatrix());
					mRenderSettings.mbMatrixWasNULL = true;
				}

				pVtxBuffer->Bind();

				if (bLog)
					Log("    Drawing vtx buffer %d\n", pVtxBuffer);
				pVtxBuffer->Draw();

				pVtxBuffer->UnBind();

				// Reset stuff
				mpLowLevelGraphics->SetColorWriteActive(true, true, true, true);
			}

			//////////////////////////////
			// Copy Screen To Texture
			if (bHasClipRect) {
				cVector2l vOffset = cVector2l(clipRect.x, clipRect.y);
				cVector2l vSize = cVector2l(clipRect.w, clipRect.h);

				/*Log("  ScreenSize: %d:%d Offset: %d:%d Size: %d:%d Scale: %d\n",vScreenSize.x,vScreenSize.y,
				vOffset.x,vOffset.y,
				vSize.x, vSize.y,
				lScale);*/

				mpLowLevelGraphics->CopyContextToTexure(pScreen, vOffset, vSize, vOffset);
				if (bLog)
					Log("   Copy clipped screen texture\n");
			} else {
				mpLowLevelGraphics->CopyContextToTexure(pScreen, 0, vScreenSize);
				if (bLog)
					Log("   Copy full screen screen texture\n");
			}

			//////////////////////////////
			// Set up programs
			bool bSpecial = false;
			if (pMaterial->GetTexture(eMaterialTexture_Specular) != NULL &&
				pMaterial->GetRefractionDiffuseTexture() == eMaterialTexture_Diffuse) {
				bSpecial = true;
				if (bLog)
					Log("   Special = true\n");
			}

			iGpuProgram *refractProgram = pMaterial->getRefractionProgram();
			if (!refractProgram) {
				if (bSpecial)
					refractProgram = _refractSpecProgram;
				else
					refractProgram = _refractProgram;
			}

			mRenderSettings.gpuProgram = refractProgram;
			refractProgram->Bind();
			Hpl1::logInfo(Hpl1::kDebugRenderer, "Binding gpu program '%s'", refractProgram->GetName().c_str());

			/////////////////////////////////////
			// Vertex shader
			mRenderSettings.mbMatrixWasNULL = false;

			// Model matrix
			if (pModelMatrix) {
				cMatrixf mtxModel = cMath::MatrixMul(apCamera->GetViewMatrix(), *pModelMatrix);
				mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, mtxModel);
				mRenderSettings.mbMatrixWasNULL = false;
			} else {
				mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, apCamera->GetViewMatrix());
				mRenderSettings.mbMatrixWasNULL = true;
			}
			refractProgram->SetMatrixf("worldViewProj",
									   eGpuProgramMatrix_ViewProjection,
									   eGpuProgramMatrixOp_Identity);

			// Eye position
			if (pMaterial->GetRefractionUsesEye()) {
				if (pModelMatrix) {
					cVector3f vLocalEye = cMath::MatrixMul(*pInvModelMatrix, apCamera->GetEyePosition());
					refractProgram->SetVec3f("EyePos", vLocalEye);
				} else {
					refractProgram->SetVec3f("EyePos", apCamera->GetEyePosition());
				}
			}

			/////////////////////////////////////
			// Fragment shader
			refractProgram->SetVec2f("screenSize", mpLowLevelGraphics->GetScreenSize());

			if (bSpecial) {
				refractProgram->SetFloat("t", mfRenderTime * pRefraction->GetFrameTime());
			} else if (pMaterial->GetRefractionUsesTime()) {
				refractProgram->SetFloat("t", mfRenderTime);
			}

			refractProgram->SetFloat("scale", pMaterial->GetValue());

			////////////////////////////////////
			// Textures
			for (int i = 0; i < MAX_TEXTUREUNITS; i++) {
				// float fTime;
				iTexture *pTex = NULL;
				switch (i) {
				case 0:
					pTex = pScreen;
					break;
				case 1:
					pTex = pRefraction;
					break;
				case 2:
					if (bSpecial) {
						pTex = pMaterial->GetTexture(eMaterialTexture_Specular);
					} else if (pMaterial->GetRefractionUsesDiffuse()) {
						pTex = pMaterial->GetTexture(pMaterial->GetRefractionDiffuseTexture());
					}
				}

				if (mRenderSettings.mpTexture[i] != pTex) {
					mpLowLevelGraphics->SetTexture(i, pTex);
					mRenderSettings.mpTexture[i] = pTex;
					if (bLog) {
						if (pTex)
							Log("   Set texture[%d] %s (%d)\n", i, pTex->GetName().c_str(), pTex);
						else
							Log("   Set texture[%d] NULL\n", i);
					}
				}
			}

			/////////////////////////////////////
			// Draw
			if (bLog)
				Log("   Drawing vtx buffer %d\n", pVtxBuffer);

			pVtxBuffer->Bind();

			pVtxBuffer->Draw();

			pVtxBuffer->UnBind();

			mRenderSettings.mpVtxBuffer = pVtxBuffer;

			if (pMaterial->GetRefractionSkipsStandardTrans() || pMaterial->GetTexture(eMaterialTexture_Diffuse) == NULL) {
				if (bLog)
					Log("   Skipping normal trans rendering!\n");
				continue;
			}
		}

		/////////////////////////////////////////////////
		// Alpha mode
		if (alphaMode != mRenderSettings.mAlphaMode) {
			mRenderSettings.mAlphaMode = alphaMode;

			if (alphaMode == eMaterialAlphaMode_Solid) {
				mpLowLevelGraphics->SetAlphaTestActive(false);
				if (bLog)
					Log("  Set alpha test off!\n");
			} else {
				mpLowLevelGraphics->SetAlphaTestActive(true);
				mpLowLevelGraphics->SetAlphaTestFunc(eAlphaTestFunc_GreaterOrEqual, 0.6f);
				if (bLog)
					Log("  Set alpha test on!\n");
			}
		}

		/////////////////////////////////////////////////
		// Blend mode
		if (blendMode != mRenderSettings.mBlendMode) {
			mRenderSettings.mBlendMode = blendMode;

			if (blendMode == eMaterialBlendMode_None) {
				mpLowLevelGraphics->SetBlendActive(false);
				if (bLog)
					Log("  Set blend mode off!\n");
			} else {
				mpLowLevelGraphics->SetBlendActive(true);

				switch (blendMode) {
				case eMaterialBlendMode_Add:
					mpLowLevelGraphics->SetBlendFunc(eBlendFunc_One, eBlendFunc_One);
					if (bLog)
						Log("  Set blend mode one-one!\n");
					break;
				case eMaterialBlendMode_Replace:
					mpLowLevelGraphics->SetBlendFunc(eBlendFunc_One, eBlendFunc_Zero);
					if (bLog)
						Log("  Set blend mode one-zero!\n");
					break;
				case eMaterialBlendMode_Mul:
					mpLowLevelGraphics->SetBlendFunc(eBlendFunc_Zero, eBlendFunc_SrcColor);
					if (bLog)
						Log("  Set blend mode zero-srccolor!\n");
					break;
				case eMaterialBlendMode_MulX2:
					mpLowLevelGraphics->SetBlendFunc(eBlendFunc_DestColor, eBlendFunc_SrcColor);
					if (bLog)
						Log("  Set blend mode destcolor-srccolor!\n");
					break;
				case eMaterialBlendMode_Alpha:
					mpLowLevelGraphics->SetBlendFunc(eBlendFunc_SrcAlpha, eBlendFunc_OneMinusSrcAlpha);
					if (bLog)
						Log("  Set blend mode srcalpha-oneminussrcalpha!\n");
					break;
				case eMaterialBlendMode_DestAlphaAdd:
					mpLowLevelGraphics->SetBlendFunc(eBlendFunc_DestAlpha, eBlendFunc_One);
					if (bLog)
						Log("  Set blend mode destalpha-one!\n");
					break;
				default:
					break;
				}
			}
		}

		iGpuProgram *gpuProgram = pMaterial->getGpuProgram(eMaterialRenderType_Diffuse, 0, nullptr);
		iMaterialProgramSetup *gpuProgramSetup = pMaterial->getGpuProgramSetup(eMaterialRenderType_Diffuse, 0, NULL);

		if (gpuProgram != mRenderSettings.gpuProgram) {
			if (mRenderSettings.gpuProgram)
				mRenderSettings.gpuProgram->UnBind();
			mRenderSettings.gpuProgram = gpuProgram;
			if (gpuProgram) {
				gpuProgram->Bind();
				if (gpuProgramSetup)
					gpuProgramSetup->Setup(gpuProgram, &mRenderSettings);
				mRenderSettings.gpuProgramSetup = gpuProgramSetup;
				mRenderSettings.mbMatrixWasNULL = false;
			}
		}

		/////////////////////////////////////////////////
		// Texture
		for (int i = 0; i < MAX_TEXTUREUNITS; i++) {
			if (mRenderSettings.mpTexture[i] != vTextures[i]) {
				mpLowLevelGraphics->SetTexture(i, vTextures[i]);
				mRenderSettings.mpTexture[i] = vTextures[i];

				if (bLog) {
					if (vTextures[i])
						Log("   Set texture[%d] %s (%d)\n", i, vTextures[i]->GetName().c_str(), vTextures[i]);
					else
						Log("   Set texture[%d] NULL\n", i);
				}
			}
		}

		/////////////////////////////////////////////////
		// Vertex buffer
		if (pVtxBuffer != mRenderSettings.mpVtxBuffer) {
			if (mRenderSettings.mpVtxBuffer)
				mRenderSettings.mpVtxBuffer->UnBind();
			mRenderSettings.mpVtxBuffer = pVtxBuffer;

			if (pVtxBuffer)
				pVtxBuffer->Bind();

			if (bLog)
				Log("  Set vtx buffer %d\n", pVtxBuffer);
		}

		/////////////////////////////////////////////////
		// Matrix
		// It is a normal matrix
		bool bSetVtxProgMatrix = false;
		if (pModelMatrix) {
			cMatrixf mtxModel = cMath::MatrixMul(apCamera->GetViewMatrix(), *pModelMatrix);

			mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, mtxModel);

			mRenderSettings.mbMatrixWasNULL = false;
			bSetVtxProgMatrix = true;
		}
		// NULL matrix
		else if (mRenderSettings.mbMatrixWasNULL == false) {
			mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, apCamera->GetViewMatrix());

			mRenderSettings.mbMatrixWasNULL = true;
			bSetVtxProgMatrix = true;
		}

		if (mRenderSettings.gpuProgram && bSetVtxProgMatrix) {
			// Might be quicker if this is set directly
			mRenderSettings.gpuProgram->SetMatrixf("worldViewProj",
												   eGpuProgramMatrix_ViewProjection,
												   eGpuProgramMatrixOp_Identity);
			if (mRenderSettings.gpuProgramSetup) {
				mRenderSettings.gpuProgramSetup->SetupMatrix(pModelMatrix, &mRenderSettings);
			}
		}

		///////////////////////////////////////////
		/// Draw
		if (bLog)
			Log("Draw\n");
		pVtxBuffer->Draw();
	}

	if (mRenderSettings.mpVtxBuffer)
		mRenderSettings.mpVtxBuffer->UnBind();
	if (mRenderSettings.gpuProgram)
		mRenderSettings.gpuProgram->UnBind();
}

//-----------------------------------------------------------------------

void cRenderer3D::RenderDebug(cCamera3D *apCamera) {
	if (mDebugFlags) {
		mpLowLevelGraphics->SetDepthWriteActive(false);

		// Render Debug for objects
		cRenderableIterator objectIt = mpRenderList->GetObjectIt();
		while (objectIt.HasNext()) {
			iRenderable *pObject = objectIt.Next();

			RenderDebugObject(apCamera, pObject, NULL, 0, NULL, eMaterialRenderType_Diffuse, NULL);
		}

		// Render debug for lights.

		if (mDebugFlags & eRendererDebugFlag_DrawLightBoundingBox) {
			mpLowLevelGraphics->SetDepthTestActive(false);
			mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, apCamera->GetViewMatrix());

			cLight3DIterator lightIt = mpRenderList->GetLightIt();
			while (lightIt.HasNext()) {
				iLight3D *pLight = lightIt.Next();
				cBoundingVolume *pBV = pLight->GetBoundingVolume();

				cColor Col = pLight->GetDiffuseColor();

				mpLowLevelGraphics->DrawSphere(pLight->GetWorldPosition(), 0.1f, Col);
				mpLowLevelGraphics->DrawBoxMaxMin(pBV->GetMax(), pBV->GetMin(), Col);
			}

			mpLowLevelGraphics->SetDepthTestActive(true);
		}

		mpLowLevelGraphics->SetDepthWriteActive(true);
	}
}

//-----------------------------------------------------------------------

void cRenderer3D::RenderDebugObject(cCamera3D *apCamera, iRenderable *&apObject, iMaterial *apPrevMat,
									int alPrevMatId, iVertexBuffer *apPrevVtxBuff,
									eMaterialRenderType aRenderType, iLight3D *apLight) {
	iVertexBuffer *pVtxBuffer = apObject->GetVertexBuffer();

	if (mDebugFlags & eRendererDebugFlag_DrawBoundingBox) {
		mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, apCamera->GetViewMatrix());

		cBoundingVolume *pBV = apObject->GetBoundingVolume();

		mpLowLevelGraphics->DrawBoxMaxMin(pBV->GetMax(), pBV->GetMin(), cColor(1, 1.0f, 1.0f, 1));
	}

	if (mDebugFlags & eRendererDebugFlag_DrawBoundingSphere) {
		mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, apCamera->GetViewMatrix());

		cBoundingVolume *pBV = apObject->GetBoundingVolume();

		mpLowLevelGraphics->DrawSphere(pBV->GetWorldCenter(), pBV->GetRadius(), cColor(1, 1.0f, 1.0f, 1));
	}

	cMatrixf mtxModel;
	cMatrixf *pModelMtx = apObject->GetModelMatrix(apCamera);

	if (pModelMtx)
		mtxModel = cMath::MatrixMul(apCamera->GetViewMatrix(), *pModelMtx);
	else
		mtxModel = cMath::MatrixMul(apCamera->GetViewMatrix(), cMatrixf::Identity);

	mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, mtxModel);

	// Draw the debug graphics for the object.
	for (int i = 0; i < pVtxBuffer->GetVertexNum(); i++) {
		cVector3f vPos = pVtxBuffer->GetVector3(eVertexFlag_Position, i);

		if (mDebugFlags & eRendererDebugFlag_DrawNormals) {
			cVector3f vNormal = pVtxBuffer->GetVector3(eVertexFlag_Normal, i);

			mpLowLevelGraphics->DrawLine(vPos, vPos + (vNormal * 0.1f), cColor(0.5f, 0.5f, 1, 1));
		}
		if (mDebugFlags & eRendererDebugFlag_DrawTangents) {
			cVector3f vTan = pVtxBuffer->GetVector4(eVertexFlag_Texture1, i);

			mpLowLevelGraphics->DrawLine(vPos, vPos + (vTan * 0.2f), cColor(1, 0.0f, 0.0f, 1));
		}
	}
}

//-----------------------------------------------------------------------

} // namespace hpl
