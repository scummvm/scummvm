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
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_SYSCLASSREGISTRY_H
#define WINTERMUTE_SYSCLASSREGISTRY_H

#include "wintypes.h"
#include "dctypes.h"
#include "persistent.h"
//#include <set>
//#include <map>
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/func.h"

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

namespace WinterMute {
class CSysClass;
}

namespace Common {
template<typename T> struct Hash;
template<> struct Hash<WinterMute::CSysClass *> : public UnaryFunction<WinterMute::CSysClass *, uint> {
	uint operator()(WinterMute::CSysClass *val) const {
		return (uint)((size_t)val);
	}
};

}

namespace WinterMute {

class CBGame;
class CBPersistMgr;
class CSysInstance;

class CSysClassRegistry {
public:
	static CSysClassRegistry *GetInstance();

	HRESULT EnumInstances(SYS_INSTANCE_CALLBACK lpCallback, const char *className, void *lpData);
	HRESULT LoadTable(CBGame *Game, CBPersistMgr *PersistMgr);
	HRESULT SaveTable(CBGame *Game, CBPersistMgr *PersistMgr, bool quickSave);
	HRESULT LoadInstances(CBGame *Game, CBPersistMgr *PersistMgr);
	HRESULT SaveInstances(CBGame *Game, CBPersistMgr *PersistMgr, bool quickSave);
	void *IDToPointer(int classID, int instanceID);
	bool GetPointerID(void *pointer, int *classID, int *instanceID);
	bool RegisterClass(CSysClass *classObj);
	bool UnregisterClass(CSysClass *classObj);
	bool RegisterInstance(const char *className, void *instance);
	bool UnregisterInstance(const char *className, void *instance);
	void DumpClasses(void *stream);
	int GetNextID();
	void AddInstanceToTable(CSysInstance *instance, void *pointer);

	CSysClassRegistry();
	virtual ~CSysClassRegistry();

	bool _disabled;
	int _count;

	typedef Common::HashMap<CSysClass *, CSysClass *> Classes;
	Classes _classes;

	typedef Common::HashMap<AnsiString, CSysClass *> NameMap;
	NameMap _nameMap;

	typedef Common::HashMap<int, CSysClass *> IdMap;
	IdMap _idMap;

	typedef Common::HashMap<void *, CSysInstance *> InstanceMap;
	InstanceMap _instanceMap;

	typedef Common::HashMap<int, CSysInstance *> SavedInstanceMap;
	SavedInstanceMap _savedInstanceMap;

};

} // end of namespace WinterMute

#endif
