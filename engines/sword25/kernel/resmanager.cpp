// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#include "sword25/kernel/resmanager.h"

#include "sword25/kernel/resource.h"
#include "sword25/kernel/resservice.h"
#include "sword25/kernel/string.h"
#include "sword25/package/packagemanager.h"

#define BS_LOG_PREFIX "RESOURCEMANAGER"

BS_ResourceManager::~BS_ResourceManager()
{
	// Alle ungelockten Resourcen freigeben.
	EmptyCache();

	// Alle übriggebliebenen Resourcen sind nicht freigegeben worden, daher Warnungen ausgeben und freigeben.
	std::list<BS_Resource*>::iterator Iter = m_Resources.begin();
	for (; Iter != m_Resources.end(); ++Iter)
	{
		BS_LOG_WARNINGLN("Resource \"%s\" was not released.", (*Iter)->GetFileName().c_str());

		// Lock-Count auf 0 setzen.
		while ((*Iter)->GetLockCount() > 0) { (*Iter)->Release(); };

		// Resource freigeben.
		delete(*Iter);
	}
}

BS_Resource* BS_ResourceManager::GetResourceByOrdinal(int Ord) const
{
	// Überprüfen ob der Index Ord innerhald der Listengrenzen liegt.
	if (Ord < 0 || Ord >= GetResourceCount())
	{
		BS_LOG_ERRORLN("Resource ordinal (%d) out of bounds (0 - %d).", Ord, GetResourceCount() - 1);
		return NULL;
	}

	// Liste durchlaufen und die Resource mit dem gewünschten Index zurückgeben.
	int CurOrd = 0;
	std::list<BS_Resource*>::const_iterator Iter = m_Resources.begin();
	for (; Iter != m_Resources.end(); ++Iter, ++CurOrd)
	{
		if (CurOrd == Ord)
			return (*Iter);
	}

	// Die Ausführung sollte nie an diesem Punkt ankommen.
	BS_LOG_EXTERRORLN("Execution reached unexpected point.");
	return NULL;
}

bool BS_ResourceManager::RegisterResourceService(BS_ResourceService* pService)
{
	if(!pService)
	{
		BS_LOG_ERRORLN("Can't register NULL resource service.");
		return false;
	}

	m_ResourceServices.push_back(pService);

	return true;
}

void BS_ResourceManager::DeleteResourcesIfNecessary()
{
	// Falls noch genügend Speicher frei ist, oder keine Ressourcen geladen sind, kann die Funktion vorzeitig beendet werden.
	if (m_KernelPtr->GetUsedMemory() < m_MaxMemoryUsage || m_Resources.empty()) return;

	// Solange Ressourcen löschen, bis der Speichernutzung des Prozesses unter den festgelegten Maximalwert fällt.
	// Dabei wird die Liste von Hinten nach vorne durchlaufen um zunächst jene Resourcen freizugeben, deren
	// Benutzung lange zurückliegt und sich somit am Ende der Liste befinden.
	std::list<BS_Resource*>::iterator Iter = m_Resources.end();
	do
	{
		--Iter;

		// Die Resource darf nur freigegeben werden, wenn sie nicht gelockt ist.
		if ((*Iter)->GetLockCount() == 0) Iter = DeleteResource(*Iter);
	} while(Iter != m_Resources.begin() && m_KernelPtr->GetUsedMemory() > m_MaxMemoryUsage);
}

void BS_ResourceManager::EmptyCache()
{
	// Resourcenliste durchlaufen und alle nicht gelockten Resourcen freigeben
	std::list<BS_Resource*>::iterator Iter = m_Resources.begin();
	while (Iter != m_Resources.end())
	{
		if ((*Iter)->GetLockCount() == 0)
		{
			// Resource entfernen
			Iter = DeleteResource(*Iter);
		}
		else
			++Iter;
	}
}

BS_Resource* BS_ResourceManager::RequestResource(const std::string& FileName)
{
	// Absoluten, eindeutigen Pfad zur Datei erzeugen.
	std::string UniqueFileName = GetUniqueFileName(FileName);
	if (UniqueFileName == "")
		return NULL;

	// Feststellen, ob die Resource schon geladen ist.
	// Wenn die Resource gefunden wurde wird sie an die Spitze der Resourcenliste gestellt, gelockt und zurückgegeben.
	{
		BS_Resource* pResource = GetResource(UniqueFileName);
		if (pResource)
		{
			MoveToFront(pResource);
			(pResource)->AddReference();
			return pResource;
		}
	}

	// Die Resource wurde nicht gefunden, muss also noch geladen werden.
	if (m_LogCacheMiss) BS_LOG_WARNINGLN("\"%s\" was not precached.", UniqueFileName.c_str());

	BS_Resource* pResource;
	if (pResource = LoadResource(UniqueFileName))
	{
		pResource->AddReference();
		return pResource;
	}

	return NULL;
}

bool BS_ResourceManager::PrecacheResource(const std::string& FileName, bool ForceReload)
{
	// Absoluten, eindeutigen Pfad zur Datei erzeugen.
	std::string UniqueFileName = GetUniqueFileName(FileName);
	if (UniqueFileName == "")
		return false;

	BS_Resource * ResourcePtr = GetResource(UniqueFileName);

	if (ForceReload && ResourcePtr)
	{
		if (ResourcePtr->GetLockCount())
		{
			BS_LOG_ERRORLN("Could not force precaching of \"%s\". The resource is locked.", FileName.c_str());
			return false;
		}
		else
		{
			DeleteResource(ResourcePtr);
			ResourcePtr = 0;
		}
	}

	if (!ResourcePtr && LoadResource(UniqueFileName) == NULL)
	{
		BS_LOG_ERRORLN("Could not precache \"%s\",", FileName.c_str());
		return false;
	}

	return true;
}

void BS_ResourceManager::MoveToFront(BS_Resource* pResource)
{
	// Resource aus der Liste löschen
	m_Resources.erase(pResource->_Iterator);
	// Resource an die Spitze der Liste setzen
	m_Resources.push_front(pResource);
	// Iterator aktualisieren
	pResource->_Iterator = m_Resources.begin();
}

BS_Resource* BS_ResourceManager::LoadResource(const std::string& FileName)
{
	// ResourceService finden, der die Resource laden kann.
	for(unsigned int i = 0; i < m_ResourceServices.size(); ++i)
	{
		if (m_ResourceServices[i]->CanLoadResource(FileName))
		{
			// Falls mehr Speicher belegt ist als gewünscht, muss Speicher freigegeben werden.
			DeleteResourcesIfNecessary();

			// Resource laden
			BS_Resource* pResource;
			if (!(pResource = m_ResourceServices[i]->LoadResource(FileName)))
			{
				BS_LOG_ERRORLN("Responsible service could not load resource \"%s\".", FileName.c_str());
				return NULL;
			}

			// Resource an die Spitze der Resourcenliste stellen.
			m_Resources.push_front(pResource);
			pResource->_Iterator = m_Resources.begin();

			// Resource in die Hashtabelle eintragen
			m_ResourceHashTable[pResource->GetFileNameHash() % HASH_TABLE_BUCKETS].push_front(pResource);

			return pResource;
		}
	}

	BS_LOG_ERRORLN("Could not find a service that can load \"%s\".", FileName.c_str());
	return NULL;
}

std::string BS_ResourceManager::GetUniqueFileName(const std::string& FileName) const
{
	// Pointer auf den PackageManager holen
	BS_PackageManager* pPackage = (BS_PackageManager*) m_KernelPtr->GetService("package");
	if (!pPackage)
	{
		BS_LOG_ERRORLN("Could not get package manager.");
		return std::string("");
	}

	// Absoluten Pfad der Datei bekommen und somit die Eindeutigkeit des Dateinamens sicherstellen
	std::string UniqueFileName = pPackage->GetAbsolutePath(FileName);
	if (UniqueFileName == "")
		BS_LOG_ERRORLN("Could not create absolute file name for \"%s\".", FileName.c_str());

	return UniqueFileName;
}

std::list<BS_Resource*>::iterator BS_ResourceManager::DeleteResource(BS_Resource* pResource)
{
	// Resource aus der Hash-Tabelle entfernen
	m_ResourceHashTable[pResource->GetFileNameHash() % HASH_TABLE_BUCKETS].remove(pResource);

	BS_Resource* pDummy = pResource;

	// Resource aus der Resourcenliste löschen
	std::list<BS_Resource*>::iterator Result = m_Resources.erase(pResource->_Iterator);

	// Resource freigeben
	delete(pDummy);

	// Iterator zurückgeben
	return Result;
}

BS_Resource* BS_ResourceManager::GetResource(const std::string& UniqueFileName) const
{
	// Feststellen, ob die Resource schon geladen ist.
	const std::list<BS_Resource*>& HashBucket = m_ResourceHashTable[BS_String::GetHash(UniqueFileName) % HASH_TABLE_BUCKETS];
	{
		std::list<BS_Resource*>::const_iterator Iter = HashBucket.begin();
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

void BS_ResourceManager::DumpLockedResources()
{
	for (std::list<BS_Resource*>::iterator Iter = m_Resources.begin(); Iter != m_Resources.end(); ++Iter)
	{
		if ((*Iter)->GetLockCount() > 0)
		{
			BS_LOGLN("%s", (*Iter)->GetFileName().c_str());
		}
	}
}

void BS_ResourceManager::SetMaxMemoryUsage(unsigned int MaxMemoryUsage)
{
	m_MaxMemoryUsage = MaxMemoryUsage;
	DeleteResourcesIfNecessary();
}