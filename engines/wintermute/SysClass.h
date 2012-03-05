/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __WmeSysClass_H__
#define __WmeSysClass_H__

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
