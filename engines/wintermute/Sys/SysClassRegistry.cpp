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
#include "SysInstance.h"
#include "SysClassRegistry.h"
#include "common/stream.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CSysClassRegistry::CSysClassRegistry() {
}


//////////////////////////////////////////////////////////////////////////
CSysClassRegistry::~CSysClassRegistry() {

}

//////////////////////////////////////////////////////////////////////////
CSysClassRegistry *CSysClassRegistry::GetInstance() {
	static CSysClassRegistry classReg;
	return &classReg;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::RegisterClass(CSysClass *classObj) {
	classObj->SetID(_count++);
	//_classes.insert(classObj);
	_classes[classObj] = classObj;

	_nameMap[classObj->GetName()] = classObj;
	_idMap[classObj->GetID()] = classObj;

	return true;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::UnregisterClass(CSysClass *classObj) {

	Classes::iterator it = _classes.find(classObj);
	if (it == _classes.end()) return false;

	if (classObj->GetNumInstances() != 0) {
		char str[MAX_PATH];
		sprintf(str, "Memory leak@class %-20s: %d instance(s) left\n", classObj->GetName().c_str(), classObj->GetNumInstances());
		CBPlatform::OutputDebugString(str);
	}
	_classes.erase(it);

	NameMap::iterator mapIt = _nameMap.find(classObj->GetName());
	if (mapIt != _nameMap.end()) _nameMap.erase(mapIt);

	IdMap::iterator idIt = _idMap.find(classObj->GetID());
	if (idIt != _idMap.end()) _idMap.erase(idIt);


	return true;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::RegisterInstance(const char *className, void *instance) {
	if (_disabled) return true;

	NameMap::iterator mapIt = _nameMap.find(className);
	if (mapIt == _nameMap.end()) return false;

	CSysInstance *inst = (*mapIt)._value->AddInstance(instance, _count++);
	return (inst != NULL);
}

//////////////////////////////////////////////////////////////////////////
void CSysClassRegistry::AddInstanceToTable(CSysInstance *instance, void *pointer) {
	_instanceMap[pointer] = instance;

	if (instance->GetSavedID() >= 0)
		_savedInstanceMap[instance->GetSavedID()] = instance;
}

//////////////////////////////////////////////////////////////////////////
int CSysClassRegistry::GetNextID() {
	return _count++;
}

//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::UnregisterInstance(const char *className, void *instance) {
	NameMap::iterator mapIt = _nameMap.find(className);
	if (mapIt == _nameMap.end()) return false;
	(*mapIt)._value->RemoveInstance(instance);

	InstanceMap::iterator instIt = _instanceMap.find(instance);
	if (instIt != _instanceMap.end()) {
		_instanceMap.erase(instIt);
		return true;
	} else return false;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::GetPointerID(void *pointer, int *classID, int *instanceID) {
	if (pointer == NULL) return true;

	InstanceMap::iterator it = _instanceMap.find(pointer);
	if (it == _instanceMap.end()) return false;


	CSysInstance *inst = (*it)._value;
	*instanceID = inst->GetID();
	*classID = inst->GetClass()->GetID();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void *CSysClassRegistry::IDToPointer(int classID, int instanceID) {
	SavedInstanceMap::iterator it = _savedInstanceMap.find(instanceID);
	if (it == _savedInstanceMap.end()) return NULL;
	else return (*it)._value->GetInstance();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::SaveTable(CBGame *Game, CBPersistMgr *PersistMgr, bool quickSave) {
	PersistMgr->PutDWORD(_classes.size());

	int counter = 0;

	Classes::iterator it;
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		counter++;

		if (!quickSave) {
			Game->_indicatorProgress = 50.0f / (float)((float)_classes.size() / (float)counter);
			Game->DisplayContent(false);
			Game->_renderer->Flip();
		}

		(it->_value)->SaveTable(Game, PersistMgr);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::LoadTable(CBGame *Game, CBPersistMgr *PersistMgr) {
	Classes::iterator it;

	// reset SavedID of current instances
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		(it->_value)->ResetSavedIDs();
	}

	for (it = _classes.begin(); it != _classes.end(); ++it) {
		if ((it->_value)->IsPersistent()) continue;
		(it->_value)->RemoveAllInstances();
	}

	_instanceMap.clear();


	int numClasses = PersistMgr->GetDWORD();

	for (int i = 0; i < numClasses; i++) {
		Game->_indicatorProgress = 50.0f / (float)((float)numClasses / (float)i);
		Game->DisplayContentSimple();
		Game->_renderer->Flip();

		char *className = PersistMgr->GetString();
		NameMap::iterator mapIt = _nameMap.find(className);
		if (mapIt != _nameMap.end())(*mapIt)._value->LoadTable(Game, PersistMgr);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::SaveInstances(CBGame *Game, CBPersistMgr *PersistMgr, bool quickSave) {

	Classes::iterator it;

	// count total instances
	int numInstances = 0;
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		numInstances += (it->_value)->GetNumInstances();
	}

	PersistMgr->PutDWORD(numInstances);

	int counter = 0;
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		counter++;

		if (!quickSave) {
			if (counter % 20 == 0) {
				Game->_indicatorProgress = 50 + 50.0f / (float)((float)_classes.size() / (float)counter);
				Game->DisplayContent(false);
				Game->_renderer->Flip();
			}
		}
		Game->MiniUpdate();

		(it->_value)->SaveInstances(Game, PersistMgr);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::LoadInstances(CBGame *Game, CBPersistMgr *PersistMgr) {
	// get total instances
	int numInstances = PersistMgr->GetDWORD();

	for (int i = 0; i < numInstances; i++) {
		if (i % 20 == 0) {
			Game->_indicatorProgress = 50 + 50.0f / (float)((float)numInstances / (float)i);
			Game->DisplayContentSimple();
			Game->_renderer->Flip();
		}

		int classID = PersistMgr->GetDWORD();
		int instanceID = PersistMgr->GetDWORD();
		void *instance = IDToPointer(classID, instanceID);


		Classes::iterator it;
		for (it = _classes.begin(); it != _classes.end(); ++it) {
			if ((it->_value)->GetSavedID() == classID) {
				(it->_value)->LoadInstance(instance, PersistMgr);
			}
		}
	}

	_savedInstanceMap.clear();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::EnumInstances(SYS_INSTANCE_CALLBACK lpCallback, const char *className, void *lpData) {
	NameMap::iterator mapIt = _nameMap.find(className);
	if (mapIt == _nameMap.end()) return E_FAIL;

	(*mapIt)._value->InstanceCallback(lpCallback, lpData);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CSysClassRegistry::DumpClasses(Common::WriteStream *stream) {
	Classes::iterator it;
	for (it = _classes.begin(); it != _classes.end(); ++it)
		(it->_value)->Dump(stream);
}

} // end of namespace WinterMute
