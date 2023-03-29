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

#include "hpl1/engine/resources/ScriptManager.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/system/Script.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/System.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cScriptManager::cScriptManager(cSystem *apSystem, cResources *apResources)
	: iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(),
					   apResources->GetLowLevelSystem()) {
	mpSystem = apSystem;
	mpResources = apResources;
}

cScriptManager::~cScriptManager() {
	DestroyAll();
	Log(" Done with scripts\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iScript *cScriptManager::CreateScript(const tString &asName) {
	tString sPath;
	iScript *pScript;
	tString asNewName;

	BeginLoad(asName);

	asNewName = cString::SetFileExt(asName, "hps");

	pScript = static_cast<iScript *>(this->FindLoadedResource(asNewName, sPath));

	if (pScript == NULL && sPath != "") {
		pScript = mpSystem->GetLowLevel()->createScript(asNewName);

		if (pScript->CreateFromFile(sPath) == false) {
			hplDelete(pScript);
			EndLoad();
			return NULL;
		}

		AddResource(pScript);
	}

	if (pScript)
		pScript->IncUserCount();
	else
		Error("Couldn't create script '%s'\n", asNewName.c_str());

	EndLoad();
	return pScript;
}

//-----------------------------------------------------------------------

iResourceBase *cScriptManager::Create(const tString &asName) {
	return CreateScript(asName);
}

//-----------------------------------------------------------------------

void cScriptManager::Unload(iResourceBase *apResource) {
}
//-----------------------------------------------------------------------

void cScriptManager::Destroy(iResourceBase *apResource) {
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
