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

#include "hpl1/engine/graphics/Material_BumpColorSpec.h"

#include "hpl1/engine/graphics/Material_Alpha.h"
#include "hpl1/engine/graphics/Material_Bump.h"
#include "hpl1/engine/graphics/Material_Fallback01_BaseLight.h"
#include "hpl1/engine/graphics/Material_Fallback02_BaseLight.h"
#include "hpl1/engine/graphics/Material_Flat.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMaterial_BumpColorSpec::cMaterial_BumpColorSpec(const tString &asName, iLowLevelGraphics *apLowLevelGraphics,
												 cImageManager *apImageManager, cTextureManager *apTextureManager,
												 cRenderer2D *apRenderer, cGpuProgramManager *apProgramManager,
												 eMaterialPicture aPicture, cRenderer3D *apRenderer3D)
	: iMaterial_BaseLight("hpl1_DiffuseSpec_Light",
						  "hpl1_BumpColorSpec_Light",
						  asName, apLowLevelGraphics, apImageManager, apTextureManager, apRenderer, apProgramManager,
						  aPicture, apRenderer3D) {
	mbUseSpecular = true;
	mbUseNormalMap = true;
	mbUseColorSpecular = true;
}

//-----------------------------------------------------------------------

cMaterial_BumpColorSpec::~cMaterial_BumpColorSpec() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iMaterial *cMaterialType_BumpColorSpec::Create(const tString &asName, iLowLevelGraphics *apLowLevelGraphics,
											   cImageManager *apImageManager, cTextureManager *apTextureManager,
											   cRenderer2D *apRenderer, cGpuProgramManager *apProgramManager,
											   eMaterialPicture aPicture, cRenderer3D *apRenderer3D) {

	if (!apLowLevelGraphics->GetCaps(eGraphicCaps_GL_GpuPrograms) || iMaterial::GetQuality() == eMaterialQuality_VeryLow) {
		return hplNew(cMaterial_Flat, (asName, apLowLevelGraphics,
									   apImageManager, apTextureManager, apRenderer,
									   apProgramManager, aPicture, apRenderer3D));
	}
	if (iMaterial::GetQuality() >= eMaterialQuality_High) {
		if (apLowLevelGraphics->GetCaps(eGraphicCaps_MaxTextureImageUnits) >= 7) {
			return hplNew(cMaterial_BumpColorSpec, (asName, apLowLevelGraphics,
													apImageManager, apTextureManager, apRenderer,
													apProgramManager, aPicture, apRenderer3D));
		} else {
			return hplNew(cMaterial_Bump, (asName, apLowLevelGraphics,
										   apImageManager, apTextureManager, apRenderer,
										   apProgramManager, aPicture, apRenderer3D));
		}
	} else if (apLowLevelGraphics->GetCaps(eGraphicCaps_MaxTextureImageUnits) >= 3 &&
			   iMaterial::GetQuality() >= eMaterialQuality_Medium) {
		return hplNew(cMaterial_Fallback01_Bump, (asName, apLowLevelGraphics,
												  apImageManager, apTextureManager, apRenderer,
												  apProgramManager, aPicture, apRenderer3D));
	} else if (iMaterial::GetQuality() >= eMaterialQuality_Low) {
		return hplNew(cMaterial_Fallback02_Diffuse, (asName, apLowLevelGraphics,
													 apImageManager, apTextureManager, apRenderer,
													 apProgramManager, aPicture, apRenderer3D));
	} else {
		return hplNew(cMaterial_Flat, (asName, apLowLevelGraphics,
									   apImageManager, apTextureManager, apRenderer,
									   apProgramManager, aPicture, apRenderer3D));
	}
}

//-----------------------------------------------------------------------
} // namespace hpl
