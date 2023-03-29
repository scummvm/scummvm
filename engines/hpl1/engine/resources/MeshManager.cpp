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

#include "hpl1/engine/resources/MeshManager.h"
#include "hpl1/engine/graphics/Mesh.h"
#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/MeshLoaderHandler.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/System.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMeshManager::cMeshManager(cGraphics *apGraphic, cResources *apResources)
	: iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(),
					   apResources->GetLowLevelSystem()) {
	mpGraphics = apGraphic;
	mpResources = apResources;
}

cMeshManager::~cMeshManager() {
	DestroyAll();

	Log(" Done with meshes\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMesh *cMeshManager::CreateMesh(const tString &asName) {
	tString sPath;
	cMesh *pMesh;
	tString asNewName;

	BeginLoad(asName);

	asNewName = asName; // cString::SetFileExt(asName,"mesh");

	// If the file is missing an extension, search for an existing file.
	if (cString::GetFileExt(asNewName) == "") {
		bool bFound = false;
		tStringVec *pTypes = mpResources->GetMeshLoaderHandler()->GetSupportedTypes();
		for (size_t i = 0; i < pTypes->size(); i++) {
			asNewName = cString::SetFileExt(asNewName, (*pTypes)[i]);
			tString sPath2 = mpResources->GetFileSearcher()->GetFilePath(asNewName);
			if (sPath2 != "") {
				bFound = true;
				break;
			}
		}

		if (bFound == false) {
			Error("Couldn't create mesh '%s'\n", asName.c_str());
			EndLoad();
			return NULL;
		}
	}

	pMesh = static_cast<cMesh *>(this->FindLoadedResource(asNewName, sPath));

	if (pMesh == NULL && sPath != "") {
		pMesh = mpResources->GetMeshLoaderHandler()->LoadMesh(sPath, 0);
		if (pMesh == NULL) {
			EndLoad();
			return NULL;
		}

		AddResource(pMesh);
	}

	if (pMesh)
		pMesh->IncUserCount();
	else
		Error("Couldn't create mesh '%s'\n", asNewName.c_str());

	EndLoad();
	return pMesh;
}

//-----------------------------------------------------------------------

iResourceBase *cMeshManager::Create(const tString &asName) {
	return CreateMesh(asName);
}

//-----------------------------------------------------------------------

void cMeshManager::Unload(iResourceBase *apResource) {
}
//-----------------------------------------------------------------------

void cMeshManager::Destroy(iResourceBase *apResource) {
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
