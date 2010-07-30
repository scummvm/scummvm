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

#ifndef SWORD25_RESOURCE_H
#define SWORD25_RESOURCE_H

#include "sword25/kernel/memlog_off.h"
#include <list>
#include "sword25/kernel/memlog_on.h"

#include "sword25/kernel/common.h"

class BS_Kernel;
class BS_ResourceManager;

class BS_Resource
{
friend class BS_ResourceManager;

public:
	enum RESOURCE_TYPES
	{
		TYPE_UNKNOWN,
		TYPE_BITMAP,
		TYPE_ANIMATION,
		TYPE_SOUND,
		TYPE_FONT
	};

	BS_Resource(const std::string& UniqueFileName, RESOURCE_TYPES Type);

	/**
	 * @brief `Lockt' die Resource, verhindert, dass sie freigegeben wird.
	 * @remarks Die Resource wird bereits `gelockt' initialisiert, sie muss also nach dem Anfordern nur 
	 *	gelockt werden, wenn sie mehrfach verwendet wird.
	 **/

	void AddReference() { ++_RefCount; }	

	/**
	 * @brief Hebt ein vorhergehendes `lock' auf.
	 * @remarks Die Resource kann ruhig öfter freigegeben als `gelockt' werden, auch wenn das nicht gerade empfehlenswert ist.
	 **/

	void Release();

	/**
	 * @brief Gibt die Anzahl der aktuellen `locks' zurück.
	 * @return Die Zahl der `locks'.
	 **/

	int GetLockCount() const { return _RefCount; }

	/**
		@brief Gibt den absoluten, eindeutigen Dateinamen der Resource zurück.
	*/

	const std::string & GetFileName() const { return _FileName; }

	/**
		@brief Gibt den Hash des Dateinames der Resource zurück.
	*/
	unsigned int GetFileNameHash() const { return _FileNameHash; }

	/**
		@brief Gibt den Typ der Ressource zurück.
	*/
	unsigned int GetType() const { return _Type; }

protected:
	virtual ~BS_Resource() {};

private:
	std::string							_FileName;			//!< Der absolute Dateiname
	unsigned int						_FileNameHash;		//!< Der Hashwert des Dateinames
	unsigned int						_RefCount;			//!< Anzahl an Locks
	unsigned int						_Type;				//!< Der Typ der Resource
	std::list<BS_Resource*>::iterator	_Iterator;			//!< Der Iterator zeigt auf Position der Resource in der LRU-Liste
};

#endif
