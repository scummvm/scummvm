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

#ifndef HPL_UPDATER_H
#define HPL_UPDATER_H

#include "common/list.h"
#include "hpl1/engine/system/SystemTypes.h"
#include "common/stablemap.h"

namespace hpl {

class iUpdateable;
class LowLevelSystem;

typedef Common::List<iUpdateable *> tUpdateableList;
typedef tUpdateableList::iterator tUpdateableListIt;

typedef Common::StableMap<tString, tUpdateableList> tUpdateContainerMap;
typedef tUpdateContainerMap::iterator tUpdateContainerMapIt;

class cUpdater {
public:
	cUpdater(LowLevelSystem *apLowLevelSystem);
	~cUpdater();

	void Reset();

	void OnDraw();
	void OnPostSceneDraw();
	void OnPostGUIDraw();
	void OnPostBufferSwap();

	void OnStart();

	void Update(float afTimeStep);

	void OnExit();

	/**
	 * Sets the active update container to be used.
	 * \param asContainer Name of the contianer
	 * \return
	 */
	bool SetContainer(tString asContainer);

	/**
	 * Gets the name of the current container in use.
	 * \return name of current container.
	 */
	tString GetCurrentContainerName();
	/**
	 * Adds a new container
	 * \todo change name to state instead of container?
	 * \param asName Name for the new container.
	 * \return
	 */
	bool AddContainer(tString asName);
	/**
	 * Adds a new update in a container.
	 * \param asContainer Container name
	 * \param apUpdate pointer to the class that will be updated
	 * \return
	 */
	bool AddUpdate(tString asContainer, iUpdateable *apUpdate);
	/**
	 * Adds a global update that runs no matter what container is set
	 * \param apUpdate
	 * \return
	 */
	bool AddGlobalUpdate(iUpdateable *apUpdate);

private:
	tString msCurrentUpdates;

	tUpdateContainerMap m_mapUpdateContainer;

	LowLevelSystem *mpLowLevelSystem;

	tUpdateableList *mpCurrentUpdates;
	tUpdateableList mlstGlobalUpdateableList;
};

} // namespace hpl

#endif // HPL_UPDATER_H
