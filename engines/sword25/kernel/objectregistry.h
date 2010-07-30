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

#ifndef SWORD25_OBJECTREGISTRY_H
#define SWORD25_OBJECTREGISTRY_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/hashmap.h"

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

template<typename T>
class BS_ObjectRegistry
{
public:
	BS_ObjectRegistry() : m_NextHandle(1) {};

	// -------------------------------------------------------------------------

	unsigned int RegisterObject(T * ObjectPtr)
	{
		// Null-Pointer können nicht registriert werden.
		if (ObjectPtr == 0)
		{
			LogErrorLn("Cannot register a null pointer.");
			return 0;
		}

		// Falls das Objekt bereits registriert wurde, wird eine Warnung ausgeben und das Handle zurückgeben.
		unsigned int Handle = FindHandleByPtr(ObjectPtr);
		if (Handle != 0)
		{
			LogWarningLn("Tried to register a object that was already registered.");
			return Handle;
		}
		// Ansonsten wird das Objekt in beide Maps eingetragen und das neue Handle zurückgeben.
		else
		{
			m_Handle2PtrMap[m_NextHandle] = ObjectPtr;
			m_Ptr2HandleMap[ObjectPtr] = m_NextHandle;

			return m_NextHandle++;
		}
	}

	// -----------------------------------------------------------------------------

	unsigned int RegisterObject(T * ObjectPtr, unsigned int Handle)
	{
		// Null-Pointer und Null-Handle können nicht registriert werden.
		if (ObjectPtr == 0 || Handle == 0)
		{
			LogErrorLn("Cannot register a null pointer or a null handle.");
			return 0;
		}

		// Falls das Objekt bereits registriert wurde, wird ein Fehler ausgegeben und 0 zurückgeben.
		unsigned int HandleTest = FindHandleByPtr(ObjectPtr);
		if (HandleTest != 0)
		{
			LogErrorLn("Tried to register a object that was already registered.");
			return 0;
		}
		// Falls das Handle bereits vergeben ist, wird ein Fehler ausgegeben und 0 zurückgegeben.
		else if (FindPtrByHandle(Handle) != 0)
		{
			LogErrorLn("Tried to register a handle that is already taken.");
			return 0;
		}
		// Ansonsten wird das Objekt in beide Maps eingetragen und das gewünschte Handle zurückgeben.
		else
		{
			m_Handle2PtrMap[Handle] = ObjectPtr;
			m_Ptr2HandleMap[ObjectPtr] = Handle;

			// Falls das vergebene Handle größer oder gleich dem nächsten automatische vergebenen Handle ist, wird das nächste automatisch
			// vergebene Handle erhöht.
			if (Handle >= m_NextHandle) m_NextHandle = Handle + 1;

			return Handle;
		}
	}

	// -----------------------------------------------------------------------------

	void DeregisterObject(T * ObjectPtr)
	{
		unsigned int Handle = FindHandleByPtr(ObjectPtr);

		if (Handle != 0)
		{
			// Registriertes Objekt aus beiden Maps entfernen.
			m_Handle2PtrMap.erase(FindHandleByPtr(ObjectPtr));
			m_Ptr2HandleMap.erase(ObjectPtr);
		}
		else
		{
			LogWarningLn("Tried to remove a object that was not registered.");
		}
	}

	// -----------------------------------------------------------------------------

	T * ResolveHandle(unsigned int Handle)
	{
		// Zum Handle gehöriges Objekt in der Hash-Map finden.
		T * ObjectPtr = FindPtrByHandle(Handle);

		// Pointer zurückgeben. Im Fehlerfall ist dieser 0.
		return ObjectPtr;
	}

	// -----------------------------------------------------------------------------

	unsigned int ResolvePtr(T * ObjectPtr)
	{
		// Zum Pointer gehöriges Handle in der Hash-Map finden.
		unsigned int Handle = FindHandleByPtr(ObjectPtr);

		// Handle zurückgeben. Im Fehlerfall ist dieses 0.
		return Handle;
	}

protected:
	typedef BS_Hashmap<unsigned int, T *>	HANDLE2PTR_MAP;
	typedef BS_Hashmap<T *, unsigned int> PTR2HANDLE_MAP;

	HANDLE2PTR_MAP	m_Handle2PtrMap;
	PTR2HANDLE_MAP	m_Ptr2HandleMap;
	unsigned int	m_NextHandle;

	// -----------------------------------------------------------------------------

	T * FindPtrByHandle(unsigned int Handle)
	{
		// Zum Handle gehörigen Pointer finden.
		HANDLE2PTR_MAP::const_iterator it = m_Handle2PtrMap.find(Handle);

		// Pointer zurückgeben, oder, falls keiner gefunden wurde, 0 zurückgeben.
		return (it != m_Handle2PtrMap.end()) ? it->second : 0;
	}

	// -----------------------------------------------------------------------------

	unsigned int FindHandleByPtr(T * ObjectPtr)
	{
		// Zum Pointer gehöriges Handle finden.
		PTR2HANDLE_MAP::const_iterator it = m_Ptr2HandleMap.find(ObjectPtr);

		// Handle zurückgeben, oder, falls keines gefunden wurde, 0 zurückgeben.
		return (it != m_Ptr2HandleMap.end()) ? it->second : 0;
	}

	// -----------------------------------------------------------------------------

	virtual void LogErrorLn(const char * Message) const = 0;
	virtual void LogWarningLn(const char * Message) const = 0;
};

#endif
