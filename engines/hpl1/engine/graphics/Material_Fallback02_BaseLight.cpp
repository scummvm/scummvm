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
#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/GpuProgramManager.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/scene/Camera.h"
#include "hpl1/engine/scene/Light.h"
#include "hpl1/engine/scene/Light3DSpot.h"
#include "hpl1/engine/system/low_level_system.h"
#include "hpl1/engine/system/String.h"

//#include <GL/GLee.h>

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// FRAGMENT PROGRAMS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGLStateTwoUnits_Diffuse::cGLStateTwoUnits_Diffuse()
	: iGLStateProgram("Internal_TwoUnit_Diffuse") {
}

void cGLStateTwoUnits_Diffuse::Bind() {
	mpLowGfx->SetActiveTextureUnit(0);
	mpLowGfx->SetTextureEnv(eTextureParam_ColorFunc, eTextureFunc_Dot3RGBA);
	mpLowGfx->SetTextureEnv(eTextureParam_ColorSource1, eTextureSource_Constant);
	mpLowGfx->SetTextureConstantColor(cColor(0.5f, 0.5f, 1, 0));
}

void cGLStateTwoUnits_Diffuse::UnBind() {
	mpLowGfx->SetActiveTextureUnit(0);
	mpLowGfx->SetTextureEnv(eTextureParam_ColorFunc, eTextureFunc_Modulate);
	mpLowGfx->SetTextureEnv(eTextureParam_ColorSource1, eTextureSource_Previous);
}

//-----------------------------------------------------------------------

cGLStateTwoUnits_ATIDiffuse::cGLStateTwoUnits_ATIDiffuse()
	: iGLStateProgram("Internal_TwoUnit_ATIDiffuse"), mlBind(0) {
}

void cGLStateTwoUnits_ATIDiffuse::InitData() {
#if 0
  		mlBind = glGenFragmentShadersATI(1);

		Log("Creating and binding ATI two unit diffuse shader to %d\n",mlBind);
		glBindFragmentShaderATI(mlBind);

		glBeginFragmentShaderATI();

		glSampleMapATI(GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI);
		glSampleMapATI(GL_REG_1_ATI, GL_TEXTURE1_ARB, GL_SWIZZLE_STR_ATI);

		//Reg0 = NormlizedVec dot3 Vector(0,0,1)
		float vConst[4] = {0,0,1,0};
		glSetFragmentShaderConstantATI( GL_CON_0_ATI,vConst);
		glColorFragmentOp2ATI(	GL_DOT3_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
								GL_REG_0_ATI, GL_NONE, GL_2X_BIT_ATI|GL_BIAS_BIT_ATI,
								GL_CON_0_ATI, GL_NONE, GL_NONE);
		glAlphaFragmentOp2ATI(	GL_DOT3_ATI, GL_REG_0_ATI, GL_NONE,
								GL_REG_0_ATI, GL_NONE, GL_2X_BIT_ATI|GL_BIAS_BIT_ATI,
								GL_CON_0_ATI, GL_NONE, GL_NONE);

		//Reg0 = Light * Attenuation
		glAlphaFragmentOp2ATI(	GL_MUL_ATI, GL_REG_0_ATI, GL_NONE,
								GL_REG_0_ATI, GL_NONE, GL_NONE,
								GL_REG_1_ATI, GL_NONE, GL_NONE);

		glEndFragmentShaderATI();
#endif
}

cGLStateTwoUnits_ATIDiffuse::~cGLStateTwoUnits_ATIDiffuse() {
#if 0
  		Log("Deleting ATI shader to %d\n",mlBind);
		if (mlBind) glDeleteFragmentShaderATI(mlBind);
#endif
}

void cGLStateTwoUnits_ATIDiffuse::Bind() {
#if 0
  		glEnable(GL_FRAGMENT_SHADER_ATI);
		glBindFragmentShaderATI(mlBind);
#endif
}

void cGLStateTwoUnits_ATIDiffuse::UnBind() {
#if 0
  		glDisable(GL_FRAGMENT_SHADER_ATI);
#endif
}

//-----------------------------------------------------------------------

cGLStateTwoUnits_Spot::cGLStateTwoUnits_Spot()
	: iGLStateProgram("Internal_TwoUnit_Spot") {
}

void cGLStateTwoUnits_Spot::Bind() {
}

void cGLStateTwoUnits_Spot::UnBind() {
}

//-----------------------------------------------------------------------
static cGLStateTwoUnits_Diffuse gDiffuseGLState;
static cGLStateTwoUnits_ATIDiffuse gATIDiffuseGLState;
static cGLStateTwoUnits_Spot gSpotGLState;

//-----------------------------------------------------------------------

iMaterial_Fallback02_BaseLight::iMaterial_Fallback02_BaseLight(
	const tString &asName, iLowLevelGraphics *apLowLevelGraphics,
	cImageManager *apImageManager, cTextureManager *apTextureManager,
	cRenderer2D *apRenderer, cGpuProgramManager *apProgramManager,
	eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
	: iMaterial(asName, apLowLevelGraphics, apImageManager, apTextureManager, apRenderer, apProgramManager,
				aPicture, apRenderer3D) {
	gDiffuseGLState.SetUp(mpLowLevelGraphics);
	gSpotGLState.SetUp(mpLowLevelGraphics);

	if (mpLowLevelGraphics->GetCaps(eGraphicCaps_GL_ATIFragmentShader)) {
		gATIDiffuseGLState.SetUp(mpLowLevelGraphics);
	}

	mbIsTransperant = false;
	mbIsGlowing = false;
	mbUsesLights = true;

	tString asLightVertexProgram1 = "Fallback02_Diffuse_Light_p1_vp.cg";
	tString asLightVertexProgram2 = "Fallback02_Diffuse_Light_p2_vp.cg";

	tString asLightSpotVertexProgram1 = "Fallback02_Diffuse_Light_p1_vp.cg";
	tString asLightSpotVertexProgram2 = "Fallback02_Diffuse_Light_Spot_p2_vp.cg";
	tString asLightSpotVertexProgram3 = "Fallback02_Diffuse_Light_Spot_p3_vp.cg";

	for (int i = 0; i < eBaseLightProgram_LastEnum; i++) {
		mvVtxPrograms[i] = NULL;
		mvFragPrograms[i] = NULL;
	}

	///////////////////////////////////////////
	// Load the light pass vertex program
	// Point
	mvVtxPrograms[eBaseLightProgram_Point1] = mpProgramManager->CreateProgram(asLightVertexProgram1,
																			  "main", eGpuProgramType_Vertex);
	mvVtxPrograms[eBaseLightProgram_Point2] = mpProgramManager->CreateProgram(asLightVertexProgram2,
																			  "main", eGpuProgramType_Vertex);
	mvVtxPrograms[eBaseLightProgram_Spot1] = mpProgramManager->CreateProgram(asLightSpotVertexProgram1,
																			 "main", eGpuProgramType_Vertex);
	mvVtxPrograms[eBaseLightProgram_Spot2] = mpProgramManager->CreateProgram(asLightSpotVertexProgram2,
																			 "main", eGpuProgramType_Vertex);
	mvVtxPrograms[eBaseLightProgram_Spot3] = mpProgramManager->CreateProgram(asLightSpotVertexProgram3,
																			 "main", eGpuProgramType_Vertex);

	///////////////////////////////////////////
	// Load the light pass fragment program
	// Point

	if (mpLowLevelGraphics->GetCaps(eGraphicCaps_GL_ATIFragmentShader)) {
		mvFragPrograms[eBaseLightProgram_Point1] = &gATIDiffuseGLState;
		mvFragPrograms[eBaseLightProgram_Spot1] = &gATIDiffuseGLState;

		mvFragPrograms[eBaseLightProgram_Spot2] = NULL; // hplNew( cGLStateTwoUnits_Spot, (mpLowLevelGraphics) );
	} else {
		mvFragPrograms[eBaseLightProgram_Point1] = &gDiffuseGLState;
		mvFragPrograms[eBaseLightProgram_Spot1] = &gDiffuseGLState;

		mvFragPrograms[eBaseLightProgram_Spot2] = NULL; // hplNew( cGLStateTwoUnits_Spot, (mpLowLevelGraphics) );
	}

	///////////////////////////////////////////
	// Load the Z pass vertex program
	iGpuProgram *pVtxProg = mpProgramManager->CreateProgram("Diffuse_Color_vp.cg", "main", eGpuProgramType_Vertex);
	SetProgram(pVtxProg, eGpuProgramType_Vertex, 1);

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
		if (mvVtxPrograms[i])
			mpProgramManager->Destroy(mvVtxPrograms[i]);
		// if(mvFragPrograms[i])	mpProgramManager->Destroy(mvFragPrograms[i]);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iGpuProgram *iMaterial_Fallback02_BaseLight::GetVertexProgram(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	eBaseLightProgram program;
	if (apLight) {
		if (apLight->GetLightType() == eLight3DType_Point)
			program = eBaseLightProgram_Point1;
		else if (apLight->GetLightType() == eLight3DType_Spot)
			program = eBaseLightProgram_Spot1;
	}

	if (aType == eMaterialRenderType_Light) {
		return mvVtxPrograms[program + alPass];
	}
	if (aType == eMaterialRenderType_Z)
		return mpProgram[eGpuProgramType_Vertex][1];
	if (aType == eMaterialRenderType_Diffuse)
		return mpProgram[eGpuProgramType_Vertex][1];

	return NULL;
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

iGpuProgram *iMaterial_Fallback02_BaseLight::GetFragmentProgram(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Light) {
		eBaseLightProgram program = eBaseLightProgram_Point1;

		if (apLight->GetLightType() == eLight3DType_Point)
			program = eBaseLightProgram_Point1;
		else if (apLight->GetLightType() == eLight3DType_Spot)
			program = eBaseLightProgram_Spot1;

		return mvFragPrograms[program + alPass];
	}
	return NULL;
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
