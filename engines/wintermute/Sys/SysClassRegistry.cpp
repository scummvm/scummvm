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

#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/wintermute.h"
#include "SysInstance.h"
#include "SysClassRegistry.h"
#include "common/stream.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CSysClassRegistry::CSysClassRegistry() {
}


//////////////////////////////////////////////////////////////////////////
CSysClassRegistry::~CSysClassRegistry() {
	unregisterClasses();
}

//////////////////////////////////////////////////////////////////////////
CSysClassRegistry *CSysClassRegistry::getInstance() {
	return g_wintermute->getClassRegistry();
}

void CSysClassRegistry::unregisterClasses() {
	// CSysClass calls UnregisterClass upon destruction.
	while (_classes.size() > 0) {
		delete _classes.begin()->_value;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::registerClass(CSysClass *classObj) {
	classObj->setID(_count++);
	//_classes.insert(classObj);
	_classes[classObj] = classObj;

	_nameMap[classObj->getName()] = classObj;
	_idMap[classObj->getID()] = classObj;

	return true;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::unregisterClass(CSysClass *classObj) {

	Classes::iterator it = _classes.find(classObj);
	if (it == _classes.end()) return false;

	if (classObj->getNumInstances() != 0) {
		char str[MAX_PATH];
		sprintf(str, "Memory leak@class %-20s: %d instance(s) left\n", classObj->getName().c_str(), classObj->getNumInstances());
		CBPlatform::OutputDebugString(str);
	}
	_classes.erase(it);

	NameMap::iterator mapIt = _nameMap.find(classObj->getName());
	if (mapIt != _nameMap.end()) _nameMap.erase(mapIt);

	IdMap::iterator idIt = _idMap.find(classObj->getID());
	if (idIt != _idMap.end()) _idMap.erase(idIt);


	return true;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::registerInstance(const char *className, void *instance) {
	if (_disabled) return true;

	NameMap::iterator mapIt = _nameMap.find(className);
	if (mapIt == _nameMap.end()) return false;

	CSysInstance *inst = (*mapIt)._value->addInstance(instance, _count++);
	return (inst != NULL);
}

//////////////////////////////////////////////////////////////////////////
void CSysClassRegistry::addInstanceToTable(CSysInstance *instance, void *pointer) {
	_instanceMap[pointer] = instance;

	if (instance->getSavedID() >= 0)
		_savedInstanceMap[instance->getSavedID()] = instance;
}

//////////////////////////////////////////////////////////////////////////
int CSysClassRegistry::getNextID() {
	return _count++;
}

//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::unregisterInstance(const char *className, void *instance) {
	NameMap::iterator mapIt = _nameMap.find(className);
	if (mapIt == _nameMap.end()) return false;
	(*mapIt)._value->removeInstance(instance);

	InstanceMap::iterator instIt = _instanceMap.find(instance);
	if (instIt != _instanceMap.end()) {
		_instanceMap.erase(instIt);
		return true;
	} else return false;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::getPointerID(void *pointer, int *classID, int *instanceID) {
	if (pointer == NULL) return true;

	InstanceMap::iterator it = _instanceMap.find(pointer);
	if (it == _instanceMap.end()) return false;


	CSysInstance *inst = (*it)._value;
	*instanceID = inst->getID();
	*classID = inst->getClass()->getID();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void *CSysClassRegistry::idToPointer(int classID, int instanceID) {
	SavedInstanceMap::iterator it = _savedInstanceMap.find(instanceID);
	if (it == _savedInstanceMap.end()) return NULL;
	else return (*it)._value->getInstance();
}

bool checkHeader(const char *tag, CBPersistMgr *pm) {
	char *test = pm->getString();
	Common::String verify = test;
	delete[] test;
	bool retVal = (verify == tag);
	if (!retVal) {
		error("Expected %s in Save-file not found", tag);
	}
	return retVal;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::saveTable(CBGame *Game, CBPersistMgr *persistMgr, bool quickSave) {
	persistMgr->putString("<CLASS_REGISTRY_TABLE>");
	persistMgr->putDWORD(_classes.size());

	int counter = 0;

	Classes::iterator it;
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		counter++;

		if (!quickSave) {
			Game->_indicatorProgress = (int)(50.0f / (float)((float)_classes.size() / (float)counter));
			Game->DisplayContent(false);
			Game->_renderer->flip();
		}

		(it->_value)->saveTable(Game, persistMgr);
	}
	persistMgr->putString("</CLASS_REGISTRY_TABLE>");
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::loadTable(CBGame *Game, CBPersistMgr *persistMgr) {
	checkHeader("<CLASS_REGISTRY_TABLE>", persistMgr);

	// reset SavedID of current instances
	Classes::iterator it;
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		(it->_value)->resetSavedIDs();
	}

	for (it = _classes.begin(); it != _classes.end(); ++it) {
		if ((it->_value)->isPersistent()) continue;
		(it->_value)->removeAllInstances();
	}

	_instanceMap.clear();

	uint32 numClasses = persistMgr->getDWORD();

	for (uint32 i = 0; i < numClasses; i++) {
		Game->_indicatorProgress = (int)(50.0f / (float)((float)numClasses / (float)i));
		Game->DisplayContentSimple();
		Game->_renderer->flip();

		char *className = persistMgr->getString();
		NameMap::iterator mapIt = _nameMap.find(className);
		if (mapIt != _nameMap.end())(*mapIt)._value->loadTable(Game, persistMgr);
	}

	checkHeader("</CLASS_REGISTRY_TABLE>", persistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::saveInstances(CBGame *Game, CBPersistMgr *persistMgr, bool quickSave) {

	Classes::iterator it;

	// count total instances
	int numInstances = 0;
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		numInstances += (it->_value)->getNumInstances();
	}

	persistMgr->putDWORD(numInstances);

	int counter = 0;
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		counter++;

		if (!quickSave) {
			if (counter % 20 == 0) {
				Game->_indicatorProgress = 50 + 50.0f / (float)((float)_classes.size() / (float)counter);
				Game->DisplayContent(false);
				Game->_renderer->flip();
			}
		}
		Game->MiniUpdate();

		(it->_value)->saveInstances(Game, persistMgr);
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::loadInstances(CBGame *Game, CBPersistMgr *persistMgr) {
	// get total instances
	int numInstances = persistMgr->getDWORD();

	for (int i = 0; i < numInstances; i++) {
		if (i % 20 == 0) {
			Game->_indicatorProgress = 50 + 50.0f / (float)((float)numInstances / (float)i);
			Game->DisplayContentSimple();
			Game->_renderer->flip();
		}

		checkHeader("<INSTANCE_HEAD>", persistMgr);

		int classID = persistMgr->getDWORD();
		int instanceID = persistMgr->getDWORD();
		void *instance = idToPointer(classID, instanceID);

		checkHeader("</INSTANCE_HEAD>", persistMgr);

		Classes::iterator it;
		for (it = _classes.begin(); it != _classes.end(); ++it) {
			if ((it->_value)->getSavedID() == classID) {
				(it->_value)->loadInstance(instance, persistMgr);
				break;
			}
		}
		checkHeader("</INSTANCE>", persistMgr);
	}

	_savedInstanceMap.clear();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::enumInstances(SYS_INSTANCE_CALLBACK lpCallback, const char *className, void *lpData) {
	NameMap::iterator mapIt = _nameMap.find(className);
	if (mapIt == _nameMap.end()) return E_FAIL;

	(*mapIt)._value->instanceCallback(lpCallback, lpData);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CSysClassRegistry::dumpClasses(Common::WriteStream *stream) {
	Classes::iterator it;
	for (it = _classes.begin(); it != _classes.end(); ++it)
		(it->_value)->dump(stream);
}

} // end of namespace WinterMute
