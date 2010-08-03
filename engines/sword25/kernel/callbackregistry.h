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

#ifndef SWORD25_CALLBACK_REGISTRY_H
#define SWORD25_CALLBACK_REGISTRY_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common/scummsys.h"
#include "common/str.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "sword25/kernel/common.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

class BS_CallbackRegistry {
public:
	static BS_CallbackRegistry & GetInstance() {
		static BS_CallbackRegistry Instance;
		return Instance;
	}

	bool		RegisterCallbackFunction(const Common::String &Name, void * Ptr);
	void *		ResolveCallbackFunction(const Common::String &Name) const;
	Common::String	ResolveCallbackPointer(void *Ptr) const;

private:
	typedef Common::HashMap<Common::String, void *, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> NameToPtrMap;
	NameToPtrMap m_NameToPtrMap;
	typedef Common::HashMap<void *, Common::String, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> PtrToNameMap;
	PtrToNameMap m_PtrToNameMap;

	void *		FindPtrByName(const Common::String &Name) const;
	Common::String	FindNameByPtr(void * Ptr) const;
	void		StoreCallbackFunction(const Common::String & Name, void * Ptr);
};

} // End of namespace Sword25

#endif
