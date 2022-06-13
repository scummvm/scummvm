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
#include "hpl1/engine/graphics/Material_Bump.h"

#include "hpl1/engine/graphics/Material_Fallback01_BaseLight.h"
#include "hpl1/engine/graphics/Material_Fallback02_BaseLight.h"
#include "hpl1/engine/graphics/Material_Flat.h"
#include "hpl1/engine/graphics/Material_Alpha.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cMaterial_Bump::cMaterial_Bump(const tString& asName,iLowLevelGraphics* apLowLevelGraphics,
		cImageManager* apImageManager, cTextureManager *apTextureManager,
		cRenderer2D* apRenderer, cGpuProgramManager* apProgramManager,
		eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
		: iMaterial_BaseLight(	"Diffuse_Light_vp.cg",
								"Bump_Light_fp.cg",
			asName,apLowLevelGraphics,apImageManager,apTextureManager,apRenderer,apProgramManager,
			aPicture,apRenderer3D)
	{
		mbUseSpecular = false;
		mbUseNormalMap = true;
	}

	//-----------------------------------------------------------------------

	cMaterial_Bump::~cMaterial_Bump()
	{

	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iMaterial* cMaterialType_Bump::Create(const tString& asName,iLowLevelGraphics* apLowLevelGraphics,
						cImageManager* apImageManager, cTextureManager *apTextureManager,
						cRenderer2D* apRenderer, cGpuProgramManager* apProgramManager,
						eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
	{
		if( apLowLevelGraphics->GetCaps(eGraphicCaps_GL_FragmentProgram) &&
			iMaterial::GetQuality() >= eMaterialQuality_High)
		{
			return hplNew( cMaterial_Bump, (asName,apLowLevelGraphics,
									apImageManager,apTextureManager,apRenderer,
									apProgramManager,aPicture,apRenderer3D) );
		}
		else if(apLowLevelGraphics->GetCaps(eGraphicCaps_MaxTextureImageUnits)>=3 &&
			iMaterial::GetQuality() >= eMaterialQuality_Medium)
		{
			return hplNew( cMaterial_Fallback01_Bump, (asName,apLowLevelGraphics,
											apImageManager,apTextureManager,apRenderer,
											apProgramManager,aPicture,apRenderer3D) );
		}
		else if(apLowLevelGraphics->GetCaps(eGraphicCaps_GL_VertexProgram) &&
			iMaterial::GetQuality() >= eMaterialQuality_Low)
		{
			return hplNew( cMaterial_Fallback02_Diffuse, (asName,apLowLevelGraphics,
										apImageManager,apTextureManager,apRenderer,
										apProgramManager,aPicture,apRenderer3D) );
		}
		else
		{
			return hplNew( cMaterial_Flat, (asName,apLowLevelGraphics,
				apImageManager,apTextureManager,apRenderer,
				apProgramManager,aPicture,apRenderer3D) );
		}
	}

	//-----------------------------------------------------------------------
}
