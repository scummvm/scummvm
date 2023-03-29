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

#include "hpl1/engine/resources/ImageEntityManager.h"
#include "common/algorithm.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/graphics/ImageEntityData.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cImageEntityManager::cImageEntityManager(cGraphics *apGraphics, cResources *apResources)
	: iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(),
					   apResources->GetLowLevelSystem()) {
	mpGraphics = apGraphics;
	mpResources = apResources;

	mvImageHandle.resize(eMaterialTexture_LastEnum);
	Common::fill(mvImageHandle.begin(), mvImageHandle.end(), -1);
}

cImageEntityManager::~cImageEntityManager() {
	DestroyAll();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iResourceBase *cImageEntityManager::Create(const tString &asName) {
	tString sPath;
	cImageEntityData *pIEData;
	tString asNewName;

	BeginLoad(asName);

	asNewName = cString::SetFileExt(asName, "hed");

	pIEData = static_cast<cImageEntityData *>(FindLoadedResource(asNewName, sPath));

	if (pIEData == NULL && sPath != "") {
		pIEData = hplNew(cImageEntityData, (asNewName, mpGraphics, mpResources));

		if (pIEData->CreateFromFile(sPath, mvImageHandle) == false) {
			EndLoad();
			return NULL;
		}

		if (pIEData)
			AddResource(pIEData);
	} else {
	}

	if (pIEData)
		pIEData->IncUserCount();
	else
		Error("Couldn't load image entity data '%s'\n", asNewName.c_str());

	EndLoad();
	return pIEData;
}

//-----------------------------------------------------------------------

cImageEntityData *cImageEntityManager::CreateData(const tString &asName) {
	return static_cast<cImageEntityData *>(Create(asName));
}

//-----------------------------------------------------------------------

void cImageEntityManager::Unload(iResourceBase *apResource) {
}
//-----------------------------------------------------------------------

void cImageEntityManager::Destroy(iResourceBase *apResource) {
	apResource->DecUserCount();

	if (apResource->HasUsers() == false) {
		RemoveResource(apResource);
		hplDelete(apResource);
	}
}

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
} // namespace hpl
