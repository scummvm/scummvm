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

#include "hpl1/engine/resources/TileSetManager.h"
#include "hpl1/engine/graphics/Graphics.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/scene/TileSet.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cTileSetManager::cTileSetManager(cGraphics *apGraphics, cResources *apResources)
	: iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(),
					   apResources->GetLowLevelSystem()) {
	mpGraphics = apGraphics;
	mpResources = apResources;
}

cTileSetManager::~cTileSetManager() {
	DestroyAll();
	Log(" Done with tilesets\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iResourceBase *cTileSetManager::Create(const tString &asName) {
	tString sPath;
	cTileSet *pTileSet;
	tString asNewName;

	BeginLoad(asName);

	asNewName = cString::SetFileExt(asName, "tsd");

	pTileSet = static_cast<cTileSet *>(this->FindLoadedResource(asNewName, sPath));

	if (pTileSet == NULL && sPath != "") {
		pTileSet = hplNew(cTileSet, (asNewName, mpGraphics, mpResources));

		if (pTileSet->CreateFromFile(sPath) == false) {
			EndLoad();
			return NULL;
		}

		AddResource(pTileSet);
	}

	if (pTileSet)
		pTileSet->IncUserCount();
	else
		Error("Couldn't load tileset '%s'\n", asNewName.c_str());

	EndLoad();
	return pTileSet;
}

//-----------------------------------------------------------------------

cTileSet *cTileSetManager::CreateTileSet(const tString &asName) {
	return static_cast<cTileSet *>(Create(asName));
}

//-----------------------------------------------------------------------

void cTileSetManager::Unload(iResourceBase *apResource) {
}
//-----------------------------------------------------------------------

void cTileSetManager::Destroy(iResourceBase *apResource) {
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
