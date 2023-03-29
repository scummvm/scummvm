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

#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/graphics/GraphicsDrawer.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/Mesh2d.h"
#include "hpl1/engine/graphics/RenderObject2D.h"
#include "hpl1/engine/graphics/Texture.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/resources/low_level_resources.h"
#include "hpl1/engine/scene/Camera2D.h"
#include "hpl1/engine/scene/GridMap2D.h"
#include "hpl1/engine/scene/Light2D.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/TileData.h"
#include "hpl1/engine/scene/TileMap.h"
#include "hpl1/engine/scene/World2D.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cRenderer2D::cRenderer2D(iLowLevelGraphics *apLowLevelGraphics, cResources *apResources, cGraphicsDrawer *apGraphicsDrawer) {
	Log("  Creating Renderer2D\n");

	mpLowLevelGraphics = apLowLevelGraphics;
	mpLowLevelResources = apResources->GetLowLevel();
	mpResources = apResources;
	mpGraphicsDrawer = apGraphicsDrawer;

	/// LIGHTING SETUP
	mpLightMap[0] = mpResources->GetTextureManager()->Create2D("PointLight2D.bmp", false);
	if (mpLightMap[0] == NULL)
		error("Couldn't load PointLight2D");

	mpLightMap[0]->SetWrapS(eTextureWrap_ClampToEdge);
	mpLightMap[0]->SetWrapT(eTextureWrap_ClampToEdge);

	mpLightMap[1] = NULL;
	mPrevLightRect = cRect2f(0, 0, -1, -1);
	mAmbientLight = cColor(0.0f, 0.0f, 0.0f, 0);
	mfShadowZ = 9.5f;

	Log("  Renderer2D created\n");
}

//-----------------------------------------------------------------------

cRenderer2D::~cRenderer2D() {
	for (int i = 0; i < 2; i++) {
		if (mpLightMap[i]) {
			mpResources->GetTextureManager()->Destroy(mpLightMap[i]);
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
////////////////////// RENDER OBJECT COMPARE ////////////////////////////
bool cRenderObject2DCompare::operator()(const cRenderObject2D &aObjectA, const cRenderObject2D &aObjectB) const {
	if (aObjectA.GetMaterial()->GetTexture(eMaterialTexture_Diffuse) !=
		aObjectB.GetMaterial()->GetTexture(eMaterialTexture_Diffuse)) {
		return aObjectA.GetMaterial()->GetTexture(eMaterialTexture_Diffuse) >
			   aObjectB.GetMaterial()->GetTexture(eMaterialTexture_Diffuse);
	} else if (aObjectA.GetMaterial()->GetType(eMaterialRenderType_Diffuse) !=
			   aObjectB.GetMaterial()->GetType(eMaterialRenderType_Diffuse)) {
		return aObjectA.GetMaterial()->GetType(eMaterialRenderType_Diffuse) >
			   aObjectB.GetMaterial()->GetType(eMaterialRenderType_Diffuse);
	}
	/*else if(aObjectA.GetMaterial()->GetType(eMaterialRenderType_Light) !=
	aObjectB.GetMaterial()->GetType(eMaterialRenderType_Light))
	{
	return aObjectA.GetMaterial()->GetType(eMaterialRenderType_Light) >
	aObjectB.GetMaterial()->GetType(eMaterialRenderType_Light);
	}*/
	/*else if(Some other thing to sort by)*/
	/*Sort by Z type aswell!*/

	return false;
}

////////////////////// TRANS RENDER OBJECT COMPARE ////////////////////////////

bool cRenderTransObjectCompare::operator()(const cRenderObject2D &aObjectA, const cRenderObject2D &aObjectB) const {
	if (aObjectA.GetZ() != aObjectB.GetZ()) {
		return aObjectA.GetZ() < aObjectB.GetZ();
	} else if (aObjectA.GetMaterial()->GetTexture(eMaterialTexture_Diffuse) !=
			   aObjectB.GetMaterial()->GetTexture(eMaterialTexture_Diffuse)) {
		return aObjectA.GetMaterial()->GetTexture(eMaterialTexture_Diffuse) >
			   aObjectB.GetMaterial()->GetTexture(eMaterialTexture_Diffuse);
	} else if (aObjectA.GetMaterial() != aObjectB.GetMaterial()) {
		return aObjectA.GetMaterial() > aObjectB.GetMaterial();
	} else {
	}

	return false;
}
//-----------------------------------------------------------------------

void cRenderer2D::RenderObject(const cRenderObject2D &aObject, unsigned int &aIdxAdd, iMaterial *pMat,
							   iLight2D *pLight, eMaterialRenderType aRenderType, cCamera2D *apCam) {
	if (aObject.GetCustomRenderer()) {

		aObject.GetCustomRenderer()->RenderToBatch(aRenderType, aIdxAdd);
		return;
	}

	int i;
	tVertexVec *pVtxVec = aObject.GetVertexVec();
	tUIntVec *pIdxVec = aObject.GetIndexVec();
	cVector3f *pTransform = aObject.GetTransform();
	// bool bUsesLights = aObject.GetMaterial()->UsesLights();

	// pMat->EditVertexes(aRenderType,apCam,pLight,pVtxVec,pTransform,aIdxAdd);

	if (pTransform == NULL) {
		for (i = 0; i < (int)pVtxVec->size(); i++)
			mpLowLevelGraphics->AddVertexToBatch((*pVtxVec)[i]);
	} else {
		for (i = 0; i < (int)pVtxVec->size(); i++)
			mpLowLevelGraphics->AddVertexToBatch(&(*pVtxVec)[i], pTransform);
	}

	for (i = 0; i < (int)pIdxVec->size(); i++) {
		mpLowLevelGraphics->AddIndexToBatch((*pIdxVec)[i] + aIdxAdd);
	}

	aIdxAdd += (unsigned int)pVtxVec->size();
}

//-----------------------------------------------------------------------

void cRenderer2D::RenderObjects(cCamera2D *apCamera, cGridMap2D *apMapLights, cWorld2D *apWorld) {
	iMaterial *pMat = NULL;
	iMaterial *pPrevMat = NULL;
	cRect2f ClipRect;
	unsigned int lIdxAdd = 0;

	apCamera->GetClipRect(ClipRect);

	// Setup the screen
	mpLowLevelGraphics->SetClearColor(mAmbientLight);
	mpLowLevelGraphics->ClearScreen();
	apCamera->SetModelViewMatrix(mpLowLevelGraphics);
	apCamera->SetProjectionMatrix(mpLowLevelGraphics);

	////// BEGIN SET UP LIGHTS /////////////

	iGridMap2DIt *pLightIt = apMapLights->GetRectIterator(ClipRect);

	while (pLightIt->HasNext()) {
		iLight2D *pLight = static_cast<iLight2D *>(pLightIt->Next());

		if (pLight->GetDiffuseColor().r == 0 && pLight->GetDiffuseColor().g == 0 &&
			pLight->GetDiffuseColor().b == 0) {
			continue;
		}

		if (pLight->GetAffectMaterial()) {
			mlstLights.push_back(pLight);
		} else {
			mlstFastLights.push_back(pLight);
		}
	}

	hplDelete(pLightIt);

	////// END SET UP LIGHTS /////////////

	/*int lCount=0;
	tRenderObjectSetIt TestIt = m_mapObject.begin();
	while(TestIt != m_mapObject.end())
	{
		iMaterial *pMat = TestIt->GetMaterial();
		Log("%d Tex: %d Light: %d Diff: %d\n", lCount, (int)pMat->GetTexture(eMaterialTexture_Diffuse),
											(int)pMat->GetType(eMaterialRenderType_Light),
											(int)pMat->GetType(eMaterialRenderType_Diffuse)
											);
		TestIt++;
		lCount++;
	}*/

	////// BEGIN RENDER ZBUFFER ////////////

	mpLowLevelGraphics->SetDepthWriteActive(true);
	mpLowLevelGraphics->SetColorWriteActive(false, false, false, false);
	mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_LessOrEqual);

	tRenderObjectSetIt ObjectIt = m_mapObject.begin();
	if (ObjectIt != m_mapObject.end())
		pMat = ObjectIt->GetMaterial();

	while (ObjectIt != m_mapObject.end()) {
		if (pMat->StartRendering(eMaterialRenderType_Z, apCamera, NULL) == false) {
			ObjectIt++; // ObjectIt = m_mapObject.erase(ObjectIt);?
			if (ObjectIt != m_mapObject.end())
				pMat = ObjectIt->GetMaterial();
			continue;
		}

		do {
			RenderObject(*ObjectIt, lIdxAdd, pMat, NULL, eMaterialRenderType_Z, apCamera);
			pPrevMat = pMat;

			ObjectIt++;

			if (ObjectIt == m_mapObject.end()) {
				pMat = NULL;
				break;
			} else {
				pMat = ObjectIt->GetMaterial();
			}
		} while (pMat->GetType(eMaterialRenderType_Z) ==
					 pPrevMat->GetType(eMaterialRenderType_Z) &&
				 pMat->GetTexture(eMaterialTexture_Diffuse) ==
					 pPrevMat->GetTexture(eMaterialTexture_Diffuse));

		lIdxAdd = 0;

		do {
			mpLowLevelGraphics->FlushTriBatch(pPrevMat->GetBatchFlags(eMaterialRenderType_Z), false);
		} while (pPrevMat->NextPass(eMaterialRenderType_Z));

		mpLowLevelGraphics->ClearBatch();

		pPrevMat->EndRendering(eMaterialRenderType_Z);
	}

	////// BEGIN RENDER ZBUFFER ////////////

	////// BEGIN RENDER LIGHTS////////////

	mpLowLevelGraphics->SetDepthWriteActive(false);
	mpLowLevelGraphics->SetColorWriteActive(true, true, true, true);

	// Iterate the lights:
	tLightListIt LightIt = mlstLights.begin();

	while (LightIt != mlstLights.end()) {
		if ((*LightIt)->IsActive() == false) {
			LightIt++;
			continue;
		}

		if ((*LightIt)->GetCastShadows())
			if (RenderShadows(apCamera, *LightIt, apWorld) == false) {
				LightIt++;
				continue;
			}

		mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_Equal);
		// Set up stencil so shaodws are not drawn
		mpLowLevelGraphics->SetStencilActive(true);
		mpLowLevelGraphics->SetStencil(eStencilFunc_NotEqual, 0x1, 0x1,
									   eStencilOp_Keep, eStencilOp_Keep, eStencilOp_Keep);

		///////////////////////////
		// Set up scissortest:
		///////////////////////////
		/*mpLowLevelGraphics->SetScissorActive(true);

		cRect2f LightBB = (*LightIt)->GetBoundingBox();
		cRect2l LightRect(	(int)floor( ( (LightBB.x - ClipRect.x)/ClipRect.w) * mpLowLevelGraphics->GetScreenSize().x),
			(int)floor( ( (LightBB.y - ClipRect.y)/ClipRect.h) * mpLowLevelGraphics->GetScreenSize().y),
			(int)((LightBB.w/ClipRect.w)*mpLowLevelGraphics->GetScreenSize().x),
			(int)((LightBB.h/ClipRect.h)*mpLowLevelGraphics->GetScreenSize().y)
			);
		if(LightRect.x<0){
			LightRect.w += LightRect.x;
			LightRect.x =0;
		}
		if(LightRect.y<0){
			LightRect.h += LightRect.y;
			LightRect.y =0;
		}
		if(LightRect.x + LightRect.w >= mpLowLevelGraphics->GetScreenSize().x)
			LightRect.w = (int)mpLowLevelGraphics->GetScreenSize().x - LightRect.x;
		if(LightRect.y + LightRect.h >= mpLowLevelGraphics->GetScreenSize().y)
			LightRect.h = (int)mpLowLevelGraphics->GetScreenSize().y - LightRect.y;

		mpLowLevelGraphics->SetScissorRect(LightRect);*/

		/////////////////////////////////
		// Draw objects the light touches
		/////////////////////////////////
		ObjectIt = m_mapObject.begin();
		if (ObjectIt != m_mapObject.end())
			pMat = ObjectIt->GetMaterial();

		while (ObjectIt != m_mapObject.end()) {
			// If light does not touch this object, don't draw it.
			if (cMath::BoxCollision((*LightIt)->GetBoundingBox(), ObjectIt->GetRect()) == false) {
				ObjectIt++;
				if (ObjectIt != m_mapObject.end())
					pMat = ObjectIt->GetMaterial();
				continue;
			}

			if (pMat->StartRendering(eMaterialRenderType_Light, apCamera, *LightIt) == false) {
				ObjectIt++;
				if (ObjectIt != m_mapObject.end())
					pMat = ObjectIt->GetMaterial();

				continue;
			}

			do {
				RenderObject(*ObjectIt, lIdxAdd, pMat, *LightIt, eMaterialRenderType_Light, apCamera);
				pPrevMat = pMat;

				do {
					ObjectIt++;
				} while (cMath::BoxCollision((*LightIt)->GetBoundingBox(), ObjectIt->GetRect()) == false && ObjectIt != m_mapObject.end());

				if (ObjectIt == m_mapObject.end()) {
					pMat = NULL;
					break;
				} else {
					pMat = ObjectIt->GetMaterial();
				}
			} while (pMat->GetType(eMaterialRenderType_Light) ==
						 pPrevMat->GetType(eMaterialRenderType_Light) &&
					 pMat->GetTexture(eMaterialTexture_Diffuse) ==
						 pPrevMat->GetTexture(eMaterialTexture_Diffuse));

			lIdxAdd = 0;

			do {
				mpLowLevelGraphics->FlushTriBatch(pPrevMat->GetBatchFlags(eMaterialRenderType_Light), false);
			} while (pPrevMat->NextPass(eMaterialRenderType_Light));

			mpLowLevelGraphics->ClearBatch();

			pPrevMat->EndRendering(eMaterialRenderType_Light);
		}

		// mpLowLevelGraphics->SetScissorActive(false);

		ClearShadows();

		mpLowLevelGraphics->SetStencilActive(false);

		LightIt++;
	}

	////// END RENDER LIGHTS ////////////

	////// BEGIN RENDER FAST LIGHTS ////////////

	/*LightIt = mlstFastLights.begin();

	while(LightIt != mlstFastLights.end())
	{
		lIdxAdd = (*LightIt)->Render(mpLowLevelGraphics,lIdxAdd);
		LightIt++;
	}

	mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_LessOrEqual);
	mpLowLevelGraphics->SetBlendActive(true);
	mpLowLevelGraphics->SetBlendFunc(eBlendFunc_One,eBlendFunc_One);

	mpLowLevelGraphics->FlushTriBatch(eVtxBatchFlag_Position | eVtxBatchFlag_Color0,true);*/

	mpLowLevelGraphics->SetBlendActive(false);

	lIdxAdd = 0;

	////// END RENDER FAST LIGHTS ////////////

	////// BEGIN RENDER DIFFUSE ////////////
	mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_Equal);

	ObjectIt = m_mapObject.begin();
	if (ObjectIt != m_mapObject.end())
		pMat = ObjectIt->GetMaterial();

	while (ObjectIt != m_mapObject.end()) {
		if (pMat->StartRendering(eMaterialRenderType_Diffuse, apCamera, NULL) == false) {
			ObjectIt++; // ObjectIt = m_mapObject.erase(ObjectIt);?
			if (ObjectIt != m_mapObject.end())
				pMat = ObjectIt->GetMaterial();

			continue;
		}

		do {
			RenderObject(*ObjectIt, lIdxAdd, pMat, NULL, eMaterialRenderType_Diffuse, apCamera);
			pPrevMat = pMat;

			ObjectIt++; // ObjectIt = m_mapObject.erase(ObjectIt);?

			// if(ObjectIt == m_mapObject.end()) pMat=NULL;
			// else pMat = ObjectIt->GetMaterial();

			if (ObjectIt == m_mapObject.end()) {
				pMat = NULL;
				break;
			} else {
				pMat = ObjectIt->GetMaterial();
			}
		}
		// while(pMat == pPrevMat);
		while (pMat->GetType(eMaterialRenderType_Diffuse) ==
				   pPrevMat->GetType(eMaterialRenderType_Diffuse) &&
			   pMat->GetTexture(eMaterialTexture_Diffuse) ==
				   pPrevMat->GetTexture(eMaterialTexture_Diffuse));

		lIdxAdd = 0;

		do {
			mpLowLevelGraphics->FlushTriBatch(pPrevMat->GetBatchFlags(eMaterialRenderType_Diffuse), false);
		} while (pPrevMat->NextPass(eMaterialRenderType_Diffuse));

		mpLowLevelGraphics->ClearBatch();

		pPrevMat->EndRendering(eMaterialRenderType_Diffuse);
	}

	////// END RENDER DIFFUSE ////////////

	////// BEGIN RENDER BACKGROUND ////////////

	cRect2f TempRect;
	apCamera->GetClipRect(TempRect);

	mpGraphicsDrawer->DrawBackgrounds(TempRect);

	////// EMD RENDER BACKGROUND ////////////

	////// BEGIN RENDER TRANS ////////////
	apCamera->SetModelViewMatrix(mpLowLevelGraphics);
	mpLowLevelGraphics->SetDepthWriteActive(false);
	mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_LessOrEqual);

	tRenderTransObjectSetIt TransIt = m_mapTransObject.begin();

	if (TransIt != m_mapTransObject.end())
		pMat = TransIt->GetMaterial();
	lIdxAdd = 0;
	while (TransIt != m_mapTransObject.end()) {
		if (pMat->StartRendering(eMaterialRenderType_Diffuse, apCamera, NULL) == false) {
			TransIt++; // TransIt = mapTransObject.erase(TransIt);?
			if (TransIt != m_mapTransObject.end())
				pMat = TransIt->GetMaterial();

			continue;
		}

		do {
			RenderObject(*TransIt, lIdxAdd, pMat, NULL, eMaterialRenderType_Diffuse, apCamera);
			pPrevMat = pMat;

			TransIt++; // TransIt = mapTransObject.erase(TransIt);?

			if (TransIt == m_mapTransObject.end())
				pMat = NULL;
			else
				pMat = TransIt->GetMaterial();
		} while (pMat == pPrevMat);
		// while(pMat->GetType() == pPrevMat->GetType()); //better right?

		lIdxAdd = 0;

		do {
			mpLowLevelGraphics->FlushTriBatch(pPrevMat->GetBatchFlags(eMaterialRenderType_Diffuse), false);
		} while (pPrevMat->NextPass(eMaterialRenderType_Diffuse));

		mpLowLevelGraphics->ClearBatch();

		pPrevMat->EndRendering(eMaterialRenderType_Diffuse);
	}

	////// END RENDER TRANS ////////////

	////// BEGIN CLEAN UP ////////////

	mpLowLevelGraphics->SetStencilActive(false);
	mpLowLevelGraphics->SetDepthWriteActive(true);
	mpLowLevelGraphics->SetColorWriteActive(true, true, true, true);
	mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_LessOrEqual);

	mlstLights.clear();
	mlstFastLights.clear();
	m_mapObject.clear();
	m_mapTransObject.clear();

	////// END CLEAN UP ////////////
}

//-----------------------------------------------------------------------

void cRenderer2D::AddObject(cRenderObject2D &aObject) {
	if (aObject.GetMaterial()->IsTransperant()) {
		// Todo non glowing trans thing need specieal treatment.
		if (aObject.GetMaterial()->IsGlowing()) {
			m_mapTransObject.insert(aObject);
		}
	} else {
		m_mapObject.insert(aObject);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cRenderer2D::ClearShadows() {
	// Todo: if the light almost covers the whole screen, it is faster to clear!

	mpLowLevelGraphics->SetStencilActive(true);
	mpLowLevelGraphics->SetDepthWriteActive(false);
	mpLowLevelGraphics->SetDepthTestActive(false);
	mpLowLevelGraphics->SetColorWriteActive(false, false, false, false); // just draw to stencil
	mpLowLevelGraphics->SetTexture(0, NULL);

	mpLowLevelGraphics->SetStencil(eStencilFunc_Always, 0x0, 0x0,
								   eStencilOp_Replace, eStencilOp_Replace, eStencilOp_Replace);

	mpLowLevelGraphics->DrawFilledRect2D(mPrevLightRect, 10, cColor(0, 0, 0, 0));

	mpLowLevelGraphics->SetDepthWriteActive(true);
	mpLowLevelGraphics->SetDepthTestActive(true);
	mpLowLevelGraphics->SetColorWriteActive(true, true, true, true); // just draw to stencil
	mpLowLevelGraphics->SetStencilActive(false);
}
//-----------------------------------------------------------------------

/**
 * This function renders the lightmap that is drawn on the normal gfx in a later pass
 */
bool cRenderer2D::RenderShadows(cCamera2D *apCamera, iLight2D *pLight, cWorld2D *apWorld) {
	cRect2f ClipRect;
	int lFirstIndex = 0;

	apCamera->GetClipRect(ClipRect);

	mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_LessOrEqual);
	mpLowLevelGraphics->SetDepthWriteActive(false);
	mpLowLevelGraphics->SetColorWriteActive(false, false, false, false); // just draw to stencil
	mpLowLevelGraphics->SetTexture(0, NULL);

	// Set the stencil buffer so above shadows don't get affected by em.
	mpLowLevelGraphics->SetStencilActive(true);

	// Not needed to clear since that is made in the GL setup, right?
	/*if(mPrevLightRect.h<0)
	{
		mpLowLevelGraphics->SetClearColorActive(false);	mpLowLevelGraphics->SetClearDepthActive(false);
		mpLowLevelGraphics->SetClearStencilActive(true);
		mpLowLevelGraphics->SetClearStencil(0);
		mpLowLevelGraphics->ClearScreen();
		mpLowLevelGraphics->SetClearStencilActive(true);
		mpLowLevelGraphics->SetClearColorActive(true);mpLowLevelGraphics->SetClearDepthActive(true);
		mpLowLevelGraphics->SetClearStencilActive(false);
	}*/

	mpLowLevelGraphics->SetStencil(eStencilFunc_Always, 0x1, 0x1,
								   eStencilOp_Keep, eStencilOp_Keep, eStencilOp_Replace);

	apCamera->SetModelViewMatrix(mpLowLevelGraphics);

	//// RENDER SHADOW VARS
	cColor ShadowColor = cColor(0, 0, 0, 0);
	float fTileSize = apWorld->GetTileMap()->GetTileSize();
	float fHalfTileSize = fTileSize / 2;

	/// BEGIN CHECK LIGHT VISIBLE //////////////

	float fSourceSize = pLight->GetSourceRadius();
	cVector3f vPos = pLight->GetWorldPosition();
	cTile *pTile = apWorld->GetTileMap()->GetWorldTile(cVector2f(vPos.x, vPos.y),
													   apWorld->GetTileMap()->GetShadowLayer());

	if (pTile) {
		cTileDataNormal *pData = static_cast<cTileDataNormal *>(pTile->GetTileData());

		if (pData->GetCollideMesh()) {
			if (pData->GetCollideMesh()->PointIsInside(cVector2f(vPos.x, vPos.y),
													   cVector2f(pTile->GetPosition().x, pTile->GetPosition().y),
													   pTile->GetAngle())) {
				mpLowLevelGraphics->SetColorWriteActive(true, true, true, true);
				mpLowLevelGraphics->SetStencilActive(false);
				mpLowLevelGraphics->SetDepthWriteActive(true);
				return false;
			}
		}
	}

	cRect2f LightRect = pLight->GetBoundingBox();

	/// END CHECK LIGHT VISIBLE //////////////

	/// BEGIN CAST SHADOWS //////////////

	if (pLight->GetCastShadows()) {
		lFirstIndex = 0; // Begin a new batch!

		// Get tile iterator
		iTileMapIt *pTileIt = apWorld->GetTileMap()->GetRectIterator(pLight->GetBoundingBox(),
																	 apWorld->GetTileMap()->GetShadowLayer());

		// Iterate all tiles
		while (pTileIt->HasNext()) {
			cTile *pTile2 = pTileIt->Next();
			cTileDataNormal *pData = static_cast<cTileDataNormal *>(pTile2->GetTileData());

			if (pData->GetCollideMesh() == NULL)
				continue;

			bool vbTileInDir[4];

			if (pData->IsSolid()) {
				int lTileDirNum = apWorld->GetTileMap()->GetTileNeighbours4Dir(
					pTileIt->GetNum(),
					apWorld->GetTileMap()->GetShadowLayer(), &vbTileInDir[0]);
				if (lTileDirNum >= 4) {
					continue;
				}
			}

			tMesh2DEdgeVec *pEdgeVec = pData->GetCollideMesh()->GetEdgeVec(pTile2->GetAngle());
			tVertexVec *pVtxVec = pData->GetCollideVertexVec(pTile2->GetAngle());

			cVector2f vTilePos = cVector2f(pTile2->GetPosition().x, pTile2->GetPosition().y);
			cVector2f vLightPos = cVector2f(pLight->GetWorldPosition().x, pLight->GetWorldPosition().y);
			float fRadius = pLight->GetFarAttenuation();
			bool bNonFit = false;

			// See if the tile fits in the light rect. If not some shadows need clipping
			//(Clipping is made in the CreateVertexes() )
			if (cMath::BoxFit(cRect2f(vTilePos.x - fHalfTileSize, vTilePos.y - fHalfTileSize,
									  fTileSize, fTileSize),
							  LightRect) == false) {
				bNonFit = true;
			}

			// Find points (edges) that cast shadows
			FindShadowPoints(pEdgeVec, vLightPos, vTilePos);

			// Add the shadows to the vertex batch
			lFirstIndex = CreateVertexes(vLightPos, LightRect, fRadius, bNonFit, vTilePos, pVtxVec,
										 ShadowColor, lFirstIndex, fSourceSize);
		}

		hplDelete(pTileIt);

		// Draw the shadows to the stencil buffer
		mpLowLevelGraphics->FlushTriBatch(eVtxBatchFlag_Color0 |
										  eVtxBatchFlag_Position);

		lFirstIndex = 0;
	}

	/// END CAST SHADOWS //////////////

	mPrevLightRect = pLight->GetBoundingBox();

	mpLowLevelGraphics->SetColorWriteActive(true, true, true, true);
	mpLowLevelGraphics->SetStencilActive(false);
	mpLowLevelGraphics->SetDepthWriteActive(true);

	return true;
}

//-----------------------------------------------------------------------

cVector2f cRenderer2D::CalcLineEnd(cVector3f avLight, cVector3f avPoint, float afRadius,
								   cVector2f &avSide, cVector2f avClipPos) {
	cVector2f vEndPos;
	cVector2f vSize(afRadius);
	float dX = avPoint.x - avLight.x;
	float dY = avPoint.y - avLight.y;
	float kY, kX;

	if (dX == 0)
		kY = 100000;
	else
		kY = dY / dX;
	if (dY == 0)
		kX = 100000;
	else
		kX = dX / dY;

	if (ABS(dX) <= ABS(dY) && dY > 0) {
		vEndPos.y = avClipPos.y + vSize.y;
		float A = avPoint.y - kY * avPoint.x;
		vEndPos.x = (vEndPos.y - A) / kY;
		avSide.y = 1;
	} else if (ABS(dX) <= ABS(dY) && dY <= 0) {
		vEndPos.y = avClipPos.y - vSize.y;
		float A = avPoint.y - kY * avPoint.x;
		vEndPos.x = (vEndPos.y - A) / kY;
		avSide.y = -1;
	} else if (ABS(dX) > ABS(dY) && dX > 0) {
		vEndPos.x = avClipPos.x + vSize.y;
		float A = avPoint.x - kX * avPoint.y;
		vEndPos.y = (vEndPos.x - A) / kX;
		avSide.x = 1;
	} else if (ABS(dX) > ABS(dY) && dX <= 0) {
		vEndPos.x = avClipPos.x - vSize.y;
		float A = avPoint.x - kX * avPoint.y;
		vEndPos.y = (vEndPos.x - A) / kX;
		avSide.x = -1;
	}
	return vEndPos;
}

//-----------------------------------------------------------------------

void cRenderer2D::FindShadowPoints(tMesh2DEdgeVec *apEdgeVec, cVector2f avLightPos, cVector2f avTilePos) {
	cVector2f vEdgeNormal;
	cVector2f vLightNormal;

	// int lPrevWasShadow = -1;
	// int lFirstWasShadow = -1;
	mlShadowPointSize = 0;
	// int lMaxEdge = (int)apEdgeVec->size() - 1;

	for (int i = 0; i < (int)apEdgeVec->size(); i++) {
		int point = (*apEdgeVec)[i].mlStartIndex;
		int next = (*apEdgeVec)[i].mlEndIndex;

		vEdgeNormal = (*apEdgeVec)[i].mvNormal;
		vLightNormal = avLightPos - avTilePos - (*apEdgeVec)[i].mvMidPos;

		float fDot = vEdgeNormal.x * vLightNormal.x + vEdgeNormal.y * vLightNormal.y;

		if (fDot < 0) {
			mvShadowPoints[mlShadowPointSize][0] = point;
			mvShadowPoints[mlShadowPointSize][1] = next;
			mlShadowPointSize++;
		}
	}
}

//-----------------------------------------------------------------------

int cRenderer2D::CreateVertexes(cVector2f vLightPos, cRect2f LightRect, float fRadius, bool bNonFit,
								cVector2f vTilePos, tVertexVec *apVtxVec, cColor ShadowColor, int lFirstIndex, float fSourceSize) {
	// int lNum = 0;

	// Walk through the edges
	for (int idx = 0; idx < mlShadowPointSize; idx++) {
		cVector3f vPointPos[2];
		cVector3f vEndPos[2];
		cVector3f vExtraPos;
		bool bExtraPos = false;
		cVector2f vSide[2];

		int point = mvShadowPoints[idx][0];
		int next = mvShadowPoints[idx][1];

		vPointPos[0] = vTilePos + cVector2f((*apVtxVec)[point].pos.x, (*apVtxVec)[point].pos.y);
		vPointPos[0].z = mfShadowZ;
		vEndPos[0] = CalcLineEnd(vLightPos, vPointPos[0], fRadius, vSide[0], vLightPos);
		vEndPos[0].z = mfShadowZ;

		vPointPos[1] = vTilePos + cVector2f((*apVtxVec)[next].pos.x, (*apVtxVec)[next].pos.y);
		vPointPos[1].z = mfShadowZ;
		vEndPos[1] = CalcLineEnd(vLightPos, vPointPos[1], fRadius, vSide[1], vLightPos);
		vEndPos[1].z = mfShadowZ;

		// Check what sides of the light rect the points are clipped at.
		// If the edges are not the same the then there must be added an extra
		// point in a corner of the light rect.
		if (vSide[0] != vSide[1]) {
			if ((vSide[0].x == 0 && vSide[1].y == 0) || (vSide[1].x == 0 && vSide[0].y == 0)) {
				bExtraPos = true;
				vExtraPos = vLightPos + (vSide[0] + vSide[1]) * fRadius;
				vExtraPos.z = mfShadowZ;
			}
		}

		// If the entire object doesn't fit in the light rect we might wanna discard some points
		if (bNonFit) {
			if (ClipPoints(&vPointPos[0], LightRect, vLightPos, fRadius))
				continue;
		}

		// MAYBE TODO: Fix so that the shadows from different edges share vertices

		// Add vertexes and indexes to the vertex batcher
		mpLowLevelGraphics->AddVertexToBatch(cVertex(vPointPos[0], ShadowColor));
		mpLowLevelGraphics->AddVertexToBatch(cVertex(vPointPos[1], ShadowColor));
		mpLowLevelGraphics->AddIndexToBatch(lFirstIndex);
		mpLowLevelGraphics->AddIndexToBatch(lFirstIndex + 1);

		mpLowLevelGraphics->AddVertexToBatch(cVertex(vEndPos[0], ShadowColor));
		mpLowLevelGraphics->AddVertexToBatch(cVertex(vEndPos[1], ShadowColor));
		mpLowLevelGraphics->AddIndexToBatch(lFirstIndex + 2);

		mpLowLevelGraphics->AddIndexToBatch(lFirstIndex + 1);
		mpLowLevelGraphics->AddIndexToBatch(lFirstIndex + 2);
		mpLowLevelGraphics->AddIndexToBatch(lFirstIndex + 3);

		// Debug:
		/*mpLowLevelGraphics->SetDepthWriteActive(true);
		mpLowLevelGraphics->DrawLine(cVector2f(vPointPos[0].x,vPointPos[0].y),
			cVector2f(vEndPos[0].x,vEndPos[0].y),100,cColor(0.4));

		mpLowLevelGraphics->DrawLine(cVector2f(vPointPos[1].x,vPointPos[1].y),
			cVector2f(vEndPos[1].x,vEndPos[1].y),100,cColor(0.4));
		mpLowLevelGraphics->SetDepthWriteActive(false);*/

		// If we had an extra point one for triangle is needed.
		if (bExtraPos) {
			mpLowLevelGraphics->AddVertexToBatch(cVertex(vExtraPos, ShadowColor));

			mpLowLevelGraphics->AddIndexToBatch(lFirstIndex + 3);
			mpLowLevelGraphics->AddIndexToBatch(lFirstIndex + 2);
			mpLowLevelGraphics->AddIndexToBatch(lFirstIndex + 4);

			lFirstIndex += 5;
		} else {
			lFirstIndex += 4;
		}
	}

	return lFirstIndex;
}

//-----------------------------------------------------------------------

/**
 *
 * \param *avPoint
 * \param aRect
 * \return True if points should be discarded, else false.
 */
bool cRenderer2D::ClipPoints(cVector3f *avPoint, cRect2f aRect, cVector2f avPos, float afSize) {
	// Both points lie outside of the rect, discard them
	if (cMath::PointBoxCollision(cVector2f(avPoint[0].x, avPoint[0].y), aRect) == false &&
		cMath::PointBoxCollision(cVector2f(avPoint[1].x, avPoint[1].y), aRect) == false) {
		return true;
	}
	// At least one is inside, so clip the points
	for (int i = 0; i < 2; i++) {
		if (avPoint[i].x < avPos.x - afSize)
			avPoint[i].x = avPos.x - afSize;
		if (avPoint[i].x > avPos.x + afSize)
			avPoint[i].x = avPos.x + afSize;

		if (avPoint[i].y < avPos.y - afSize)
			avPoint[i].y = avPos.y - afSize;
		if (avPoint[i].y > avPos.y + afSize)
			avPoint[i].y = avPos.y + afSize;
	}
	return false;
}

//-----------------------------------------------------------------------

} // namespace hpl
