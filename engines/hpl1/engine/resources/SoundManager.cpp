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

#include "hpl1/engine/resources/SoundManager.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/sound/LowLevelSound.h"
#include "hpl1/engine/sound/Sound.h"
#include "hpl1/engine/sound/SoundData.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/debug.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSoundManager::cSoundManager(cSound *apSound, cResources *apResources)
	: iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(),
					   apResources->GetLowLevelSystem()) {
	mpSound = apSound;
	mpResources = apResources;

	mpSound->GetLowLevel()->GetSupportedFormats(mlstFileFormats);
}

cSoundManager::~cSoundManager() {
	DestroyAll();
	Log(" Done with sounds\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iSoundData *cSoundManager::CreateSoundData(const tString &asName, bool abStream, bool abLoopStream) {
	tString sPath;
	iSoundData *pSound = NULL;

	BeginLoad(asName);

	pSound = FindData(asName, sPath);

	if (pSound == NULL && sPath != "") {
		pSound = mpSound->GetLowLevel()->LoadSoundData(cString::GetFileName(sPath), sPath, "", abStream,
													   abLoopStream);
		if (pSound) {
			AddResource(pSound);
			pSound->SetSoundManager(mpResources->GetSoundManager());
		} else {
			Hpl1::logError(Hpl1::kDebugAudio, "sound data %s could not be loaded\n", asName.c_str());
		}

	} else {
	}

	// if(!pSound) Error("Couldn't load sound data '%s'\n",asName.c_str());
	EndLoad();
	return pSound;
}

//-----------------------------------------------------------------------

iResourceBase *cSoundManager::Create(const tString &asName) {
	return CreateSoundData(asName, false);
}

//-----------------------------------------------------------------------

void cSoundManager::Unload(iResourceBase *apResource) {
}
//-----------------------------------------------------------------------

void cSoundManager::Destroy(iResourceBase *apResource) {
	// Log("Destroying %s users: %d\n",apResource->GetName().c_str(),
	//								apResource->GetUserCount());
	if (apResource->HasUsers()) {
		apResource->DecUserCount();

		iSoundData *pData = static_cast<iSoundData *>(apResource);
		if (pData->IsStream() && pData->HasUsers() == false) {
			RemoveResource(pData);
			hplDelete(pData);
		}
	}
}

//-----------------------------------------------------------------------

void cSoundManager::DestroyAll() {
	// FIX FIX FIX
	// Fix what? :P
	tResourceHandleMapIt it = m_mapHandleResources.begin();
	while (it != m_mapHandleResources.end()) {
		iResourceBase *pData = it->second;
		RemoveResource(pData);
		hplDelete(pData);

		it = m_mapHandleResources.begin();
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iSoundData *cSoundManager::FindData(const tString &asName, tString &asFilePath) {
	iSoundData *pData = NULL;

	if (cString::GetFileExt(asName) == "") {
		for (tStringListIt it = mlstFileFormats.begin(); it != mlstFileFormats.end(); ++it) {
			tString sNewName = cString::SetFileExt(asName, *it);
			pData = static_cast<iSoundData *>(FindLoadedResource(sNewName, asFilePath));

			if ((pData == NULL && asFilePath != "") || pData != NULL)
				break;
		}
	} else {
		pData = static_cast<iSoundData *>(FindLoadedResource(asName, asFilePath));
	}

	return pData;
}

//-----------------------------------------------------------------------
} // namespace hpl
