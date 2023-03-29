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

#include "hpl1/engine/graphics/Material_Fallback02_BaseLight.h"
#include "common/algorithm.h"
#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/graphics/Material_BaseLight.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/GpuProgramManager.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/scene/Camera.h"
#include "hpl1/engine/scene/Light.h"
#include "hpl1/engine/scene/Light3DSpot.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

iMaterial_Fallback02_BaseLight::iMaterial_Fallback02_BaseLight(
	const tString &asName, iLowLevelGraphics *apLowLevelGraphics,
	cImageManager *apImageManager, cTextureManager *apTextureManager,
	cRenderer2D *apRenderer, cGpuProgramManager *apProgramManager,
	eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
	: iMaterial(asName, apLowLevelGraphics, apImageManager, apTextureManager, apRenderer, apProgramManager,
				aPicture, apRenderer3D) {

	mbIsTransperant = false;
	mbIsGlowing = false;
	mbUsesLights = true;

	Common::fill(_gpuPrograms, _gpuPrograms + eBaseLightProgram_LastEnum, nullptr);

	_gpuPrograms[eBaseLightProgram_Point1] = mpProgramManager->CreateProgram("hpl1_Fallback02_Diffuse_Light_p1", "hpl1_Fallback02_Diffuse_Light_p1");
	// the second pass is the same as the second pass of fallback01
	_gpuPrograms[eBaseLightProgram_Point2] = mpProgramManager->CreateProgram("hpl1_Fallback02_Diffuse_Light_p2", "hpl1_Fallback01_Diffuse_Light_p2");
	_gpuPrograms[eBaseLightProgram_Spot1] = mpProgramManager->CreateProgram("hpl1_Fallback02_Diffuse_Light_p1", "hpl1_Fallback02_Diffuse_Light_p1");
	_gpuPrograms[eBaseLightProgram_Spot2] = mpProgramManager->CreateProgram("hpl1_Fallback02_Diffuse_Light_Spot_p2", "hpl1_Fallback02_Diffuse_Light_Spot_p2");
	_gpuPrograms[eBaseLightProgram_Spot3] = mpProgramManager->CreateProgram("hpl1_Fallback02_Diffuse_Light_Spot_p3", "hpl1_Fallback02_Diffuse_Light_Spot_p3");

	_diffuseGpuProgram = mpProgramManager->CreateProgram("hpl1_Diffuse_Color", "hpl1_Diffuse_Color");
	_ambientGpuProgram = mpProgramManager->CreateProgram("hpl1_Diffuse_Color", "hpl1_Ambient_Color");

	mpNormalizationMap = mpTextureManager->CreateCubeMap("Normalization", false);
	mpNormalizationMap->SetWrapS(eTextureWrap_ClampToEdge);
	mpNormalizationMap->SetWrapT(eTextureWrap_ClampToEdge);

	mpSpotNegativeRejectMap = mpTextureManager->Create1D("core_spot_negative_reject", false);
	if (mpSpotNegativeRejectMap) {
		mpSpotNegativeRejectMap->SetWrapS(eTextureWrap_ClampToEdge);
		mpSpotNegativeRejectMap->SetWrapT(eTextureWrap_ClampToEdge);
	}
}

//-----------------------------------------------------------------------

iMaterial_Fallback02_BaseLight::~iMaterial_Fallback02_BaseLight() {
	if (mpNormalizationMap)
		mpTextureManager->Destroy(mpNormalizationMap);
	if (mpSpotNegativeRejectMap)
		mpTextureManager->Destroy(mpSpotNegativeRejectMap);

	for (int i = 0; i < eBaseLightProgram_LastEnum; i++) {
		if (_gpuPrograms[i])
			mpProgramManager->Destroy(_gpuPrograms[i]);
	}
	if (_diffuseGpuProgram)
		mpProgramManager->Destroy(_diffuseGpuProgram);
	if (_ambientGpuProgram)
		mpProgramManager->Destroy(_ambientGpuProgram);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iGpuProgram *iMaterial_Fallback02_BaseLight::getGpuProgram(const eMaterialRenderType aType, const int alPass, iLight3D *apLight) {
	eBaseLightProgram program = eBaseLightProgram_Point1;
	if (apLight) {
		if (apLight->GetLightType() == eLight3DType_Point)
			program = eBaseLightProgram_Point1;
		else if (apLight->GetLightType() == eLight3DType_Spot)
			program = eBaseLightProgram_Spot1;
	}

	if (aType == eMaterialRenderType_Light)
		return _gpuPrograms[program + alPass];
	if (aType == eMaterialRenderType_Z)
		return _ambientGpuProgram;
	if (aType == eMaterialRenderType_Diffuse)
		return _diffuseGpuProgram;

	return nullptr;
}

iMaterialProgramSetup *iMaterial_Fallback02_BaseLight::getGpuProgramSetup(const eMaterialRenderType aType, const int alPass, iLight3D *apLight) {
	static cAmbProgramSetup ambProgramSetup;
	if (aType == eMaterialRenderType_Z)
		return &ambProgramSetup;
	return nullptr;
}

//------------------------------------------------------------------------------------

bool iMaterial_Fallback02_BaseLight::VertexProgramUsesLight(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Light) {
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------------

bool iMaterial_Fallback02_BaseLight::VertexProgramUsesEye(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return false;
}

//------------------------------------------------------------------------------------

eMaterialAlphaMode iMaterial_Fallback02_BaseLight::GetAlphaMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Z && mbHasAlpha)
		return eMaterialAlphaMode_Trans;

	return eMaterialAlphaMode_Solid;
}

//------------------------------------------------------------------------------------

eMaterialBlendMode iMaterial_Fallback02_BaseLight::GetBlendMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	//////////////////////////////
	// Light
	if (aType == eMaterialRenderType_Light) {
		if (apLight->GetLightType() == eLight3DType_Point) {
			if (alPass == 0)
				return eMaterialBlendMode_Replace;
			else if (alPass == 1)
				return eMaterialBlendMode_DestAlphaAdd;
		} else {
			switch (alPass) {
			case 0:
				return eMaterialBlendMode_Replace;
			case 1:
				return eMaterialBlendMode_Mul;
			case 2:
				return eMaterialBlendMode_DestAlphaAdd;
			}
		}
	}
	//////////////////////////////
	// Z
	else if (aType == eMaterialRenderType_Z)
		return eMaterialBlendMode_Replace;

	//////////////////////////////
	// Other
	return eMaterialBlendMode_Add;
}

//------------------------------------------------------------------------------------

eMaterialChannelMode iMaterial_Fallback02_BaseLight::GetChannelMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	////////////////////////////
	// Light
	if (aType == eMaterialRenderType_Light) {
		if (apLight->GetLightType() == eLight3DType_Point) {
			if (alPass == 0)
				return eMaterialChannelMode_A;
		} else {
			if (alPass == 0 || alPass == 1)
				return eMaterialChannelMode_A;
		}
	}
	//////////////////////////////
	// Other
	else if (aType == eMaterialRenderType_Z) {
		// return eMaterialChannelMode_Z;
		return eMaterialChannelMode_RGBA;
	}

	return eMaterialChannelMode_RGBA;
}

//-----------------------------------------------------------------------

iTexture *iMaterial_Fallback02_BaseLight::GetTexture(int alUnit, eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	////////////////////////////////////////
	// Z
	if (aType == eMaterialRenderType_Z) {
		// if(alUnit==0 && mbHasAlpha) return mvTexture[eMaterialTexture_Diffuse];
		if (alUnit == 0)
			return mvTexture[eMaterialTexture_Diffuse];
		return NULL;
	}
	////////////////////////////////////////
	// Diffuse
	else if (aType == eMaterialRenderType_Diffuse) {
		if (alUnit == 0)
			return mvTexture[eMaterialTexture_Illumination];
		return NULL;
	}
	////////////////////////////////////////
	// Light
	else if (aType == eMaterialRenderType_Light) {
		if (alPass == 0) {
			switch (alUnit) {
			case 0:
				return mpNormalizationMap;
			case 1: {
				if (apLight->GetTempTexture(0) == NULL) {
					iTexture *pTex = apLight->GetFalloffMap();
					apLight->SetTempTexture(0, mpTextureManager->CreateAttenuation(pTex->GetName()));
				}
				return apLight->GetTempTexture(0);
			}
			}
		} else if (alPass == 1) {
			if (apLight->GetLightType() == eLight3DType_Point) {
				if (alUnit == 0)
					return mvTexture[eMaterialTexture_Diffuse];
			} else {
				if (alUnit == 0)
					return mpSpotNegativeRejectMap;
			}
		} else if (alPass == 2) {
			switch (alUnit) {
			case 0:
				return mvTexture[eMaterialTexture_Diffuse];
			case 1:
				if (apLight->GetLightType() == eLight3DType_Spot) {
					cLight3DSpot *pSpotLight = static_cast<cLight3DSpot *>(apLight);
					return pSpotLight->GetTexture();
				};
				break;
			}
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------

eMaterialBlendMode iMaterial_Fallback02_BaseLight::GetTextureBlend(int alUnit, eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return eMaterialBlendMode_None;
}

//-----------------------------------------------------------------------

int iMaterial_Fallback02_BaseLight::GetNumOfPasses(eMaterialRenderType aType, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Light) {
		if (apLight->GetLightType() == eLight3DType_Point)
			return 2;
		else
			return 3;
	}

	return 1;
}

//-----------------------------------------------------------------------

bool iMaterial_Fallback02_BaseLight::UsesType(eMaterialRenderType aType) {
	if (aType == eMaterialRenderType_Diffuse) {
		if (mvTexture[eMaterialTexture_Illumination])
			return true;
		else
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------

tTextureTypeList iMaterial_Fallback02_BaseLight::GetTextureTypes() {
	tTextureTypeList vTypes;
	vTypes.push_back(cTextureType("", eMaterialTexture_Diffuse));
	vTypes.push_back(cTextureType("_illum", eMaterialTexture_Illumination));

	return vTypes;
}

//-----------------------------------------------------------------------
} // namespace hpl
