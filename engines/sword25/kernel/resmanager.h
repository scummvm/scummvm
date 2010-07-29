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

#ifndef BS_RESOURCEMANAGER_H
#define BS_RESOURCEMANAGER_H

// Includes
#include "memlog_off.h"
#include <vector>
#include <list>
#include "memlog_on.h"

#include "common.h"

// Klassendefinition
class BS_ResourceService;
class BS_Resource;
class BS_Kernel;

class BS_ResourceManager
{
friend BS_Kernel;

public:
	/**
		@brief Fordert eine Resource an.
		@param FileName Dateiname
		@return Gibt die Resource zurück, falls erfolgreich, sonst NULL
	*/
	BS_Resource* RequestResource(const std::string& FileName);

	/**
		@brief Lädt eine Resource in den Cache.
		@param FileName der Dateiname der zu cachenden Resource
		@param ForceReload gibt an, ob die Datei auch neu geladen werden soll, wenn sie bereits geladen wurde.
						   Dies ist nützlich bei Dateien, die sich in der Zwischenzeit verändert haben.
		@return Gibt true zurück, wenn das Caching durchgeführt werden konnte, ansonsten false.
	*/
	bool PrecacheResource(const std::string& FileName, bool ForceReload = false);

	/**
		@brief Gibt die Anzahl der geladenen Resourcen zurück.
	*/
	int GetResourceCount() const { return static_cast<int>(m_Resources.size()); }

	/**
		@brief Gibt einen Pointer auf eine Resource anhand deren laufender Nummer zurück.
		@param Ord laufende Nummer der Resource. Dieser Wert muss zwischen 0 und GetResourceCount() - 1 liegen.
		@return Gibt einen Pointer auf die Resource zurück wenn erfolgreich, ansonsten NULL.
		@remark Diese Methode ist nicht auf Geschwindigkeit optimiert und sollte nur für Debugzwecke eingesetzt werden.
	*/
	BS_Resource* GetResourceByOrdinal(int Ord) const;

	/**
		@brief RegisterResourceService Diese Methode wird vom Konstruktor von 
										BS_ResourceService aufgerufen und trägt somit alle 
										Resource-Services in einen Liste des 
										Resource-Managers ein.
		@param pService welches Service
		@return gibt true zurück, falls erfolgreich
	*/
	bool RegisterResourceService(BS_ResourceService* pService);

	/**
	 * @brief gibt alle Resourcen frei, die nicht gelocked sind.
	 **/
	void EmptyCache();

	/**
		@brief Gibt zurück wie viel Speicher die Engine maximal belegen soll.
	*/
	int GetMaxMemoryUsage() const { return m_MaxMemoryUsage; }

	/**
		@brief Legt fest wie viel Speicher die Engine maximal belegen soll.

		Wenn dieser Wert überschritten wird, werden Resourcen entladen um Platz zu schaffen. Dieser Wert ist als Richtgröße zu verstehen und nicht als feste Grenze.
		Es ist unter KEINEN Umständen garantiert, dass die Engine tatsächlich nur so viel Speicher benutzt.
	*/
	void SetMaxMemoryUsage(unsigned int MaxMemoryUsage);

	/**
		@brief Gibt an, ob eine Warnung ins Log geschrieben wird, wenn ein Cache-Miss auftritt.
		Der Standardwert ist "false".
	*/
	bool IsLogCacheMiss() const { return m_LogCacheMiss; }

	/**
		@brief Legt fest, ob eine Warnung ins Log geschrieben wird, wenn in Cache-Miss auftritt.
		@param Flag wenn "true" wird die Warnung in Zukunft ausgegeben, ansonsten nicht.
	*/
	void SetLogCacheMiss(bool Flag) { m_LogCacheMiss = Flag; }

	/**
		@brief Schreibt die Namen aller gelockten Resourcen in die Log-Datei.
	*/
	void DumpLockedResources();

private:
	/**
		@brief Erzeugt einen neuen Resource-Manager.
		@param pKernel ein Pointer auf den Kernel.
		@remark Nur der BS_Kernel darf Exemplare dieser Klasse erzeugen. Daher ist der Konstruktor private.
	*/
	BS_ResourceManager(BS_Kernel* pKernel) :
		m_KernelPtr(pKernel),
		m_MaxMemoryUsage(100000000),
		m_LogCacheMiss(false)
	{};
	virtual ~BS_ResourceManager();
	
	enum
	{
		HASH_TABLE_BUCKETS = 256
	};

	/**
		@brief Verschiebt eine Resource an die Spitze der Resourcenliste.
		@param pResource die Resource
	*/
	void MoveToFront(BS_Resource* pResource);

	/**
		@brief Lädt eine Resource und aktualisiert m_UsedMemory.

		Die Resource darf nicht bereits geladen sein.
		
		@param FileName der absolute und eindeutige Dateiname der zu ladenen Resource
		@return Gibt einen Pointer auf die geladene Resource zurück wenn das Laden erfolgreich war, ansonsten NULL.
	*/
	BS_Resource* LoadResource(const std::string& FileName);

	/**
		@brief Gibt zu einer Datei ihren absoluten, eindeutigen Pfad zurück.
		@param FileName der Dateiname
		@return Der absolute, eindeutige Pfad zur Datei inklusive des Dateinamens.<br>
				Gibt einen leeren std::string zurück, wenn der Pfad nicht erzeugt werden konnte.
	*/
	std::string GetUniqueFileName(const std::string& FileName) const;

	/**
		@brief Löscht eine Resource entfernt sie aus den Listen und aktualisiert m_UsedMemory.
		@param pResource die zu löschende Resource
		@return Gibt einen Iterator zurück, der auf die nächste Resource in der Resourcenliste zeigt.
	*/
	std::list<BS_Resource*>::iterator DeleteResource(BS_Resource* pResource);

	/**
		@brief Holt einen Pointer zu einer geladenen Resource.
		@param UniqueFileName der absolute, eindeutige Pfad zur Datei inklusive des Dateinamens.
		@return Gibt einen Pointer auf die angeforderte Resource zurück, oder NULL, wenn die Resourcen nicht geladen ist.
	*/
	BS_Resource* GetResource(const std::string& UniqueFileName) const;

	/**
		@brief Löscht solange Resourcen, bis der Prozess unter das angegebene Maximun an Speicherbelegung kommt.
	*/
	void DeleteResourcesIfNecessary();

	BS_Kernel*							m_KernelPtr;
	unsigned int						m_MaxMemoryUsage;
	std::vector<BS_ResourceService*>	m_ResourceServices;
	std::list<BS_Resource*>				m_Resources;
	std::list<BS_Resource*>				m_ResourceHashTable[HASH_TABLE_BUCKETS];
	bool								m_LogCacheMiss;
};

#endif

