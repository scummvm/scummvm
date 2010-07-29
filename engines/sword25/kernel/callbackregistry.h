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

#ifndef BS_CALLBACK_REGISTRY_H
#define BS_CALLBACK_REGISTRY_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"

#include "kernel/memlog_off.h"
#include <map>
#include "kernel/memlog_on.h"

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

class BS_CallbackRegistry
{
public:
	static BS_CallbackRegistry & GetInstance()
	{
		static BS_CallbackRegistry Instance;
		return Instance;
	}

	bool		RegisterCallbackFunction(const std::string & Name, void * Ptr);
	void *		ResolveCallbackFunction(const std::string & Name) const;
	std::string	ResolveCallbackPointer(void * Ptr) const;

private:
	typedef std::map<std::string, void *> NameToPtrMap;
	NameToPtrMap m_NameToPtrMap;
	typedef std::map<void *, std::string> PtrToNameMap;
	PtrToNameMap m_PtrToNameMap;

	void *		FindPtrByName(const std::string & Name) const;
	std::string	FindNameByPtr(void * Ptr) const;
	void		StoreCallbackFunction(const std::string & Name, void * Ptr);
};


#endif
