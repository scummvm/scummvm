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

#include "hpl1/engine/resources/Resources.h"

#include "hpl1/engine/resources/AnimationManager.h"
#include "hpl1/engine/resources/ConfigFile.h"
#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/FontManager.h"
#include "hpl1/engine/resources/GpuProgramManager.h"
#include "hpl1/engine/resources/ImageEntityManager.h"
#include "hpl1/engine/resources/ImageManager.h"
#include "hpl1/engine/resources/LanguageFile.h"
#include "hpl1/engine/resources/MaterialManager.h"
#include "hpl1/engine/resources/MeshLoaderHandler.h"
#include "hpl1/engine/resources/MeshManager.h"
#include "hpl1/engine/resources/ParticleManager.h"
#include "hpl1/engine/resources/ScriptManager.h"
#include "hpl1/engine/resources/SoundEntityManager.h"
#include "hpl1/engine/resources/SoundManager.h"
#include "hpl1/engine/resources/TextureManager.h"
#include "hpl1/engine/resources/TileSetManager.h"
#include "hpl1/engine/resources/VideoManager.h"
#include "hpl1/engine/resources/low_level_resources.h"
#include "hpl1/engine/scene/Area2D.h"
#include "hpl1/engine/system/System.h"

#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/impl/tinyXML/tinyxml.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cResources::cResources(LowLevelResources *apLowLevelResources, iLowLevelGraphics *apLowLevelGraphics)
	: iUpdateable("Resources") {
	mpLowLevelResources = apLowLevelResources;
	mpLowLevelGraphics = apLowLevelGraphics;

	mpFileSearcher = hplNew(cFileSearcher, (mpLowLevelResources));

	mpDefaultEntity3DLoader = NULL;
	mpDefaultArea3DLoader = NULL;

	mpLanguageFile = NULL;
}

//-----------------------------------------------------------------------

cResources::~cResources() {
	Log("Exiting Resources Module\n");
	Log("--------------------------------------------------------\n");

	STLMapDeleteAll(m_mEntity3DLoaders);
	STLMapDeleteAll(m_mArea3DLoaders);
	STLMapDeleteAll(m_mMapEntity2DLoaders);
	STLMapDeleteAll(m_mMapArea2DLoaders);

	hplDelete(mpTileSetManager);
	hplDelete(mpFontManager);
	hplDelete(mpScriptManager);
	hplDelete(mpParticleManager);
	hplDelete(mpSoundManager);
	hplDelete(mpImageEntityManager);
	hplDelete(mpMeshManager);
	hplDelete(mpMaterialManager);
	hplDelete(mpGpuProgramManager);
	hplDelete(mpImageManager);
	hplDelete(mpTextureManager);
	hplDelete(mpSoundEntityManager);
	hplDelete(mpAnimationManager);
	hplDelete(mpVideoManager);

	Log(" All resources deleted\n");

	hplDelete(mpFileSearcher);
	hplDelete(mpMeshLoaderHandler);

	if (mpLanguageFile)
		hplDelete(mpLanguageFile);

	mlstManagers.clear();
	Log("--------------------------------------------------------\n\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cResources::Init(cGraphics *apGraphics, cSystem *apSystem, cSound *apSound, cScene *apScene,
					  cGui *apGui) {
	Log("Initializing Resources Module\n");
	Log("--------------------------------------------------------\n");

	mpLowLevelSystem = apSystem->GetLowLevel();

	Log(" Creating resource managers\n");

	mpImageManager = hplNew(cImageManager, (mpFileSearcher, mpLowLevelGraphics, mpLowLevelResources, mpLowLevelSystem));
	mlstManagers.push_back(mpImageManager);
	mpGpuProgramManager = hplNew(cGpuProgramManager, (mpFileSearcher, mpLowLevelGraphics, mpLowLevelResources, mpLowLevelSystem));
	mlstManagers.push_back(mpGpuProgramManager);
	mpTileSetManager = hplNew(cTileSetManager, (apGraphics, this));
	mlstManagers.push_back(mpTileSetManager);
	mpImageEntityManager = hplNew(cImageEntityManager, (apGraphics, this));
	mlstManagers.push_back(mpImageEntityManager);
	mpParticleManager = hplNew(cParticleManager, (apGraphics, this));
	mlstManagers.push_back(mpParticleManager);
	mpSoundManager = hplNew(cSoundManager, (apSound, this));
	mlstManagers.push_back(mpParticleManager);
	mpFontManager = hplNew(cFontManager, (apGraphics, apGui, this));
	mlstManagers.push_back(mpFontManager);
	mpScriptManager = hplNew(cScriptManager, (apSystem, this));
	mlstManagers.push_back(mpScriptManager);
	mpTextureManager = hplNew(cTextureManager, (apGraphics, this));
	mlstManagers.push_back(mpTextureManager);
	mpMaterialManager = hplNew(cMaterialManager, (apGraphics, this));
	mlstManagers.push_back(mpMaterialManager);
	mpMeshManager = hplNew(cMeshManager, (apGraphics, this));
	mlstManagers.push_back(mpMeshManager);
	mpSoundEntityManager = hplNew(cSoundEntityManager, (apSound, this));
	mlstManagers.push_back(mpSoundEntityManager);
	mpAnimationManager = hplNew(cAnimationManager, (apGraphics, this));
	mlstManagers.push_back(mpAnimationManager);
	mpVideoManager = hplNew(cVideoManager, (apGraphics, this));
	mlstManagers.push_back(mpVideoManager);

	Log(" Misc Creation\n");

	mpMeshLoaderHandler = hplNew(cMeshLoaderHandler, (this, apScene));

	mpLowLevelResources->addMeshLoaders(mpMeshLoaderHandler);
	mpLowLevelResources->addVideoLoaders(mpVideoManager);

	Log("--------------------------------------------------------\n\n");
}

//-----------------------------------------------------------------------

void cResources::Update(float afTimeStep) {
	tResourceManagerListIt it = mlstManagers.begin();
	for (; it != mlstManagers.end(); ++it) {
		iResourceManager *pManager = *it;

		pManager->Update(afTimeStep);
	}
}

//-----------------------------------------------------------------------

cFileSearcher *cResources::GetFileSearcher() {
	return mpFileSearcher;
}

//-----------------------------------------------------------------------

/**
 * \todo File searcher should check so if the dir is allready added and if so return false and not add
 * \param &asDir
 * \param &asMask
 * \return
 */
bool cResources::AddResourceDir(const tString &asDir, const tString &asMask) {
	mpFileSearcher->AddDirectory(asDir, asMask);
	if (iResourceBase::GetLogCreateAndDelete())
		Log(" Added resource directory '%s'\n", asDir.c_str());
	return true;
}

void cResources::ClearResourceDirs() {
	mpFileSearcher->ClearDirectories();
}

//-----------------------------------------------------------------------

bool cResources::SetLanguageFile(const tString &asFile) {
	if (mpLanguageFile) {
		hplDelete(mpLanguageFile);
		mpLanguageFile = NULL;
	}

	tString asPath = mpFileSearcher->GetFilePath(asFile);
	if (asPath == "") {
		Error("Couldn't load language file '%s'\n", asFile.c_str());
		return false;
	}

	mpLanguageFile = hplNew(cLanguageFile, (this));

	return mpLanguageFile->LoadFromFile(asPath);
}

const tWString &cResources::Translate(const tString &asCat, const tString &asName) {
	if (mpLanguageFile) {
		return mpLanguageFile->Translate(asCat, asName);
	} else {
		return mwsEmptyString;
	}
}

//-----------------------------------------------------------------------

void cResources::AddEntity2DLoader(iEntity2DLoader *apLoader) {
	m_mMapEntity2DLoaders.insert(tEntity2DLoaderMap::value_type(apLoader->GetName(), apLoader));
}

iEntity2DLoader *cResources::GetEntity2DLoader(const tString &asName) {
	tEntity2DLoaderMapIt it = m_mMapEntity2DLoaders.find(asName);
	if (it == m_mMapEntity2DLoaders.end()) {
		Warning("No loader for type '%s' found!\n", asName.c_str());
		return NULL;
	}

	return it->second;
}

//-----------------------------------------------------------------------

void cResources::AddArea2DLoader(iArea2DLoader *apLoader) {
	m_mMapArea2DLoaders.insert(tArea2DLoaderMap::value_type(apLoader->GetName(), apLoader));
}

iArea2DLoader *cResources::GetArea2DLoader(const tString &asName) {
	tArea2DLoaderMapIt it = m_mMapArea2DLoaders.find(asName);
	if (it == m_mMapArea2DLoaders.end()) {
		Warning("No loader for type '%s' found!\n", asName.c_str());
		return NULL;
	}

	return it->second;
}

//-----------------------------------------------------------------------

void cResources::AddEntity3DLoader(iEntity3DLoader *apLoader, bool abSetAsDefault) {
	m_mEntity3DLoaders.insert(tEntity3DLoaderMap::value_type(apLoader->GetName(), apLoader));

	if (abSetAsDefault) {
		mpDefaultEntity3DLoader = apLoader;
	}
}

iEntity3DLoader *cResources::GetEntity3DLoader(const tString &asName) {
	tEntity3DLoaderMapIt it = m_mEntity3DLoaders.find(asName);
	if (it == m_mEntity3DLoaders.end()) {
		Warning("No loader for type '%s' found!\n", asName.c_str());

		if (mpDefaultEntity3DLoader) {
			Log("Using default loader!\n");
			return mpDefaultEntity3DLoader;
		} else {
			return NULL;
		}
	}

	return it->second;
}

//-----------------------------------------------------------------------

void cResources::AddArea3DLoader(iArea3DLoader *apLoader, bool abSetAsDefault) {
	m_mArea3DLoaders.insert(tArea3DLoaderMap::value_type(apLoader->GetName(), apLoader));

	if (abSetAsDefault) {
		mpDefaultArea3DLoader = apLoader;
	}
}

iArea3DLoader *cResources::GetArea3DLoader(const tString &asName) {
	tArea3DLoaderMapIt it = m_mArea3DLoaders.find(asName);
	if (it == m_mArea3DLoaders.end()) {
		Warning("No loader for area type '%s' found!\n", asName.c_str());

		if (mpDefaultArea3DLoader) {
			Log("Using default loader!\n");
			return mpDefaultArea3DLoader;
		} else {
			return NULL;
		}
	}

	return it->second;
}

//-----------------------------------------------------------------------

bool cResources::LoadResourceDirsFile(const tString &asFile) {
	TiXmlDocument *pXmlDoc = hplNew(TiXmlDocument, (asFile.c_str()));
	if (pXmlDoc->LoadFile() == false) {
		Error("Couldn't load XML file '%s'!\n", asFile.c_str());
		hplDelete(pXmlDoc);
		return false;
	}

	// Get the root.
	TiXmlElement *pRootElem = pXmlDoc->RootElement();

	TiXmlElement *pChildElem = pRootElem->FirstChildElement();
	for (; pChildElem != NULL; pChildElem = pChildElem->NextSiblingElement()) {
		tString sPath = cString::ToString(pChildElem->Attribute("Path"), "");
		if (sPath == "") {
			continue;
		}

		if (sPath[0] == '/' || sPath[0] == '\\')
			sPath = sPath.substr(1);

		AddResourceDir(sPath);
	}

	hplDelete(pXmlDoc);
	return true;
}

//-----------------------------------------------------------------------

LowLevelResources *cResources::GetLowLevel() {
	return mpLowLevelResources;
}

//-----------------------------------------------------------------------

} // namespace hpl
