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

#include "hpl1/engine/graphics/RendererPostEffects.h"
#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/RenderList.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/graphics/Texture.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/GpuProgramManager.h"
#include "hpl1/engine/resources/low_level_resources.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cRendererPostEffects::cRendererPostEffects(iLowLevelGraphics *apLowLevelGraphics, cResources *apResources,
										   cRenderList *apRenderList, cRenderer3D *apRenderer3D) {
	mpLowLevelGraphics = apLowLevelGraphics;
	mpLowLevelResources = apResources->GetLowLevel();
	mpResources = apResources;
	mpRenderer3D = apRenderer3D;

	mpGpuManager = mpResources->GetGpuProgramManager();

	mvScreenSize = mpLowLevelGraphics->GetScreenSize();

	mpRenderList = apRenderList;

	///////////////////////////////////////////
	// Create screen buffers
	Log(" Creating screen buffers size %s\n", mvScreenSize.ToString().c_str());
	for (int i = 0; i < 2; i++) {
		if (mpLowLevelGraphics->GetCaps(eGraphicCaps_TextureTargetRectangle)) {
			mpScreenBuffer[i] = mpLowLevelGraphics->CreateTexture(cVector2l(
																	  (int)mvScreenSize.x, (int)mvScreenSize.y),
																  32, cColor(0, 0, 0, 0), false,
																  eTextureType_Normal, eTextureTarget_Rect);

			if (mpScreenBuffer[i] == NULL) {
				Error("Couldn't create screenbuffer!\n");
				mpScreenBuffer[0] = NULL;
				mpScreenBuffer[1] = NULL;
				break;
			}
			mpScreenBuffer[i]->SetWrapS(eTextureWrap_ClampToEdge);
			mpScreenBuffer[i]->SetWrapT(eTextureWrap_ClampToEdge);

		} else {
			mpScreenBuffer[i] = NULL;
			Error("Texture rectangle not supported. Posteffects will be turned off.");
		}
	}

	///////////////////////////////////////////
	// Create programs

	Log(" Creating programs\n");

	/////////////////
	// Blur programs
	mbBlurFallback = false; // Set to true if the fallbacks are used.

	mpBlurVP = mpGpuManager->CreateProgram("PostEffect_Blur_vp.cg", "main", eGpuProgramType_Vertex);
	if (!mpBlurVP)
		Error("Couldn't load 'PostEffect_Blur_vp.cg'!\n");

	mpBlurRectFP = mpGpuManager->CreateProgram("PostEffect_Blur_Rect_fp.cg", "main", eGpuProgramType_Fragment);
	/*if(!mpBlurRectFP)
	{
		mbBlurFallback = true;
		Log(" Using Blur Rect FP fallback\n");
		mpBlurRectFP = mpGpuManager->CreateProgram("PostEffect_Fallback01_Blur_Rect_fp.cg","main",eGpuProgramType_Fragment);
		if(!mpBlurRectFP) Error("Couldn't load 'PostEffect_Blur_Rect_fp.cg'!\n");
	}*/

	mpBlur2dFP = mpGpuManager->CreateProgram("PostEffect_Blur_2D_fp.cg", "main", eGpuProgramType_Fragment);
	/*if(!mpBlur2dFP)
	{
		mbBlurFallback = true;
		Log(" Using Blur 2D FP fallback\n");
		mpBlur2dFP = mpGpuManager->CreateProgram("PostEffect_Fallback01_Blur_2D_fp.cg","main",eGpuProgramType_Fragment);
		if(!mpBlur2dFP) Error("Couldn't load 'PostEffect_Blur_2D_fp.cg'!\n");
	}*/

	/////////////////
	// Bloom programs
	mpBloomVP = mpGpuManager->CreateProgram("PostEffect_Bloom_vp.cg", "main", eGpuProgramType_Vertex);
	if (!mpBloomVP)
		Error("Couldn't load 'PostEffect_Bloom_vp.cg'!\n");

	mpBloomFP = mpGpuManager->CreateProgram("PostEffect_Bloom_fp.cg", "main", eGpuProgramType_Fragment);
	if (!mpBloomFP)
		Error("Couldn't load 'PostEffect_Bloom_fp.cg'!\n");

	// Bloom blur textures
	mpBloomBlurTexture = mpLowLevelGraphics->CreateTexture(
		cVector2l(256, 256),
		32, cColor(0, 0, 0, 0), false,
		eTextureType_Normal, eTextureTarget_2D);

	if (mpBloomBlurTexture == NULL) {
		Error("Couldn't create bloom blur textures!\n");
	} else {
		mpBloomBlurTexture->SetWrapS(eTextureWrap_ClampToEdge);
		mpBloomBlurTexture->SetWrapT(eTextureWrap_ClampToEdge);
	}

	/////////////////
	// MotionBlur programs
	mpMotionBlurVP = mpGpuManager->CreateProgram("PostEffect_Motion_vp.cg", "main", eGpuProgramType_Vertex);
	if (!mpMotionBlurVP)
		Error("Couldn't load 'PostEffect_Motion_vp.cg'!\n");

	// Disable the dynamic loop version on Mac OS X until we figure why it doesn't work
	mpMotionBlurFP = NULL;
#ifndef __APPLE__
	mpMotionBlurFP = mpGpuManager->CreateProgram("PostEffect_Motion_fp.cg", "main", eGpuProgramType_Fragment);
#endif
	if (!mpMotionBlurFP) {
		Log("Dynamic loops in motion blur fp not supported, loading static instead.\n");
		mpMotionBlurFP = mpGpuManager->CreateProgram("PostEffect_Motion_staticloop_fp.cg", "main", eGpuProgramType_Fragment);
		if (!mpMotionBlurFP)
			Error("Couldn't load 'PostEffect_Motion_fp.cg'!\n");
	}

	/////////////////
	// Depth of Field programs
	mpDepthOfFieldVP = mpGpuManager->CreateProgram("PostEffect_DoF_vp.cg", "main", eGpuProgramType_Vertex);
	if (!mpDepthOfFieldVP)
		Error("Couldn't load 'PostEffect_DoF_vp.cg'!\n");

	mpDepthOfFieldFP = mpGpuManager->CreateProgram("PostEffect_DoF_fp.cg", "main", eGpuProgramType_Fragment);
	if (!mpDepthOfFieldFP)
		Error("Couldn't load 'PostEffect_DoF_fp.cg'!\n");

	// Depth of Field blur textures
	mpDofBlurTexture = mpLowLevelGraphics->CreateTexture(cVector2l(256, 256),
														 32, cColor(0, 0, 0, 0), false,
														 eTextureType_Normal, eTextureTarget_2D);

	if (mpDofBlurTexture == NULL) {
		Error("Couldn't create Depth of Field blur textures!\n");
	} else {
		mpDofBlurTexture->SetWrapS(eTextureWrap_ClampToEdge);
		mpDofBlurTexture->SetWrapT(eTextureWrap_ClampToEdge);
	}

	Log("  RendererPostEffects created\n");

	////////////////////////////////////
	// Variable setup

	// General
	mbActive = false;
	mvTexRectVtx.resize(4);

	// Bloom
	mbBloomActive = false;
	mfBloomSpread = 2.0f;

	// Motion blur
	mbMotionBlurActive = false;
	mfMotionBlurAmount = 1.0f;
	mbMotionBlurFirstTime = true;

	// Depth of Field
	mbDofActive = false;
	mfDofMaxBlur = 1.0f;
	mfDofFocalPlane = 1.0f;
	mfDofNearPlane = 2.0f;
	mfDofFarPlane = 3.0f;
}

//-----------------------------------------------------------------------

cRendererPostEffects::~cRendererPostEffects() {
	for (int i = 0; i < 2; i++)
		if (mpScreenBuffer[i])
			hplDelete(mpScreenBuffer[i]);

	if (mpBlurVP)
		mpGpuManager->Destroy(mpBlurVP);
	if (mpBlur2dFP)
		mpGpuManager->Destroy(mpBlur2dFP);
	if (mpBlurRectFP)
		mpGpuManager->Destroy(mpBlurRectFP);

	if (mpBloomVP)
		mpGpuManager->Destroy(mpBloomVP);
	if (mpBloomFP)
		mpGpuManager->Destroy(mpBloomFP);

	if (mpMotionBlurVP)
		mpGpuManager->Destroy(mpMotionBlurVP);
	if (mpMotionBlurFP)
		mpGpuManager->Destroy(mpMotionBlurFP);

	if (mpDepthOfFieldVP)
		mpGpuManager->Destroy(mpDepthOfFieldVP);
	if (mpDepthOfFieldFP)
		mpGpuManager->Destroy(mpDepthOfFieldFP);

	if (mpBloomBlurTexture)
		hplDelete(mpBloomBlurTexture);
	if (mpDofBlurTexture)
		hplDelete(mpDofBlurTexture);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cRendererPostEffects::SetBloomActive(bool abX) {
	mbBloomActive = abX;
}

//-----------------------------------------------------------------------

void cRendererPostEffects::SetMotionBlurActive(bool abX) {
	mbMotionBlurActive = abX;
	mbMotionBlurFirstTime = true;
}

//-----------------------------------------------------------------------

void cRendererPostEffects::Render() {
	if (mpScreenBuffer[0] == NULL || mpScreenBuffer[1] == NULL)
		return;
	if (!mpLowLevelGraphics->GetCaps(eGraphicCaps_TextureTargetRectangle))
		return;

	if (mbActive == false ||
		(mImageTrailData.mbActive == false && mbBloomActive == false &&
		 mbMotionBlurActive == false && mbDofActive == false)) {
		return;
	}

	mvScreenSize = mpLowLevelGraphics->GetScreenSize();

	RenderDepthOfField();
	RenderMotionBlur();

	mpLowLevelGraphics->SetDepthTestActive(false);
	mpLowLevelGraphics->PushMatrix(eMatrix_ModelView);
	mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
	mpLowLevelGraphics->SetOrthoProjection(mpLowLevelGraphics->GetVirtualSize(), -1000, 1000);

	RenderBloom();

	RenderImageTrail();

	mpLowLevelGraphics->PopMatrix(eMatrix_ModelView);
}

//-----------------------------------------------------------------------

// TODO: Support source texture as 2D

void cRendererPostEffects::RenderBlurTexture(iTexture *apDestination, iTexture *apSource,
											 float afBlurAmount) {
	bool bProgramsLoaded = false;
	if (mpBlurRectFP && mpBlur2dFP && mpBlurVP)
		bProgramsLoaded = true;

	iLowLevelGraphics *pLowLevel = mpLowLevelGraphics;
	cVector2l vBlurSize = cVector2l(apDestination->getWidth(), apDestination->getHeight());
	cVector2f vBlurDrawSize;
	vBlurDrawSize.x = ((float)vBlurSize.x / mvScreenSize.x) * mpLowLevelGraphics->GetVirtualSize().x;
	vBlurDrawSize.y = ((float)vBlurSize.y / mvScreenSize.y) * mpLowLevelGraphics->GetVirtualSize().y;

	pLowLevel->SetBlendActive(false);

	///////////////////////////////////////////
	// Horizontal blur pass

	// Shader setup
	if (bProgramsLoaded) {
		// Setup vertex program
		mpBlurVP->Bind();
		mpBlurVP->SetFloat("xOffset", 1);
		mpBlurVP->SetFloat("yOffset", 0);
		mpBlurVP->SetFloat("amount", afBlurAmount);
		mpBlurVP->SetMatrixf("worldViewProj", eGpuProgramMatrix_ViewProjection, eGpuProgramMatrixOp_Identity);

		// Setup fragment program
		mpBlurRectFP->Bind();
	}

	// Draw the screen texture with blur
	pLowLevel->SetTexture(0, apSource);
	if (mbBlurFallback) {
		mpLowLevelGraphics->SetTexture(1, apSource);
		mpLowLevelGraphics->SetTexture(2, apSource);
	}

	{
		mvTexRectVtx[0] = cVertex(cVector3f(0, 0, 40), cVector2f(0, mvScreenSize.y), cColor(1, 1.0f));
		mvTexRectVtx[1] = cVertex(cVector3f(vBlurDrawSize.x, 0, 40), cVector2f(mvScreenSize.x, mvScreenSize.y), cColor(1, 1.0f));
		mvTexRectVtx[2] = cVertex(cVector3f(vBlurDrawSize.x, vBlurDrawSize.y, 40), cVector2f(mvScreenSize.x, 0), cColor(1, 1.0f));
		mvTexRectVtx[3] = cVertex(cVector3f(0, vBlurDrawSize.y, 40), cVector2f(0, 0), cColor(1, 1.0f));

		mpLowLevelGraphics->DrawQuad(mvTexRectVtx);
	}

	pLowLevel->CopyContextToTexure(apDestination, 0, vBlurSize);

	///////////////////////////////////////////
	// Vertical blur pass

	// Setup shaders
	// Shader setup
	if (bProgramsLoaded) {
		mpBlurVP->SetFloat("xOffset", 0);
		mpBlurVP->SetFloat("yOffset", 1);
		mpBlurVP->SetFloat("amount", (1 / pLowLevel->GetScreenSize().x) * afBlurAmount);

		mpBlurRectFP->UnBind();
		mpBlur2dFP->Bind();
	}

	// Set texture and draw
	pLowLevel->SetTexture(0, apDestination);
	if (mbBlurFallback) {
		mpLowLevelGraphics->SetTexture(1, apDestination);
		mpLowLevelGraphics->SetTexture(2, apDestination);
	}

	{
		mvTexRectVtx[0] = cVertex(cVector3f(0, 0, 40), cVector2f(0, 1), cColor(1, 1.0f));
		mvTexRectVtx[1] = cVertex(cVector3f(vBlurDrawSize.x, 0, 40), cVector2f(1, 1), cColor(1, 1.0f));
		mvTexRectVtx[2] = cVertex(cVector3f(vBlurDrawSize.x, vBlurDrawSize.y, 40), cVector2f(1, 0), cColor(1, 1.0f));
		mvTexRectVtx[3] = cVertex(cVector3f(0, vBlurDrawSize.y, 40), cVector2f(0, 0), cColor(1, 1.0f));

		mpLowLevelGraphics->DrawQuad(mvTexRectVtx);
	}

	// Shader setup
	if (bProgramsLoaded) {
		mpBlur2dFP->UnBind();
		mpBlurVP->UnBind();
	}

	pLowLevel->CopyContextToTexure(apDestination, 0, vBlurSize);

	if (mbBlurFallback) {
		mpLowLevelGraphics->SetTexture(1, NULL);
		mpLowLevelGraphics->SetTexture(2, NULL);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cRendererPostEffects::RenderDepthOfField() {
	if (mbDofActive == false)
		return;

	if (mpDepthOfFieldFP == NULL || mpDepthOfFieldVP == NULL)
		return;

	//////////////////////////////
	// Setup
	cCamera3D *pCam = mpRenderList->GetCamera();

	iTexture *pScreenTexture = mpScreenBuffer[mImageTrailData.mlCurrentBuffer == 0 ? 1 : 0];

	// Size of the virtual screen
	cVector2f vVirtSize = mpLowLevelGraphics->GetVirtualSize();

	// Copy screen to texture
	mpLowLevelGraphics->CopyContextToTexure(pScreenTexture, 0, cVector2l((int)mvScreenSize.x, (int)mvScreenSize.y));

	// Set up things needed for blurring
	mpLowLevelGraphics->SetDepthWriteActive(false);
	mpLowLevelGraphics->SetDepthTestActive(false);
	mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
	mpLowLevelGraphics->SetOrthoProjection(mpLowLevelGraphics->GetVirtualSize(), -1000, 1000);

	// Render blur texture
	RenderBlurTexture(mpDofBlurTexture, pScreenTexture, 2.0f);

	// Set texture
	mpLowLevelGraphics->SetTexture(0, pScreenTexture);
	mpLowLevelGraphics->SetTexture(1, mpDofBlurTexture);

	// Render entire screen textur to screen, this to cover up what has been done by the
	// blur filter, should have support for frame_buffer_object so this can be skipped.
	if (mpRenderer3D->GetSkyBoxActive()) {
		tVector3fVec vUvVec;
		vUvVec.resize(4);

		vUvVec[0] = cVector2f(0, 1);
		vUvVec[1] = cVector2f(1, 1);
		vUvVec[2] = cVector2f(1, 0);
		vUvVec[3] = cVector2f(0, 0);

		mvTexRectVtx[0] = cVertex(cVector3f(0, 0, 40), cVector2f(0, mvScreenSize.y), cColor(1, 1.0f));
		mvTexRectVtx[1] = cVertex(cVector3f(vVirtSize.x, 0, 40), cVector2f(mvScreenSize.x, mvScreenSize.y), cColor(1, 1.0f));
		mvTexRectVtx[2] = cVertex(cVector3f(vVirtSize.x, vVirtSize.y, 40), cVector2f(mvScreenSize.x, 0), cColor(1, 1.0f));
		mvTexRectVtx[3] = cVertex(cVector3f(0, vVirtSize.y, 40), cVector2f(0, 0), cColor(1, 1.0f));

		mpLowLevelGraphics->SetActiveTextureUnit(1);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorSource0, eTextureSource_Texture);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorSource1, eTextureSource_Previous);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorSource2, eTextureSource_Constant);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorFunc, eTextureFunc_Interpolate);
		mpLowLevelGraphics->SetTextureConstantColor(cColor(mfDofMaxBlur, mfDofMaxBlur));

		mpLowLevelGraphics->DrawQuadMultiTex(mvTexRectVtx, vUvVec);

		mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorFunc, eTextureFunc_Modulate);
	}

	// Set things back to normal
	mpLowLevelGraphics->SetMatrix(eMatrix_Projection, pCam->GetProjectionMatrix());

	///////////////////////////////////////////
	// Draw motion blur objects
	mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_Equal);
	mpLowLevelGraphics->SetDepthTestActive(true);
	mpLowLevelGraphics->SetBlendActive(false);

	// Setup
	mpDepthOfFieldVP->Bind();

	mpDepthOfFieldFP->Bind();
	mpDepthOfFieldFP->SetVec3f("planes", cVector3f(mfDofNearPlane, mfDofFocalPlane, mfDofFarPlane));
	mpDepthOfFieldFP->SetFloat("maxBlur", mfDofMaxBlur);
	mpDepthOfFieldFP->SetVec2f("screenSize", mvScreenSize);

	//////////////////
	// Render objects
	cMotionBlurObjectIterator it = mpRenderList->GetMotionBlurIterator();
	while (it.HasNext()) {
		iRenderable *pObject = it.Next();
		cMatrixf *pMtx = pObject->GetModelMatrix(pCam);

		//////////////////
		// Non static models
		if (pMtx) {
			mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, cMath::MatrixMul(pCam->GetViewMatrix(), *pMtx));

			mpDepthOfFieldVP->SetMatrixf("worldViewProj", eGpuProgramMatrix_ViewProjection,
										 eGpuProgramMatrixOp_Identity);
		}
		//////////////////
		// NULL Model view matrix (static)
		else {
			mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, pCam->GetViewMatrix());

			mpDepthOfFieldVP->SetMatrixf("worldViewProj", eGpuProgramMatrix_ViewProjection,
										 eGpuProgramMatrixOp_Identity);
		}

		pObject->GetVertexBuffer()->Bind();

		pObject->GetVertexBuffer()->Draw();

		pObject->GetVertexBuffer()->UnBind();

		// Set the previous postion to the current
		if (pMtx)
			pObject->SetPrevMatrix(*pMtx);
	}

	mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_LessOrEqual);
	mpLowLevelGraphics->SetDepthWriteActive(true);

	// Reset stuff
	mpDepthOfFieldFP->UnBind();
	mpDepthOfFieldVP->UnBind();
	mpLowLevelGraphics->SetTexture(0, NULL);
	mpLowLevelGraphics->SetTexture(1, NULL);
}

//-----------------------------------------------------------------------

void cRendererPostEffects::RenderMotionBlur() {
	if (mbMotionBlurActive == false)
		return;

	if (mpMotionBlurFP == NULL || mpMotionBlurVP == NULL)
		return;

	//////////////////////////////
	// Setup
	iTexture *pScreenTexture = mpScreenBuffer[mImageTrailData.mlCurrentBuffer == 0 ? 1 : 0];

	// Size of the virtual screen
	cVector2f vVirtSize = mpLowLevelGraphics->GetVirtualSize();

	// Copy screen to texture
	mpLowLevelGraphics->CopyContextToTexure(pScreenTexture, 0, cVector2l((int)mvScreenSize.x, (int)mvScreenSize.y));

	///////////////////////////////////////////
	// Draw motion blur objects
	mpLowLevelGraphics->SetDepthTestActive(true);
	mpLowLevelGraphics->SetBlendActive(false);

	cCamera3D *pCam = mpRenderList->GetCamera();
	if (mbMotionBlurFirstTime) {
		pCam->SetPrevView(pCam->GetViewMatrix());
		pCam->SetPrevProjection(pCam->GetProjectionMatrix());
		mbMotionBlurFirstTime = false;
	}

	cMotionBlurObjectIterator it = mpRenderList->GetMotionBlurIterator();

	// Setup
	mpMotionBlurVP->Bind();
	mpMotionBlurVP->SetFloat("blurScale", mfMotionBlurAmount);

	mpMotionBlurFP->Bind();
	mpMotionBlurFP->SetVec2f("halfScreenSize",
							 cVector2f((float)pScreenTexture->getWidth() / 2.0f,
									   (float)pScreenTexture->getHeight() / 2.0f));

	mpLowLevelGraphics->SetTexture(0, pScreenTexture);

	//////////////////
	// Render objects
	while (it.HasNext()) {
		iRenderable *pObject = it.Next();
		cMatrixf *pMtx = pObject->GetModelMatrix(pCam);

		//////////////////
		// Non static models
		if (pMtx) {
			cMatrixf mtxPrev = pObject->GetPrevMatrix();

			mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, cMath::MatrixMul(pCam->GetViewMatrix(), *pMtx));

			mpMotionBlurVP->SetMatrixf("worldViewProj", eGpuProgramMatrix_ViewProjection,
									   eGpuProgramMatrixOp_Identity);

			cMatrixf mtxModelView = cMath::MatrixMul(pCam->GetViewMatrix(), *pMtx);
			cMatrixf mtxPrevModelView = cMath::MatrixMul(pCam->GetPrevView(), mtxPrev);

			cMatrixf mtxPrevViewProj = cMath::MatrixMul(pCam->GetPrevProjection(), mtxPrevModelView);

			mpMotionBlurVP->SetMatrixf("prevWorldViewProj", mtxPrevViewProj);
			mpMotionBlurVP->SetMatrixf("modelView", mtxModelView);
			mpMotionBlurVP->SetMatrixf("prevModelView", mtxPrevModelView);
		}
		//////////////////
		// NULL Model view matrix (static)
		else {
			mpLowLevelGraphics->SetMatrix(eMatrix_ModelView, pCam->GetViewMatrix());

			mpMotionBlurVP->SetMatrixf("worldViewProj", eGpuProgramMatrix_ViewProjection,
									   eGpuProgramMatrixOp_Identity);

			const cMatrixf &mtxModelView = pCam->GetViewMatrix();
			const cMatrixf &mtxPrevModelView = pCam->GetPrevView();

			cMatrixf mtxPrevViewProj = cMath::MatrixMul(pCam->GetPrevProjection(), mtxPrevModelView);

			mpMotionBlurVP->SetMatrixf("prevWorldViewProj", mtxPrevViewProj);
			mpMotionBlurVP->SetMatrixf("modelView", mtxModelView);
			mpMotionBlurVP->SetMatrixf("prevModelView", mtxPrevModelView);
		}

		pObject->GetVertexBuffer()->Bind();

		pObject->GetVertexBuffer()->Draw();

		pObject->GetVertexBuffer()->UnBind();

		// Set the previous postion to the current
		if (pMtx)
			pObject->SetPrevMatrix(*pMtx);
	}

	// Reset stuff
	mpMotionBlurFP->UnBind();
	mpMotionBlurVP->UnBind();
	mpLowLevelGraphics->SetTexture(0, NULL);
	mpLowLevelGraphics->SetTexture(1, NULL);

	// Set the new pervious values.
	pCam->SetPrevView(pCam->GetViewMatrix());
	pCam->SetPrevProjection(pCam->GetProjectionMatrix());
}

//-----------------------------------------------------------------------

void cRendererPostEffects::RenderBloom() {
	if (mbBloomActive == false)
		return;

	if (mpBloomFP == NULL || mpBloomVP == NULL)
		return;

	//////////////////////////////
	// Setup

	iTexture *pScreenTexture = mpScreenBuffer[mImageTrailData.mlCurrentBuffer == 0 ? 1 : 0];

	// Copy screen to texture
	mpLowLevelGraphics->CopyContextToTexure(pScreenTexture, 0,
											cVector2l((int)mvScreenSize.x, (int)mvScreenSize.y));

	// Get the blur texture
	RenderBlurTexture(mpBloomBlurTexture, pScreenTexture, mfBloomSpread);

	// Size of blur texture
	cVector2f vBlurSize = cVector2f((float)mpBloomBlurTexture->getWidth(), (float)mpBloomBlurTexture->getHeight());

	// Size of the virtual screen
	cVector2f vVirtSize = mpLowLevelGraphics->GetVirtualSize();

	///////////////////////////////////////////
	// Draw Bloom

	// Setup vertex program
	mpBloomVP->Bind();
	mpBloomVP->SetMatrixf("worldViewProj", eGpuProgramMatrix_ViewProjection, eGpuProgramMatrixOp_Identity);

	// Setup fragment program
	mpBloomFP->Bind();

	mpLowLevelGraphics->SetTexture(0, mpBloomBlurTexture);
	mpLowLevelGraphics->SetTexture(1, pScreenTexture);

	// Draw
	{
		tVector3fVec vUvVec;
		vUvVec.resize(4);

		vUvVec[0] = cVector2f(0, mvScreenSize.y);
		vUvVec[1] = cVector2f(mvScreenSize.x, mvScreenSize.y);
		vUvVec[2] = cVector2f(mvScreenSize.x, 0);
		vUvVec[3] = cVector2f(0, 0);

		mvTexRectVtx[0] = cVertex(cVector3f(0, 0, 40), cVector2f(0, 1), cColor(1, 1.0f));
		mvTexRectVtx[1] = cVertex(cVector3f(vVirtSize.x, 0, 40), cVector2f(1, 1), cColor(1, 1.0f));
		mvTexRectVtx[2] = cVertex(cVector3f(vVirtSize.x, vVirtSize.y, 40), cVector2f(1, 0), cColor(1, 1.0f));
		mvTexRectVtx[3] = cVertex(cVector3f(0, vVirtSize.y, 40), cVector2f(0, 0), cColor(1, 1.0f));

		mpLowLevelGraphics->DrawQuadMultiTex(mvTexRectVtx, vUvVec);
	}

	mpBloomVP->UnBind();
	mpBloomFP->UnBind();

	mpLowLevelGraphics->SetTexture(0, NULL);
	mpLowLevelGraphics->SetTexture(1, NULL);
}

//-----------------------------------------------------------------------

void cRendererPostEffects::RenderImageTrail() {
	if (mImageTrailData.mbActive == false)
		return;

	tVertexVec vVtx;
	vVtx.push_back(cVertex(cVector3f(0, 0, 40), cVector2f(0, mvScreenSize.y), cColor(1, 0.6f)));
	vVtx.push_back(cVertex(cVector3f(800, 0, 40), cVector2f(mvScreenSize.x, mvScreenSize.y), cColor(1, 0.6f)));
	vVtx.push_back(cVertex(cVector3f(800, 600, 40), cVector2f(mvScreenSize.x, 0), cColor(1, 0.6f)));
	vVtx.push_back(cVertex(cVector3f(0, 600, 40), cVector2f(0, 0), cColor(1, 0.6f)));

	mpLowLevelGraphics->CopyContextToTexure(mpScreenBuffer[mImageTrailData.mlCurrentBuffer == 0 ? 1 : 0], 0, cVector2l((int)mpLowLevelGraphics->GetScreenSize().x, (int)mpLowLevelGraphics->GetScreenSize().y));
	{
		if (mImageTrailData.mbActive) {
			if (mImageTrailData.mbFirstPass) {
				mpLowLevelGraphics->SetBlendActive(false);
				mpLowLevelGraphics->SetTexture(0, mpScreenBuffer[mImageTrailData.mlCurrentBuffer == 0 ? 1 : 0]);
			} else {

				mpLowLevelGraphics->SetClearDepthActive(false);
				mpLowLevelGraphics->SetClearColor(cColor(0, 0));
				mpLowLevelGraphics->ClearScreen();
				mpLowLevelGraphics->SetClearDepthActive(true);

				// Draw the new image to screen transparently
				mpLowLevelGraphics->SetBlendActive(true);
				mpLowLevelGraphics->SetBlendFunc(eBlendFunc_SrcAlpha, eBlendFunc_Zero);

				mpLowLevelGraphics->SetTexture(0, mpScreenBuffer[mImageTrailData.mlCurrentBuffer == 0 ? 1 : 0]);

				mpLowLevelGraphics->SetTextureConstantColor(cColor(1, 1.0f - mImageTrailData.mfAmount));
				mpLowLevelGraphics->SetTextureEnv(eTextureParam_AlphaSource0, eTextureSource_Constant);
				mpLowLevelGraphics->SetTextureEnv(eTextureParam_AlphaFunc, eTextureFunc_Replace);
			}
		} else {
			mpLowLevelGraphics->SetBlendActive(false);
			mpLowLevelGraphics->SetTexture(0, mpScreenBuffer[mImageTrailData.mlCurrentBuffer == 0 ? 1 : 0]);
		}

		mpLowLevelGraphics->DrawQuad(vVtx);

		if (mImageTrailData.mbActive) {
			if (!mImageTrailData.mbFirstPass) {
				// Draw the old Blurred image to screen
				mpLowLevelGraphics->SetBlendFunc(eBlendFunc_SrcAlpha, eBlendFunc_One);

				mpLowLevelGraphics->SetTexture(0, mpScreenBuffer[mImageTrailData.mlCurrentBuffer]);

				mpLowLevelGraphics->SetTextureConstantColor(cColor(1, mImageTrailData.mfAmount));
				mpLowLevelGraphics->SetTextureEnv(eTextureParam_AlphaSource0, eTextureSource_Constant);
				mpLowLevelGraphics->SetTextureEnv(eTextureParam_AlphaFunc, eTextureFunc_Replace);

				mpLowLevelGraphics->DrawQuad(vVtx);
			} else {
				mImageTrailData.mbFirstPass = false;
			}
			mImageTrailData.mlCurrentBuffer = mImageTrailData.mlCurrentBuffer == 0 ? 1 : 0;

			// Copy screen to new blur buffer
			mpLowLevelGraphics->CopyContextToTexure(mpScreenBuffer[mImageTrailData.mlCurrentBuffer], 0, cVector2l((int)mpLowLevelGraphics->GetScreenSize().x, (int)mpLowLevelGraphics->GetScreenSize().y));
		}

		mpLowLevelGraphics->SetTextureEnv(eTextureParam_AlphaSource0, eTextureSource_Texture);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_AlphaSource1, eTextureSource_Previous);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_AlphaFunc, eTextureFunc_Modulate);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorSource0, eTextureSource_Texture);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorSource1, eTextureSource_Previous);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorFunc, eTextureFunc_Modulate);

		mpLowLevelGraphics->SetBlendActive(false);
		mpLowLevelGraphics->SetTexture(0, NULL);
	}
}

//-----------------------------------------------------------------------

} // namespace hpl
