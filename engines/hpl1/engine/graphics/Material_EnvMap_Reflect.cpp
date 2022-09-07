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

#include "hpl1/engine/graphics/Material_EnvMap_Reflect.h"
#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/GpuProgramManager.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/scene/Camera.h"
#include "hpl1/engine/scene/Camera3D.h"
#include "hpl1/engine/scene/Light.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// MATERIAL SETUP
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cEnvMapReflect_SetUp::Setup(iGpuProgram *apProgram, cRenderSettings *apRenderSettings) {
	apProgram->SetVec3f("eyeWorldPos", apRenderSettings->mpCamera->GetPosition());
}

//-----------------------------------------------------------------------

void cEnvMapReflect_SetUp::SetupMatrix(cMatrixf *apModelMatrix, cRenderSettings *apRenderSettings) {
	// Put here so it is updated with every matrix, just aswell...
	if (apModelMatrix)
		apRenderSettings->gpuProgram->SetMatrixf("objectWorldMatrix", *apModelMatrix);
	else {
		apRenderSettings->gpuProgram->SetMatrixf("objectWorldMatrix", cMatrixf::Identity);
	}
}

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMaterial_EnvMap_Reflect::cMaterial_EnvMap_Reflect(const tString &asName, iLowLevelGraphics *apLowLevelGraphics,
												   cImageManager *apImageManager, cTextureManager *apTextureManager,
												   cRenderer2D *apRenderer, cGpuProgramManager *apProgramManager,
												   eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
	: iMaterial(asName, apLowLevelGraphics, apImageManager, apTextureManager, apRenderer, apProgramManager,
				aPicture, apRenderer3D) {
	mbIsTransperant = false;
	mbIsGlowing = false;
	mbUsesLights = false;

	_diffuseProgram = mpProgramManager->CreateProgram("Diffuse_Color", "Diffuse_Color");
	_diffuseReflectProgram = mpProgramManager->CreateProgram("Diffuse_EnvMap_Reflect", "Diffuse_EnvMap_Reflect");
}

//-----------------------------------------------------------------------

cMaterial_EnvMap_Reflect::~cMaterial_EnvMap_Reflect() {
	if (_diffuseProgram)
		mpProgramManager->Destroy(_diffuseProgram);
	if (_diffuseReflectProgram)
		mpProgramManager->Destroy(_diffuseReflectProgram);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iGpuProgram *cMaterial_EnvMap_Reflect::getGpuProgram(const eMaterialRenderType aType, const int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Z)
		return _diffuseProgram;
	return _diffuseReflectProgram;
}

iMaterialProgramSetup *cMaterial_EnvMap_Reflect::getGpuProgramSetup(const eMaterialRenderType aType, const int alPass, iLight3D *apLight) {
	static cEnvMapReflect_SetUp envMaterialSetup;
	if (aType == eMaterialRenderType_Diffuse)
		return &envMaterialSetup;
	return nullptr;
}

bool cMaterial_EnvMap_Reflect::VertexProgramUsesLight(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return false;
}

bool cMaterial_EnvMap_Reflect::VertexProgramUsesEye(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return false;
}

eMaterialAlphaMode cMaterial_EnvMap_Reflect::GetAlphaMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return eMaterialAlphaMode_Solid;
}

eMaterialBlendMode cMaterial_EnvMap_Reflect::GetBlendMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Z)
		return eMaterialBlendMode_None;

	return eMaterialBlendMode_Replace;
}

eMaterialChannelMode cMaterial_EnvMap_Reflect::GetChannelMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Z)
		return eMaterialChannelMode_Z;
	return eMaterialChannelMode_RGBA;
}

//-----------------------------------------------------------------------

iTexture *cMaterial_EnvMap_Reflect::GetTexture(int alUnit, eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Z)
		return NULL;

	if (alUnit == 0)
		return mvTexture[eMaterialTexture_Diffuse];
	if (alUnit == 1)
		return mvTexture[eMaterialTexture_CubeMap];

	return NULL;
}

eMaterialBlendMode cMaterial_EnvMap_Reflect::GetTextureBlend(int alUnit, eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return eMaterialBlendMode_Mul;
}

//-----------------------------------------------------------------------

bool cMaterial_EnvMap_Reflect::UsesType(eMaterialRenderType aType) {
	if (aType == eMaterialRenderType_Diffuse || aType == eMaterialRenderType_Z)
		return true;
	return false;
}

//-----------------------------------------------------------------------

tTextureTypeList cMaterial_EnvMap_Reflect::GetTextureTypes() {
	tTextureTypeList vTypes;
	vTypes.push_back(cTextureType("", eMaterialTexture_Diffuse));
	vTypes.push_back(cTextureType("cube", eMaterialTexture_CubeMap));

	return vTypes;
}

//-----------------------------------------------------------------------
} // namespace hpl
