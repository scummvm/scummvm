/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_RESOURCEMANAGER_H
#define SWORD25_RESOURCEMANAGER_H

// Includes
#include "common/list.h"

#include "sword25/kernel/common.h"

namespace Sword25 {

// Class definitions
class ResourceService;
class Resource;
class Kernel;

class ResourceManager {
	friend class Kernel;

public:
	/**
	 * Returns a requested resource. If any error occurs, returns NULL
	 * @param FileName      Filename of resource
	 */
	Resource *RequestResource(const Common::String &FileName);

	/**
	 * Loads a resource into the cache
	 * @param FileName      The filename of the resource to be cached
	 * @param ForceReload   Indicates whether the file should be reloaded if it's already in the cache.
	 * This is useful for files that may have changed in the interim
	 */
	bool PrecacheResource(const Common::String &FileName, bool ForceReload = false);

	/**
	 * Returns the number of loaded resources
	 */
	int GetResourceCount() const {
		return static_cast<int>(m_Resources.size());
	}

	/**
	 * Returns a resource by it's ordinal index. Returns NULL if any error occurs
	 * Note: This method is not optimised for speed and should be used only for debugging purposes
	 * @param Ord       Ordinal number of the resource. Must be between 0 and GetResourceCount() - 1.
	*/
	Resource *GetResourceByOrdinal(int Ord) const;

	/**
	 * Registers a RegisterResourceService. This method is the constructor of
	 * BS_ResourceService, and thus helps all resource services in the ResourceManager list
	 * @param pService      Which service
	 */
	bool RegisterResourceService(ResourceService *pService);

	/**
	 * Releases all resources that are not locked.
	 **/
	void EmptyCache();

	/**
	 * Returns the maximum memory the kernel has used
	 */
	int GetMaxMemoryUsage() const {
		return m_MaxMemoryUsage;
	}

	/**
	 * Specifies the maximum amount of memory the engine is allowed to use.
	 * If this value is exceeded, resources will be unloaded to make room. This value is meant
	 * as a guideline, and not as a fixed boundary. It is not guaranteed not to be exceeded;
	 * the whole game engine may still use more memory than any amount specified.
	 */
	void SetMaxMemoryUsage(uint MaxMemoryUsage);

	/**
	 * Specifies whether a warning is written to the log when a cache miss occurs.
	 * THe default value is "false".
	 */
	bool IsLogCacheMiss() const {
		return m_LogCacheMiss;
	}

	/**
	 * Sets whether warnings are written to the log if a cache miss occurs.
	 * @param Flag      If "true", then future warnings will be logged
	 */
	void SetLogCacheMiss(bool Flag) {
		m_LogCacheMiss = Flag;
	}

	/**
	 * Writes the names of all currently locked resources to the log file
	 */
	void DumpLockedResources();

private:
	/**
	 * Creates a new resource manager
	 * Only the BS_Kernel class can generate copies this class. Thus, the constructor is private
	 */
	ResourceManager(Kernel *pKernel) :
		m_KernelPtr(pKernel),
		m_MaxMemoryUsage(100000000),
		m_LogCacheMiss(false)
	{};
	virtual ~ResourceManager();

	enum {
		HASH_TABLE_BUCKETS = 256
	};

	/**
	 * Moves a resource to the top of the resource list
	 * @param pResource     The resource
	 */
	void MoveToFront(Resource *pResource);

	/**
	 * Loads a resource and updates the m_UsedMemory total
	 *
	 * The resource must not already be loaded
	 * @param FileName      The unique filename of the resource to be loaded
	 */
	Resource *loadResource(const Common::String &fileName);

	/**
	 * Returns the full path of a given resource filename.
	 * It will return an empty string if a path could not be created.
	*/
	Common::String GetUniqueFileName(const Common::String &FileName) const;

	/**
	 * Deletes a resource, removes it from the lists, and updates m_UsedMemory
	 */
	Common::List<Resource *>::iterator DeleteResource(Resource *pResource);

	/**
	 * Returns a pointer to a loaded resource. If any error occurs, NULL will be returned.
	 * @param UniqueFileName        The absolute path and filename
	 * Gibt einen Pointer auf die angeforderte Resource zurück, oder NULL, wenn die Resourcen nicht geladen ist.
	 */
	Resource *GetResource(const Common::String &UniqueFileName) const;

	/**
	 * Deletes resources as necessary until the specified memory limit is not being exceeded.
	 */
	void DeleteResourcesIfNecessary();

	Kernel                          *m_KernelPtr;
	uint                        m_MaxMemoryUsage;
	Common::Array<ResourceService *> m_ResourceServices;
	Common::List<Resource *>         m_Resources;
	Common::List<Resource *>         m_ResourceHashTable[HASH_TABLE_BUCKETS];
	bool                                m_LogCacheMiss;
};

} // End of namespace Sword25

#endif
