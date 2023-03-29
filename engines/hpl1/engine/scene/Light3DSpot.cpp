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

#include "hpl1/engine/scene/Light3DSpot.h"

#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/math/Frustum.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/scene/Camera3D.h"

#include "hpl1/engine/game/Game.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/World3D.h"

#include "hpl1/engine/scene/PortalContainer.h"
#include "hpl1/engine/scene/SectorVisibility.h"

namespace hpl {

static constexpr cMatrixf g_mtxTextureUnitFix(0.5f, 0, 0, 0.5f,
											  0, -0.5f, 0, 0.5f,
											  0, 0, 0.5f, 0.5f,
											  0, 0, 0, 1.0f);

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLight3DSpot::cLight3DSpot(tString asName, cResources *apResources) : iLight3D(asName, apResources) {
	mbProjectionUpdated = true;
	mbViewProjUpdated = true;
	mbFrustumUpdated = true;

	mLightType = eLight3DType_Spot;

	mpFrustum = hplNew(cFrustum, ());

	mlViewProjMatrixCount = -1;
	mlViewMatrixCount = -1;
	mlFrustumMatrixCount = -1;

	mpTexture = NULL;

	mfFOV = cMath::ToRad(60.0f);
	mfAspect = 1.0f;
	mfFarAttenuation = 100.0f;
	mfNearClipPlane = 0.1f;

	m_mtxView = cMatrixf::Identity;
	m_mtxViewProj = cMatrixf::Identity;
	m_mtxProjection = cMatrixf::Identity;

	UpdateBoundingVolume();
}

cLight3DSpot::~cLight3DSpot() {
	if (mpTexture)
		mpTextureManager->Destroy(mpTexture);
	hplDelete(mpFrustum);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLight3DSpot::SetTexture(iTexture *apTexture) {
	// Destroy any old texture.
	if (mpTexture)
		mpTextureManager->Destroy(mpTexture);

	mpTexture = apTexture;
	mpTexture->SetWrapS(eTextureWrap_ClampToBorder);
	mpTexture->SetWrapT(eTextureWrap_ClampToBorder);
}

//-----------------------------------------------------------------------

iTexture *cLight3DSpot::GetTexture() {
	return mpTexture;
}

//-----------------------------------------------------------------------

void cLight3DSpot::SetFarAttenuation(float afX) {
	mfFarAttenuation = afX;

	UpdateBoundingVolume();

	// This is so that the render container is updated.
	SetTransformUpdated();
	mbProjectionUpdated = true;
}
//-----------------------------------------------------------------------

void cLight3DSpot::SetNearAttenuation(float afX) {
	mfNearAttenuation = afX;
	if (mfNearAttenuation > mfFarAttenuation)
		SetFarAttenuation(mfNearAttenuation);
}

//-----------------------------------------------------------------------

const cMatrixf &cLight3DSpot::GetViewMatrix() {
	if (mlViewMatrixCount != GetTransformUpdateCount()) {
		mlViewMatrixCount = GetTransformUpdateCount();
		m_mtxView = cMath::MatrixInverse(GetWorldMatrix());
	}

	return m_mtxView;
}

//-----------------------------------------------------------------------

const cMatrixf &cLight3DSpot::GetProjectionMatrix() {
	if (mbProjectionUpdated) {
		float fFar = mfFarAttenuation;
		float fNear = mfNearClipPlane;
		float fTop = tan(mfFOV * 0.5f) * fNear;
		float fBottom = -fTop;
		float fRight = mfAspect * fTop;
		float fLeft = mfAspect * fBottom;

		float A = (2.0f * fNear) / (fRight - fLeft);
		float B = (2.0f * fNear) / (fTop - fBottom);
		float D = -1.0f;
		float C = -(2.0f * fFar * fNear) / (fFar - fNear);
		float Z = -(fFar + fNear) / (fFar - fNear);

		float X = 0;
		float Y = 0;

		m_mtxProjection = cMatrixf(
			A, 0, X, 0,
			0, B, Y, 0,
			0, 0, Z, C,
			0, 0, D, 0);

		mbProjectionUpdated = false;
		mbViewProjUpdated = true;
		mbFrustumUpdated = true;
	}

	return m_mtxProjection;
}

//-----------------------------------------------------------------------

const cMatrixf &cLight3DSpot::GetViewProjMatrix() {
	if (mlViewProjMatrixCount != GetTransformUpdateCount() || mbViewProjUpdated || mbProjectionUpdated) {
		m_mtxViewProj = cMath::MatrixMul(GetProjectionMatrix(), GetViewMatrix());
		m_mtxViewProj = cMath::MatrixMul(g_mtxTextureUnitFix, m_mtxViewProj);

		mlViewProjMatrixCount = GetTransformUpdateCount();
		mbViewProjUpdated = false;
	}

	return m_mtxViewProj;
}

//-----------------------------------------------------------------------

cFrustum *cLight3DSpot::GetFrustum() {
	if (mlFrustumMatrixCount != GetTransformUpdateCount() || mbFrustumUpdated || mbProjectionUpdated) {
		mpFrustum->SetViewProjMatrix(GetProjectionMatrix(),
									 GetViewMatrix(),
									 mfFarAttenuation, mfNearClipPlane,
									 mfFOV, mfAspect, GetWorldPosition(), false);
		mbFrustumUpdated = false;
		mlFrustumMatrixCount = GetTransformUpdateCount();
	}

	return mpFrustum;
}

//-----------------------------------------------------------------------

bool cLight3DSpot::CollidesWithBV(cBoundingVolume *apBV) {
	if (cMath::CheckCollisionBV(*GetBoundingVolume(), *apBV) == false)
		return false;

	return GetFrustum()->CollideBoundingVolume(apBV) != eFrustumCollision_Outside;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSectorVisibilityContainer *cLight3DSpot::CreateSectorVisibility() {
	return mpWorld3D->GetPortalContainer()->CreateVisibiltyFromFrustum(mpFrustum);
}
//-----------------------------------------------------------------------

static eTextureAnimMode GetAnimMode(const tString &asType) {
	if (cString::ToLowerCase(asType) == "none")
		return eTextureAnimMode_None;
	else if (cString::ToLowerCase(asType) == "loop")
		return eTextureAnimMode_Loop;
	else if (cString::ToLowerCase(asType) == "oscillate")
		return eTextureAnimMode_Oscillate;

	return eTextureAnimMode_None;
}

void cLight3DSpot::ExtraXMLProperties(TiXmlElement *apMainElem) {
	tString sTexture = cString::ToString(apMainElem->Attribute("ProjectionImage"), "");

	eTextureAnimMode animMode = GetAnimMode(cString::ToString(apMainElem->Attribute("ProjectionAnimMode"), "None"));
	float fFrameTime = cString::ToFloat(apMainElem->Attribute("ProjectionFrameTime"), 1.0f);
	iTexture *pTex = NULL;

	if (animMode != eTextureAnimMode_None) {
		pTex = mpTextureManager->CreateAnim2D(sTexture, true);
		pTex->SetAnimMode(animMode);
		pTex->SetFrameTime(fFrameTime);
	} else {
		pTex = mpTextureManager->Create2D(sTexture, true);
	}

	if (pTex) {
		SetTexture(pTex);
	}

	mfAspect = cString::ToFloat(apMainElem->Attribute("Aspect"), mfAspect);

	mfNearClipPlane = cString::ToFloat(apMainElem->Attribute("NearClipPlane"), mfNearClipPlane);
}

//-----------------------------------------------------------------------

void cLight3DSpot::UpdateBoundingVolume() {
	mBoundingVolume = GetFrustum()->GetBoundingVolume();
}

//-----------------------------------------------------------------------

bool cLight3DSpot::CreateClipRect(cRect2l &aClipRect, cRenderSettings *apRenderSettings,
								  iLowLevelGraphics *apLowLevelGraphics) {
	cVector2f vScreenSize = apLowLevelGraphics->GetScreenSize();
	bool bVisible = cMath::GetClipRectFromBV(aClipRect, *GetBoundingVolume(),
											 apRenderSettings->mpCamera->GetViewMatrix(),
											 apRenderSettings->mpCamera->GetProjectionMatrix(),
											 apRenderSettings->mpCamera->GetNearClipPlane(),
											 cVector2l((int)vScreenSize.x, (int)vScreenSize.y));
	return bVisible;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cSaveData_cLight3DSpot, cSaveData_iLight3D)
	kSerializeVar(msTexture, eSerializeType_String)

		kSerializeVar(mfFOV, eSerializeType_Float32)
			kSerializeVar(mfAspect, eSerializeType_Float32)
				kSerializeVar(mfNearClipPlane, eSerializeType_Float32)
					kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject *cSaveData_cLight3DSpot::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	cWorld3D *pWorld = apGame->GetScene()->GetWorld3D();
	cResources *pResources = apGame->GetResources();

	cLight3DSpot *pLight = pWorld->CreateLightSpot(msName);
	if (pLight && msTexture != "") {
		iTexture *pTex = pResources->GetTextureManager()->Create2D(msTexture, false);
		if (pTex)
			pLight->SetTexture(pTex);
	}

	return pLight;
}

//-----------------------------------------------------------------------

int cSaveData_cLight3DSpot::GetSaveCreatePrio() {
	return 3;
}

//-----------------------------------------------------------------------

iSaveData *cLight3DSpot::CreateSaveData() {
	return hplNew(cSaveData_cLight3DSpot, ());
}

//-----------------------------------------------------------------------

void cLight3DSpot::SaveToSaveData(iSaveData *apSaveData) {
	kSaveData_SaveToBegin(cLight3DSpot);

	pData->msTexture = mpTexture == NULL ? "" : mpTexture->GetName();

	kSaveData_SaveTo(mfFOV);
	kSaveData_SaveTo(mfAspect);
	kSaveData_SaveTo(mfNearClipPlane);
}

//-----------------------------------------------------------------------

void cLight3DSpot::LoadFromSaveData(iSaveData *apSaveData) {
	kSaveData_LoadFromBegin(cLight3DSpot);

	kSaveData_LoadFrom(mfFOV);
	kSaveData_LoadFrom(mfAspect);
	kSaveData_LoadFrom(mfNearClipPlane);
}

//-----------------------------------------------------------------------

void cLight3DSpot::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	kSaveData_SetupBegin(cLight3DSpot);

	// Make sure all is updated.
	SetTransformUpdated();
	mbProjectionUpdated = true;
	mbViewProjUpdated = true;
	mbFrustumUpdated = true;
}

//-----------------------------------------------------------------------

} // namespace hpl
