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

#include "hpl1/engine/gui/GuiGfxElement.h"

#include "hpl1/engine/system/LowLevelSystem.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/resources/FrameBitmap.h"
#include "hpl1/engine/resources/ImageManager.h"
#include "hpl1/engine/resources/ResourceImage.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/TextureManager.h"

#include "hpl1/engine/gui/Gui.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// ANIMATION
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGuiGfxAnimation::AddFrame(int alNum) {
	mvFrames.push_back(alNum);
}
void cGuiGfxAnimation::SetType(eGuiGfxAnimationType aType) {
	mType = aType;
}
void cGuiGfxAnimation::SetFrameLength(float afLength) {
	mfFrameLength = afLength;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGuiGfxElement::cGuiGfxElement(cGui *apGui) {
	mpGui = apGui;

	/////////////////////////////
	// Set up vertices
	mvVtx.resize(4);

	for (int i = 0; i < 4; ++i) {
		mvVtx[i].pos = 0;
		mvVtx[i].tex = 0;
		mvVtx[i].col = cColor(1, 1);
	}

	// Position
	mvVtx[1].pos.x = 1;
	mvVtx[2].pos.x = 1;
	mvVtx[2].pos.y = 1;
	mvVtx[3].pos.y = 1;

	// Texture coordinate
	mvVtx[1].tex.x = 1;
	mvVtx[2].tex.x = 1;
	mvVtx[2].tex.y = 1;
	mvVtx[3].tex.y = 1;

	mvImageSize = 0;

	mvOffset = 0;
	mvActiveSize = 0;

	////////////////////////////
	// Set up textures
	for (int i = 0; i < kMaxGuiTextures; ++i) {
		mvTextures[i] = NULL;
		mvImages[i] = NULL;
	}

	mlTextureNum = 0;
	mlCurrentAnimation = 0;
	mfCurrentFrame = 0;
	mbForwardAnim = true;
	mlActiveImage = 0;
	mbAnimationPaused = false;

	mbFlushed = false;
}

//---------------------------------------------------

cGuiGfxElement::~cGuiGfxElement() {
	STLDeleteAll(mvAnimations);

	////////////////////////////////
	// Delete all textures / Images
	if (mvImageBufferVec.size() > 0) {
		for (int i = 0; i < (int)mvImageBufferVec.size(); ++i) {
			// Skip for now, memory might be fucked..
			// mpGui->GetResources()->GetImageManager()->Destroy(mvImageBufferVec[i]);
		}
	} else {
		for (int i = 0; i < mlTextureNum; ++i) {
			if (mvImages[i]) {
				// Skip for now, memory might be fucked..
				// mpGui->GetResources()->GetImageManager()->Destroy(mvImages[i]);
			} else if (mvTextures[i]) {
				mpGui->GetResources()->GetTextureManager()->Destroy(mvTextures[i]);
			}
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGuiGfxElement::Update(float afTimeStep) {

	//////////////////////////////////////
	// Update image animations
	if (mvAnimations.empty() || mbAnimationPaused)
		return;
	if (mlCurrentAnimation >= (int)mvAnimations.size())
		return;

	cGuiGfxAnimation *pAnim = mvAnimations[mlCurrentAnimation];

	int lFrame = 0;
	//////////////////////////////////
	// Non random animation update
	if (pAnim->mType != eGuiGfxAnimationType_Random) {
		mfCurrentFrame += afTimeStep * (1.0f / pAnim->mfFrameLength);
		lFrame = (int)mfCurrentFrame;
		if (lFrame >= (int)mvImageBufferVec.size()) {
			// Log("Over max!\n");
			// Stop at end
			if (pAnim->mType == eGuiGfxAnimationType_StopAtEnd) {
				lFrame = (int)mvImageBufferVec.size() - 1;
				mfCurrentFrame = (float)lFrame;
			}
			// Loop
			else if (pAnim->mType == eGuiGfxAnimationType_Loop) {
				lFrame = 0;
				mfCurrentFrame = 0;
			}
			// Oscillate
			else if (pAnim->mType == eGuiGfxAnimationType_Oscillate) {
				lFrame = 1;
				mfCurrentFrame = 1;
				mbForwardAnim = !mbForwardAnim;
			}
		}
		// Log("Frame %d %f actual_frame: %d size: %d\n",lFrame,mfCurrentFrame,((int)mvImageBufferVec.size()-1) - lFrame,mvImageBufferVec.size());

		// Oscillate fix
		if (mbForwardAnim == false && pAnim->mType == eGuiGfxAnimationType_Oscillate) {
			lFrame = ((int)mvImageBufferVec.size() - 1) - lFrame;
		}
	}
	//////////////////////////////////
	// Random animation update
	else if (mvImageBufferVec.size() > 1) {
		float fPrev = mfCurrentFrame;
		mfCurrentFrame += afTimeStep * (1.0f / pAnim->mfFrameLength);
		lFrame = (int)mfCurrentFrame;
		if ((int)mfCurrentFrame != (int)fPrev) {
			int lPrev = (int)fPrev;
			do {
				lFrame = cMath::RandRectl(0, (int)mvImageBufferVec.size() - 1);
			} while (lFrame == lPrev);

			mfCurrentFrame = (float)lFrame;
		}
	}

	//////////////////////////////////
	// Set new image
	if (lFrame != mlActiveImage) {
		mlActiveImage = lFrame;
		SetImage(mvImageBufferVec[mlActiveImage], 0);
	}
}

//-----------------------------------------------------------------------

void cGuiGfxElement::AddImage(cResourceImage *apImage) {
	SetImage(apImage, mlTextureNum);

	mvActiveSize = GetImageSize();

	++mlTextureNum;
}

//---------------------------------------------------

void cGuiGfxElement::AddTexture(iTexture *apTexture) {
	mvTextures[mlTextureNum] = apTexture;

	if (mlTextureNum == 0) {
		mvImageSize.x = (float)apTexture->GetWidth();
		mvImageSize.y = (float)apTexture->GetHeight();
	}

	mvActiveSize = GetImageSize();

	++mlTextureNum;
}

//---------------------------------------------------

void cGuiGfxElement::AddImageToBuffer(cResourceImage *apImage) {
	if (mvImageBufferVec.size() == 0) {
		SetImage(apImage, 0);
	}

	mvImageBufferVec.push_back(apImage);
}

//---------------------------------------------------

cGuiGfxAnimation *cGuiGfxElement::CreateAnimtion(const tString &asName) {
	cGuiGfxAnimation *pAnimation = hplNew(cGuiGfxAnimation, ());
	pAnimation->msName = asName;

	mvAnimations.push_back(pAnimation);

	return pAnimation;
}

//---------------------------------------------------

void cGuiGfxElement::PlayAnimation(int alNum) {
	if (mlCurrentAnimation == alNum)
		return;

	mlCurrentAnimation = alNum;

	mfCurrentFrame = 0;
	mbForwardAnim = true;
	mlActiveImage = 0;
	SetImage(mvImageBufferVec[mlActiveImage], 0);
}

void cGuiGfxElement::SetAnimationTime(float afTime) {
	if (mlCurrentAnimation >= 0)
		mfCurrentFrame = afTime / mvAnimations[mlCurrentAnimation]->mfFrameLength;
	else
		mfCurrentFrame = afTime;
}

//---------------------------------------------------

void cGuiGfxElement::SetMaterial(iGuiMaterial *apMat) {
	mpMaterial = apMat;
}

//---------------------------------------------------

void cGuiGfxElement::SetColor(const cColor &aColor) {
	for (int i = 0; i < 4; ++i)
		mvVtx[i].col = aColor;
}

//-----------------------------------------------------------------------

cVector2f cGuiGfxElement::GetImageSize() {
	return mvImageSize;
}

//-----------------------------------------------------------------------

void cGuiGfxElement::Flush() {
	if (mbFlushed)
		return;

	for (int i = 0; i < mlTextureNum; ++i) {
		if (mvImages[i])
			mvImages[i]->GetFrameBitmap()->FlushToTexture();
	}
	for (size_t i = 0; i < mvImageBufferVec.size(); ++i) {
		if (mvImageBufferVec[i])
			mvImageBufferVec[i]->GetFrameBitmap()->FlushToTexture();
	}

	mbFlushed = true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGuiGfxElement::SetImage(cResourceImage *apImage, int alNum) {
	// Set image and texture (for sorting)
	mvImages[alNum] = apImage;
	mvTextures[alNum] = apImage->GetTexture();

	// Get texture coords
	const tVertexVec &vImageVtx = apImage->GetVertexVec();
	for (int i = 0; i < 4; ++i)
		mvVtx[i].tex = vImageVtx[i].tex;

	if (alNum == 0) {
		mvImageSize.x = (float)apImage->GetWidth();
		mvImageSize.y = (float)apImage->GetHeight();
	}
}

//-----------------------------------------------------------------------

} // namespace hpl
