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

#ifndef WINTERMUTE_SYSCLASS_H
#define WINTERMUTE_SYSCLASS_H

#include "persistent.h"
#include <set>
#include <map>
#include "dctypes.h"

namespace WinterMute {
class CSysInstance;
class CBGame;
class CBPersistMgr;
class CSysClass {
public:
	CSysClass(const AnsiString &name, PERSISTBUILD build, PERSISTLOAD load, bool persistent_class);
	~CSysClass();

	int GetNumInstances();
	bool RemoveInstance(void *instance);
	CSysInstance *AddInstance(void *instance, int id, int savedId = -1);
	bool RemoveAllInstances();

	int GetInstanceID(void *pointer);
	void *IDToPointer(int savedID);

	void SetID(int id) {
		m_ID = id;
	}
	int GetID() const {
		return m_ID;
	}

	int GetSavedID() const {
		return m_SavedID;
	}

	bool IsPersistent() const {
		return m_Persistent;
	}

	AnsiString GetName() const {
		return m_Name;
	}

	void SaveTable(CBGame *Game, CBPersistMgr *PersistMgr);
	void LoadTable(CBGame *Game, CBPersistMgr *PersistMgr);

	void SaveInstances(CBGame *Game, CBPersistMgr *PersistMgr);
	void LoadInstance(void *instance, CBPersistMgr *PersistMgr);

	void InstanceCallback(SYS_INSTANCE_CALLBACK lpCallback, void *lpData);

	void ResetSavedIDs();

	void Dump(FILE *stream);

private:
	int m_NumInst;
	bool m_Persistent;
	CSysClass *m_Next;
	int m_ID;
	int m_SavedID;
	AnsiString m_Name;
	PERSISTBUILD m_Build;
	PERSISTLOAD m_Load;

	typedef std::set<CSysInstance *> Instances;
	Instances m_Instances;

	typedef std::map<void *, CSysInstance *> InstanceMap;
	InstanceMap m_InstanceMap;
};

} // end of namespace WinterMute

#endif
