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

#include "hpl1/engine/graphics/Material_BaseLight.h"
#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/GpuProgramManager.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/scene/Camera.h"
#include "hpl1/engine/scene/Light.h"
#include "hpl1/engine/scene/Light3DSpot.h"
#include "hpl1/engine/scene/PortalContainer.h"
#include "hpl1/engine/system/String.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// FRAGMENT PRORGAM SETUP
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

class cAmbProgramSetup : public iMaterialProgramSetup {
public:
	void Setup(iGpuProgram *apProgram, cRenderSettings *apRenderSettings) {
		if (apRenderSettings->mpSector)
			apProgram->SetColor3f("ambientColor", apRenderSettings->mAmbientColor * apRenderSettings->mpSector->GetAmbientColor());
		else
			apProgram->SetColor3f("ambientColor", apRenderSettings->mAmbientColor);
	}
};

static cAmbProgramSetup gAmbProgramSetup;

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iMaterial_BaseLight::iMaterial_BaseLight(const tString &asLightVertexProgram,
										 const tString &asLightFragmentProgram,
										 const tString &asName, iLowLevelGraphics *apLowLevelGraphics,
										 cImageManager *apImageManager, cTextureManager *apTextureManager,
										 cRenderer2D *apRenderer, cGpuProgramManager *apProgramManager,
										 eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
	: iMaterial(asName, apLowLevelGraphics, apImageManager, apTextureManager, apRenderer, apProgramManager,
				aPicture, apRenderer3D) {
	mbIsTransperant = false;
	mbIsGlowing = false;
	mbUsesLights = true;
	mbUseColorSpecular = false;

	for (int i = 0; i < eBaseLightProgram_LastEnum; i++) {
		mvVtxPrograms[i] = NULL;
		mvFragPrograms[i] = NULL;
	}

	////////////////////////////////////////
	// Get names for other light programs
	tString sSpotVtxProgram = cString::Sub(asLightVertexProgram, 0, (int)asLightVertexProgram.size() - 5) +
							  "Spot_vp.cg";

	///////////////////////////////////////////
	// Load the light pass vertex program
	// Point
	mvVtxPrograms[eBaseLightProgram_Point1] = mpProgramManager->CreateProgram(asLightVertexProgram, "main",
																			  eGpuProgramType_Vertex);
	// Spot
	mvVtxPrograms[eBaseLightProgram_Spot1] = mpProgramManager->CreateProgram(sSpotVtxProgram, "main",
																			 eGpuProgramType_Vertex);

	///////////////////////////////////////////
	// Load the light pass fragment program
	// Point
	mvFragPrograms[eBaseLightProgram_Point1] = mpProgramManager->CreateProgram(asLightFragmentProgram, "main",
																			   eGpuProgramType_Fragment);

	//////////////////////////////////////////////////////
	// Check if there is enough texture units for 1 pass spot
	if (mpLowLevelGraphics->GetCaps(eGraphicCaps_MaxTextureImageUnits) > 4) {
		mbUsesTwoPassSpot = false;

		tString sSpotFragProgram = cString::Sub(asLightFragmentProgram, 0, (int)asLightFragmentProgram.size() - 5) +
								   "Spot_fp.cg";

		mvFragPrograms[eBaseLightProgram_Spot1] = mpProgramManager->CreateProgram(sSpotFragProgram, "main",
																				  eGpuProgramType_Fragment);
	} else {
		mbUsesTwoPassSpot = true;

		tString sSpotFragProgram1 = "Diffuse_Light_Spot_fp_pass1.cg"; // cString::Sub(asLightFragmentProgram,0, (int)asLightFragmentProgram.size() - 5) +
																	  //			"Spot_fp_pass1.cg";
		tString sSpotFragProgram2 = cString::Sub(asLightFragmentProgram, 0, (int)asLightFragmentProgram.size() - 5) +
									"Spot_fp_pass2.cg";

		mvFragPrograms[eBaseLightProgram_Spot1] = mpProgramManager->CreateProgram(sSpotFragProgram1, "main",
																				  eGpuProgramType_Fragment);

		mvFragPrograms[eBaseLightProgram_Spot2] = mpProgramManager->CreateProgram(sSpotFragProgram2, "main",
																				  eGpuProgramType_Fragment);
	}

	///////////////////////////////////////////
	// Load the Z pass vertex program
	iGpuProgram *pVtxProg = mpProgramManager->CreateProgram("Diffuse_Color_vp.cg", "main", eGpuProgramType_Vertex);
	SetProgram(pVtxProg, eGpuProgramType_Vertex, 1);

	///////////////////////////////////////////
	// More fragment programs
	mpSimpleFP = mpProgramManager->CreateProgram("Diffuse_Color_fp.cg", "main", eGpuProgramType_Fragment);
	mpAmbientFP = mpProgramManager->CreateProgram("Ambient_Color_fp.cg", "main", eGpuProgramType_Fragment);

	///////////////////////////////////////////
	// Normalization map
	mpNormalizationMap = mpTextureManager->CreateCubeMap("Normalization", false);
	mpNormalizationMap->SetWrapS(eTextureWrap_ClampToEdge);
	mpNormalizationMap->SetWrapT(eTextureWrap_ClampToEdge);

	///////////////////////////////////////////
	// Negative rejection
	mpSpotNegativeRejectMap = mpTextureManager->Create1D("core_spot_negative_reject", false);
	if (mpSpotNegativeRejectMap) {
		mpSpotNegativeRejectMap->SetWrapS(eTextureWrap_ClampToEdge);
		mpSpotNegativeRejectMap->SetWrapT(eTextureWrap_ClampToEdge);
	}

	mbUseSpecular = false;
	mbUseNormalMap = false;
}

//-----------------------------------------------------------------------

iMaterial_BaseLight::~iMaterial_BaseLight() {
	if (mpNormalizationMap)
		mpTextureManager->Destroy(mpNormalizationMap);
	if (mpSpotNegativeRejectMap)
		mpTextureManager->Destroy(mpSpotNegativeRejectMap);

	for (int i = 0; i < eBaseLightProgram_LastEnum; i++) {
		if (mvVtxPrograms[i])
			mpProgramManager->Destroy(mvVtxPrograms[i]);
		if (mvFragPrograms[i])
			mpProgramManager->Destroy(mvFragPrograms[i]);
	}

	if (mpSimpleFP)
		mpProgramManager->Destroy(mpSimpleFP);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iGpuProgram *iMaterial_BaseLight::GetVertexProgram(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (apLight && aType == eMaterialRenderType_Light) {
		eBaseLightProgram program;

		if (apLight->GetLightType() == eLight3DType_Point)
			program = eBaseLightProgram_Point1;
		else if (apLight->GetLightType() == eLight3DType_Spot)
			program = eBaseLightProgram_Spot1;

		return mvVtxPrograms[program];
	}

	if (aType == eMaterialRenderType_Z)
		return mpProgram[eGpuProgramType_Vertex][1];
	if (aType == eMaterialRenderType_Diffuse)
		return mpProgram[eGpuProgramType_Vertex][1];

	return NULL;
}

//------------------------------------------------------------------------------------

bool iMaterial_BaseLight::VertexProgramUsesLight(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Light)
		return true;
	return false;
}

//------------------------------------------------------------------------------------

bool iMaterial_BaseLight::VertexProgramUsesEye(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Light && mbUseSpecular)
		return true;
	return false;
}

//------------------------------------------------------------------------------------

iGpuProgram *iMaterial_BaseLight::GetFragmentProgram(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Light) {
		eBaseLightProgram program;

		///////////////
		// Spot two pass
		if (aType == eMaterialRenderType_Light && apLight->GetLightType() == eLight3DType_Spot && mbUsesTwoPassSpot) {
			if (alPass == 0)
				program = eBaseLightProgram_Spot1;
			else
				program = eBaseLightProgram_Spot2;
		}
		//////////////////
		// Other
		else {
			if (apLight->GetLightType() == eLight3DType_Point)
				program = eBaseLightProgram_Point1;
			else if (apLight->GetLightType() == eLight3DType_Spot)
				program = eBaseLightProgram_Spot1;
		}

		return mvFragPrograms[program];
	} else if (aType == eMaterialRenderType_Diffuse) {
		return mpSimpleFP;
	} else if (aType == eMaterialRenderType_Z) {
		return mpAmbientFP;
	}
	return NULL;
}

iMaterialProgramSetup *iMaterial_BaseLight::GetFragmentProgramSetup(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Z) {
		return &gAmbProgramSetup;
	}

	return NULL;
}

//------------------------------------------------------------------------------------

eMaterialAlphaMode iMaterial_BaseLight::GetAlphaMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Z && mbHasAlpha)
		return eMaterialAlphaMode_Trans;

	return eMaterialAlphaMode_Solid;
}

//------------------------------------------------------------------------------------

eMaterialBlendMode iMaterial_BaseLight::GetBlendMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	//////////////////////////////
	// Spot two pass
	if (aType == eMaterialRenderType_Light && apLight->GetLightType() == eLight3DType_Spot && mbUsesTwoPassSpot) {
		if (alPass == 0)
			return eMaterialBlendMode_Replace;
		else if (alPass == 1)
			return eMaterialBlendMode_DestAlphaAdd;
	}

	//////////////////////////////
	// Other
	if (aType == eMaterialRenderType_Z)
		return eMaterialBlendMode_Replace;

	return eMaterialBlendMode_Add;
}

//------------------------------------------------------------------------------------

eMaterialChannelMode iMaterial_BaseLight::GetChannelMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	////////////////////////////
	// Spot two pass:
	if (aType == eMaterialRenderType_Light && apLight->GetLightType() == eLight3DType_Spot && mbUsesTwoPassSpot) {
		if (alPass == 0)
			return eMaterialChannelMode_A;
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

iTexture *iMaterial_BaseLight::GetTexture(int alUnit, eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Z) {
		// if(alUnit==0 && mbHasAlpha) return mvTexture[eMaterialTexture_Diffuse];
		if (alUnit == 0)
			return mvTexture[eMaterialTexture_Diffuse];
		return NULL;
	} else if (aType == eMaterialRenderType_Diffuse) {
		if (alUnit == 0)
			return mvTexture[eMaterialTexture_Illumination];
		return NULL;
	} else if (aType == eMaterialRenderType_Light) {
		//////////////////////////////////////////////
		// Two Pass Spot LIght
		if (mbUsesTwoPassSpot && apLight->GetLightType() == eLight3DType_Spot) {
			if (alPass == 0) {
				switch (alUnit) {
				// Falloff map
				case 0:
					return apLight->GetFalloffMap();

				// Negative rejection:
				case 1:
					return mpSpotNegativeRejectMap;
				}
			} else {
				switch (alUnit) {
				// Diffuse texture
				case 0:
					return mvTexture[eMaterialTexture_Diffuse];

				// Normalmap
				case 1:
					if (mbUseNormalMap)
						return mvTexture[eMaterialTexture_NMap];
					break;

				// Normalization map
				case 2:
					return mpNormalizationMap;
					break;

				// Spotlight texture
				case 3: {
					cLight3DSpot *pSpotLight = static_cast<cLight3DSpot *>(apLight);
					return pSpotLight->GetTexture();
				} break;
				}
			}
		}
		//////////////////////////////////////////////
		// All Other Lighting
		else {
			switch (alUnit) {
				// Diffuse texture
			case 0:
				return mvTexture[eMaterialTexture_Diffuse];

				// Normalmap
			case 1:
				if (mbUseNormalMap)
					return mvTexture[eMaterialTexture_NMap];
				break;

				// Normalization map
			case 2:
				return mpNormalizationMap;
				break;

				// Falloff map
			case 3:
				return apLight->GetFalloffMap();

				// Spotlight texture
			case 4:
				if (apLight->GetLightType() == eLight3DType_Spot) {
					cLight3DSpot *pSpotLight = static_cast<cLight3DSpot *>(apLight);
					return pSpotLight->GetTexture();
				};
				break;
				// Negative rejection
			case 5:
				if (apLight->GetLightType() == eLight3DType_Spot) {
					return mpSpotNegativeRejectMap;
				}
				break;
				// Color specular
			case 6:
				if (mbUseColorSpecular) {
					return mvTexture[eMaterialTexture_Specular];
				}
				break;
			}
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------

eMaterialBlendMode iMaterial_BaseLight::GetTextureBlend(int alUnit, eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return eMaterialBlendMode_None;
}

//-----------------------------------------------------------------------

int iMaterial_BaseLight::GetNumOfPasses(eMaterialRenderType aType, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Light && apLight->GetLightType() == eLight3DType_Spot && mbUsesTwoPassSpot) {
		return 2;
	}

	return 1;
}

//-----------------------------------------------------------------------

bool iMaterial_BaseLight::UsesType(eMaterialRenderType aType) {
	if (aType == eMaterialRenderType_Diffuse) {
		if (mvTexture[eMaterialTexture_Illumination])
			return true;
		else
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------

tTextureTypeList iMaterial_BaseLight::GetTextureTypes() {
	tTextureTypeList vTypes;
	vTypes.push_back(cTextureType("", eMaterialTexture_Diffuse));
	if (mbUseNormalMap)
		vTypes.push_back(cTextureType("_bump", eMaterialTexture_NMap));

	if (mbUseColorSpecular)
		vTypes.push_back(cTextureType("_spec", eMaterialTexture_Specular));

	vTypes.push_back(cTextureType("_illum", eMaterialTexture_Illumination));

	return vTypes;
}

//-----------------------------------------------------------------------
} // namespace hpl
