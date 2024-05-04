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
	setTimer(false);
	SetString();
	SetRandom(false);
	VAR_MANAGER->RegisterVariable(this);
}

CBagVar::CBagVar(const CBofString &sName, const CBofString &sValue, bool bAddToList) {
	SetConstant(false);
	SetReference(false);
	setTimer(false);
	SetString();
	SetName(sName);

	SetValue(sValue);
	SetRandom(false);
	SetGlobal(false);

	if (bAddToList)
		VAR_MANAGER->RegisterVariable(this);
}

CBagVar::~CBagVar() {
	if (CBagel::getBagApp() &&
		CBagel::getBagApp()->getMasterWnd() &&
		CBagel::getBagApp()->getMasterWnd()->getVariableManager())
		VAR_MANAGER->UnRegisterVariable(this);
}

void CBagVar::SetValue(const CBofString &s) {
	assert(isValidObject(this));

	if (!s.isEmpty()) {
		char c = s[0];
		if (Common::isDigit(c) || c == '-')
			SetNumeric();
	}
	m_sVarValue = s;
}

const CBofString &CBagVar::GetValue() {
	assert(isValidObject(this));

	// WORKAROUND: If you finish the Deven7 flashback without having previously
	// asked him about betting, it hangs him. Work around this by force setting
	// betting to have been discussed
	if (m_sVarName == "BETWITHDEVEN") {
		if (VAR_MANAGER->GetVariable("DEVENCODE1")->GetValue() != "NOTSETYET")
			// Finished flashback, so ensure betting flag is set
			m_sVarValue = "1";
	}

	// Check if these items should be replaced by the current sdev
	if (!m_sVarName.isEmpty() && !m_sVarName.Find(CURRSDEV_TOKEN)) {
		CBofString CurrSDev;
		if (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()) {
			m_sVarValue = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()->GetName();
		}
	} else {

		// Check if these items should be replaced by the previous sdev
		if (!m_sVarName.isEmpty() && !m_sVarName.Find(PREVSDEV_TOKEN)) {
			CBofString CurrSDev;
			if (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()) {
				m_sVarValue = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()->GetPrevSDev();
			}
		}
	}

	return m_sVarValue;
}

void CBagVar::SetBoolValue(bool bVal) {
	assert(isValidObject(this));

	if (bVal)
		m_sVarValue = "TRUE";
	else
		m_sVarValue = "FALSE";
}

void CBagVar::SetValue(int nVal) {
	assert(isValidObject(this));

	SetNumeric();

	Common::String tmp = Common::String::format("%d", nVal);
	m_sVarValue = tmp.c_str();
}

int CBagVar::GetNumValue() {
	assert(isValidObject(this));

	if (IsRandom())
		return g_engine->getRandomNumber();

	return atoi(m_sVarValue);
}

void CBagVar::Increment() {
	assert(isValidObject(this));

	if (IsNumeric())
		SetValue(GetNumValue() + 1);
}

PARSE_CODES CBagVar::setInfo(CBagIfstream &istr) {
	assert(isValidObject(this));

	char szLocalStr[256];
	szLocalStr[0] = 0;
	CBofString sStr(szLocalStr, 256);
	istr.eatWhite();

	GetAlphaNumFromStream(istr, sStr);
	SetName(sStr);

	istr.eatWhite();

	if (istr.peek() == 'A') {
		char sz2LocalStr[256];
		sz2LocalStr[0] = 0;
		sStr = CBofString(sz2LocalStr, 256);
		GetAlphaNumFromStream(istr, sStr);

		if (!sStr.Find("AS")) {
			istr.eatWhite();
			GetAlphaNumFromStream(istr, sStr);
			if (!sStr.Find("TIMER")) {
				setTimer();
				VAR_MANAGER->UpdateRegistration();
			} else if (!sStr.Find("RANDOM")) {
				SetRandom(true);
				VAR_MANAGER->UpdateRegistration();
			} else if (!sStr.Find("GLOBAL")) {
				SetGlobal(true);
				VAR_MANAGER->UpdateRegistration();
			} else {
				PutbackStringOnStream(istr, sStr);
				PutbackStringOnStream(istr, "AS ");
			}
		} else {
			PutbackStringOnStream(istr, sStr);
		}
	}

	istr.eatWhite();

	if (istr.peek() == '=') {
		istr.getCh();
		istr.eatWhite();
		GetAlphaNumFromStream(istr, sStr);
		SetValue(sStr);
	}

	istr.eatWhite();

	return PARSING_DONE;
}

CBagVarManager::CBagVarManager() {
	++nVarMngrs;
}

CBagVarManager::~CBagVarManager() {
	if (nVarMngrs) {
		nVarMngrs--;
		ReleaseVariables();
		m_xVarList.removeAll();

		for (int i = 0; i < VAR_HTABLE_SIZE; i++) {
			m_xVarHashList[i].removeAll();
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
	m_xVarList.addToTail(pVar);

	return ERR_NONE;
}

// Arranges the list so that timer variables are in the front
ErrorCode CBagVarManager::UpdateRegistration() {
	bool bFoundLastTimer = false;
	int i;

	// Read the timers at the beginning
	for (i = 0; i < m_xVarList.getCount() && !bFoundLastTimer; ++i) {
		if (!m_xVarList[i]->IsTimer()) {
			bFoundLastTimer = true;
		}
	}

	// Make sure there are no more timers in the list
	if (bFoundLastTimer) {
		for (/*- i determined in previous for loop -*/ ; i < m_xVarList.getCount(); ++i) {
			if (m_xVarList[i]->IsTimer()) {
				CBagVar *pVar = m_xVarList[i];
				m_xVarList.remove(i);
				m_xVarList.addToHead(pVar);
			}
		}
	}

	return ERR_UNKNOWN;
}

ErrorCode CBagVarManager::UnRegisterVariable(CBagVar *pVar) {
	// Find and remove specified variable from the Var manager list
	//

	CBofListNode<CBagVar *> *pList = m_xVarList.getTail();
	while (pList != nullptr) {

		if (pList->getNodeItem() == pVar) {
			m_xVarList.remove(pList);
			break;
		}

		pList = pList->_pPrev;
	}

	// Remove it from the hash table also.
	char szLocalBuff[256];
	CBofString varStr(szLocalBuff, 256);
	varStr = pVar->GetName();

	// Hash it
	int nHashVal = HASHVAR(szLocalBuff, varStr.getLength());
	CBofList<CBagVar *> *pVarList = &m_xVarHashList[nHashVal];

	// Search the hash table and remove it when we're done.
	for (int i = 0; i < pVarList->getCount(); ++i) {
		CBagVar *pHashVar = pVarList->getNodeItem(i);
		if (pVar == pHashVar) {
			pVarList->remove(i);
			break;
		}
	}

	return ERR_NONE;
}

// The timers must be at the beginning of the list
ErrorCode CBagVarManager::IncrementTimers() {
	volatile bool bFoundLastTimer = false;

	// Read the timers at the beginning
	for (int i = 0; i < m_xVarList.getCount() && !bFoundLastTimer; ++i) {
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
	CBagEventSDev::setEvalTurnEvents(true);

	return ERR_NONE;
}

ErrorCode CBagVarManager::ReleaseVariables(bool bIncludeGlobals) {

	if (bIncludeGlobals) {
		while (m_xVarList.getCount()) {
			CBagVar *pVar = m_xVarList.removeHead();

			if (pVar) {
				delete pVar;
			}
		}
	} else { // Do not include globals
		for (int i = m_xVarList.getCount() - 1; i >= 0; i--) {
			CBagVar *pVar = m_xVarList[i];
			if (pVar && !pVar->IsGlobal()) {
				m_xVarList.remove(i);
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

	int nHashVal = HASHVAR(szLocalBuff, varStr.getLength());

	CBofList<CBagVar *> *pVarList = &m_xVarHashList[nHashVal];
	for (int i = 0; i < pVarList->getCount(); ++i) {
		pVar = pVarList->getNodeItem(i);
		if (pVar != nullptr && (pVar->GetName().getLength() == sName.getLength()) && !pVar->GetName().Find(sName)) {
			return pVar;
		}
	}

	return nullptr;
}

void CBagVar::SetName(const CBofString &s) {
	m_sVarName = s;

	CBagel *pApp = CBagel::getBagApp();
	if (pApp) {
		CBagMasterWin *pMainWin = pApp->getMasterWnd();

		if (!s.isEmpty() && pMainWin && pMainWin->getVariableManager()) {
			char szLocalBuff[256];
			CBofString varStr(szLocalBuff, 256);
			varStr = m_sVarName;
			int nHashVal = HASHVAR(szLocalBuff, varStr.getLength());
			VAR_MANAGER->m_xVarHashList[nHashVal].addToTail(this);
		}
	}
}

} // namespace Bagel
