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

#include "hpl1/engine/resources/MeshLoaderHandler.h"

#include "hpl1/engine/resources/MeshLoader.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

bool iMeshLoader::mbRestricStaticLightToSector = false;
bool iMeshLoader::mbUseFastMaterial = false;

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMeshLoaderHandler::cMeshLoaderHandler(cResources *apResources, cScene *apScene) {
	mpResources = apResources;
	mpScene = apScene;
}

//-----------------------------------------------------------------------

cMeshLoaderHandler::~cMeshLoaderHandler() {
	tMeshLoaderListIt it = mlstLoaders.begin();
	for (; it != mlstLoaders.end(); it++) {
		hplDelete(*it);
	}

	mlstLoaders.clear();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMesh *cMeshLoaderHandler::LoadMesh(const tString &asFile, tMeshLoadFlag aFlags) {
	tString sType = cString::ToLowerCase(cString::GetFileExt(asFile));

	tMeshLoaderListIt it = mlstLoaders.begin();
	for (; it != mlstLoaders.end(); it++) {
		iMeshLoader *pLoader = *it;

		if (pLoader->IsSupported(sType)) {
			return pLoader->LoadMesh(asFile, aFlags);
		}
	}

	Log("No loader for '%s' found!\n", sType.c_str());
	return NULL;
}

//-----------------------------------------------------------------------

bool cMeshLoaderHandler::SaveMesh(cMesh *apMesh, const tString &asFile) {
	tString sType = cString::ToLowerCase(cString::GetFileExt(asFile));

	tMeshLoaderListIt it = mlstLoaders.begin();
	for (; it != mlstLoaders.end(); it++) {
		iMeshLoader *pLoader = *it;

		if (pLoader->IsSupported(sType)) {
			return pLoader->SaveMesh(apMesh, asFile);
		}
	}

	Log("No loader for '%s' found!\n", sType.c_str());
	return false;
}

//-----------------------------------------------------------------------

cWorld3D *cMeshLoaderHandler::LoadWorld(const tString &asFile, tWorldLoadFlag aFlags) {
	tString sType = cString::ToLowerCase(cString::GetFileExt(asFile));

	tMeshLoaderListIt it = mlstLoaders.begin();
	for (; it != mlstLoaders.end(); it++) {
		iMeshLoader *pLoader = *it;

		if (pLoader->IsSupported(sType)) {
			return pLoader->LoadWorld(asFile, mpScene, aFlags);
		}
	}

	Log("No loader for '%s' found!\n", sType.c_str());
	return NULL;
}

//-----------------------------------------------------------------------

cAnimation *cMeshLoaderHandler::LoadAnimation(const tString &asFile) {
	tString sType = cString::ToLowerCase(cString::GetFileExt(asFile));

	tMeshLoaderListIt it = mlstLoaders.begin();
	for (; it != mlstLoaders.end(); it++) {
		iMeshLoader *pLoader = *it;

		if (pLoader->IsSupported(sType)) {
			return pLoader->LoadAnimation(asFile);
		}
	}

	Log("No loader for '%s' found!\n", sType.c_str());
	return NULL;
}

//-----------------------------------------------------------------------

void cMeshLoaderHandler::AddLoader(iMeshLoader *apLoader) {
	mlstLoaders.push_back(apLoader);

	apLoader->mpMaterialManager = mpResources->GetMaterialManager();
	apLoader->mpMeshManager = mpResources->GetMeshManager();
	apLoader->mpAnimationManager = mpResources->GetAnimationManager();
	apLoader->mpSystem = mpScene->GetSystem();

	apLoader->AddSupportedTypes(&mvSupportedTypes);
}

//-----------------------------------------------------------------------
} // namespace hpl
