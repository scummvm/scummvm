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

#include "hpl1/engine/scene/ImageEntity.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/ImageEntityData.h"
#include "hpl1/engine/graphics/RenderObject2D.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/ImageEntityManager.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cImageEntity::cImageEntity(tString asName, cResources *apResources, cGraphics *apGraphics,
						   bool abAutoDeleteData)
	: iEntity2D(asName) {
	mvSize = cVector2f(-1, -1);

	mbFlipH = false;
	mbFlipV = false;

	mpAnimation = NULL;
	mfFrameNum = 0;
	mfAnimSpeed = 0.5f;
	mlFrame = 0;
	mlLastFrame = -1;

	mbFlashing = false;

	mfAlpha = 1.0f;

	mpEntityData = NULL;

	mpResources = apResources;
	mpGraphics = apGraphics;

	UpdateBoundingBox();

	msTempString = "Default";

	mbAnimationPaused = false;
	mbRotationHasChanged = true;

	mbAutoDeleteData = abAutoDeleteData;

	mbLoopAnimation = true;
}

//-----------------------------------------------------------------------

cImageEntity::~cImageEntity() {
	if (mbAutoDeleteData) {
		if (mpEntityData)
			mpResources->GetImageEntityManager()->Destroy(mpEntityData);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
int cImageEntity::GetMaxFrameNum() {
	if (mpAnimation == NULL)
		return 0;

	// One for the -1, one for the frame jump, and one becuase we start with 0.
	return (int)mpAnimation->mvFrameNums.size() - 3;
}

//-----------------------------------------------------------------------

bool cImageEntity::SetAnimation(const tString &asName, bool abLoop) {
	mbLoopAnimation = abLoop;

	if (mpAnimation->msName == asName)
		return true;

	cImageAnimation *pAnim = mpEntityData->GetAnimationByName(asName);
	if (pAnim == NULL)
		return false;

	mpAnimation = pAnim;
	mfFrameNum = 0;
	return true;
}

//-----------------------------------------------------------------------

const tString &cImageEntity::GetCurrentAnimation() const {
	if (mpAnimation == NULL) {
		return msTempString;
	}

	return mpAnimation->msName;
}
//-----------------------------------------------------------------------

void cImageEntity::StopAnimation() {
	for (int i = 0; i < (int)mpAnimation->mvFrameNums.size(); i++) {
		if (mpAnimation->mvFrameNums[i] == -1) {
			mlFrame = i;
			mlLastFrame = i;
			mfFrameNum = (float)i;
			break;
		}
	}
	if (mbLoopAnimation)
		mbLoopAnimation = false;
}

//-----------------------------------------------------------------------

void cImageEntity::SetSize(const cVector2f &avSize) {
	if (avSize.x == mvSize.x && avSize.y == mvSize.y)
		return;

	mvSize.x = avSize.x == 0 ? 0.001f : avSize.x;
	mvSize.y = avSize.y == 0 ? 0.001f : avSize.y;

	mbSizeHasChanged = true;
	mbRotationHasChanged = true;
}

//-----------------------------------------------------------------------

void cImageEntity::SetFlipH(bool abX) {
	if (mbFlipH == abX)
		return;

	mbFlipH = abX;
	mbRotationHasChanged = true;
}

//-----------------------------------------------------------------------

void cImageEntity::SetFlipV(bool abX) {
	if (mbFlipV == abX)
		return;

	mbFlipV = abX;
	mbRotationHasChanged = true;
}

//-----------------------------------------------------------------------

void cImageEntity::UpdateLogic(float afTimeStep) {
	if (mbFlashing) {
		float fAlpha = GetAlpha();
		if (mfFlashAdd < 0) {
			fAlpha += mfFlashAdd * 7;
			if (fAlpha < 0) {
				fAlpha = 0;
				mfFlashAdd = -mfFlashAdd;
			}
			SetAlpha(fAlpha);
		} else {
			fAlpha += mfFlashAdd;
			if (fAlpha > 1) {
				fAlpha = 1;
				mbFlashing = false;
			}
			SetAlpha(fAlpha);
		}
	}

	if (mpAnimation && !mbAnimationPaused) {
		if (mpAnimation->mvFrameNums[(int)mfFrameNum] != -1) {
			mfFrameNum += mfAnimSpeed * mpAnimation->mfSpeed;

			int lFrameNum = (int)mfFrameNum;

			if (mpAnimation->mvFrameNums[lFrameNum] == -1) {
				if (mbLoopAnimation) {
					float fTemp = (float)mpAnimation->mvFrameNums[lFrameNum + 1];

					mfFrameNum = fTemp + cMath::GetFraction(mfFrameNum);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

bool cImageEntity::AnimationIsPlaying() {
	int lFrameNum = (int)mfFrameNum;
	return mpAnimation->mvFrameNums[lFrameNum] != -1;
}

//-----------------------------------------------------------------------

void cImageEntity::Render() {
	if (!mbIsActive)
		return;

	/*Update frame stuff if needed here*/

	mvTransform = GetWorldPosition();

	/// Get the frame to be used //////////////
	if (mpEntityData->GetFrameNum() == 1) {
		mlFrame = 0;
		mlLastFrame = 0;
	} else {
		mlLastFrame = mlFrame;
		mlFrame = mpAnimation->mvFrameNums[(int)mfFrameNum];

		if (mlFrame == -1)
			mlFrame = mpAnimation->mvFrameNums[((int)mfFrameNum) - 1];

		/// If Frame has changed update vertexes
		if (mlLastFrame != mlFrame) {
			tVertexVec *pVtx = &mpEntityData->GetImageFrame(mlFrame)->mvVtx;

			for (int i = 0; i < (int)pVtx->size(); i++) {
				mvVtx[i].tex.x = (*pVtx)[i].tex.x;
				mvVtx[i].tex.y = (*pVtx)[i].tex.y;
				mvVtx[i].tex.z = (*pVtx)[i].tex.z;
			}
		}
	}

	if (mbSizeHasChanged) {
		for (int i = 0; i < (int)mvBaseVtx.size(); i++) {
			// Slow as hell!! Change this perhaps?
			// This also only works on square meshes...
			mvBaseVtx[i].pos.x = ABS(mvBaseVtx[i].pos.x) / mvBaseVtx[i].pos.x;
			mvBaseVtx[i].pos.y = ABS(mvBaseVtx[i].pos.y) / mvBaseVtx[i].pos.y;

			mvBaseVtx[i].pos.x *= mvSize.x / 2;
			mvBaseVtx[i].pos.y *= mvSize.y / 2;
			mvBaseVtx[i].pos.z = 0;
		}
	}

	/// If rotation or frame has changed, update the vertexes
	if (mbRotationHasChanged || mfCurrentAngle != GetWorldRotation().z) {
		mbRotationHasChanged = false;
		mfCurrentAngle = GetWorldRotation().z;

		float fSin = sin(mfCurrentAngle);
		float fCos = cos(mfCurrentAngle);

		float fNormZ = 3;
		if (mbFlipV)
			fNormZ = 1;
		if (mbFlipH)
			fNormZ *= -1;

		for (int i = 0; i < (int)mvVtx.size(); i++) {
			mvVtx[i].pos.x = mvBaseVtx[i].pos.x * fCos - mvBaseVtx[i].pos.y * fSin;
			mvVtx[i].pos.y = mvBaseVtx[i].pos.x * fSin + mvBaseVtx[i].pos.y * fCos;

			if (mbFlipH)
				mvVtx[i].pos.x = -mvVtx[i].pos.x;
			if (mbFlipV)
				mvVtx[i].pos.y = -mvVtx[i].pos.y;

			mvVtx[i].norm.x = fCos;
			mvVtx[i].norm.y = fSin;
			mvVtx[i].norm.z = fNormZ;
		}
	}
	cRenderObject2D _obj = cRenderObject2D(
		mpEntityData->GetImageFrame(mlFrame)->mpMaterial,
		&mvVtx, &mvIdxVec,
		ePrimitiveType_Quad, GetWorldPosition().z,
		mBoundingBox, NULL, &mvTransform);
	// Add the render object.
	mpGraphics->GetRenderer2D()->AddObject(_obj);
}

//-----------------------------------------------------------------------

bool cImageEntity::LoadData(TiXmlElement *apRootElem) {
	tString sDataName = cString::ToString(apRootElem->Attribute("DataName"), "");

	mbFlipH = cString::ToBool(apRootElem->Attribute("FlipH"), false);
	mbFlipV = cString::ToBool(apRootElem->Attribute("FlipV"), false);
	mvRotation.z = cMath::ToRad(cString::ToFloat(apRootElem->Attribute("Angle"), 0));
	mvSize.x = cString::ToFloat(apRootElem->Attribute("Width"), 2);
	mvSize.y = cString::ToFloat(apRootElem->Attribute("Height"), 2);
	int lAnimNum = cString::ToInt(apRootElem->Attribute("AnimNum"), 0);

	return LoadEntityData(sDataName, lAnimNum);
}

//-----------------------------------------------------------------------
bool cImageEntity::LoadEntityData(tString asDataName, int alAnimNum) {
	cImageEntityData *pImageData = mpResources->GetImageEntityManager()->CreateData(asDataName);

	if (pImageData == NULL) {
		error("Couldn't load Data '%s' for entity '%s'", asDataName.c_str(), msName.c_str());
		return false;
	}

	return LoadEntityData(pImageData, alAnimNum);
}

//-----------------------------------------------------------------------

bool cImageEntity::LoadEntityData(cImageEntityData *apData, int alAnimNum) {
	mpEntityData = apData;

	if (mpEntityData->GetFrameNum() > 1) {
		mpAnimation = mpEntityData->GetAnimationByHandle(alAnimNum);
		mlFrame = mpAnimation->mvFrameNums[0];
		mlLastFrame = -1;
	} else {
		mlFrame = 0;
	}

	mvBaseVtx = mpEntityData->GetImageFrame(mlFrame)->mvVtx;

	if (mvSize.x < 0 && mvSize.y < 0) {
		mvSize = mpEntityData->GetImageSize();
	}

	mvVtx = mvBaseVtx;
	mvIdxVec = *mpEntityData->GetIndexVec();
	mbRotationHasChanged = true;
	mbSizeHasChanged = true;
	mfCurrentAngle = 0;

	return true;
}

//-----------------------------------------------------------------------

const cRect2f &cImageEntity::GetBoundingBox() {
	return mBoundingBox;
}

//-----------------------------------------------------------------------

bool cImageEntity::UpdateBoundingBox() {
	cVector2f vSize;

	if (mvRotation.z != 0) {
		// Only Temp...
		float fMaxSize = sqrt(mvSize.x * mvSize.x + mvSize.y * mvSize.y);

		vSize.x = fMaxSize;
		vSize.y = fMaxSize;
	} else {
		vSize = mvSize;
	}

	mBoundingBox = cRect2f(cVector2f(GetWorldPosition().x - vSize.x / 2,
									 GetWorldPosition().y - vSize.y / 2),
						   vSize);

	return true;
}
//-----------------------------------------------------------------------

void cImageEntity::SetAlpha(float afX) {
	if (mfAlpha != afX) {
		mfAlpha = afX;

		for (int i = 0; i < (int)mvVtx.size(); i++) {
			mvVtx[i].col.a = mfAlpha;
		}
	}
}

//-----------------------------------------------------------------------

void cImageEntity::Flash(float afAdd) {
	mbFlashing = true;
	mfFlashAdd = -ABS(afAdd);
}

//-----------------------------------------------------------------------
} // namespace hpl
