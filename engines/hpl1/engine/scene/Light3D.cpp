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

#include "hpl1/engine/scene/Light3D.h"

#include "hpl1/engine/graphics/BillBoard.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/Mesh.h"
#include "hpl1/engine/graphics/ParticleSystem3D.h"
#include "hpl1/engine/graphics/RenderList.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/graphics/SubMesh.h"
#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/scene/Camera3D.h"
#include "hpl1/engine/scene/MeshEntity.h"
#include "hpl1/engine/scene/PortalContainer.h"
#include "hpl1/engine/scene/SectorVisibility.h"
#include "hpl1/engine/scene/World3D.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iLight3D::iLight3D(tString asName, cResources *apResources) : iLight(), iRenderable(asName) {
	mbStaticCasterAdded = false;

	mbOnlyAffectInInSector = false;

	mbApplyTransformToBV = false;

	mpTextureManager = apResources->GetTextureManager();
	mpFileSearcher = apResources->GetFileSearcher();

	mpFalloffMap = mpTextureManager->Create1D("core_falloff_linear", false);
	if (mpFalloffMap) {
		mpFalloffMap->SetWrapS(eTextureWrap_ClampToEdge);
		mpFalloffMap->SetWrapT(eTextureWrap_ClampToEdge);
	}

	mpVisSectorCont = NULL;
	mlSectorVisibilityCount = -1;

	for (int i = 0; i < 3; ++i)
		mvTempTextures[i] = NULL;
}

//-----------------------------------------------------------------------

iLight3D::~iLight3D() {
	if (mpFalloffMap)
		mpTextureManager->Destroy(mpFalloffMap);

	if (mpVisSectorCont)
		hplDelete(mpVisSectorCont);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void iLight3D::SetVisible(bool abVisible) {
	SetRendered(abVisible);

	for (size_t i = 0; i < mvBillboards.size(); ++i) {
		mvBillboards[i]->SetVisible(abVisible);
	}
}

//-----------------------------------------------------------------------

void iLight3D::AddShadowCaster(iRenderable *apObject, cFrustum *apFrustum, bool abStatic, cRenderList *apRenderList) {
	// Log("Testing: %s\n",apObject->GetName().c_str());
	/////////////////////////////////////////////////////
	// Check if the object should be added
	eRenderableType renderType = apObject->GetRenderType();

	// Is it affected by the light.
	if (GetOnlyAffectInSector() && apObject->IsInSector(GetCurrentSector()) == false)
		return;

	// Is it visible
	if (apObject->GetForceShadow() == false) {
		if (apObject->IsVisible() == false)
			return;
		// Does the object cast shadows?
		if (apObject->IsShadowCaster() == false)
			return;

		// Can be material cast shadows?
		iMaterial *pMaterial = apObject->GetMaterial();
		if (pMaterial) {
			if (pMaterial->IsTransperant() || pMaterial->HasAlpha())
				return;
		}

		// Check so that the object is the right type
		if (renderType != eRenderableType_Mesh && renderType != eRenderableType_Normal) {
			return;
		}
	}

	// Log("Right type!\n");

	// Check if the object has all ready been added
	if (abStatic) {
		if (m_setStaticCasters.find(apObject) != m_setStaticCasters.end())
			return;
	} else {
		if (m_setDynamicCasters.find(apObject) != m_setDynamicCasters.end())
			return;
	}

	// Log("Not in list!\n");

	// Check if the object touches the light.
	// if(CollidesWithBV(apObject->GetBoundingVolume())==false){
	//	return;
	// }

	if (CheckObjectIntersection(apObject) == false)
		return;

	// Log("Collides!\n");

	// Log("Shadow is in frustum!\n");

	////////////////////////////////////////////////////
	// All checks passed, add the object!
	if (renderType == eRenderableType_Mesh) {
		cMeshEntity *pMesh = static_cast<cMeshEntity *>(apObject);

		/// Need to add shadow casted objects else shadows might get choppy.
		if (abStatic == false) {
			pMesh->SetGlobalRenderCount(cRenderList::GetGlobalRenderCount());
		}

		for (int i = 0; i < pMesh->GetSubMeshEntityNum(); i++) {
			cSubMeshEntity *pSub = pMesh->GetSubMeshEntity(i);

			if (pSub->IsVisible() == false)
				continue;
			if (apObject->GetForceShadow() == false) {
				iMaterial *pMaterial = pSub->GetMaterial();
				if (pMaterial) {
					if (pMaterial->IsTransperant() || pMaterial->HasAlpha())
						continue;
				}
			}

			if (abStatic) {
				m_setStaticCasters.insert(pSub);
			} else {
				m_setDynamicCasters.insert(pSub);
				/// Need to add shadow casted objects else shadows might get choppy.
				pSub->SetGlobalRenderCount(cRenderList::GetGlobalRenderCount());
			}
		}
	} else {
		if (abStatic) {
			m_setStaticCasters.insert(apObject);
		} else {
			m_setDynamicCasters.insert(apObject);
			apObject->SetGlobalRenderCount(cRenderList::GetGlobalRenderCount());
		}
	}
}

bool iLight3D::HasStaticCasters() {
	return m_setStaticCasters.empty() ? false : true;
}

void iLight3D::ClearCasters(bool abClearStatic) {
	if (abClearStatic)
		m_setStaticCasters.clear();
	m_setDynamicCasters.clear();
}

//-----------------------------------------------------------------------

bool iLight3D::CheckObjectIntersection(iRenderable *apObject) {
	// Log("------ Checking %s with light %s -----\n",apObject->GetName().c_str(), GetName().c_str());
	// Log(" BV: min: %s max: %s\n",	apObject->GetBoundingVolume()->GetMin().ToString().c_str(),
	//								apObject->GetBoundingVolume()->GetMax().ToString().c_str());

	//////////////////////////////////////////////////////////////
	// If the lights cast shadows, cull objects that are in shadow
	if (mbCastShadows) {
		/////////////////////////////////////
		// Check if the visibility needs update
		if (mlSectorVisibilityCount != GetMatrixUpdateCount()) {
			mlSectorVisibilityCount = GetMatrixUpdateCount();
			if (mpVisSectorCont)
				hplDelete(mpVisSectorCont);

			mpVisSectorCont = CreateSectorVisibility();
			// Log("Creating Visibility container!\n");
		}

		// Get the data list containing the sectors the object is connected to
		tRenderContainerDataList *pDataList = apObject->GetRenderContainerDataList();

		// It is not attached to any room. Just use BV test.
		if (pDataList->empty()) {
			// Log("Empty data list using BV\n");
			return CollidesWithBV(apObject->GetBoundingVolume());
		}
		// The object is in one or more sectors
		else {
			// Iterate the sectors and remove the object from them.
			tRenderContainerDataListIt it = pDataList->begin();
			for (; it != pDataList->end(); ++it) {
				cSector *pSector = static_cast<cSector *>(*it);

				// Log("Checking intersection in sector %s\n",pSector->GetId().c_str());

				cSectorVisibility *pVisSector = mpVisSectorCont->GetSectorVisibilty(pSector);
				if (pVisSector) {
					if (pVisSector->IntersectionBV(apObject->GetBoundingVolume())) {
						// Log("Intersected!\n");
						// Log("-----------------------");
						return true;
					}
				}
			}

			// Log("-----------------------");
			return false;
		}

	}
	/////////////////////////////////////////////////
	// Light is not in shadow, do not do any culling
	else {
		// Log("No shadow, using BV\n");
		return CollidesWithBV(apObject->GetBoundingVolume());
	}
}

//-----------------------------------------------------------------------

bool iLight3D::BeginDraw(cRenderSettings *apRenderSettings, iLowLevelGraphics *apLowLevelGraphics) {
	// Clear Stencil Buffer
	/*		apLowLevelGraphics->SetClearStencilActive(true);
			apLowLevelGraphics->SetClearDepthActive(false);
			apLowLevelGraphics->SetClearColorActive(false);

			apLowLevelGraphics->SetClearStencil(0);

			apLowLevelGraphics->ClearScreen();

			apLowLevelGraphics->SetClearStencilActive(false);
			apLowLevelGraphics->SetClearDepthActive(true);
			apLowLevelGraphics->SetClearColorActive(true);*/

	cRect2l ClipRect;
	bool bVisible = CreateClipRect(ClipRect, apRenderSettings, apLowLevelGraphics);

	if (bVisible) {
		apLowLevelGraphics->SetScissorActive(true);
		apLowLevelGraphics->SetScissorRect(ClipRect);

		if (apRenderSettings->mbLog)
			Log("Cliprect pos: (%d, %d) size: (%d, %d)\n", ClipRect.x, ClipRect.y, ClipRect.w, ClipRect.h);
	} else {
		if (apRenderSettings->mbLog)
			Log("Cliprect entire screen\n");
	}

	//////////////////////////////////////////////////////////
	// Cast shadows
	if (mbCastShadows && apRenderSettings->mShowShadows != eRendererShowShadows_None && apRenderSettings->extrudeProgram) {
		// Get temp index array. (Remove this when the index pool
		//  is implemented.).
		mpIndexArray = apRenderSettings->mpTempIndexArray;

		// Setup for shadow drawing
		apLowLevelGraphics->SetStencilActive(true);
		// Do no set this when debugging.
		apLowLevelGraphics->SetColorWriteActive(false, false, false, false);

		///////////////////////////////////////////////////////////////////////////
		// Clear stencil, since scissor is set this should be fast.
		apLowLevelGraphics->SetClearStencilActive(true);
		apLowLevelGraphics->SetClearDepthActive(false);
		apLowLevelGraphics->SetClearColorActive(false);

		apLowLevelGraphics->SetClearStencil(0);

		apLowLevelGraphics->ClearScreen();

		apLowLevelGraphics->SetClearStencilActive(false);
		apLowLevelGraphics->SetClearDepthActive(true);
		apLowLevelGraphics->SetClearColorActive(true);

		if (apLowLevelGraphics->GetCaps(eGraphicCaps_TwoSideStencil)) {
			apLowLevelGraphics->SetCullActive(false);
		}

		apLowLevelGraphics->SetDepthWriteActive(false);

		// Setup the depth test so that shadow volume is not rendered in front
		// off the normal graphics.
		apLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_Less);

		// Resert the algo (zfail or zpass) used.
		apRenderSettings->mlLastShadowAlgo = 0;

		// Reset this variable so it can be used when rendering shadows.
		apRenderSettings->mbMatrixWasNULL = false;

		// Set the fragment program.
		if (apRenderSettings->extrudeProgram) {
			if (apRenderSettings->mbLog)
				Log("Setting fragment program: '%s'\n",
					apRenderSettings->extrudeProgram->GetName().c_str());
			apRenderSettings->extrudeProgram->Bind();
			apRenderSettings->gpuProgram = apRenderSettings->extrudeProgram;
		}

		// Render shadows
		tCasterCacheSetIt it = m_setDynamicCasters.begin();

		if (apRenderSettings->mShowShadows == eRendererShowShadows_All) {
			it = m_setDynamicCasters.begin();
			for (; it != m_setDynamicCasters.end(); ++it) {
				RenderShadow(*it, apRenderSettings, apLowLevelGraphics);
			}
		}

		it = m_setStaticCasters.begin();
		for (; it != m_setStaticCasters.end(); ++it) {
			RenderShadow(*it, apRenderSettings, apLowLevelGraphics);
		}

		// Make rendering ready for the objects.
		// apLowLevelGraphics->SetStencilTwoSideActive(false);

		apLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_Equal);

		apLowLevelGraphics->SetColorWriteActive(true, true, true, true);
		apLowLevelGraphics->SetCullActive(true);

		apLowLevelGraphics->SetStencil(eStencilFunc_Equal, 0, 0xFF,
									   eStencilOp_Keep, eStencilOp_Keep, eStencilOp_Keep);
	}

	// Reset this var so that the new light properties are set.
	apRenderSettings->mbMatrixWasNULL = false;

	return true;
}

//-----------------------------------------------------------------------

void iLight3D::EndDraw(cRenderSettings *apRenderSettings, iLowLevelGraphics *apLowLevelGraphics) {
	apLowLevelGraphics->SetScissorActive(false);
	apLowLevelGraphics->SetStencilActive(false);
}

//-----------------------------------------------------------------------

void iLight3D::SetFarAttenuation(float afX) {
	mfFarAttenuation = afX;

	mbUpdateBoundingVolume = true;

	// This is so that the render container is updated.
	SetTransformUpdated();
}
//-----------------------------------------------------------------------

void iLight3D::SetNearAttenuation(float afX) {
	mfNearAttenuation = afX;
	if (mfNearAttenuation > mfFarAttenuation)
		SetFarAttenuation(mfNearAttenuation);
}
//-----------------------------------------------------------------------

cVector3f iLight3D::GetLightPosition() {
	return GetWorldPosition();
}

//-----------------------------------------------------------------------

void iLight3D::UpdateLogic(float afTimeStep) {
	UpdateLight(afTimeStep);
	if (mfFadeTime > 0 || mbFlickering) {
		mbUpdateBoundingVolume = true;

		// This is so that the render container is updated.
		SetTransformUpdated();
	}
}

//-----------------------------------------------------------------------

cBoundingVolume *iLight3D::GetBoundingVolume() {
	if (mbUpdateBoundingVolume) {
		UpdateBoundingVolume();
		mbUpdateBoundingVolume = false;
	}

	return &mBoundingVolume;
}

//-----------------------------------------------------------------------

cMatrixf *iLight3D::GetModelMatrix(cCamera3D *apCamera) {
	mtxTemp = GetWorldMatrix();
	return &mtxTemp;
}

//-----------------------------------------------------------------------

bool iLight3D::IsVisible() {
	if (mDiffuseColor.r <= 0 && mDiffuseColor.g <= 0 && mDiffuseColor.b <= 0 && mDiffuseColor.a <= 0)
		return false;
	if (mfFarAttenuation <= 0)
		return false;

	return IsRendered();
}

//-----------------------------------------------------------------------

iTexture *iLight3D::GetFalloffMap() {
	return mpFalloffMap;
}

void iLight3D::SetFalloffMap(iTexture *apTexture) {
	if (mpFalloffMap)
		mpTextureManager->Destroy(mpFalloffMap);

	if (apTexture) {
		mpFalloffMap = apTexture;
		mpFalloffMap->SetWrapS(eTextureWrap_ClampToEdge);
		mpFalloffMap->SetWrapT(eTextureWrap_ClampToEdge);
	} else {
		mpFalloffMap = nullptr;
	}

	Common::fill(mvTempTextures, mvTempTextures + 3, nullptr);
}

//-----------------------------------------------------------------------

void iLight3D::LoadXMLProperties(const tString asFile) {
	tString sPath = mpFileSearcher->GetFilePath(asFile);
	if (sPath != "") {
		TiXmlDocument *pDoc = hplNew(TiXmlDocument, (sPath.c_str()));
		if (pDoc->LoadFile()) {
			TiXmlElement *pRootElem = pDoc->RootElement();

			TiXmlElement *pMainElem = pRootElem->FirstChildElement("MAIN");
			if (pMainElem != NULL) {
				mbCastShadows = cString::ToBool(pMainElem->Attribute("CastsShadows"), mbCastShadows);

				mDiffuseColor.a = cString::ToFloat(pMainElem->Attribute("Specular"), mDiffuseColor.a);

				tString sFalloffImage = cString::ToString(pMainElem->Attribute("FalloffImage"), "");
				iTexture *pTexture = mpTextureManager->Create1D(sFalloffImage, false);
				if (pTexture)
					SetFalloffMap(pTexture);

				ExtraXMLProperties(pMainElem);
			} else {
				Error("Cannot find main element in %s\n", asFile.c_str());
			}
		} else {
			Error("Couldn't load file '%s'\n", asFile.c_str());
		}
		hplDelete(pDoc);
	} else {
		Error("Couldn't find file '%s'\n", asFile.c_str());
	}
}

//-----------------------------------------------------------------------

void iLight3D::AttachBillboard(cBillboard *apBillboard) {
	mvBillboards.push_back(apBillboard);
	apBillboard->SetColor(cColor(mDiffuseColor.r, mDiffuseColor.g, mDiffuseColor.b, 1));
	apBillboard->SetVisible(IsVisible());
}

void iLight3D::RemoveBillboard(cBillboard *apBillboard) {
	Common::Array<cBillboard *>::iterator it = mvBillboards.begin();
	for (; it != mvBillboards.end(); ++it) {
		if (*it == apBillboard) {
			mvBillboards.erase(it);
		}
	}
}

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void iLight3D::RenderShadow(iRenderable *apObject, cRenderSettings *apRenderSettings,
							iLowLevelGraphics *apLowLevelGraphics) {
	int lIndexCount = 0;

	////////////////////////////////////////////////////////////////////////
	// Check if the shadow volume collides with the frustum
	cShadowVolumeBV *pVolume = apObject->GetBoundingVolume()->GetShadowVolume(
		GetWorldPosition(), mfFarAttenuation, true);
	cFrustum *pFrustum = apRenderSettings->mpFrustum;
	if (pVolume && pFrustum) {
		if (pFrustum->CheckVolumeIntersection(pVolume) == false) {
			// This can be skipped if the AABB shadow volume is better.
			/// that is includes the AABB.
			if (pFrustum->CollideBoundingVolume(apObject->GetBoundingVolume()) == eFrustumCollision_Outside) {
				return;
			}
		}
	}

	if (apRenderSettings->mbLog)
		Log("Rendering shadow for '%s'\n", apObject->GetName().c_str());

	cSubMeshEntity *pSubEntity = static_cast<cSubMeshEntity *>(apObject);
	cSubMesh *pSubMesh = pSubEntity->GetSubMesh();

	//////////////////////////////////////////
	// Check what method to use.
	bool bZFail = false;

	if (pVolume && pFrustum) {
		cBoundingVolume *pFrustumBV = pFrustum->GetOriginBV();
		if (cMath::CheckSphereInPlanes(pFrustumBV->GetWorldCenter(), pFrustumBV->GetRadius(),
									   pVolume->mvPlanes, pVolume->mlPlaneCount)) {
			bZFail = true;
		}
		// This is because the AABB-shadow volume is not perfect
		else if (cMath::CheckCollisionBV(*pFrustumBV, *apObject->GetBoundingVolume())) {
			bZFail = true;
		}
	} else {
		bZFail = true;
	}
	if (apRenderSettings->mbLog)
		Log("Rendering shadow with '%s'\n", bZFail ? "ZFail" : "ZPass");

	//////////////////////////////////////////
	// Setup the stencil buffer.
	// If two sided stencil is not supported, do the set up later on.
	if (apLowLevelGraphics->GetCaps(eGraphicCaps_TwoSideStencil)) {
		if (bZFail) {
			if (apRenderSettings->mlLastShadowAlgo != 1) {
				apLowLevelGraphics->SetStencilTwoSide(eStencilFunc_Always, eStencilFunc_Always, 0, 0x00,
													  eStencilOp_Keep, eStencilOp_DecrementWrap, eStencilOp_Keep,
													  eStencilOp_Keep, eStencilOp_IncrementWrap, eStencilOp_Keep);

				apRenderSettings->mlLastShadowAlgo = 1;
			}
		} else {
			if (apRenderSettings->mlLastShadowAlgo != 2) {
				apLowLevelGraphics->SetStencilTwoSide(eStencilFunc_Always, eStencilFunc_Always, 0, 0x00,
													  eStencilOp_Keep, eStencilOp_Keep, eStencilOp_IncrementWrap,
													  eStencilOp_Keep, eStencilOp_Keep, eStencilOp_DecrementWrap);

				apRenderSettings->mlLastShadowAlgo = 2;
			}
		}
	}

	//////////////////////////////////////////
	// Check if the cache as data.
	/* TO BE IMPLEMENTED*/

	///////////////////////////////////////////
	// Get local light position
	cVector3f vLocalLight = GetWorldPosition();
	cMatrixf *pInvModelMtx = apObject->GetInvModelMatrix();
	if (pInvModelMtx) {
		vLocalLight = cMath::MatrixMul(*pInvModelMtx, vLocalLight);
	}

	/////////////////////////////////////////////
	// Set the model matrix
	cMatrixf *pModelMtx = apObject->GetModelMatrix(NULL);
	if (pModelMtx) {
		apLowLevelGraphics->SetMatrix(eMatrix_ModelView, cMath::MatrixMul(
															 apRenderSettings->mpCamera->GetViewMatrix(),
															 *pModelMtx));
	} else if (apRenderSettings->mbMatrixWasNULL == false) {
		apLowLevelGraphics->SetMatrix(eMatrix_ModelView, apRenderSettings->mpCamera->GetViewMatrix());
	}

	/////////////////////////////////////////////////////////
	// Get the data arrays
	const float *pPosArray = pSubEntity->GetVertexBuffer()->GetArray(eVertexFlag_Position);
	unsigned int *pIdxArray = pSubEntity->GetVertexBuffer()->GetIndices();

	int lVtxStride = kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)];

	const bool bDoubleSided = pSubMesh->GetDoubleSided();

	/////////////////////////////////////////////////////////
	// Iterate faces and check which are facing the light.
	cTriangleData *pTriangles = &(*pSubEntity->GetTriangleVecPtr())[0];
	const int lTriNum = pSubEntity->GetTriangleNum();
	for (int tri = 0, idx = 0; tri < lTriNum; tri++, idx += 3) {
		cTriangleData &Tri = pTriangles[tri];

		const float *pPoint = &pPosArray[pIdxArray[idx] * lVtxStride];

		const cVector3f &vNormal = Tri.normal;

		// Use Dot product to check
		Tri.facingLight = ((pPoint[0] - vLocalLight.x) * vNormal.x +
						   (pPoint[1] - vLocalLight.y) * vNormal.y +
						   (pPoint[2] - vLocalLight.z) * vNormal.z) < 0;
	}

	/////////////////////////////////////////////////////////
	// Iterate edges and find possible silhouette
	// Get edge pointer, index pointer and offset
	unsigned int *pCurrentIndexPos = &mpIndexArray[0];
	const cTriEdge *pEdges = &(*pSubMesh->GetEdgeVecPtr())[0];
	int lOffset = pSubEntity->GetVertexBuffer()->GetVertexNum();
	// Iterate
	const int lEdgeNum = pSubMesh->GetEdgeNum();
	for (int edge = 0; edge < lEdgeNum; edge++) {
		const cTriEdge &Edge = pEdges[edge];

		const cTriangleData *pTri1 = &pTriangles[Edge.tri1];
		const cTriangleData *pTri2 = nullptr;
		if (Edge.invert_tri2 == false)
			pTri2 = &pTriangles[Edge.tri2];

		// Check if this edge has one triangle facing and one not facing the light.
		// If the triangel is onesided (invert_tri2) then it is always a silhouette
		if ((Edge.invert_tri2) ||
			(pTri1->facingLight && !pTri2->facingLight) ||
			(pTri2->facingLight && !pTri1->facingLight)) {
			if (pTri1->facingLight) {
				*(pCurrentIndexPos++) = Edge.point1;
				*(pCurrentIndexPos++) = Edge.point2;
				*(pCurrentIndexPos++) = Edge.point2 + lOffset;
				*(pCurrentIndexPos++) = Edge.point1;
				*(pCurrentIndexPos++) = Edge.point2 + lOffset;
				*(pCurrentIndexPos++) = Edge.point1 + lOffset;
				lIndexCount += 6;
			}
			// Do not draw if the edge only has one face and it is not facing the light.
			else if (!Edge.invert_tri2) {
				*(pCurrentIndexPos++) = Edge.point2;
				*(pCurrentIndexPos++) = Edge.point1;
				*(pCurrentIndexPos++) = Edge.point1 + lOffset;
				*(pCurrentIndexPos++) = Edge.point2;
				*(pCurrentIndexPos++) = Edge.point1 + lOffset;
				*(pCurrentIndexPos++) = Edge.point2 + lOffset;
				lIndexCount += 6;
			}

			// DEBUG:
			/*if(!(bDoubleSided && Edge.invert_tri2==false))
			{
				//apRenderSettings->mpVtxExtrudeProgram->UnBind();
				apLowLevelGraphics->SetDepthTestActive(false);
				apLowLevelGraphics->SetStencilActive(false);
				apLowLevelGraphics->DrawLine(
					pSubMesh->GetVertexBuffer()->GetVector3(eVertexFlag_Position,Edge.point1),
					pSubMesh->GetVertexBuffer()->GetVector3(eVertexFlag_Position,Edge.point2),
					cColor(1,0.9f,0,1));
				apLowLevelGraphics->SetStencilActive(true);
				apLowLevelGraphics->SetDepthTestActive(true);
			}*/
		}
	}

	///////////////////////////////////////////////////////
	// If Z fail is used, generate front and back cap
	if (bZFail) {
		for (int tri = 0, idx = 0; tri < lTriNum; tri++, idx += 3) {
			cTriangleData &Data = pSubEntity->GetTriangle(tri);

			// Front cap
			if (Data.facingLight) {
				memcpy(pCurrentIndexPos, &pIdxArray[idx], 3 * sizeof(unsigned int));
				pCurrentIndexPos += 3;
				lIndexCount += 3;
				/*mpIndexArray[lIndexCount+0] = pIdxArray[idx+0];
				mpIndexArray[lIndexCount+1] = pIdxArray[idx+1];
				mpIndexArray[lIndexCount+2] = pIdxArray[idx+2];*/

				if (bDoubleSided) {
					mpIndexArray[lIndexCount + 0] = pIdxArray[idx + 2] + lOffset;
					mpIndexArray[lIndexCount + 1] = pIdxArray[idx + 1] + lOffset;
					mpIndexArray[lIndexCount + 2] = pIdxArray[idx + 0] + lOffset;
					pCurrentIndexPos += 3;
					lIndexCount += 3;
				}
			}
			// Back Cap
			// If double sided, the sides facing the light supply their own
			else if (!bDoubleSided) {
				mpIndexArray[lIndexCount + 0] = pIdxArray[idx + 0] + lOffset;
				mpIndexArray[lIndexCount + 1] = pIdxArray[idx + 1] + lOffset;
				mpIndexArray[lIndexCount + 2] = pIdxArray[idx + 2] + lOffset;
				pCurrentIndexPos += 3;
				lIndexCount += 3;
			}
		}
	}

	///////////////////////////////////////////////////////
	// Draw the volume:

	// Set light position and model view matrix, this does not have to be set if last
	// object was static.
	if (pModelMtx || apRenderSettings->mbMatrixWasNULL == false) {
		apRenderSettings->extrudeProgram->SetVec3f("lightPosition", vLocalLight);
		apRenderSettings->extrudeProgram->SetMatrixf("worldViewProj",
													 eGpuProgramMatrix_ViewProjection,
													 eGpuProgramMatrixOp_Identity);

		// If a null matrix has been set, let other passes know.
		if (pModelMtx)
			apRenderSettings->mbMatrixWasNULL = false;
		else
			apRenderSettings->mbMatrixWasNULL = true;
	}

	// Set vertex buffer
	if (apRenderSettings->mpVtxBuffer != pSubEntity->GetVertexBuffer()) {
		if (apRenderSettings->mbLog)
			Log(" Setting vertex buffer %d\n", (size_t)pSubEntity->GetVertexBuffer());

		pSubEntity->GetVertexBuffer()->Bind();
		apRenderSettings->mpVtxBuffer = pSubEntity->GetVertexBuffer();
	}

	// Draw vertex buffer
	if (apLowLevelGraphics->GetCaps(eGraphicCaps_TwoSideStencil)) {
		pSubEntity->GetVertexBuffer()->DrawIndices(mpIndexArray, lIndexCount);
		if (apRenderSettings->mbLog)
			Log(" Drawing front and back simultaneously.\n");
	} else {
		if (apRenderSettings->mbLog)
			Log(" Drawing front and back separately.\n");

		if (bZFail) {
			// Front
			apLowLevelGraphics->SetStencil(eStencilFunc_Always, 0, 0x0,
										   eStencilOp_Keep, eStencilOp_DecrementWrap, eStencilOp_Keep);
			pSubEntity->GetVertexBuffer()->DrawIndices(mpIndexArray, lIndexCount);

			// Back
			apLowLevelGraphics->SetCullMode(eCullMode_Clockwise);
			apLowLevelGraphics->SetStencil(eStencilFunc_Always, 0, 0x0,
										   eStencilOp_Keep, eStencilOp_IncrementWrap, eStencilOp_Keep);
			pSubEntity->GetVertexBuffer()->DrawIndices(mpIndexArray, lIndexCount);
		} else {
			// Front
			apLowLevelGraphics->SetStencil(eStencilFunc_Always, 0, 0x0,
										   eStencilOp_Keep, eStencilOp_Keep, eStencilOp_IncrementWrap);
			pSubEntity->GetVertexBuffer()->DrawIndices(mpIndexArray, lIndexCount);

			// Back
			apLowLevelGraphics->SetCullMode(eCullMode_Clockwise);
			apLowLevelGraphics->SetStencil(eStencilFunc_Always, 0, 0x0,
										   eStencilOp_Keep, eStencilOp_Keep, eStencilOp_DecrementWrap);
			pSubEntity->GetVertexBuffer()->DrawIndices(mpIndexArray, lIndexCount);
		}

		apLowLevelGraphics->SetCullMode(eCullMode_CounterClockwise);
	}

	if (apLowLevelGraphics->GetCaps(eGraphicCaps_TwoSideStencil)) {
		apLowLevelGraphics->SetStencilTwoSide(false);
		apRenderSettings->mlLastShadowAlgo = 0;
	}
}

//-----------------------------------------------------------------------

void iLight3D::OnFlickerOff() {
	// Particle system
	if (msFlickerOffPS != "") {
		/*cParticleSystem3D *pPS = */ mpWorld3D->CreateParticleSystem(GetName() + "_PS",
																	  msFlickerOffPS, cVector3f(1, 1, 1), GetWorldMatrix());
	}
}

//-----------------------------------------------------------------------

void iLight3D::OnFlickerOn() {
	// Particle system
	if (msFlickerOnPS != "") {
		/*cParticleSystem3D *pPS = */ mpWorld3D->CreateParticleSystem(GetName() + "_PS",
																	  msFlickerOnPS, cVector3f(1, 1, 1), GetWorldMatrix());
	}
}

//-----------------------------------------------------------------------

void iLight3D::OnSetDiffuse() {
	for (size_t i = 0; i < mvBillboards.size(); ++i) {
		cBillboard *pBill = mvBillboards[i];
		pBill->SetColor(cColor(mDiffuseColor.r, mDiffuseColor.g, mDiffuseColor.b, 1));
	}
}

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerializeVirtual(cSaveData_iLight3D, cSaveData_iRenderable)
	kSerializeVar(msFalloffMap, eSerializeType_String)
		kSerializeVarContainer(mlstBillboardIds, eSerializeType_Int32)

			kSerializeVar(mDiffuseColor, eSerializeType_Color)
				kSerializeVar(mSpecularColor, eSerializeType_Color)
					kSerializeVar(mfIntensity, eSerializeType_Float32)
						kSerializeVar(mfFarAttenuation, eSerializeType_Float32)
							kSerializeVar(mfNearAttenuation, eSerializeType_Float32)
								kSerializeVar(mfSourceRadius, eSerializeType_Float32)

									kSerializeVar(mbCastShadows, eSerializeType_Bool)
										kSerializeVar(mbAffectMaterial, eSerializeType_Bool)

											kSerializeVar(mColAdd, eSerializeType_Color)
												kSerializeVar(mfRadiusAdd, eSerializeType_Float32)
													kSerializeVar(mDestCol, eSerializeType_Color)
														kSerializeVar(mfDestRadius, eSerializeType_Float32)
															kSerializeVar(mfFadeTime, eSerializeType_Float32)

																kSerializeVar(mbFlickering, eSerializeType_Bool)
																	kSerializeVar(msFlickerOffSound, eSerializeType_String)
																		kSerializeVar(msFlickerOnSound, eSerializeType_String)
																			kSerializeVar(msFlickerOffPS, eSerializeType_String)
																				kSerializeVar(msFlickerOnPS, eSerializeType_String)
																					kSerializeVar(mfFlickerOnMinLength, eSerializeType_Float32)
																						kSerializeVar(mfFlickerOffMinLength, eSerializeType_Float32)
																							kSerializeVar(mfFlickerOnMaxLength, eSerializeType_Float32)
																								kSerializeVar(mfFlickerOffMaxLength, eSerializeType_Float32)
																									kSerializeVar(mFlickerOffColor, eSerializeType_Color)
																										kSerializeVar(mfFlickerOffRadius, eSerializeType_Float32)
																											kSerializeVar(mbFlickerFade, eSerializeType_Bool)
																												kSerializeVar(mfFlickerOnFadeLength, eSerializeType_Float32)
																													kSerializeVar(mfFlickerOffFadeLength, eSerializeType_Float32)

																														kSerializeVar(mFlickerOnColor, eSerializeType_Color)
																															kSerializeVar(mfFlickerOnRadius, eSerializeType_Float32)

																																kSerializeVar(mbFlickerOn, eSerializeType_Bool)
																																	kSerializeVar(mfFlickerTime, eSerializeType_Float32)
																																		kSerializeVar(mfFlickerStateLength, eSerializeType_Float32)
																																			kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveData *iLight3D::CreateSaveData() {
	return NULL;
}

//-----------------------------------------------------------------------

void iLight3D::SaveToSaveData(iSaveData *apSaveData) {
	kSaveData_SaveToBegin(iLight3D);

	//////////////////////////
	// Data
	pData->msFalloffMap = mpFalloffMap == NULL ? "" : mpFalloffMap->GetName();

	pData->mlstBillboardIds.Clear();
	for (size_t i = 0; i < mvBillboards.size(); ++i) {
		pData->mlstBillboardIds.Add(mvBillboards[i]->GetSaveObjectId());
	}

	//////////////////////////
	// Variables
	kSaveData_SaveTo(mDiffuseColor);
	kSaveData_SaveTo(mSpecularColor);
	kSaveData_SaveTo(mfIntensity);
	kSaveData_SaveTo(mfFarAttenuation);
	kSaveData_SaveTo(mfNearAttenuation);
	kSaveData_SaveTo(mfSourceRadius);

	kSaveData_SaveTo(mbCastShadows);
	kSaveData_SaveTo(mbAffectMaterial);

	kSaveData_SaveTo(mColAdd);
	kSaveData_SaveTo(mfRadiusAdd);
	kSaveData_SaveTo(mDestCol);
	kSaveData_SaveTo(mfDestRadius);
	kSaveData_SaveTo(mfFadeTime);

	kSaveData_SaveTo(mbFlickering);
	kSaveData_SaveTo(msFlickerOffSound);
	kSaveData_SaveTo(msFlickerOnSound);
	kSaveData_SaveTo(msFlickerOffPS);
	kSaveData_SaveTo(msFlickerOnPS);
	kSaveData_SaveTo(mfFlickerOnMinLength);
	kSaveData_SaveTo(mfFlickerOffMinLength);
	kSaveData_SaveTo(mfFlickerOnMaxLength);
	kSaveData_SaveTo(mfFlickerOffMaxLength);
	kSaveData_SaveTo(mFlickerOffColor);
	kSaveData_SaveTo(mfFlickerOffRadius);
	kSaveData_SaveTo(mbFlickerFade);
	kSaveData_SaveTo(mfFlickerOnFadeLength);
	kSaveData_SaveTo(mfFlickerOffFadeLength);

	kSaveData_SaveTo(mFlickerOnColor);
	kSaveData_SaveTo(mfFlickerOnRadius);

	kSaveData_SaveTo(mbFlickerOn);
	kSaveData_SaveTo(mfFlickerTime);
	kSaveData_SaveTo(mfFlickerStateLength);
}

//-----------------------------------------------------------------------

void iLight3D::LoadFromSaveData(iSaveData *apSaveData) {
	kSaveData_LoadFromBegin(iLight3D);

	//////////////////////////
	// Data
	if (pData->msFalloffMap != "") {
		iTexture *pTex = mpTextureManager->Create1D(pData->msFalloffMap, false);
		if (pTex)
			SetFalloffMap(pTex);
	}

	//////////////////////////
	// Variables
	kSaveData_LoadFrom(mDiffuseColor);
	kSaveData_LoadFrom(mSpecularColor);
	kSaveData_LoadFrom(mfIntensity);
	kSaveData_LoadFrom(mfFarAttenuation);
	kSaveData_LoadFrom(mfNearAttenuation);
	kSaveData_LoadFrom(mfSourceRadius);

	kSaveData_LoadFrom(mbCastShadows);
	kSaveData_LoadFrom(mbAffectMaterial);

	kSaveData_LoadFrom(mColAdd);
	kSaveData_LoadFrom(mfRadiusAdd);
	kSaveData_LoadFrom(mDestCol);
	kSaveData_LoadFrom(mfDestRadius);
	kSaveData_LoadFrom(mfFadeTime);

	kSaveData_LoadFrom(mbFlickering);
	kSaveData_LoadFrom(msFlickerOffSound);
	kSaveData_LoadFrom(msFlickerOnSound);
	kSaveData_LoadFrom(msFlickerOffPS);
	kSaveData_LoadFrom(msFlickerOnPS);
	kSaveData_LoadFrom(mfFlickerOnMinLength);
	kSaveData_LoadFrom(mfFlickerOffMinLength);
	kSaveData_LoadFrom(mfFlickerOnMaxLength);
	kSaveData_LoadFrom(mfFlickerOffMaxLength);
	kSaveData_LoadFrom(mFlickerOffColor);
	kSaveData_LoadFrom(mfFlickerOffRadius);
	kSaveData_LoadFrom(mbFlickerFade);
	kSaveData_LoadFrom(mfFlickerOnFadeLength);
	kSaveData_LoadFrom(mfFlickerOffFadeLength);

	kSaveData_LoadFrom(mFlickerOnColor);
	kSaveData_LoadFrom(mfFlickerOnRadius);

	kSaveData_LoadFrom(mbFlickerOn);
	kSaveData_LoadFrom(mfFlickerTime);
	kSaveData_LoadFrom(mfFlickerStateLength);
}

//-----------------------------------------------------------------------

void iLight3D::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	kSaveData_SetupBegin(iLight3D);

	// Get attached billboards.
	cContainerListIterator<int> it = pData->mlstBillboardIds.GetIterator();
	while (it.HasNext()) {
		int lId = it.Next();
		iSaveObject *pObject = apSaveObjectHandler->Get(lId);
		cBillboard *pBill = static_cast<cBillboard *>(pObject);

		if (pBill == NULL) {
			Error("Couldn't find billboard id %s\n", lId);
			continue;
		}

		AttachBillboard(pBill);
	}
}

//-----------------------------------------------------------------------

} // namespace hpl
