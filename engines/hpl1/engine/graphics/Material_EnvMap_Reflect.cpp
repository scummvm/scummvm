/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "hpl1/engine/graphics/Material_EnvMap_Reflect.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/scene/Light.h"
#include "hpl1/engine/scene/Camera.h"
#include "hpl1/engine/resources/GpuProgramManager.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/system/LowLevelSystem.h"
#include "hpl1/engine/scene/Camera3D.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL SETUP
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cEnvMapReflect_SetUp::Setup(iGpuProgram *apProgram,cRenderSettings* apRenderSettings)
	{
		apProgram->SetVec3f("eyeWorldPos",apRenderSettings->mpCamera->GetPosition());
	}

	//-----------------------------------------------------------------------

	void cEnvMapReflect_SetUp::SetupMatrix(cMatrixf *apModelMatrix, cRenderSettings* apRenderSettings)
	{
		//Put here so it is updated with every matrix, just aswell...
		if(apModelMatrix)
			apRenderSettings->mpVertexProgram->SetMatrixf("objectWorldMatrix",*apModelMatrix);
		else
		{
			apRenderSettings->mpVertexProgram->SetMatrixf("objectWorldMatrix",cMatrixf::Identity);
		}
	}

	//-----------------------------------------------------------------------

	static cEnvMapReflect_SetUp gEnvMaterialSetup;

	//////////////////////////////////////////////////////////////////////////
	// FRAGEMENT SETUP
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cGLState_EnvMapReflect::cGLState_EnvMapReflect()
		: iGLStateProgram("Internal_Diffuse")
	{
	}

	void cGLState_EnvMapReflect::Bind()
	{
		mpLowGfx->SetActiveTextureUnit(1);
		mpLowGfx->SetTextureEnv(eTextureParam_ColorFunc,eTextureFunc_Interpolate);
		mpLowGfx->SetTextureEnv(eTextureParam_ColorSource2,eTextureSource_Previous);
		mpLowGfx->SetTextureEnv(eTextureParam_ColorOp2,eTextureOp_Alpha);
	}

	void cGLState_EnvMapReflect::UnBind()
	{
		mpLowGfx->SetActiveTextureUnit(1);
		mpLowGfx->SetTextureEnv(eTextureParam_ColorFunc,eTextureFunc_Modulate);

	}

	//-----------------------------------------------------------------------

	cGLState_EnvMapReflect gGLState_EnvMapReflect;

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cMaterial_EnvMap_Reflect::cMaterial_EnvMap_Reflect(	const tString& asName,iLowLevelGraphics* apLowLevelGraphics,
		cImageManager* apImageManager, cTextureManager *apTextureManager,
		cRenderer2D* apRenderer, cGpuProgramManager* apProgramManager,
		eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
		: iMaterial(asName,apLowLevelGraphics,apImageManager,apTextureManager,apRenderer,apProgramManager,
		aPicture,apRenderer3D)
	{
		mbIsTransperant = false;
		mbIsGlowing= false;
		mbUsesLights = false;

		gGLState_EnvMapReflect.SetUp(apLowLevelGraphics);

		///////////////////////////////////////////
		//Load the Z pass vertex program
		iGpuProgram *pVtxProg = mpProgramManager->CreateProgram("Diffuse_Color_vp.cg","main",eGpuProgramType_Vertex);
		SetProgram(pVtxProg,eGpuProgramType_Vertex,0);


		///////////////////////////////////////////
		//Load the Z pass vertex program
		pVtxProg = mpProgramManager->CreateProgram("Diffuse_EnvMap_Reflect_vp.cg","main",eGpuProgramType_Vertex);
		SetProgram(pVtxProg,eGpuProgramType_Vertex,1);
	}

	//-----------------------------------------------------------------------

	cMaterial_EnvMap_Reflect::~cMaterial_EnvMap_Reflect()
	{
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iGpuProgram* cMaterial_EnvMap_Reflect::GetVertexProgram(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		if(aType == eMaterialRenderType_Z)
			return mpProgram[eGpuProgramType_Vertex][0];
		else
			return mpProgram[eGpuProgramType_Vertex][1];
	}

	bool cMaterial_EnvMap_Reflect::VertexProgramUsesLight(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		return false;
	}

	bool cMaterial_EnvMap_Reflect::VertexProgramUsesEye(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		return false;
	}

	iMaterialProgramSetup * cMaterial_EnvMap_Reflect::GetVertexProgramSetup(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		if(aType == eMaterialRenderType_Diffuse) return &gEnvMaterialSetup;

		return NULL;
	}

	iGpuProgram* cMaterial_EnvMap_Reflect::GetFragmentProgram(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		if(aType == eMaterialRenderType_Diffuse) return &gGLState_EnvMapReflect;
		return NULL;
	}

	eMaterialAlphaMode cMaterial_EnvMap_Reflect::GetAlphaMode(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		return eMaterialAlphaMode_Solid;
	}

	eMaterialBlendMode cMaterial_EnvMap_Reflect::GetBlendMode(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		if(aType == eMaterialRenderType_Z) return eMaterialBlendMode_None;

		return eMaterialBlendMode_Replace;
	}

	eMaterialChannelMode cMaterial_EnvMap_Reflect::GetChannelMode(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		if(aType == eMaterialRenderType_Z) return eMaterialChannelMode_Z;
		return eMaterialChannelMode_RGBA;
	}

	//-----------------------------------------------------------------------

	iTexture* cMaterial_EnvMap_Reflect::GetTexture(int alUnit,eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		if(aType == eMaterialRenderType_Z) return NULL;

		if(alUnit == 0)
			return mvTexture[eMaterialTexture_Diffuse];
		if(alUnit == 1)
			return mvTexture[eMaterialTexture_CubeMap];

		return NULL;
	}

	eMaterialBlendMode cMaterial_EnvMap_Reflect::GetTextureBlend(int alUnit,eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		return eMaterialBlendMode_Mul;
	}

	//-----------------------------------------------------------------------

	bool cMaterial_EnvMap_Reflect::UsesType(eMaterialRenderType aType)
	{
		if(aType == eMaterialRenderType_Diffuse || aType == eMaterialRenderType_Z) return true;
		return false;
	}

	//-----------------------------------------------------------------------

	tTextureTypeList cMaterial_EnvMap_Reflect::GetTextureTypes()
	{
		tTextureTypeList vTypes;
		vTypes.push_back(cTextureType("",eMaterialTexture_Diffuse));
		vTypes.push_back(cTextureType("cube",eMaterialTexture_CubeMap));

		return vTypes;
	}

	//-----------------------------------------------------------------------
}
