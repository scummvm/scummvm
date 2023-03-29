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

#include "hpl1/engine/resources/VideoManager.h"

#include "hpl1/engine/graphics/VideoStream.h"
#include "hpl1/engine/resources/FileSearcher.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cVideoManager::cVideoManager(cGraphics *apGraphics, cResources *apResources)
	: iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(),
					   apResources->GetLowLevelSystem()) {
	mpGraphics = apGraphics;
	mpResources = apResources;
}

cVideoManager::~cVideoManager() {
	STLDeleteAll(mlstVideoLoaders);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iResourceBase *cVideoManager::Create(const tString &asName) {
	return CreateVideo(asName);
}

//-----------------------------------------------------------------------

iVideoStream *cVideoManager::CreateVideo(const tString &asName) {
	BeginLoad(asName);

	tString sPath = mpFileSearcher->GetFilePath(asName);
	if (sPath == "") {
		EndLoad();
		Error("Video file '%s' could not be found!\n", asName.c_str());
		return NULL;
	}

	iVideoStreamLoader *pLoader = GetLoader(asName);
	if (pLoader == NULL) {
		Error("Could not find a loader for '%s'\n", asName.c_str());
		return NULL;
	}

	iVideoStream *pVideo = pLoader->Create(asName);

	if (pVideo->LoadFromFile(sPath) == false) {
		EndLoad();
		hplDelete(pVideo);
		Error("Could not load video '%s'\n", asName.c_str());
		return NULL;
	}

	AddResource(pVideo);

	EndLoad();
	return pVideo;
}

//-----------------------------------------------------------------------

void cVideoManager::AddVideoLoader(iVideoStreamLoader *apLoader) {
	mlstVideoLoaders.push_back(apLoader);
}

//-----------------------------------------------------------------------

void cVideoManager::Unload(iResourceBase *apResource) {
}
//-----------------------------------------------------------------------

void cVideoManager::Destroy(iResourceBase *apResource) {
	if (apResource) {
		RemoveResource(apResource);
		hplDelete(apResource);
	}
}

//-----------------------------------------------------------------------

void cVideoManager::Update(float afTimeStep) {
	tResourceHandleMapIt it = m_mapHandleResources.begin();
	for (; it != m_mapHandleResources.end(); ++it) {
		iResourceBase *pBase = it->second;
		iVideoStream *pVideo = static_cast<iVideoStream *>(pBase);

		pVideo->Update(afTimeStep);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iVideoStreamLoader *cVideoManager::GetLoader(const tString &asFileName) {
	tString sExt = cString::ToLowerCase(cString::GetFileExt(asFileName));

	tVideoStreamLoaderListIt it = mlstVideoLoaders.begin();
	for (; it != mlstVideoLoaders.end(); ++it) {
		iVideoStreamLoader *pLoader = *it;

		tStringVec &vExt = pLoader->GetExtensions();
		for (size_t i = 0; i < vExt.size(); ++i) {
			if (vExt[i] == sExt) {
				return pLoader;
			}
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------
} // namespace hpl
