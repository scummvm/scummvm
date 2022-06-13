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
#include "hpl1/engine/resources/SoundEntityManager.h"

#include "hpl1/engine/system/String.h"
#include "hpl1/engine/sound/Sound.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/sound/SoundEntityData.h"
#include "hpl1/engine/system/LowLevelSystem.h"
#include "hpl1/engine/sound/SoundHandler.h"
#include "hpl1/engine/sound/SoundChannel.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cSoundEntityManager::cSoundEntityManager(cSound* apSound,cResources *apResources)
		: iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(),
							apResources->GetLowLevelSystem())
	{
		mpSound = apSound;
		mpResources = apResources;
	}

	cSoundEntityManager::~cSoundEntityManager()
	{
		DestroyAll();

		Log(" Done with sound entities\n");
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cSoundEntityManager::Preload(const tString& asFile)
	{
		cSoundEntityData *pData = CreateSoundEntity(asFile);
		if(pData == NULL) {
			Warning("Couldn't preload sound '%s'\n",asFile.c_str());
			return;
		}

		if(pData->GetMainSoundName() != ""){
			iSoundChannel *pChannel = mpSound->GetSoundHandler()->CreateChannel(pData->GetMainSoundName(),0);
			if(pChannel) hplDelete(pChannel);
		}
		if(pData->GetStartSoundName() != ""){
			iSoundChannel *pChannel = mpSound->GetSoundHandler()->CreateChannel(pData->GetStartSoundName(),0);
			if(pChannel) hplDelete(pChannel);
		}
		if(pData->GetStopSoundName() != ""){
			iSoundChannel *pChannel = mpSound->GetSoundHandler()->CreateChannel(pData->GetStopSoundName(),0);
			if(pChannel) hplDelete(pChannel);
		}
	}

	//-----------------------------------------------------------------------

	cSoundEntityData* cSoundEntityManager::CreateSoundEntity(const tString& asName)
	{
		tString sPath;
		cSoundEntityData* pSoundEntity;
		tString asNewName;

		BeginLoad(asName);

		asNewName = cString::SetFileExt(asName,"snt");

		pSoundEntity = static_cast<cSoundEntityData*>(this->FindLoadedResource(asNewName,sPath));

		if(pSoundEntity==NULL && sPath!="")
		{
			pSoundEntity = hplNew( cSoundEntityData, (asNewName) );

			if(pSoundEntity->CreateFromFile(sPath))
			{
				AddResource(pSoundEntity);
			}
			else
			{
				hplDelete(pSoundEntity);
				pSoundEntity =NULL;
			}
		}

		if(pSoundEntity)pSoundEntity->IncUserCount();
		else Error("Couldn't create SoundEntity '%s'\n",asNewName.c_str());

		EndLoad();
		return pSoundEntity;
	}

	//-----------------------------------------------------------------------

	iResourceBase* cSoundEntityManager::Create(const tString& asName)
	{
		return CreateSoundEntity(asName);
	}

	//-----------------------------------------------------------------------

	void cSoundEntityManager::Unload(iResourceBase* apResource)
	{

	}
	//-----------------------------------------------------------------------

	void cSoundEntityManager::Destroy(iResourceBase* apResource)
	{
		apResource->DecUserCount();

		if(apResource->HasUsers()==false){
			RemoveResource(apResource);
			hplDelete(apResource);
		}
	}

	//-----------------------------------------------------------------------
}
