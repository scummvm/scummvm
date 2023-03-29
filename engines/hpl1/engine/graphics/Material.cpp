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

#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/resources/GpuProgramManager.h"
#include "hpl1/engine/resources/ImageManager.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/scene/Camera.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

eMaterialQuality iMaterial::mQuality = eMaterialQuality_High;

//-----------------------------------------------------------------------

iMaterial::iMaterial(const tString &asName, iLowLevelGraphics *apLowLevelGraphics,
					 cImageManager *apImageManager, cTextureManager *apTextureManager,
					 cRenderer2D *apRenderer, cGpuProgramManager *apProgramManager,
					 eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
	: iResourceBase(asName, 0) {
	if (aPicture == eMaterialPicture_Image) {
		mvImage.resize(eMaterialTexture_LastEnum);
		Common::fill(mvImage.begin(), mvImage.end(), nullptr);
	} else {
		mvTexture.resize(eMaterialTexture_LastEnum);
		Common::fill(mvTexture.begin(), mvTexture.end(), nullptr);
	}

	mType = eMaterialType_Null;
	mPicture = aPicture;

	mpLowLevelGraphics = apLowLevelGraphics;
	mpImageManager = apImageManager;
	mpTextureManager = apTextureManager;
	mpRenderer = apRenderer;
	mpRenderer3D = apRenderer3D;
	mpRenderSettings = mpRenderer3D->GetRenderSettings();
	mpProgramManager = apProgramManager;

	mbUsesLights = false;
	mbIsTransperant = false;
	mbIsGlowing = false;
	mbHasAlpha = false;
	mbDepthTest = true;
	mfValue = 1;

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < kMaxProgramNum; j++)
			mpProgram[i][j] = NULL;

	mlPassCount = 0;

	mlId = -1;
}

iMaterial::~iMaterial() {
	int i;

	for (i = 0; i < (int)mvImage.size(); i++) {
		if (mvImage[i])
			mpImageManager->Destroy(mvImage[i]);
	}
	for (i = 0; i < (int)mvTexture.size(); i++) {
		if (mvTexture[i])
			mpTextureManager->Destroy(mvTexture[i]);
	}

	for (i = 0; i < 2; i++) {
		for (int j = 0; j < kMaxProgramNum; j++) {
			if (mpProgram[i][j])
				mpProgramManager->Destroy(mpProgram[i][j]);
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

/*void iMaterial::Destroy()
{

}*/

//-----------------------------------------------------------------------

iTexture *iMaterial::GetTexture(eMaterialTexture aType) {
	if (mPicture == eMaterialPicture_Image) {
		if (mvImage[aType] == NULL) {
			Log("2: %d\n", aType);
			return NULL;
		}
		return mvImage[aType]->GetTexture();
	} else {
		return mvTexture[aType];
	}
}

//-----------------------------------------------------------------------

cRect2f iMaterial::GetTextureOffset(eMaterialTexture aType) {
	cRect2f SizeRect;

	if (mPicture == eMaterialPicture_Image) {
		tVertexVec VtxVec = mvImage[aType]->GetVertexVecCopy(0, 0);

		SizeRect.x = VtxVec[0].tex.x;
		SizeRect.y = VtxVec[0].tex.y;
		SizeRect.w = VtxVec[2].tex.x - VtxVec[0].tex.x;
		SizeRect.h = VtxVec[2].tex.y - VtxVec[0].tex.y;
	} else {
		SizeRect.x = 0;
		SizeRect.y = 0;
		SizeRect.w = 1; //(float) mvTexture[aType]->GetWidth();
		SizeRect.h = 1; //(float) mvTexture[aType]->GetHeight();
	}

	return SizeRect;
}

//-----------------------------------------------------------------------
} // namespace hpl
