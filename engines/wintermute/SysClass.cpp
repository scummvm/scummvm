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

#include "persistent.h"
#include "SysInstance.h"
#include "SysClass.h"
#include "SysClassRegistry.h"
#include "BGame.h"
#include "BPersistMgr.h"

namespace WinterMute {

// TODO: Note that the set was removed, this might have bizarre side-effects.

//////////////////////////////////////////////////////////////////////////
CSysClass::CSysClass(const AnsiString &name, PERSISTBUILD build, PERSISTLOAD load, bool persistent_class) {
	m_Name = name;

	m_Build = build;
	m_Load = load;
	m_Next = NULL;
	m_SavedID = -1;
	m_Persistent = persistent_class;
	m_NumInst = 0;

	CSysClassRegistry::GetInstance()->RegisterClass(this);
}


//////////////////////////////////////////////////////////////////////////
CSysClass::~CSysClass() {
	CSysClassRegistry::GetInstance()->UnregisterClass(this);
	RemoveAllInstances();
}

//////////////////////////////////////////////////////////////////////////
bool CSysClass::RemoveAllInstances() {
	InstanceMap::iterator it;
	for (it = m_InstanceMap.begin(); it != m_InstanceMap.end(); ++it) {
		delete(it->_value);
	}
	//m_Instances.clear();
	m_InstanceMap.clear();

	return true;
}

//////////////////////////////////////////////////////////////////////////
CSysInstance *CSysClass::AddInstance(void *instance, int id, int savedId) {
	CSysInstance *inst = new CSysInstance(instance, id, this);
	inst->SetSavedID(savedId);
	//m_Instances.insert(inst);

	m_InstanceMap[instance] = inst;

	CSysClassRegistry::GetInstance()->AddInstanceToTable(inst, instance);

	return inst;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClass::RemoveInstance(void *instance) {
	InstanceMap::iterator mapIt = m_InstanceMap.find(instance);
	if (mapIt == m_InstanceMap.end()) return false;
/*
	Instances::iterator it = m_Instances.find((*mapIt).second);
	if (it != m_Instances.end()) {
		delete(*it);
		m_Instances.erase(it);
	}*/

	delete mapIt->_value;
	m_InstanceMap.erase(mapIt);

	return false;
}

//////////////////////////////////////////////////////////////////////////
int CSysClass::GetInstanceID(void *pointer) {
	InstanceMap::iterator mapIt = m_InstanceMap.find(pointer);
	if (mapIt == m_InstanceMap.end()) return -1;
	else return (*mapIt)._value->GetID();
}

//////////////////////////////////////////////////////////////////////////
void *CSysClass::IDToPointer(int savedID) {
	//slow
	/*Instances::iterator it;
	for (it = m_Instances.begin(); it != m_Instances.end(); ++it) {
		if ((*it)->GetSavedID() == savedID) return (*it)->GetInstance();
	}*/
	InstanceMap::iterator it;
	for (it = m_InstanceMap.begin(); it != m_InstanceMap.end(); ++it) {
		if ((it->_value)->GetSavedID() == savedID) return (it->_value)->GetInstance();
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
int CSysClass::GetNumInstances() {
	//return m_Instances.size();
	return m_InstanceMap.size(); // TODO: This might break, if we have multiple keys per value.
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::Dump(void *stream) {
	fprintf((FILE*)stream, "%03d %c %-20s instances: %d\n", m_ID, m_Persistent ? 'p' : ' ', m_Name.c_str(), GetNumInstances());
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::SaveTable(CBGame *Game, CBPersistMgr *PersistMgr) {
	PersistMgr->PutString(m_Name.c_str());
	PersistMgr->PutDWORD(m_ID);
	PersistMgr->PutDWORD(m_InstanceMap.size());

	InstanceMap::iterator it;
	for (it = m_InstanceMap.begin(); it != m_InstanceMap.end(); ++it) {
		PersistMgr->PutDWORD((it->_value)->GetID());
	}
	/*
	Instances::iterator it;
	for (it = m_Instances.begin(); it != m_Instances.end(); ++it) {
		PersistMgr->PutDWORD((*it)->GetID());
	}*/
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::LoadTable(CBGame *Game, CBPersistMgr *PersistMgr) {
	m_SavedID = PersistMgr->GetDWORD();
	int numInstances = PersistMgr->GetDWORD();

	for (int i = 0; i < numInstances; i++) {
		if (m_Persistent) {
			int instId = PersistMgr->GetDWORD();

			if (i > 0) {
				Game->LOG(0, "Warning: attempting to load multiple instances of persistent class %s (%d)", m_Name.c_str(), numInstances);
				continue;
			}

			InstanceMap::iterator it = m_InstanceMap.begin();
/*			Instances::iterator it = m_Instances.begin();*/
			if (it != m_InstanceMap.end()) {
				(it->_value)->SetSavedID(instId);
				CSysClassRegistry::GetInstance()->AddInstanceToTable((it->_value), (it->_value)->GetInstance());
			} else Game->LOG(0, "Warning: instance %d of persistent class %s not found", i, m_Name.c_str());
		}
		// normal instances, create empty objects
		else {
			void *emptyObject = m_Build();
			AddInstance(emptyObject, CSysClassRegistry::GetInstance()->GetNextID(), PersistMgr->GetDWORD());
		}

	}
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::SaveInstances(CBGame *Game, CBPersistMgr *PersistMgr) {
	InstanceMap::iterator it;
	for (it = m_InstanceMap.begin(); it != m_InstanceMap.end(); ++it) {
		// write instace header
		PersistMgr->PutDWORD(m_ID);
		PersistMgr->PutDWORD((it->_value)->GetID());

		m_Load((it->_value)->GetInstance(), PersistMgr);
	}
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::LoadInstance(void *instance, CBPersistMgr *PersistMgr) {
	m_Load(instance, PersistMgr);
}


//////////////////////////////////////////////////////////////////////////
void CSysClass::ResetSavedIDs() {
	InstanceMap::iterator it;
	for (it = m_InstanceMap.begin(); it != m_InstanceMap.end(); ++it) {
		(it->_value)->SetSavedID(-1);
	}
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::InstanceCallback(SYS_INSTANCE_CALLBACK lpCallback, void *lpData) {
	InstanceMap::iterator it;
	for (it = m_InstanceMap.begin(); it != m_InstanceMap.end(); ++it) {
		lpCallback((it->_value)->GetInstance(), lpData);
	}
}

} // end of namespace WinterMute
