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

#include "BGame.h"
#include "PlatformSDL.h"
#include "SysInstance.h"
#include "SysClassRegistry.h"

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
	classObj->SetID(m_Count++);
	//m_Classes.insert(classObj);
	m_Classes[classObj] = classObj;

	m_NameMap[classObj->GetName()] = classObj;
	m_IdMap[classObj->GetID()] = classObj;

	return true;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::UnregisterClass(CSysClass *classObj) {

	Classes::iterator it = m_Classes.find(classObj);
	if (it == m_Classes.end()) return false;

	if (classObj->GetNumInstances() != 0) {
		char str[MAX_PATH];
		sprintf(str, "Memory leak@class %-20s: %d instance(s) left\n", classObj->GetName().c_str(), classObj->GetNumInstances());
		CBPlatform::OutputDebugString(str);
	}
	m_Classes.erase(it);

	NameMap::iterator mapIt = m_NameMap.find(classObj->GetName());
	if (mapIt != m_NameMap.end()) m_NameMap.erase(mapIt);

	IdMap::iterator idIt = m_IdMap.find(classObj->GetID());
	if (idIt != m_IdMap.end()) m_IdMap.erase(idIt);


	return true;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::RegisterInstance(const char *className, void *instance) {
	if (m_Disabled) return true;

	NameMap::iterator mapIt = m_NameMap.find(className);
	if (mapIt == m_NameMap.end()) return false;

	CSysInstance *inst = (*mapIt)._value->AddInstance(instance, m_Count++);
	return (inst != NULL);
}

//////////////////////////////////////////////////////////////////////////
void CSysClassRegistry::AddInstanceToTable(CSysInstance *instance, void *pointer) {
	m_InstanceMap[pointer] = instance;

	if (instance->GetSavedID() >= 0)
		m_SavedInstanceMap[instance->GetSavedID()] = instance;
}

//////////////////////////////////////////////////////////////////////////
int CSysClassRegistry::GetNextID() {
	return m_Count++;
}

//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::UnregisterInstance(const char *className, void *instance) {
	NameMap::iterator mapIt = m_NameMap.find(className);
	if (mapIt == m_NameMap.end()) return false;
	(*mapIt)._value->RemoveInstance(instance);

	InstanceMap::iterator instIt = m_InstanceMap.find(instance);
	if (instIt != m_InstanceMap.end()) {
		m_InstanceMap.erase(instIt);
		return true;
	} else return false;
}


//////////////////////////////////////////////////////////////////////////
bool CSysClassRegistry::GetPointerID(void *pointer, int *classID, int *instanceID) {
	if (pointer == NULL) return true;

	InstanceMap::iterator it = m_InstanceMap.find(pointer);
	if (it == m_InstanceMap.end()) return false;


	CSysInstance *inst = (*it)._value;
	*instanceID = inst->GetID();
	*classID = inst->GetClass()->GetID();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void *CSysClassRegistry::IDToPointer(int classID, int instanceID) {
	SavedInstanceMap::iterator it = m_SavedInstanceMap.find(instanceID);
	if (it == m_SavedInstanceMap.end()) return NULL;
	else return (*it)._value->GetInstance();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::SaveTable(CBGame *Game, CBPersistMgr *PersistMgr, bool quickSave) {
	PersistMgr->PutDWORD(m_Classes.size());

	int counter = 0;

	Classes::iterator it;
	for (it = m_Classes.begin(); it != m_Classes.end(); ++it) {
		counter++;

		if (!quickSave) {
			Game->m_IndicatorProgress = 50.0f / (float)((float)m_Classes.size() / (float)counter);
			Game->DisplayContent(false);
			Game->m_Renderer->Flip();
		}

		(it->_value)->SaveTable(Game, PersistMgr);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::LoadTable(CBGame *Game, CBPersistMgr *PersistMgr) {
	Classes::iterator it;

	// reset SavedID of current instances
	for (it = m_Classes.begin(); it != m_Classes.end(); ++it) {
		(it->_value)->ResetSavedIDs();
	}

	for (it = m_Classes.begin(); it != m_Classes.end(); ++it) {
		if ((it->_value)->IsPersistent()) continue;
		(it->_value)->RemoveAllInstances();
	}

	m_InstanceMap.clear();


	int numClasses = PersistMgr->GetDWORD();

	for (int i = 0; i < numClasses; i++) {
		Game->m_IndicatorProgress = 50.0f / (float)((float)numClasses / (float)i);
		Game->DisplayContentSimple();
		Game->m_Renderer->Flip();

		char *className = PersistMgr->GetString();
		NameMap::iterator mapIt = m_NameMap.find(className);
		if (mapIt != m_NameMap.end())(*mapIt)._value->LoadTable(Game, PersistMgr);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::SaveInstances(CBGame *Game, CBPersistMgr *PersistMgr, bool quickSave) {

	Classes::iterator it;

	// count total instances
	int numInstances = 0;
	for (it = m_Classes.begin(); it != m_Classes.end(); ++it) {
		numInstances += (it->_value)->GetNumInstances();
	}

	PersistMgr->PutDWORD(numInstances);

	int counter = 0;
	for (it = m_Classes.begin(); it != m_Classes.end(); ++it) {
		counter++;

		if (!quickSave) {
			if (counter % 20 == 0) {
				Game->m_IndicatorProgress = 50 + 50.0f / (float)((float)m_Classes.size() / (float)counter);
				Game->DisplayContent(false);
				Game->m_Renderer->Flip();
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
			Game->m_IndicatorProgress = 50 + 50.0f / (float)((float)numInstances / (float)i);
			Game->DisplayContentSimple();
			Game->m_Renderer->Flip();
		}

		int classID = PersistMgr->GetDWORD();
		int instanceID = PersistMgr->GetDWORD();
		void *instance = IDToPointer(classID, instanceID);


		Classes::iterator it;
		for (it = m_Classes.begin(); it != m_Classes.end(); ++it) {
			if ((it->_value)->GetSavedID() == classID) {
				(it->_value)->LoadInstance(instance, PersistMgr);
			}
		}
	}

	m_SavedInstanceMap.clear();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSysClassRegistry::EnumInstances(SYS_INSTANCE_CALLBACK lpCallback, const char *className, void *lpData) {
	NameMap::iterator mapIt = m_NameMap.find(className);
	if (mapIt == m_NameMap.end()) return E_FAIL;

	(*mapIt)._value->InstanceCallback(lpCallback, lpData);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CSysClassRegistry::DumpClasses(void *stream) {
	Classes::iterator it;
	for (it = m_Classes.begin(); it != m_Classes.end(); ++it)
		(it->_value)->Dump(stream);
}

} // end of namespace WinterMute
