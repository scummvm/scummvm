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
#include "hpl1/engine/graphics/Material_Diffuse2D.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/scene/Light.h"
#include "hpl1/engine/scene/Camera.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cMaterial_Diffuse2D::cMaterial_Diffuse2D(const tString& asName,iLowLevelGraphics* apLowLevelGraphics,
		cImageManager* apImageManager, cTextureManager *apTextureManager,
		cRenderer2D* apRenderer, cGpuProgramManager* apProgramManager,
		eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
	: iMaterial(asName,apLowLevelGraphics,apImageManager,apTextureManager,apRenderer,apProgramManager,
				aPicture,apRenderer3D)
	{
		mbIsTransperant = false;
		mbIsGlowing= false;

		mType = eMaterialType_Diffuse;
	}

	//-----------------------------------------------------------------------

	cMaterial_Diffuse2D::~cMaterial_Diffuse2D()
	{

	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cMaterial_Diffuse2D::Compile()
	{

	}

	//-----------------------------------------------------------------------

	bool  cMaterial_Diffuse2D::StartRendering(eMaterialRenderType aType,iCamera* apCam,iLight *pLight)
	{
		if(aType == eMaterialRenderType_Z)
		{
			mpLowLevelGraphics->SetBlendActive(false);
			mpLowLevelGraphics->SetTexture(0, GetTexture(eMaterialTexture_Diffuse));
			mpLowLevelGraphics->SetTextureEnv(eTextureParam_AlphaFunc,eTextureFunc_Replace);
			mpLowLevelGraphics->SetAlphaTestActive(true);
			mpLowLevelGraphics->SetAlphaTestFunc(eAlphaTestFunc_GreaterOrEqual, 0.6f);
		}
		else if(aType == eMaterialRenderType_Light)
		{
			return false;
		}
		else if(aType == eMaterialRenderType_Diffuse)
		{
			mpLowLevelGraphics->SetBlendActive(true);
			mpLowLevelGraphics->SetBlendFunc(eBlendFunc_One,eBlendFunc_Zero);

			mpLowLevelGraphics->SetTexture(0, GetTexture(eMaterialTexture_Diffuse));
		}
		return true;
	}
	//-----------------------------------------------------------------------

	void cMaterial_Diffuse2D::EndRendering(eMaterialRenderType aType)
	{
		if(aType == eMaterialRenderType_Z)
		{
			mpLowLevelGraphics->SetAlphaTestActive(false);
			mpLowLevelGraphics->SetTexture(0, NULL);
			mpLowLevelGraphics->SetTextureEnv(eTextureParam_AlphaFunc,eTextureFunc_Modulate);
		}
		if(aType == eMaterialRenderType_Diffuse)
		{
			mpLowLevelGraphics->SetBlendActive(false);
			mpLowLevelGraphics->SetTexture(0, NULL);
		}
		else if(aType == eMaterialRenderType_Light)
		{
		}
	}

	//-----------------------------------------------------------------------

	tVtxBatchFlag cMaterial_Diffuse2D::GetBatchFlags(eMaterialRenderType aType)
	{
		return eVtxBatchFlag_Position |	eVtxBatchFlag_Texture0
				| eVtxBatchFlag_Color0;
	}
	//-----------------------------------------------------------------------

	bool cMaterial_Diffuse2D::NextPass(eMaterialRenderType aType)
	{
		return false;
	}

	//-----------------------------------------------------------------------

	bool cMaterial_Diffuse2D::HasMultiplePasses(eMaterialRenderType aType)
	{
		return false;
	}

	//-----------------------------------------------------------------------

	eMaterialType cMaterial_Diffuse2D::GetType(eMaterialRenderType aType)
	{
		if(aType == eMaterialRenderType_Z) return eMaterialType_DiffuseAlpha;
		return mType;
	}

	//-----------------------------------------------------------------------

	void cMaterial_Diffuse2D::EditVertexes(eMaterialRenderType aType, iCamera* apCam, iLight *pLight,
		tVertexVec *apVtxVec, cVector3f *apTransform, unsigned int alIndexAdd)
	{
	}

	//-----------------------------------------------------------------------
}
