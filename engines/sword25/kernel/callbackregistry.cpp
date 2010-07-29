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

// Alle Callbackfunktionen die von Objekten gerufen werden, die persistiert werden können, müssen hier registriert werden.
// Beim Speichern wird statt des Pointers der Bezeichner gespeichert. Beim Laden wird der Bezeichner wieder in einen Pointer umgewandelt.
// Diese Klasse führt also so etwas ähnliches wie eine Importtabelle für Callback-Funktionen.
//
// Dieses Vorgehen hat mehrere Vorteile:
// 1. Die Speicherstände sind plattformunabhängig. Es werden keine Pointer auf Funktionen gespeichert, sondern nur Namen von Callbackfunktionen.
//	  Diese können beim Laden über diese Klasse in systemabhängige Pointer umgewandelt werden.
// 2. Speicherstände können auch nach einem Engineupdate weiterhin benutzt werden. Beim Erstellen einer neun Binary verschieben sich häufig die
//	  Funktionen. Eine Callbackfunktion könnte sich also nach einem Update an einer anderen Stelle befinden als davor. Wenn im Spielstand der
//	  Pointer gespeichert war, stürtzt das Programm beim Äufrufen dieser Callbackfunktion ab. Durch das Auflösungverfahren wird beim Laden der
//	  Callbackbezeichner in den neuen Funktionspointer umgewandelt und der Aufruf kann erfolgen.

// -----------------------------------------------------------------------------
// Logging
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "CALLBACKREGISTRY"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "callbackregistry.h"

// -----------------------------------------------------------------------------

bool BS_CallbackRegistry::RegisterCallbackFunction(const std::string & Name, void * Ptr)
{
	if (Name == "")
	{
		BS_LOG_ERRORLN("The empty string is not allowed as a callback function name.");
		return false;
	}

	if (FindPtrByName(Name) != 0)
	{
		BS_LOG_ERRORLN("There is already a callback function with the name \"%s\".", Name.c_str());
		return false;
	}
	if (FindNameByPtr(Ptr) != "")
	{
		BS_LOG_ERRORLN("There is already a callback function with the pointer 0x%x.", Ptr);
		return false;
	}

	StoreCallbackFunction(Name, Ptr);
	
	return true;
}

// -----------------------------------------------------------------------------

void * BS_CallbackRegistry::ResolveCallbackFunction(const std::string & Name) const
{
	void * Result = FindPtrByName(Name);

	if (!Result)
	{
		BS_LOG_ERRORLN("There is no callback function with the name \"%s\".", Name.c_str());
	}

	return Result;
}

// -----------------------------------------------------------------------------

std::string BS_CallbackRegistry::ResolveCallbackPointer(void * Ptr) const
{
	const std::string & Result = FindNameByPtr(Ptr);

	if (Result == "")
	{
		BS_LOG_ERRORLN("There is no callback function with the pointer 0x%x.", Ptr);
	}

	return Result;
}

// -----------------------------------------------------------------------------

void * BS_CallbackRegistry::FindPtrByName(const std::string & Name) const
{
	// Eintrag in der Map finden und den Pointer zurückgeben.
	NameToPtrMap::const_iterator It = m_NameToPtrMap.find(Name);
	return It == m_NameToPtrMap.end() ? 0 : It->second;
}

// -----------------------------------------------------------------------------

std::string BS_CallbackRegistry::FindNameByPtr(void * Ptr) const
{
	// Eintrag in der Map finden und den Namen zurückgeben.
	PtrToNameMap::const_iterator It = m_PtrToNameMap.find(Ptr);
	return It == m_PtrToNameMap.end() ? "" : It->second;
}

// -----------------------------------------------------------------------------

void BS_CallbackRegistry::StoreCallbackFunction(const std::string & Name, void * Ptr)
{
	// Callback-Funktion in beide Maps eintragen.
	m_NameToPtrMap[Name] = Ptr;
	m_PtrToNameMap[Ptr] = Name;
}
