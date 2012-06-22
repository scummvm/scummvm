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

#include "engines/wintermute/persistent.h"
#include "SysInstance.h"
#include "SysClass.h"
#include "SysClassRegistry.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BPersistMgr.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CSysClass::CSysClass(const AnsiString &name, PERSISTBUILD build, PERSISTLOAD load, bool persistent_class) {
	_name = name;

	_build = build;
	_load = load;
	_next = NULL;
	_savedID = -1;
	_persistent = persistent_class;
	_numInst = 0;

	CSysClassRegistry::GetInstance()->RegisterClass(this);
}


//////////////////////////////////////////////////////////////////////////
CSysClass::~CSysClass() {
	CSysClassRegistry::GetInstance()->UnregisterClass(this);
	RemoveAllInstances();
}

//////////////////////////////////////////////////////////////////////////
bool CSysClass::RemoveAllInstances() {
	Instances::iterator it;
	for (it = _instances.begin(); it != _instances.end(); ++it) {
		delete(it->_value);
	}
	_instances.clear();
	_instanceMap.clear();

	return true;
}

//////////////////////////////////////////////////////////////////////////
CSysInstance *CSysClass::AddInstance(void *instance, int id, int savedId) {
	CSysInstance *inst = new CSysInstance(instance, id, this);
	inst->SetSavedID(savedId);
	_instances[inst] = (inst);

	_instanceMap[instance] = inst;

	CSysClassRegistry::GetInstance()->AddInstanceToTable(inst, instance);

	return inst;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClass::RemoveInstance(void *instance) {
	InstanceMap::iterator mapIt = _instanceMap.find(instance);
	if (mapIt == _instanceMap.end()) return false;

	Instances::iterator it = _instances.find((mapIt->_value));
	if (it != _instances.end()) {
		delete(it->_value);
		_instances.erase(it);
	}

	_instanceMap.erase(mapIt);

	return false;
}

//////////////////////////////////////////////////////////////////////////
int CSysClass::GetInstanceID(void *pointer) {
	InstanceMap::iterator mapIt = _instanceMap.find(pointer);
	if (mapIt == _instanceMap.end()) return -1;
	else return (mapIt->_value)->GetID();
}

//////////////////////////////////////////////////////////////////////////
void *CSysClass::IDToPointer(int savedID) {
	//slow
	Instances::iterator it;
	for (it = _instances.begin(); it != _instances.end(); ++it) {
		if ((it->_value)->GetSavedID() == savedID) return (it->_value)->GetInstance();
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
int CSysClass::GetNumInstances() {
	return _instances.size();
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::Dump(Common::WriteStream *stream) {
	Common::String str;
	str = Common::String::format("%03d %c %-20s instances: %d\n", _iD, _persistent ? 'p' : ' ', _name.c_str(), GetNumInstances());
	stream->write(str.c_str(), str.size());
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::SaveTable(CBGame *Game, CBPersistMgr *PersistMgr) {
	PersistMgr->PutString(_name.c_str());
	PersistMgr->PutDWORD(_iD);
	PersistMgr->PutDWORD(_instances.size());

	Instances::iterator it;
	for (it = _instances.begin(); it != _instances.end(); ++it) {
		PersistMgr->PutDWORD((it->_value)->GetID());
	}
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::LoadTable(CBGame *Game, CBPersistMgr *PersistMgr) {
	_savedID = PersistMgr->GetDWORD();
	int numInstances = PersistMgr->GetDWORD();

	for (int i = 0; i < numInstances; i++) {
		int instID = PersistMgr->GetDWORD();
		if (_persistent) {

			if (i > 0) {
				Game->LOG(0, "Warning: attempting to load multiple instances of persistent class %s (%d)", _name.c_str(), numInstances);
				continue;
			}

			Instances::iterator it = _instances.begin();
			if (it != _instances.end()) {
				(it->_value)->SetSavedID(instID);
				CSysClassRegistry::GetInstance()->AddInstanceToTable((it->_value), (it->_value)->GetInstance());
			} else Game->LOG(0, "Warning: instance %d of persistent class %s not found", i, _name.c_str());
		}
		// normal instances, create empty objects
		else {
			void *emptyObject = _build();
			if (!emptyObject) {
				warning("HALT");
			}

			AddInstance(emptyObject, CSysClassRegistry::GetInstance()->GetNextID(), instID);
		}

	}
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::SaveInstances(CBGame *Game, CBPersistMgr *PersistMgr) {
	Instances::iterator it;
	for (it = _instances.begin(); it != _instances.end(); ++it) {
		// write instace header
		PersistMgr->PutString("<INSTANCE_HEAD>");
		PersistMgr->PutDWORD(_iD);
		PersistMgr->PutDWORD((it->_value)->GetID());
		PersistMgr->PutString("</INSTANCE_HEAD>");
		_load((it->_value)->GetInstance(), PersistMgr);
		PersistMgr->PutString("</INSTANCE>");
	}
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::LoadInstance(void *instance, CBPersistMgr *PersistMgr) {
	_load(instance, PersistMgr);
}


//////////////////////////////////////////////////////////////////////////
void CSysClass::ResetSavedIDs() {
	Instances::iterator it;
	for (it = _instances.begin(); it != _instances.end(); ++it) {
		(it->_value)->SetSavedID(-1);
	}
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::InstanceCallback(SYS_INSTANCE_CALLBACK lpCallback, void *lpData) {
	Instances::iterator it;
	for (it = _instances.begin(); it != _instances.end(); ++it) {
		lpCallback((it->_value)->GetInstance(), lpData);
	}
}

} // end of namespace WinterMute
