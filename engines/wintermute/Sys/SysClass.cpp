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

	CSysClassRegistry::getInstance()->registerClass(this);
}


//////////////////////////////////////////////////////////////////////////
CSysClass::~CSysClass() {
	CSysClassRegistry::getInstance()->unregisterClass(this);
	removeAllInstances();
}

//////////////////////////////////////////////////////////////////////////
bool CSysClass::removeAllInstances() {
	Instances::iterator it;
	for (it = _instances.begin(); it != _instances.end(); ++it) {
		delete(it->_value);
	}
	_instances.clear();
	_instanceMap.clear();

	return true;
}

//////////////////////////////////////////////////////////////////////////
CSysInstance *CSysClass::addInstance(void *instance, int id, int savedId) {
	CSysInstance *inst = new CSysInstance(instance, id, this);
	inst->setSavedID(savedId);
	_instances[inst] = (inst);

	_instanceMap[instance] = inst;

	CSysClassRegistry::getInstance()->addInstanceToTable(inst, instance);

	return inst;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClass::removeInstance(void *instance) {
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
int CSysClass::getInstanceID(void *pointer) {
	InstanceMap::iterator mapIt = _instanceMap.find(pointer);
	if (mapIt == _instanceMap.end()) return -1;
	else return (mapIt->_value)->getID();
}

//////////////////////////////////////////////////////////////////////////
void *CSysClass::idToPointer(int savedID) {
	//slow
	Instances::iterator it;
	for (it = _instances.begin(); it != _instances.end(); ++it) {
		if ((it->_value)->getSavedID() == savedID) return (it->_value)->getInstance();
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
int CSysClass::getNumInstances() {
	return _instances.size();
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::dump(Common::WriteStream *stream) {
	Common::String str;
	str = Common::String::format("%03d %c %-20s instances: %d\n", _iD, _persistent ? 'p' : ' ', _name.c_str(), getNumInstances());
	stream->write(str.c_str(), str.size());
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::saveTable(CBGame *gameRef, CBPersistMgr *persistMgr) {
	persistMgr->putString(_name.c_str());
	persistMgr->putDWORD(_iD);
	persistMgr->putDWORD(_instances.size());

	Instances::iterator it;
	for (it = _instances.begin(); it != _instances.end(); ++it) {
		persistMgr->putDWORD((it->_value)->getID());
	}
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::loadTable(CBGame *gameRef, CBPersistMgr *persistMgr) {
	_savedID = persistMgr->getDWORD();
	int numInstances = persistMgr->getDWORD();

	for (int i = 0; i < numInstances; i++) {
		int instID = persistMgr->getDWORD();
		if (_persistent) {

			if (i > 0) {
				gameRef->LOG(0, "Warning: attempting to load multiple instances of persistent class %s (%d)", _name.c_str(), numInstances);
				continue;
			}

			Instances::iterator it = _instances.begin();
			if (it != _instances.end()) {
				(it->_value)->setSavedID(instID);
				CSysClassRegistry::getInstance()->addInstanceToTable((it->_value), (it->_value)->getInstance());
			} else gameRef->LOG(0, "Warning: instance %d of persistent class %s not found", i, _name.c_str());
		}
		// normal instances, create empty objects
		else {
			void *emptyObject = _build();
			if (!emptyObject) {
				warning("HALT");
			}

			addInstance(emptyObject, CSysClassRegistry::getInstance()->getNextID(), instID);
		}

	}
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::saveInstances(CBGame *Game, CBPersistMgr *persistMgr) {
	Instances::iterator it;
	for (it = _instances.begin(); it != _instances.end(); ++it) {
		// write instace header
		persistMgr->putString("<INSTANCE_HEAD>");
		persistMgr->putDWORD(_iD);
		persistMgr->putDWORD((it->_value)->getID());
		persistMgr->putString("</INSTANCE_HEAD>");
		_load((it->_value)->getInstance(), persistMgr);
		persistMgr->putString("</INSTANCE>");
	}
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::loadInstance(void *instance, CBPersistMgr *persistMgr) {
	_load(instance, persistMgr);
}


//////////////////////////////////////////////////////////////////////////
void CSysClass::resetSavedIDs() {
	Instances::iterator it;
	for (it = _instances.begin(); it != _instances.end(); ++it) {
		(it->_value)->setSavedID(-1);
	}
}

//////////////////////////////////////////////////////////////////////////
void CSysClass::instanceCallback(SYS_INSTANCE_CALLBACK lpCallback, void *lpData) {
	Instances::iterator it;
	for (it = _instances.begin(); it != _instances.end(); ++it) {
		lpCallback((it->_value)->getInstance(), lpData);
	}
}

} // end of namespace WinterMute
