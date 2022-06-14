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

#include "hpl1/engine/graphics/Material_Modulative.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/scene/Light.h"
#include "hpl1/engine/scene/Camera.h"
#include "hpl1/engine/resources/GpuProgramManager.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/math/Math.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// FRAGMENT PRORGAM SETUP
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	class cGLState_ModulateFog : public iGLStateProgram
	{
	public:

		cGLState_ModulateFog() : iGLStateProgram("ModulateFog"){}

		void Bind()
		{
			mpLowGfx->SetActiveTextureUnit(1);
			mpLowGfx->SetTextureEnv(eTextureParam_ColorSource2,eTextureSource_Texture);
			mpLowGfx->SetTextureEnv(eTextureParam_ColorSource0,eTextureSource_Constant);
			mpLowGfx->SetTextureEnv(eTextureParam_ColorFunc, eTextureFunc_Interpolate);
			mpLowGfx->SetTextureEnv(eTextureParam_ColorOp2,eTextureOp_OneMinusColor);
			mpLowGfx->SetTextureConstantColor(cColor(1,1,1,1));
		}

		void UnBind()
		{
			mpLowGfx->SetActiveTextureUnit(1);
			mpLowGfx->SetTextureEnv(eTextureParam_ColorSource0,eTextureSource_Texture);
			mpLowGfx->SetTextureEnv(eTextureParam_ColorSource1,eTextureSource_Previous);
			mpLowGfx->SetTextureEnv(eTextureParam_ColorSource2,eTextureSource_Constant);
			mpLowGfx->SetTextureEnv(eTextureParam_ColorOp2,eTextureOp_Color);
			mpLowGfx->SetTextureEnv(eTextureParam_ColorFunc, eTextureFunc_Modulate);
		}

	private:
		void InitData(){}
	};

	static cGLState_ModulateFog gModulateFog;

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// VERTEX PRORGAM SETUP
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	class cFogProgramSetup : public iMaterialProgramSetup
	{
	public:
		void Setup(iGpuProgram *apProgram,cRenderSettings* apRenderSettings)
		{
			apProgram->SetFloat("fogStart",apRenderSettings->mfFogStart);
			apProgram->SetFloat("fogEnd",apRenderSettings->mfFogEnd);
		}
	};

	static cFogProgramSetup gFogProgramSetup;

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cMaterial_Modulative::cMaterial_Modulative(	const tString& asName,iLowLevelGraphics* apLowLevelGraphics,
		cImageManager* apImageManager, cTextureManager *apTextureManager,
		cRenderer2D* apRenderer, cGpuProgramManager* apProgramManager,
		eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
		: iMaterial(asName,apLowLevelGraphics,apImageManager,apTextureManager,apRenderer,apProgramManager,
					aPicture,apRenderer3D)
	{
		mbIsTransperant = true;
		mbIsGlowing= false;
		mbUsesLights = false;

		gModulateFog.SetUp(apLowLevelGraphics);

		mpFogVtxProg = mpProgramManager->CreateProgram("Fog_Trans_vp.cg","main",eGpuProgramType_Vertex);

		if(mpLowLevelGraphics->GetCaps(eGraphicCaps_GL_FragmentProgram))
			mpFogFragProg = mpProgramManager->CreateProgram("Fog_Trans_Mod_fp.cg","main",eGpuProgramType_Fragment);
		else
			mpFogFragProg = NULL;
	}

	//-----------------------------------------------------------------------

	cMaterial_Modulative::~cMaterial_Modulative()
	{
		if(mpFogVtxProg) mpProgramManager->Destroy(mpFogVtxProg);
		if(mpFogFragProg) mpProgramManager->Destroy(mpFogFragProg);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iGpuProgram* cMaterial_Modulative::GetVertexProgram(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		if(mpRenderSettings->mbFogActive)
			return mpFogVtxProg;
		else
			return NULL;
	}

	iMaterialProgramSetup* cMaterial_Modulative::GetVertexProgramSetup(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		if(mpRenderSettings->mbFogActive)
			return &gFogProgramSetup;
		else
			return NULL;
	}

	bool cMaterial_Modulative::VertexProgramUsesLight(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		return false;
	}

	bool cMaterial_Modulative::VertexProgramUsesEye(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		return false;
	}

	iGpuProgram* cMaterial_Modulative::GetFragmentProgram(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		if(mpRenderSettings->mbFogActive)
		{
			if(mpFogFragProg)
				return mpFogFragProg;
			else
				return &gModulateFog;
		}
		else
		{
			return NULL;
		}
	}

	eMaterialAlphaMode cMaterial_Modulative::GetAlphaMode(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		return eMaterialAlphaMode_Solid;
	}

	eMaterialBlendMode cMaterial_Modulative::GetBlendMode(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		return eMaterialBlendMode_Mul;
	}

	eMaterialChannelMode cMaterial_Modulative::GetChannelMode(eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		return eMaterialChannelMode_RGBA;
	}

	//-----------------------------------------------------------------------

	iTexture* cMaterial_Modulative::GetTexture(int alUnit,eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		if(mpRenderSettings->mbFogActive)
		{
			if(alUnit == 0)
				return mvTexture[eMaterialTexture_Diffuse];
			else if(alUnit == 1)
				return mpRenderer3D->GetFogAddTexture();
		}
		else
		{
			if(alUnit == 0)
				return mvTexture[eMaterialTexture_Diffuse];
		}

		return NULL;
	}

	eMaterialBlendMode cMaterial_Modulative::GetTextureBlend(int alUnit,eMaterialRenderType aType, int alPass, iLight3D *apLight)
	{
		return eMaterialBlendMode_Mul;
	}

	//-----------------------------------------------------------------------

	bool cMaterial_Modulative::UsesType(eMaterialRenderType aType)
	{
		if(aType == eMaterialRenderType_Diffuse) return true;
		return false;
	}

	//-----------------------------------------------------------------------

	tTextureTypeList cMaterial_Modulative::GetTextureTypes()
	{
		tTextureTypeList vTypes;
		vTypes.push_back(cTextureType("",eMaterialTexture_Diffuse));
		vTypes.push_back(cTextureType("_ref",eMaterialTexture_Refraction));
		vTypes.push_back(cTextureType("_spec",eMaterialTexture_Specular));

		return vTypes;
	}

	//-----------------------------------------------------------------------
}
