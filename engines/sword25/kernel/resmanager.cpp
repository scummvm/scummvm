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
 */

#include "sword25/kernel/resmanager.h"

#include "sword25/kernel/resource.h"
#include "sword25/kernel/resservice.h"
#include "sword25/kernel/string.h"
#include "sword25/package/packagemanager.h"

namespace Sword25 {

#define BS_LOG_PREFIX "RESOURCEMANAGER"

BS_ResourceManager::~BS_ResourceManager() {
	// Clear all unlocked resources
	EmptyCache();

	// All remaining resources are not released, so print warnings and release
	Common::List<BS_Resource *>::iterator Iter = m_Resources.begin();
	for (; Iter != m_Resources.end(); ++Iter) {
		BS_LOG_WARNINGLN("Resource \"%s\" was not released.", (*Iter)->GetFileName().c_str());

		// Set the lock count to zero
		while ((*Iter)->GetLockCount() > 0) { (*Iter)->Release(); };

		// Delete the resource
		delete(*Iter);
	}
}

/**
 * Returns a resource by it's ordinal index. Returns NULL if any error occurs
 * Note: This method is not optimised for speed and should be used only for debugging purposes
 * @param Ord		Ordinal number of the resource. Must be between 0 and GetResourceCount() - 1.
 */
BS_Resource *BS_ResourceManager::GetResourceByOrdinal(int Ord) const {
	// Überprüfen ob der Index Ord innerhald der Listengrenzen liegt.
	if (Ord < 0 || Ord >= GetResourceCount()) {
		BS_LOG_ERRORLN("Resource ordinal (%d) out of bounds (0 - %d).", Ord, GetResourceCount() - 1);
		return NULL;
	}

	// Liste durchlaufen und die Resource mit dem gewünschten Index zurückgeben.
	int CurOrd = 0;
	Common::List<BS_Resource*>::const_iterator Iter = m_Resources.begin();
	for (; Iter != m_Resources.end(); ++Iter, ++CurOrd) {
		if (CurOrd == Ord)
			return (*Iter);
	}

	// Die Ausführung sollte nie an diesem Punkt ankommen.
	BS_LOG_EXTERRORLN("Execution reached unexpected point.");
	return NULL;
}

/**
 * Registers a RegisterResourceService. This method is the constructor of
 * BS_ResourceService, and thus helps all resource services in the ResourceManager list
 * @param pService		Which service
 */
bool BS_ResourceManager::RegisterResourceService(BS_ResourceService *pService) {
	if (!pService) {
		BS_LOG_ERRORLN("Can't register NULL resource service.");
		return false;
	}

	m_ResourceServices.push_back(pService);

	return true;
}

/**
 * Deletes resources as necessary until the specified memory limit is not being exceeded.
 */
void BS_ResourceManager::DeleteResourcesIfNecessary() {
	// If enough memory is available, or no resources are loaded, then the function can immediately end
	if (m_KernelPtr->GetUsedMemory() < m_MaxMemoryUsage || m_Resources.empty()) return;

	// Keep deleting resources until the memory usage of the process falls below the set maximum limit.
	// The list is processed backwards in order to first release those resources who have been 
	// not been accessed for the longest
	Common::List<BS_Resource *>::iterator Iter = m_Resources.end();
	do {
		--Iter;

		// The resource may be released only if it isn't locked
		if ((*Iter)->GetLockCount() == 0) Iter = DeleteResource(*Iter);
	} while (Iter != m_Resources.begin() && m_KernelPtr->GetUsedMemory() > m_MaxMemoryUsage);
}

/**
 * Releases all resources that are not locked.
 **/
void BS_ResourceManager::EmptyCache() {
	// Scan through the resource list
	Common::List<BS_Resource *>::iterator Iter = m_Resources.begin();
	while (Iter != m_Resources.end()) {
		if ((*Iter)->GetLockCount() == 0) {
			// Delete the resource
			Iter = DeleteResource(*Iter);
		} else
			++Iter;
	}
}

/**
 * Returns a requested resource. If any error occurs, returns NULL
 * @param FileName		Filename of resource
 */
BS_Resource *BS_ResourceManager::RequestResource(const Common::String &FileName) {
	// Get the absolute path to the file
	Common::String UniqueFileName = GetUniqueFileName(FileName);
	if (UniqueFileName == "")
		return NULL;

	// Determine whether the resource is already loaded
	// If the resource is found, it will be placed at the head of the resource list and returned
	{
		BS_Resource *pResource = GetResource(UniqueFileName);
		if (pResource) {
			MoveToFront(pResource);
			(pResource)->AddReference();
			return pResource;
		}
	}

	// The resource was not found, therefore, must not be loaded yet
	if (m_LogCacheMiss) BS_LOG_WARNINGLN("\"%s\" was not precached.", UniqueFileName.c_str());

	BS_Resource *pResource;
	if (pResource = LoadResource(UniqueFileName)) {
		pResource->AddReference();
		return pResource;
	}

	return NULL;
}

/**
 * Loads a resource into the cache
 * @param FileName		The filename of the resource to be cached
 * @param ForceReload	Indicates whether the file should be reloaded if it's already in the cache.
 * This is useful for files that may have changed in the interim
 */
bool BS_ResourceManager::PrecacheResource(const Common::String& FileName, bool ForceReload) {
	// Get the absolute path to the file
	Common::String UniqueFileName = GetUniqueFileName(FileName);
	if (UniqueFileName == "")
		return false;

	BS_Resource *ResourcePtr = GetResource(UniqueFileName);

	if (ForceReload && ResourcePtr) {
		if (ResourcePtr->GetLockCount()) {
			BS_LOG_ERRORLN("Could not force precaching of \"%s\". The resource is locked.", FileName.c_str());
			return false;
		} else {
			DeleteResource(ResourcePtr);
			ResourcePtr = 0;
		}
	}

	if (!ResourcePtr && LoadResource(UniqueFileName) == NULL) {
		BS_LOG_ERRORLN("Could not precache \"%s\",", FileName.c_str());
		return false;
	}

	return true;
}

/**
 * Moves a resource to the top of the resource list
 * @param pResource		The resource
 */
void BS_ResourceManager::MoveToFront(BS_Resource *pResource) {
	// Erase the resource from it's current position
	m_Resources.erase(pResource->_Iterator);
	// Re-add the resource at the front of the list
	m_Resources.push_front(pResource);
	// Reset the resource iterator to the repositioned item
	pResource->_Iterator = m_Resources.begin();
}

/**
 * Loads a resource and updates the m_UsedMemory total
 *
 * The resource must not already be loaded
 * @param FileName		The unique filename of the resource to be loaded
 */
BS_Resource *BS_ResourceManager::LoadResource(const Common::String &FileName) {
	// ResourceService finden, der die Resource laden kann.
	for (unsigned int i = 0; i < m_ResourceServices.size(); ++i) {
		if (m_ResourceServices[i]->CanLoadResource(FileName)) {
			// If more memory is desired, memory must be released
			DeleteResourcesIfNecessary();

			// Load the resource
			BS_Resource *pResource;
			if (!(pResource = m_ResourceServices[i]->LoadResource(FileName))) {
				BS_LOG_ERRORLN("Responsible service could not load resource \"%s\".", FileName.c_str());
				return NULL;
			}

			// Add the resource to the front of the list
			m_Resources.push_front(pResource);
			pResource->_Iterator = m_Resources.begin();

			// Also store the resource in the hash table for quick lookup
			m_ResourceHashTable[pResource->GetFileNameHash() % HASH_TABLE_BUCKETS].push_front(pResource);

			return pResource;
		}
	}

	BS_LOG_ERRORLN("Could not find a service that can load \"%s\".", FileName.c_str());
	return NULL;
}

/**
 * Returns the full path of a given resource filename.
 * It will return an empty string if a path could not be created.
*/
Common::String BS_ResourceManager::GetUniqueFileName(const Common::String& FileName) const {
	// Get a pointer to the package manager
	BS_PackageManager *pPackage = (BS_PackageManager *)m_KernelPtr->GetService("package");
	if (!pPackage) {
		BS_LOG_ERRORLN("Could not get package manager.");
		return Common::String("");
	}

	// Absoluten Pfad der Datei bekommen und somit die Eindeutigkeit des Dateinamens sicherstellen
	Common::String UniqueFileName = pPackage->GetAbsolutePath(FileName);
	if (UniqueFileName == "")
		BS_LOG_ERRORLN("Could not create absolute file name for \"%s\".", FileName.c_str());

	return UniqueFileName;
}

/**
 * Deletes a resource, removes it from the lists, and updates m_UsedMemory
 */
Common::List<BS_Resource *>::iterator BS_ResourceManager::DeleteResource(BS_Resource *pResource) {
	// Remove the resource from the hash table
	m_ResourceHashTable[pResource->GetFileNameHash() % HASH_TABLE_BUCKETS].remove(pResource);

	BS_Resource *pDummy = pResource;

	// Delete the resource from the resource list
	Common::List<BS_Resource *>::iterator Result = m_Resources.erase(pResource->_Iterator);

	// Delete the resource
	delete (pDummy);

	// Return the iterator
	return Result;
}

/**
 * Returns a pointer to a loaded resource. If any error occurs, NULL will be returned.
 * @param UniqueFileName		The absolute path and filename
 * Gibt einen Pointer auf die angeforderte Resource zurück, oder NULL, wenn die Resourcen nicht geladen ist.
 */
BS_Resource * BS_ResourceManager::GetResource(const Common::String &UniqueFileName) const {
	// Determine whether the resource is already loaded
	const Common::List<BS_Resource *>& HashBucket = m_ResourceHashTable[
		BS_String::GetHash(UniqueFileName) % HASH_TABLE_BUCKETS];
	{
		Common::List<BS_Resource*>::const_iterator Iter = HashBucket.begin();
		for (; Iter != HashBucket.end(); ++Iter)
		{
			// Wenn die Resource gefunden wurde wird sie zurückgegeben.
			if ((*Iter)->GetFileName() == UniqueFileName)
				return *Iter;
		}
	}

	// Resource wurde nicht gefunden, ist also nicht geladen
	return NULL;
}

/**
 * Writes the names of all currently locked resources to the log file
 */
void BS_ResourceManager::DumpLockedResources() {
	for (Common::List<BS_Resource *>::iterator Iter = m_Resources.begin(); Iter != m_Resources.end(); ++Iter) {
		if ((*Iter)->GetLockCount() > 0) {
			BS_LOGLN("%s", (*Iter)->GetFileName().c_str());
		}
	}
}

/**
 * Specifies the maximum amount of memory the engine is allowed to use.
 * If this value is exceeded, resources will be unloaded to make room. This value is meant
 * as a guideline, and not as a fixed boundary. It is not guaranteed not to be exceeded;
 * the whole game engine may still use more memory than any amount specified.
 */
void BS_ResourceManager::SetMaxMemoryUsage(unsigned int MaxMemoryUsage) {
	m_MaxMemoryUsage = MaxMemoryUsage;
	DeleteResourcesIfNecessary();
}

} // End of namespace Sword25
