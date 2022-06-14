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

#include "hpl1/engine/graphics/Material_Smoke2D.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMaterial_Smoke2D::cMaterial_Smoke2D(const tString &asName, iLowLevelGraphics *apLowLevelGraphics,
									 cImageManager *apImageManager, cTextureManager *apTextureManager,
									 cRenderer2D *apRenderer, cGpuProgramManager *apProgramManager,
									 eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
	: iMaterial(asName, apLowLevelGraphics, apImageManager, apTextureManager, apRenderer, apProgramManager,
				aPicture, apRenderer3D) {
	mbIsTransperant = true;
	mbIsGlowing = true;
	mType = eMaterialType_Smoke;
}

//-----------------------------------------------------------------------

cMaterial_Smoke2D::~cMaterial_Smoke2D() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cMaterial_Smoke2D::Compile() {
}

//-----------------------------------------------------------------------

bool cMaterial_Smoke2D::StartRendering(eMaterialRenderType aType, iCamera *apCam, iLight *pLight) {
	if (aType == eMaterialRenderType_Diffuse) {
		mpLowLevelGraphics->SetBlendActive(true);
		mpLowLevelGraphics->SetBlendFunc(eBlendFunc_Zero, eBlendFunc_OneMinusSrcColor);

		mpLowLevelGraphics->SetTexture(0, GetTexture(eMaterialTexture_Diffuse));
		mpLowLevelGraphics->SetActiveTextureUnit(0);
		// mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorOp1,eTextureOp_Alpha);
		// mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorFunc, eTextureFunc_Modulate);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorOp1, eTextureOp_Color);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorFunc, eTextureFunc_Modulate);

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------

void cMaterial_Smoke2D::EndRendering(eMaterialRenderType aType) {
	if (aType == eMaterialRenderType_Diffuse) {
		mpLowLevelGraphics->SetActiveTextureUnit(0);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorOp1, eTextureOp_Color);
		mpLowLevelGraphics->SetTextureEnv(eTextureParam_ColorFunc, eTextureFunc_Modulate);
	}
}

//-----------------------------------------------------------------------

tVtxBatchFlag cMaterial_Smoke2D::GetBatchFlags(eMaterialRenderType aType) {
	return eVtxBatchFlag_Position | eVtxBatchFlag_Texture0 | eVtxBatchFlag_Color0;
}

//-----------------------------------------------------------------------

bool cMaterial_Smoke2D::NextPass(eMaterialRenderType aType) {
	return false;
}

//-----------------------------------------------------------------------

bool cMaterial_Smoke2D::HasMultiplePasses(eMaterialRenderType aType) {
	return false;
}

//-----------------------------------------------------------------------

eMaterialType cMaterial_Smoke2D::GetType(eMaterialRenderType aType) {
	return mType;
}

//-----------------------------------------------------------------------

void cMaterial_Smoke2D::EditVertexes(eMaterialRenderType aType, iCamera *apCam, iLight *pLight,
									 tVertexVec *apVtxVec, cVector3f *apTransform, unsigned int alIndexAdd) {
}

//-----------------------------------------------------------------------
} // namespace hpl
