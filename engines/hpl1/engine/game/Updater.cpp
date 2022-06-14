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

#include "hpl1/engine/game/Updater.h"

#include "hpl1/engine/game/Updateable.h"
#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cUpdater::cUpdater(iLowLevelSystem *apLowLevelSystem) {
	mpCurrentUpdates = NULL;

	mpLowLevelSystem = apLowLevelSystem;
}

//-----------------------------------------------------------------------

cUpdater::~cUpdater() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHOD
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cUpdater::OnDraw() {
	for (tUpdateableListIt it = mlstGlobalUpdateableList.begin(); it != mlstGlobalUpdateableList.end(); ++it) {
		(*it)->OnDraw();
	}

	if (mpCurrentUpdates) {
		for (tUpdateableListIt it = mpCurrentUpdates->begin(); it != mpCurrentUpdates->end(); ++it) {
			(*it)->OnDraw();
		}
	}
}
//-----------------------------------------------------------------------

void cUpdater::OnPostSceneDraw() {
	for (tUpdateableListIt it = mlstGlobalUpdateableList.begin(); it != mlstGlobalUpdateableList.end(); ++it) {
		(*it)->OnPostSceneDraw();
	}

	if (mpCurrentUpdates) {
		for (tUpdateableListIt it = mpCurrentUpdates->begin(); it != mpCurrentUpdates->end(); ++it) {
			(*it)->OnPostSceneDraw();
		}
	}
}

void cUpdater::OnPostGUIDraw() {
	for (tUpdateableListIt it = mlstGlobalUpdateableList.begin(); it != mlstGlobalUpdateableList.end(); ++it) {
		(*it)->OnPostGUIDraw();
	}

	if (mpCurrentUpdates) {
		for (tUpdateableListIt it = mpCurrentUpdates->begin(); it != mpCurrentUpdates->end(); ++it) {
			(*it)->OnPostGUIDraw();
		}
	}
}

//-----------------------------------------------------------------------

void cUpdater::OnPostBufferSwap() {
	for (tUpdateableListIt it = mlstGlobalUpdateableList.begin(); it != mlstGlobalUpdateableList.end(); ++it) {
		(*it)->OnPostBufferSwap();
	}

	if (mpCurrentUpdates) {
		for (tUpdateableListIt it = mpCurrentUpdates->begin(); it != mpCurrentUpdates->end(); ++it) {
			(*it)->OnPostBufferSwap();
		}
	}
}

//-----------------------------------------------------------------------

void cUpdater::OnStart() {
	for (tUpdateableListIt it = mlstGlobalUpdateableList.begin(); it != mlstGlobalUpdateableList.end(); ++it) {
		(*it)->OnStart();
	}

	tUpdateContainerMapIt ContIt = m_mapUpdateContainer.begin();
	while (ContIt != m_mapUpdateContainer.end()) {
		tUpdateableListIt UpIt = ContIt->second.begin();
		while (UpIt != ContIt->second.end()) {
			(*UpIt)->OnStart();
			UpIt++;
		}

		ContIt++;
	}
}

//-----------------------------------------------------------------------

void cUpdater::Reset() {
	for (tUpdateableListIt it = mlstGlobalUpdateableList.begin(); it != mlstGlobalUpdateableList.end(); ++it) {
		(*it)->Reset();
	}

	tUpdateContainerMapIt ContIt = m_mapUpdateContainer.begin();
	while (ContIt != m_mapUpdateContainer.end()) {

		tUpdateableList *pUpdates = &ContIt->second;
		tUpdateableListIt UpIt = pUpdates->begin();
		while (UpIt != pUpdates->end()) {
			iUpdateable *pUpdate = *UpIt;

			pUpdate->Reset();

			++UpIt;
		}

		++ContIt;
	}
}

//-----------------------------------------------------------------------

void cUpdater::OnExit() {
	for (tUpdateableListIt it = mlstGlobalUpdateableList.begin(); it != mlstGlobalUpdateableList.end(); ++it) {
		// Log(" Exiting %s\n",(*it)->GetName().c_str());
		(*it)->OnExit();
	}

	tUpdateContainerMapIt ContIt = m_mapUpdateContainer.begin();
	while (ContIt != m_mapUpdateContainer.end()) {
		tUpdateableListIt UpIt = ContIt->second.begin();
		while (UpIt != ContIt->second.end()) {
			// Log(" Exiting %s\n",(*UpIt)->GetName().c_str());
			(*UpIt)->OnExit();
			UpIt++;
		}

		ContIt++;
	}
}

//-----------------------------------------------------------------------

void cUpdater::Update(float afTimeStep) {
	for (tUpdateableListIt it = mlstGlobalUpdateableList.begin(); it != mlstGlobalUpdateableList.end(); ++it) {
		START_TIMING_EX((*it)->GetName().c_str(), game)
		(*it)->Update(afTimeStep);
		STOP_TIMING(game)
	}

	if (mpCurrentUpdates) {
		tUpdateableList *pList = mpCurrentUpdates;
		for (tUpdateableListIt it = pList->begin(); it != pList->end(); ++it) {
			START_TIMING_EX((*it)->GetName().c_str(), game)
			(*it)->Update(afTimeStep);
			STOP_TIMING(game)
		}
	}
}

//-----------------------------------------------------------------------

bool cUpdater::SetContainer(tString asContainer) {
	tUpdateContainerMapIt it = m_mapUpdateContainer.find(asContainer);
	if (it == m_mapUpdateContainer.end())
		return false;

	msCurrentUpdates = asContainer;
	if (msCurrentUpdates == "Default") {
		SetUpdateLogActive(true);
	} else {
		SetUpdateLogActive(false);
	}

	mpCurrentUpdates = &it->second;

	return true;
}

tString cUpdater::GetCurrentContainerName() {
	if (mpCurrentUpdates == NULL)
		return "";

	return msCurrentUpdates;
}

//-----------------------------------------------------------------------

bool cUpdater::AddContainer(tString asName) {
	// Create the value for the map with key and Updateable
	tUpdateContainerMap::value_type val = tUpdateContainerMap::value_type(
		asName, tUpdateableList());
	// Add it to the map
	m_mapUpdateContainer.insert(val);

	return true;
}

//-----------------------------------------------------------------------

bool cUpdater::AddUpdate(tString asContainer, iUpdateable *apUpdate) {
	if (apUpdate == NULL) {
		Error("Couldn't add NULL updatable!");
		return false;
	}

	// Search the map for the container name
	tUpdateContainerMapIt it = m_mapUpdateContainer.find(asContainer);
	if (it == m_mapUpdateContainer.end())
		return false;

	// Add the updatable
	it->second.push_back(apUpdate);

	return true;
}

//-----------------------------------------------------------------------

bool cUpdater::AddGlobalUpdate(iUpdateable *apUpdate) {
	mlstGlobalUpdateableList.push_back(apUpdate);
	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
} // namespace hpl
