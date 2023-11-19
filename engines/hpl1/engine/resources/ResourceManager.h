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

#ifndef HPL_RESOURCEMANAGER_H
#define HPL_RESOURCEMANAGER_H

#include "hpl1/engine/system/SystemTypes.h"
#include "common/stablemap.h"

namespace hpl {

class LowLevelResources;
class LowLevelSystem;
class cFileSearcher;
class iResourceBase;

typedef Common::StableMap<unsigned long, iResourceBase *> tResourceHandleMap;
typedef tResourceHandleMap::iterator tResourceHandleMapIt;

typedef Common::StableMap<tString, iResourceBase *> tResourceNameMap;
typedef tResourceNameMap::iterator tResourceNameMapIt;

typedef Common::List<iResourceBase *> tResourceBaseList;
typedef tResourceBaseList::iterator tResourceBaseListIt;

typedef cSTLMapIterator<iResourceBase *, tResourceNameMap, tResourceNameMapIt> cResourceBaseIterator;

class iResourceManager {
public:
	iResourceManager(cFileSearcher *apFileSearcher, LowLevelResources *apLowLevelResources,
					 LowLevelSystem *apLowLevelSystem);
	virtual ~iResourceManager() {}

	virtual iResourceBase *Create(const tString &asName) = 0;

	iResourceBase *GetByName(const tString &asName);
	iResourceBase *GetByHandle(unsigned long alHandle);

	cResourceBaseIterator GetResourceBaseIterator();

	void DestroyUnused(int alMaxToKeep);

	virtual void Destroy(iResourceBase *apResource) = 0;
	virtual void DestroyAll();

	virtual void Unload(iResourceBase *apResource) = 0;

	virtual void Update(float afTimeStep) {}

protected:
	unsigned long mlHandleCount;
	tResourceNameMap m_mapNameResources;
	tResourceHandleMap m_mapHandleResources;

	cFileSearcher *mpFileSearcher;
	LowLevelResources *mpLowLevelResources;
	LowLevelSystem *mpLowLevelSystem;

	void BeginLoad(const tString &asFile);
	void EndLoad();

	unsigned long mlTimeStart;

	/**
	 * Checks if a resource alllready is in the manager, else searches the resources.
	 * \param &asName Name of the resource.
	 * \param &asFilePath If the file is not in the manager, the path is put here. "" if there is no such file.
	 * \return A pointer to the resource. NULL if not in manager.
	 */
	iResourceBase *FindLoadedResource(const tString &asName, tString &asFilePath);
	void AddResource(iResourceBase *apResource, bool abLog = true);
	void RemoveResource(iResourceBase *apResource);

	unsigned long GetHandle();

	tString GetTabs();
	static int mlTabCount;
};

} // namespace hpl

#endif // HPL_RESOURCEMANAGER_H
