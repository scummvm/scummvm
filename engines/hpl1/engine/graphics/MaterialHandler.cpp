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

#include "hpl1/engine/graphics/MaterialHandler.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/resources/ResourceImage.h"
#include "hpl1/engine/resources/Resources.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMaterialHandler::cMaterialHandler(cGraphics *apGraphics, cResources *apResources) {
	mpGraphics = apGraphics;
	mpResources = apResources;
}

//-----------------------------------------------------------------------

cMaterialHandler::~cMaterialHandler() {
	tMaterialTypeListIt it = mlstMatTypes.begin();
	for (; it != mlstMatTypes.end(); it++) {
		hplDelete(*it);
	}

	mlstMatTypes.clear();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cMaterialHandler::Add(iMaterialType *apTypedata) {
	mlstMatTypes.push_back(apTypedata);
}

//-----------------------------------------------------------------------
iMaterial *cMaterialHandler::Create(tString asMatName, eMaterialPicture mPicType) {
	return Create("", asMatName, mPicType);
}

iMaterial *cMaterialHandler::Create(const tString &asName, tString asMatName, eMaterialPicture mPicType) {
	iMaterial *pMat = NULL;
	// cResourceImage *pImage = NULL;
	unsigned int lIdCount = 1;

	for (tMaterialTypeListIt it = mlstMatTypes.begin(); it != mlstMatTypes.end(); it++) {
		if ((*it)->IsCorrect(asMatName)) {
			pMat = (*it)->Create(asName, mpGraphics->GetLowLevel(), mpResources->GetImageManager(),
								 mpResources->GetTextureManager(),
								 mpGraphics->GetRenderer2D(), mpResources->GetGpuProgramManager(),
								 mPicType, mpGraphics->GetRenderer3D());

			// Set an id to the material for easier rendering later on.
			pMat->SetId(lIdCount);

			break;
		}

		lIdCount++;
	}

	return pMat;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

} // namespace hpl
