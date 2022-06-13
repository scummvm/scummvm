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
#include "hpl1/engine/resources/ScriptManager.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/System.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/system/Script.h"
#include "hpl1/engine/system/LowLevelSystem.h"



namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cScriptManager::cScriptManager(cSystem* apSystem,cResources *apResources)
		: iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(),
							apResources->GetLowLevelSystem())
	{
		mpSystem = apSystem;
		mpResources = apResources;
	}

	cScriptManager::~cScriptManager()
	{
		DestroyAll();
		Log(" Done with scripts\n");
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iScript* cScriptManager::CreateScript(const tString& asName)
	{
		tString sPath;
		iScript* pScript;
		tString asNewName;

		BeginLoad(asName);

		asNewName = cString::SetFileExt(asName,"hps");

		pScript = static_cast<iScript*>(this->FindLoadedResource(asNewName,sPath));

		if(pScript==NULL && sPath!="")
		{
			pScript = mpSystem->GetLowLevel()->CreateScript(asNewName);

			if(pScript->CreateFromFile(sPath)==false){
				hplDelete(pScript);
				EndLoad();
				return NULL;
			}

			AddResource(pScript);
		}

		if(pScript)pScript->IncUserCount();
		else Error("Couldn't create script '%s'\n",asNewName.c_str());

		EndLoad();
		return pScript;
	}

	//-----------------------------------------------------------------------

	iResourceBase* cScriptManager::Create(const tString& asName)
	{
		return CreateScript(asName);
	}

	//-----------------------------------------------------------------------

	void cScriptManager::Unload(iResourceBase* apResource)
	{

	}
	//-----------------------------------------------------------------------

	void cScriptManager::Destroy(iResourceBase* apResource)
	{
		apResource->DecUserCount();

		if(apResource->HasUsers()==false){
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
}
