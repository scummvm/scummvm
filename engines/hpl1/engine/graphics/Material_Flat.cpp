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

#include "hpl1/engine/graphics/Material_Flat.h"
#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/GpuProgramManager.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/scene/Camera.h"
#include "hpl1/engine/scene/Light.h"
#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMaterial_Flat::cMaterial_Flat(const tString &asName, iLowLevelGraphics *apLowLevelGraphics,
							   cImageManager *apImageManager, cTextureManager *apTextureManager,
							   cRenderer2D *apRenderer, cGpuProgramManager *apProgramManager,
							   eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
	: iMaterial(asName, apLowLevelGraphics, apImageManager, apTextureManager, apRenderer, apProgramManager,
				aPicture, apRenderer3D) {
	mbIsTransperant = false;
	mbIsGlowing = false;
	mbUsesLights = false;

	///////////////////////////////////////////
	// Load the Z pass vertex program
	iGpuProgram *pVtxProg = mpProgramManager->CreateProgram("Diffuse_Color_vp.cg", "main", eGpuProgramType_Vertex);
	SetProgram(pVtxProg, eGpuProgramType_Vertex, 0);
}

//-----------------------------------------------------------------------

cMaterial_Flat::~cMaterial_Flat() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iGpuProgram *cMaterial_Flat::GetVertexProgram(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return NULL; // return mpProgram[eGpuProgramType_Vertex][0];
}

bool cMaterial_Flat::VertexProgramUsesLight(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return false;
}

bool cMaterial_Flat::VertexProgramUsesEye(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return false;
}

iGpuProgram *cMaterial_Flat::GetFragmentProgram(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return NULL;
}

eMaterialAlphaMode cMaterial_Flat::GetAlphaMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Z && mbHasAlpha)
		return eMaterialAlphaMode_Trans;

	return eMaterialAlphaMode_Solid;
}

eMaterialBlendMode cMaterial_Flat::GetBlendMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Z)
		return eMaterialBlendMode_None;

	if (alPass == 1)
		return eMaterialBlendMode_Add;
	else
		return eMaterialBlendMode_Replace;
}

eMaterialChannelMode cMaterial_Flat::GetChannelMode(eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Z)
		return eMaterialChannelMode_Z;
	return eMaterialChannelMode_RGBA;
}

//-----------------------------------------------------------------------

iTexture *cMaterial_Flat::GetTexture(int alUnit, eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	if (aType == eMaterialRenderType_Z) {
		if (alUnit == 0 && mbHasAlpha)
			return mvTexture[eMaterialTexture_Diffuse];
		return NULL;
	}

	if (alUnit == 0) {
		if (alPass == 0)
			return mvTexture[eMaterialTexture_Diffuse];
		else
			return mvTexture[eMaterialTexture_Illumination];
	}

	return NULL;
}

eMaterialBlendMode cMaterial_Flat::GetTextureBlend(int alUnit, eMaterialRenderType aType, int alPass, iLight3D *apLight) {
	return eMaterialBlendMode_Mul;
}

//-----------------------------------------------------------------------

bool cMaterial_Flat::UsesType(eMaterialRenderType aType) {
	if (aType == eMaterialRenderType_Diffuse || aType == eMaterialRenderType_Z)
		return true;
	return false;
}

int cMaterial_Flat::GetNumOfPasses(eMaterialRenderType aType, iLight3D *apLight) {
	if (mvTexture[eMaterialTexture_Illumination])
		return 2;
	return 1;
}

//-----------------------------------------------------------------------

tTextureTypeList cMaterial_Flat::GetTextureTypes() {
	tTextureTypeList vTypes;
	vTypes.push_back(cTextureType("", eMaterialTexture_Diffuse));
	vTypes.push_back(cTextureType("", eMaterialTexture_Illumination));

	return vTypes;
}

//-----------------------------------------------------------------------
} // namespace hpl
