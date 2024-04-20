/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bagel/baglib/var.h"
#include "bagel/baglib/event_sdev.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/bagel.h"

#if BOF_MAC
#include <ctype.h> // need ctype to get numeric macros
#include <mac.h>   // need ctype to get itoa macros
#endif

namespace Bagel {

static int HASHVAR(const char *p, int l);

int CBagVarManager::nVarMngrs;

void CBagVarManager::initialize() {
	nVarMngrs = 0;
}

CBagVar::CBagVar() {
	SetGlobal(false);
	SetConstant(false);
	SetReference(false);
	SetTimer(false);
	SetString();
	SetRandom(false);
	VARMNGR->RegisterVariable(this);
}

CBagVar::CBagVar(CBagVar &xVar) {
	SetName(xVar.GetName());
	SetValue(xVar.GetValue());
	m_xVarType = xVar.GetType();
	SetConstant(xVar.IsConstant());
	SetReference(xVar.IsReference());
	SetTimer(xVar.IsTimer());
	SetRandom(false);
	SetGlobal(false);
}

CBagVar::CBagVar(const CBofString &sName, const CBofString &sValue, bool bAddToList) {
	SetConstant(false);
	SetReference(false);
	SetTimer(false);
	SetString();
	SetName(sName);

	SetValue(sValue);
	SetRandom(false);
	SetGlobal(false);

	if (bAddToList)
		VARMNGR->RegisterVariable(this);
}

CBagVar::~CBagVar() {
	if (CBagel::GetBagApp() &&
	        CBagel::GetBagApp()->GetMasterWnd() &&
	        CBagel::GetBagApp()->GetMasterWnd()->GetVariableManager())
		VARMNGR->UnRegisterVariable(this);
}

void CBagVar::SetValue(const CBofString &s) {
	Assert(IsValidObject(this));

	if (!s.IsEmpty()) {
		char c = s[0];
		if (Common::isDigit(c) || c == '-')
			SetNumeric();
	}
	m_sVarValue = s;
}

const CBofString &CBagVar::GetValue() {
	Assert(IsValidObject(this));

	// WORKAROUND: If you finish the Deven7 flashback without having previously
	// asked him about betting, it hangs him. Work around this by force setting
	// betting to have been discussed
	if (m_sVarName == "BETWITHDEVEN") {
		if (VARMNGR->GetVariable("DEVENCODE1")->GetValue() != "NOTSETYET")
			// Finished flashback, so ensure betting flag is set
			m_sVarValue = "1";
	}

	// Check if these items should be replaced by the current sdev
	if (!m_sVarName.IsEmpty() && !m_sVarName.Find(CURRSDEV_TOKEN)) {
		CBofString CurrSDev;
		if (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()) {
			m_sVarValue = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()->GetName();
		}
	} else {

		// Check if these items should be replaced by the previous sdev
		if (!m_sVarName.IsEmpty() && !m_sVarName.Find(PREVSDEV_TOKEN)) {
			CBofString CurrSDev;
			if (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()) {
				m_sVarValue = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()->GetPrevSDev();
			}
		}
	}

	return m_sVarValue;
}

void CBagVar::SetBoolValue(bool bVal) {
	Assert(IsValidObject(this));

	if (bVal)
		m_sVarValue = "TRUE";
	else
		m_sVarValue = "FALSE";
}

void CBagVar::SetValue(int nVal) {
	Assert(IsValidObject(this));

	SetNumeric();

	Common::String tmp = Common::String::format("%d", nVal);
	m_sVarValue = tmp.c_str();
}

int CBagVar::GetNumValue() {
	Assert(IsValidObject(this));

	if (IsRandom())
		return g_engine->getRandomNumber();

	return atoi(m_sVarValue);
}

void CBagVar::Increment() {
	Assert(IsValidObject(this));

	if (IsNumeric())
		SetValue(GetNumValue() + 1);
}

PARSE_CODES CBagVar::SetInfo(bof_ifstream &istr) {
	Assert(IsValidObject(this));

	char szLocalStr[256];
	szLocalStr[0] = 0;
	CBofString sStr(szLocalStr, 256);
	istr.EatWhite();

	GetAlphaNumFromStream(istr, sStr);
	SetName(sStr);

	istr.EatWhite();

	if (istr.peek() == 'A') {
		char sz2LocalStr[256];
		sz2LocalStr[0] = 0;
		sStr = CBofString(sz2LocalStr, 256);
		GetAlphaNumFromStream(istr, sStr);

		if (!sStr.Find("AS")) {
			istr.EatWhite();
			GetAlphaNumFromStream(istr, sStr);
			if (!sStr.Find("TIMER")) {
				SetTimer();
				VARMNGR->UpdateRegistration();
			} else if (!sStr.Find("RANDOM")) {
				SetRandom(true);
				VARMNGR->UpdateRegistration();
			} else if (!sStr.Find("GLOBAL")) {
				SetGlobal(true);
				VARMNGR->UpdateRegistration();
			} else {
				PutbackStringOnStream(istr, sStr);
				PutbackStringOnStream(istr, "AS ");
			}
		} else {
			PutbackStringOnStream(istr, sStr);
		}
	}

	istr.EatWhite();

	if (istr.peek() == '=') {
		istr.Get();
		istr.EatWhite();
		GetAlphaNumFromStream(istr, sStr);
		SetValue(sStr);
	}

	istr.EatWhite();

	return PARSING_DONE;
}

CBagVarManager::CBagVarManager() {
	++nVarMngrs;
}

CBagVarManager::~CBagVarManager() {
	if (nVarMngrs) {
		nVarMngrs--;
		ReleaseVariables();
		m_xVarList.RemoveAll();

		for (int i = 0; i < VAR_HTABLE_SIZE; i++) {
			m_xVarHashList[i].RemoveAll();
		}
	}
}

static int HASHVAR(const char *p, int l) {
	int h = 0;
	for (int j = 0; j < l; j++) {
		h += p[j];
	}
	h %= VAR_HTABLE_SIZE;

	return h;
}
ErrorCode CBagVarManager::RegisterVariable(CBagVar *pVar) {
	m_xVarList.AddToTail(pVar);

	return ERR_NONE;
}

// Arranges the list so that timer variables are in the front
ErrorCode CBagVarManager::UpdateRegistration() {
	bool bFoundLastTimer = false;
	int i;

	// Read the timers at the beginning
	for (i = 0; i < m_xVarList.GetCount() && !bFoundLastTimer; ++i) {
		if (!m_xVarList[i]->IsTimer()) {
			bFoundLastTimer = true;
		}
	}

	// Make sure there are no more timers in the list
	if (bFoundLastTimer) {
		for (/*- i determined in previous for loop -*/ ; i < m_xVarList.GetCount(); ++i) {
			if (m_xVarList[i]->IsTimer()) {
				CBagVar *pVar = m_xVarList[i];
				m_xVarList.Remove(i);
				m_xVarList.AddToHead(pVar);
			}
		}
	}

	return ERR_UNKNOWN;
}

ErrorCode CBagVarManager::UnRegisterVariable(CBagVar *pVar) {
	// Find and remove specified variable from the Var manager list
	//

	CBofListNode<CBagVar *> *pList = m_xVarList.GetTail();
	while (pList != nullptr) {

		if (pList->GetNodeItem() == pVar) {
			m_xVarList.Remove(pList);
			break;
		}

		pList = pList->m_pPrev;
	}

	// Remove it from the hash table also.
	char szLocalBuff[256];
	CBofString varStr(szLocalBuff, 256);
	varStr = pVar->GetName();

	// Hash it
	int nHashVal = HASHVAR(szLocalBuff, varStr.GetLength());
	CBofList<CBagVar *> *pVarList = &m_xVarHashList[nHashVal];

	// Search the hash table and remove it when we're done.
	for (int i = 0; i < pVarList->GetCount(); ++i) {
		CBagVar *pHashVar = pVarList->GetNodeItem(i);
		if (pVar == pHashVar) {
			pVarList->Remove(i);
			break;
		}
	}

	return ERR_NONE;
}

// The timers must be at the beginning of the list
ErrorCode CBagVarManager::IncrementTimers() {
	volatile bool bFoundLastTimer = false;

	// Read the timers at the beginning
	for (int i = 0; i < m_xVarList.GetCount() && !bFoundLastTimer; ++i) {
		CBagVar *pVar = m_xVarList[i];
		if (pVar->IsTimer()) {

			// Hack to keep the game time from exceeding 22:50
			if (pVar->GetName().CompareNoCase("TURNCOUNT") == 0) {
				if (pVar->GetNumValue() == 2250) {
					continue;
				}
			}
			pVar->Increment();
		}
	}

	// Separate turn world out of event world and only execute when we
	// increment the timers.
	CBagEventSDev::SetEvalTurnEvents(true);

	return ERR_NONE;
}

ErrorCode CBagVarManager::ReleaseVariables(bool bIncludeGlobals) {

	if (bIncludeGlobals) {
		while (m_xVarList.GetCount()) {
			CBagVar *pVar = m_xVarList.RemoveHead();

			if (pVar) {
				delete pVar;
			}
		}
	} else { // Do not include globals
		for (int i = m_xVarList.GetCount() - 1; i >= 0; i--) {
			CBagVar *pVar = m_xVarList[i];
			if (pVar && !pVar->IsGlobal()) {
				m_xVarList.Remove(i);
				delete pVar;
			}
		}
	}
	return ERR_NONE;
}

CBagVar *CBagVarManager::GetVariable(const CBofString &sName) {
	CBagVar *pVar = nullptr;

	// Use the hash table to find the variable.
	char szLocalBuff[256];
	CBofString varStr(szLocalBuff, 256);
	varStr = sName;

	int nHashVal = HASHVAR(szLocalBuff, varStr.GetLength());

	CBofList<CBagVar *> *pVarList = &m_xVarHashList[nHashVal];
	for (int i = 0; i < pVarList->GetCount(); ++i) {
		pVar = pVarList->GetNodeItem(i);
		if (pVar != nullptr && (pVar->GetName().GetLength() == sName.GetLength()) && !pVar->GetName().Find(sName)) {
			return pVar;
		}
	}

	return nullptr;
}

void CBagVar::SetName(const CBofString &s) {
	m_sVarName = s;

	CBagel *pApp = CBagel::GetBagApp();
	if (pApp) {
		CBagMasterWin *pMainWin = pApp->GetMasterWnd();

		if (!s.IsEmpty() && pMainWin && pMainWin->GetVariableManager()) {
			char szLocalBuff[256];
			CBofString varStr(szLocalBuff, 256);
			varStr = m_sVarName;
			int nHashVal = HASHVAR(szLocalBuff, varStr.GetLength());
			VARMNGR->m_xVarHashList[nHashVal].AddToTail(this);
		}
	}
}

} // namespace Bagel
