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

#include "hpl1/engine/graphics/Material_Modulative.h"
#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/GpuProgramManager.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/scene/Camera.h"
#include "hpl1/engine/scene/Light.h"

namespace hpl {

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// VERTEX PRORGAM SETUP
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

class cFogProgramSetup : public iMaterialProgramSetup {
public:
	void Setup(iGpuProgram *apProgram, cRenderSettings *apRenderSettings) {
		apProgram->SetFloat("fogStart", apRenderSettings->mfFogStart);
		apProgram->SetFloat("fogEnd", apRenderSettings->mfFogEnd);
	}
};

static cFogProgramSetup gFogProgramSetup;

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMaterial_Modulative::cMaterial_Modulative(const tString &asName, iLowLevelGraphics *apLowLevelGraphics,
										   cImageManager *apImageManager, cTextureManager *apTextureManager,
										   cRenderer2D *apRenderer, cGpuProgramManager *apProgramManager,
										   eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
	: iMaterial(asName, apLowLevelGraphics, apImageManager, apTextureManager, apRenderer, apProgramManager,
				aPicture, apRenderer3D) {
	mbIsTransperant = true;
	mbIsGlowing = false;
	mbUsesLights = false;

	_fogShader = mpProgramManager->CreateProgram("Fog_Trans", "Fog_Trans_Mod");
}

//-----------------------------------------------------------------------

cMaterial_Modulative::~cMaterial_Modulative() {
	if (_fogShader)
		mpProgramManager->Destroy(_fogShader);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iGpuProgram *cMaterial_Modulative::getGpuProgram(const eMaterialRenderType aType, const int alPass, iLight3D *apLight) {
	if (mpRenderSettings->mbFogActive)
		return _fogShader;
	return nullptr;
}

iMaterialProgramSetup *cMaterial_Modulative::getGpuProgramSetup(const eMaterialRenderType aType, const int alPass, iLight3D *apLight) {
	if (mpRenderSettings->mbFogActive)
		return &gFogProgramSetup;
	return nullptr;
}

bool cMaterial_Modulative::VertexProgramUsesLight(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return false;
}

bool cMaterial_Modulative::VertexProgramUsesEye(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return false;
}

eMaterialAlphaMode cMaterial_Modulative::GetAlphaMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return eMaterialAlphaMode_Solid;
}

eMaterialBlendMode cMaterial_Modulative::GetBlendMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return eMaterialBlendMode_Mul;
}

eMaterialChannelMode cMaterial_Modulative::GetChannelMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return eMaterialChannelMode_RGBA;
}

//-----------------------------------------------------------------------

iTexture *cMaterial_Modulative::GetTexture(int alUnit, eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (mpRenderSettings->mbFogActive) {
		if (alUnit == 0)
			return mvTexture[eMaterialTexture_Diffuse];
		else if (alUnit == 1)
			return mpRenderer3D->GetFogAddTexture();
	} else {
		if (alUnit == 0)
			return mvTexture[eMaterialTexture_Diffuse];
	}

	return NULL;
}

eMaterialBlendMode cMaterial_Modulative::GetTextureBlend(int alUnit, eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return eMaterialBlendMode_Mul;
}

//-----------------------------------------------------------------------

bool cMaterial_Modulative::UsesType(eMaterialRenderType aType) {
	if (aType == eMaterialRenderType_Diffuse)
		return true;
	return false;
}

//-----------------------------------------------------------------------

tTextureTypeList cMaterial_Modulative::GetTextureTypes() {
	tTextureTypeList vTypes;
	vTypes.push_back(cTextureType("", eMaterialTexture_Diffuse));
	vTypes.push_back(cTextureType("_ref", eMaterialTexture_Refraction));
	vTypes.push_back(cTextureType("_spec", eMaterialTexture_Specular));

	return vTypes;
}

//-----------------------------------------------------------------------
} // namespace hpl
