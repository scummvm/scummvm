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

#include "hpl1/engine/graphics/GraphicsDrawer.h"

#include "hpl1/engine/graphics/GfxObject.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/MaterialHandler.h"
#include "hpl1/engine/resources/FrameBitmap.h"
#include "hpl1/engine/resources/ResourceImage.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/system/LowLevelSystem.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/resources/ImageManager.h"
#include "hpl1/engine/resources/Resources.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGraphicsDrawer::cGraphicsDrawer(iLowLevelGraphics *apLowLevelGraphics, cMaterialHandler *apMaterialHandler,
								 cResources *apResources) {
	mpLowLevelGraphics = apLowLevelGraphics;
	mpMaterialHandler = apMaterialHandler;
	mpResources = apResources;
}

//-----------------------------------------------------------------------

cGraphicsDrawer::~cGraphicsDrawer() {
	ClearBackgrounds();

	STLDeleteAll(mlstGfxObjects);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iMaterial *cGfxBufferObject::GetMaterial() const {
	return mpObject->GetMaterial();
}

//-----------------------------------------------------------------------

bool cGfxBufferCompare::operator()(const cGfxBufferObject &aObjectA, const cGfxBufferObject &aObjectB) const {
	if (aObjectA.GetZ() != aObjectB.GetZ()) {
		return aObjectA.GetZ() < aObjectB.GetZ();
	} else if (aObjectA.GetMaterial()->GetTexture(eMaterialTexture_Diffuse) !=
			   aObjectB.GetMaterial()->GetTexture(eMaterialTexture_Diffuse)) {
		return aObjectA.GetMaterial()->GetTexture(eMaterialTexture_Diffuse) >
			   aObjectB.GetMaterial()->GetTexture(eMaterialTexture_Diffuse);
	} else if (aObjectA.GetMaterial()->GetType(eMaterialRenderType_Diffuse) !=
			   aObjectB.GetMaterial()->GetType(eMaterialRenderType_Diffuse)) {
		return aObjectA.GetMaterial()->GetType(eMaterialRenderType_Diffuse) >
			   aObjectB.GetMaterial()->GetType(eMaterialRenderType_Diffuse);
	} else {
	}
	return false;
}

//-----------------------------------------------------------------------

static void FlushImage(cGfxObject *apObject) {
	if (apObject->IsImage()) {
		cResourceImage *pImage = apObject->GetMaterial()->GetImage(eMaterialTexture_Diffuse);
		pImage->GetFrameBitmap()->FlushToTexture();
	}
}

void cGraphicsDrawer::DrawGfxObject(cGfxObject *apObject, const cVector3f &avPos,
									const cVector2f &avSize, const cColor &aColor,
									bool abFlipH, bool abFlipV, float afAngle) {
	FlushImage(apObject);

	cGfxBufferObject BuffObj;
	BuffObj.mpObject = apObject;
	BuffObj.mvTransform = avPos;
	BuffObj.mvSize = avSize;
	BuffObj.mColor = aColor;
	BuffObj.mbFlipH = abFlipH;
	BuffObj.mbFlipV = abFlipV;
	BuffObj.mfAngle = afAngle;

	BuffObj.mbIsColorAndSize = true;

	m_setGfxBuffer.insert(BuffObj);
}

//-----------------------------------------------------------------------

void cGraphicsDrawer::DrawGfxObject(cGfxObject *apObject, const cVector3f &avPos) {
	FlushImage(apObject);

	cGfxBufferObject BuffObj;
	BuffObj.mpObject = apObject;
	BuffObj.mvTransform = avPos;
	BuffObj.mbIsColorAndSize = false;

	m_setGfxBuffer.insert(BuffObj);
}
//-----------------------------------------------------------------------

void cGraphicsDrawer::DrawAll() {
	// Set all states
	mpLowLevelGraphics->SetDepthTestActive(false);
	mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);

	mpLowLevelGraphics->SetOrthoProjection(mpLowLevelGraphics->GetVirtualSize(), -1000, 1000);

	int lIdxAdd = 0;
	iMaterial *pPrevMat = NULL;
	iMaterial *pMat = NULL;
	const cGfxBufferObject *pObj = NULL;
	tGfxBufferSetIt ObjectIt = m_setGfxBuffer.begin();

	if (ObjectIt != m_setGfxBuffer.end())
		pMat = ObjectIt->GetMaterial();

	while (ObjectIt != m_setGfxBuffer.end()) {
		if (pMat->StartRendering(eMaterialRenderType_Diffuse, NULL, NULL) == false) {
			ObjectIt++;
			if (ObjectIt != m_setGfxBuffer.end())
				pMat = ObjectIt->GetMaterial();

			continue;
		}

		do {
			pObj = &(*ObjectIt);
			if (pObj->mbIsColorAndSize) {
				cVector3f vPos[4];
				float fW = pObj->mvSize.x * 0.5f;
				float fH = pObj->mvSize.y * 0.5f;
				cMatrixf mtxTrans = cMath::MatrixTranslate(pObj->mvTransform + cVector3f(fW, fH, 0));
				vPos[0] = cVector3f(-fW, -fH, 0);
				vPos[1] = cVector3f(fW, -fH, 0);
				vPos[2] = cVector3f(fW, fH, 0);
				vPos[3] = cVector3f(-fW, fH, 0);

				if (pObj->mfAngle != 0) {
					cMatrixf mtxRot = cMath::MatrixRotateZ(pObj->mfAngle);
					vPos[0] = cMath::MatrixMul(mtxRot, vPos[0]);
					vPos[1] = cMath::MatrixMul(mtxRot, vPos[1]);
					vPos[2] = cMath::MatrixMul(mtxRot, vPos[2]);
					vPos[3] = cMath::MatrixMul(mtxRot, vPos[3]);
				}

				vPos[0] = cMath::MatrixMul(mtxTrans, vPos[0]);
				vPos[1] = cMath::MatrixMul(mtxTrans, vPos[1]);
				vPos[2] = cMath::MatrixMul(mtxTrans, vPos[2]);
				vPos[3] = cMath::MatrixMul(mtxTrans, vPos[3]);

				if (pObj->mbFlipH) {
					mpLowLevelGraphics->AddVertexToBatch_Size2D(pObj->mpObject->GetVtxPtr(0),
																&vPos[0],
																&pObj->mColor,
																0, 0);
					mpLowLevelGraphics->AddVertexToBatch_Size2D(pObj->mpObject->GetVtxPtr(1),
																&vPos[1],
																&pObj->mColor,
																0, 0);
					mpLowLevelGraphics->AddVertexToBatch_Size2D(pObj->mpObject->GetVtxPtr(2),
																&vPos[2],
																&pObj->mColor,
																0, 0);
					mpLowLevelGraphics->AddVertexToBatch_Size2D(pObj->mpObject->GetVtxPtr(3),
																&vPos[3],
																&pObj->mColor,
																pObj->mvSize.x, pObj->mvSize.y);
				} else {
					mpLowLevelGraphics->AddVertexToBatch_Size2D(pObj->mpObject->GetVtxPtr(0),
																&vPos[0],
																&pObj->mColor,
																0, 0);
					mpLowLevelGraphics->AddVertexToBatch_Size2D(pObj->mpObject->GetVtxPtr(1),
																&vPos[1],
																&pObj->mColor,
																0, 0);
					mpLowLevelGraphics->AddVertexToBatch_Size2D(pObj->mpObject->GetVtxPtr(2),
																&vPos[2],
																&pObj->mColor,
																0, 0);
					mpLowLevelGraphics->AddVertexToBatch_Size2D(pObj->mpObject->GetVtxPtr(3),
																&vPos[3],
																&pObj->mColor,
																0, 0);
				}

				for (int i = 0; i < 4; i++)
					mpLowLevelGraphics->AddIndexToBatch(lIdxAdd + i);
			} else {
				for (int i = 0; i < (int)pObj->mpObject->GetVertexVec()->size(); i++) {
					mpLowLevelGraphics->AddVertexToBatch(pObj->mpObject->GetVtxPtr(i),
														 &pObj->mvTransform);
					mpLowLevelGraphics->AddIndexToBatch(lIdxAdd + i);
				}
			}
			lIdxAdd += (int)pObj->mpObject->GetVertexVec()->size();

			pPrevMat = pMat;
			ObjectIt++;

			if (ObjectIt == m_setGfxBuffer.end()) {
				pMat = NULL;
				break;
			} else {
				pMat = ObjectIt->GetMaterial();
			}
		} while (pMat->GetType(eMaterialRenderType_Diffuse) ==
					 pPrevMat->GetType(eMaterialRenderType_Diffuse) &&
				 pMat->GetTexture(eMaterialTexture_Diffuse) ==
					 pPrevMat->GetTexture(eMaterialTexture_Diffuse));

		lIdxAdd = 0;

		do {
			mpLowLevelGraphics->FlushQuadBatch(pPrevMat->GetBatchFlags(eMaterialRenderType_Diffuse), false);
		} while (pPrevMat->NextPass(eMaterialRenderType_Diffuse));

		mpLowLevelGraphics->ClearBatch();

		pPrevMat->EndRendering(eMaterialRenderType_Diffuse);
	}

	// Clear the buffer of objects.
	m_setGfxBuffer.clear();

	// Reset all states
	mpLowLevelGraphics->SetDepthTestActive(true);
}

//-----------------------------------------------------------------------

cGfxObject *cGraphicsDrawer::CreateGfxObject(const tString &asFileName, const tString &asMaterialName,
											 bool abAddToList) {
	cResourceImage *pImage = mpResources->GetImageManager()->CreateImage(asFileName);
	if (pImage == NULL) {
		FatalError("Couldn't load image '%s'!\n", asFileName.c_str());
		return NULL;
	}

	iMaterial *pMat = mpMaterialHandler->Create(asMaterialName, eMaterialPicture_Image);
	if (pMat == NULL) {
		FatalError("Couldn't create material '%s'!\n", asMaterialName.c_str());
		return NULL;
	}
	// mpResources->GetImageManager()->FlushAll();

	pMat->SetImage(pImage, eMaterialTexture_Diffuse);

	cGfxObject *pObject = hplNew(cGfxObject, (pMat, asFileName, true));

	if (abAddToList)
		mlstGfxObjects.push_back(pObject);

	return pObject;
}

//-----------------------------------------------------------------------

cGfxObject *cGraphicsDrawer::CreateGfxObject(iBitmap2D *apBmp, const tString &asMaterialName,
											 bool abAddToList) {
	cResourceImage *pImage = mpResources->GetImageManager()->CreateFromBitmap("", apBmp);
	if (pImage == NULL) {
		FatalError("Couldn't create image\n");
		return NULL;
	}

	iMaterial *pMat = mpMaterialHandler->Create(asMaterialName, eMaterialPicture_Image);
	if (pMat == NULL) {
		FatalError("Couldn't create material '%s'!\n", asMaterialName.c_str());
		return NULL;
	}
	// mpResources->GetImageManager()->FlushAll();

	pMat->SetImage(pImage, eMaterialTexture_Diffuse);

	cGfxObject *pObject = hplNew(cGfxObject, (pMat, "", true));

	if (abAddToList)
		mlstGfxObjects.push_back(pObject);

	return pObject;
}

//-----------------------------------------------------------------------

cGfxObject *cGraphicsDrawer::CreateGfxObjectFromTexture(const tString &asFileName, const tString &asMaterialName,
														bool abAddToList) {
	iTexture *pTex = mpResources->GetTextureManager()->Create2D(asFileName, false);
	if (pTex == NULL) {
		FatalError("Couldn't create texture '%s'!\n", asFileName.c_str());
		return NULL;
	}

	iMaterial *pMat = mpMaterialHandler->Create(asMaterialName, eMaterialPicture_Texture);
	if (pMat == NULL) {
		FatalError("Couldn't create material '%s'!\n", asMaterialName.c_str());
		return NULL;
	}
	// mpResources->GetImageManager()->FlushAll();

	pMat->SetTexture(pTex, eMaterialTexture_Diffuse);

	cGfxObject *pObject = hplNew(cGfxObject, (pMat, asFileName, false));

	if (abAddToList)
		mlstGfxObjects.push_back(pObject);

	return pObject;
}

//-----------------------------------------------------------------------

void cGraphicsDrawer::DestroyGfxObject(cGfxObject *apObject) {
	STLFindAndDelete(mlstGfxObjects, apObject);
}

//-----------------------------------------------------------------------

cBackgroundImage *cGraphicsDrawer::AddBackgroundImage(const tString &asFileName,
													  const tString &asMaterialName,
													  const cVector3f &avPos,
													  bool abTile, const cVector2f &avSize, const cVector2f &avPosPercent, const cVector2f &avVel) {
	cResourceImage *pImage = mpResources->GetImageManager()->CreateImage(asFileName);
	if (pImage == NULL) {
		FatalError("Couldn't load image '%s'!\n", asFileName.c_str());
		return NULL;
	}

	iMaterial *pMat = mpMaterialHandler->Create(asMaterialName, eMaterialPicture_Image);
	if (pMat == NULL) {
		FatalError("Couldn't create material '%s'!\n", asMaterialName.c_str());
		return NULL;
	}
	// mpResources->GetImageManager()->FlushAll();

	pMat->SetImage(pImage, eMaterialTexture_Diffuse);

	cBackgroundImage *pBG = hplNew(cBackgroundImage, (pMat, avPos, abTile, avSize, avPosPercent, avVel));

	m_mapBackgroundImages.insert(tBackgroundImageMap::value_type(avPos.z, pBG));

	return pBG;
}

//-----------------------------------------------------------------------

void cGraphicsDrawer::UpdateBackgrounds() {
	tBackgroundImageMapIt it = m_mapBackgroundImages.begin();
	for (; it != m_mapBackgroundImages.end(); it++) {
		it->second->Update();
	}
}

//-----------------------------------------------------------------------

void cGraphicsDrawer::DrawBackgrounds(const cRect2f &aCollideRect) {
	mpLowLevelGraphics->SetIdentityMatrix(eMatrix_ModelView);
	mpLowLevelGraphics->SetDepthTestActive(true);
	mpLowLevelGraphics->SetDepthWriteActive(false);
	mpLowLevelGraphics->SetAlphaTestFunc(eAlphaTestFunc_Greater, 0.1f);
	mpLowLevelGraphics->SetDepthTestFunc(eDepthTestFunc_LessOrEqual);

	tBackgroundImageMapIt it = m_mapBackgroundImages.begin();
	for (; it != m_mapBackgroundImages.end(); it++) {
		it->second->Draw(aCollideRect, mpLowLevelGraphics);
	}

	mpLowLevelGraphics->SetAlphaTestFunc(eAlphaTestFunc_Greater, 0.05f);
	mpLowLevelGraphics->SetDepthWriteActive(true);
}

//-----------------------------------------------------------------------

void cGraphicsDrawer::ClearBackgrounds() {
	tBackgroundImageMapIt it = m_mapBackgroundImages.begin();
	for (; it != m_mapBackgroundImages.end(); it++) {
		hplDelete(it->second);
	}
	m_mapBackgroundImages.clear();
}

//-----------------------------------------------------------------------

} // namespace hpl
