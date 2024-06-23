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
	setGlobal(false);
	setConstant(false);
	setReference(false);
	setTimer(false);
	setString();
	setRandom(false);
	g_VarManager->registerVariable(this);
}

CBagVar::CBagVar(const CBofString &sName, const CBofString &sValue, bool bAddToList) {
	setConstant(false);
	setReference(false);
	setTimer(false);
	setString();
	setName(sName);

	setValue(sValue);
	setRandom(false);
	setGlobal(false);

	if (bAddToList)
		g_VarManager->registerVariable(this);
}

CBagVar::~CBagVar() {
	if (CBagel::getBagApp() &&
		CBagel::getBagApp()->getMasterWnd() &&
		CBagel::getBagApp()->getMasterWnd()->getVariableManager())
		g_VarManager->unRegisterVariable(this);
}

void CBagVar::setValue(const CBofString &s) {
	assert(isValidObject(this));

	if (!_freeze) {
		if (!s.isEmpty()) {
			char c = s[0];
			if (Common::isDigit(c) || c == '-')
				setNumeric();
		}
		_sVarValue = s;
	}
}

const CBofString &CBagVar::getValue() {
	assert(isValidObject(this));

	// WORKAROUND: If you finish the Deven7 flashback without having previously
	// asked him about betting, it hangs him. Work around this by force setting
	// betting to have been discussed
	if (_sVarName == "BETWITHDEVEN") {
		if (g_VarManager->getVariable("DEVENCODE1")->getValue() != "NOTSETYET")
			// Finished flashback, so ensure betting flag is set
			_sVarValue = "1";
	}

	// Check if these items should be replaced by the current sdev
	if (!_sVarName.isEmpty() && !_sVarName.find(CURRSDEV_TOKEN)) {
		if (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()) {
			_sVarValue = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()->getName();
		}
	} else if (!_sVarName.isEmpty() && !_sVarName.find(PREVSDEV_TOKEN)) {
		// Check if these items should be replaced by the previous sdev
		if (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()) {
			_sVarValue = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()->getPrevSDev();
		}
	}

	return _sVarValue;
}

void CBagVar::setBoolValue(bool bVal) {
	assert(isValidObject(this));

	if (bVal)
		_sVarValue = "TRUE";
	else
		_sVarValue = "FALSE";
}

void CBagVar::setValue(int nVal) {
	assert(isValidObject(this));

	if (!_freeze) {
		setNumeric();

		Common::String tmp = Common::String::format("%d", nVal);
		_sVarValue = tmp.c_str();
	}
}

int CBagVar::getNumValue() {
	assert(isValidObject(this));

	if (isRandom())
		return g_engine->getRandomNumber();

	return atoi(_sVarValue);
}

void CBagVar::increment() {
	assert(isValidObject(this));

	if (isNumeric())
		setValue(getNumValue() + 1);
}

ParseCodes CBagVar::setInfo(CBagIfstream &istr) {
	assert(isValidObject(this));

	char szLocalStr[256];
	szLocalStr[0] = 0;
	CBofString sStr(szLocalStr, 256);
	istr.eatWhite();

	getAlphaNumFromStream(istr, sStr);
	setName(sStr);

	istr.eatWhite();

	if (istr.peek() == 'A') {
		char sz2LocalStr[256];
		sz2LocalStr[0] = 0;
		sStr = CBofString(sz2LocalStr, 256);
		getAlphaNumFromStream(istr, sStr);

		if (!sStr.find("AS")) {
			istr.eatWhite();
			getAlphaNumFromStream(istr, sStr);
			if (!sStr.find("TIMER")) {
				setTimer();
				g_VarManager->updateRegistration();
			} else if (!sStr.find("RANDOM")) {
				setRandom(true);
				g_VarManager->updateRegistration();
			} else if (!sStr.find("GLOBAL")) {
				setGlobal(true);
				g_VarManager->updateRegistration();
			} else {
				putbackStringOnStream(istr, sStr);
				putbackStringOnStream(istr, "AS ");
			}
		} else {
			putbackStringOnStream(istr, sStr);
		}
	}

	istr.eatWhite();

	if (istr.peek() == '=') {
		istr.getCh();
		istr.eatWhite();
		getAlphaNumFromStream(istr, sStr);
		setValue(sStr);
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
		releaseVariables();
		_xVarList.removeAll();

		for (int i = 0; i < VAR_HASH_TABLE_SIZE; i++) {
			_xVarHashList[i].removeAll();
		}
	}
}

static int HASHVAR(const char *p, int l) {
	int h = 0;
	for (int j = 0; j < l; j++) {
		h += p[j];
	}
	h %= VAR_HASH_TABLE_SIZE;

	return h;
}
ErrorCode CBagVarManager::registerVariable(CBagVar *pVar) {
	_xVarList.addToTail(pVar);

	return ERR_NONE;
}

// Arranges the list so that timer variables are in the front
ErrorCode CBagVarManager::updateRegistration() {
	bool bFoundLastTimer = false;
	int i;

	// Read the timers at the beginning
	for (i = 0; i < _xVarList.getCount() && !bFoundLastTimer; ++i) {
		if (!_xVarList[i]->isTimer()) {
			bFoundLastTimer = true;
		}
	}

	// Make sure there are no more timers in the list
	if (bFoundLastTimer) {
		for (/*- i determined in previous for loop -*/ ; i < _xVarList.getCount(); ++i) {
			if (_xVarList[i]->isTimer()) {
				CBagVar *pVar = _xVarList[i];
				_xVarList.remove(i);
				_xVarList.addToHead(pVar);
			}
		}
	}

	return ERR_UNKNOWN;
}

ErrorCode CBagVarManager::unRegisterVariable(CBagVar *pVar) {
	// Find and remove specified variable from the Var manager list
	//

	CBofListNode<CBagVar *> *pList = _xVarList.getTail();
	while (pList != nullptr) {

		if (pList->getNodeItem() == pVar) {
			_xVarList.remove(pList);
			break;
		}

		pList = pList->_pPrev;
	}

	// Remove it from the hash table also.
	char szLocalBuff[256];
	CBofString varStr(szLocalBuff, 256);
	varStr = pVar->getName();

	// Hash it
	int nHashVal = HASHVAR(szLocalBuff, varStr.getLength());
	CBofList<CBagVar *> *pVarList = &_xVarHashList[nHashVal];

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
ErrorCode CBagVarManager::incrementTimers() {
	volatile bool bFoundLastTimer = false;

	// Read the timers at the beginning
	for (int i = 0; i < _xVarList.getCount() && !bFoundLastTimer; ++i) {
		CBagVar *pVar = _xVarList[i];
		if (pVar->isTimer()) {

			// Hack to keep the game time from exceeding 22:50
			if (pVar->getName().compareNoCase("TURNCOUNT") == 0) {
				if (pVar->getNumValue() == 2250) {
					continue;
				}
			}
			pVar->increment();
		}
	}

	// Separate turn world out of event world and only execute when we
	// increment the timers.
	CBagEventSDev::setEvalTurnEvents(true);

	return ERR_NONE;
}

ErrorCode CBagVarManager::releaseVariables(bool bIncludeGlobals) {

	if (bIncludeGlobals) {
		while (_xVarList.getCount()) {
			CBagVar *pVar = _xVarList.removeHead();
			delete pVar;
		}
	} else { // Do not include globals
		for (int i = _xVarList.getCount() - 1; i >= 0; i--) {
			CBagVar *pVar = _xVarList[i];
			if (pVar && !pVar->isGlobal()) {
				_xVarList.remove(i);
				delete pVar;
			}
		}
	}
	return ERR_NONE;
}

CBagVar *CBagVarManager::getVariable(const CBofString &sName) {
	// Use the hash table to find the variable.
	char szLocalBuff[256];
	CBofString varStr(szLocalBuff, 256);
	varStr = sName;

	int nHashVal = HASHVAR(szLocalBuff, varStr.getLength());

	CBofList<CBagVar *> *pVarList = &_xVarHashList[nHashVal];
	for (int i = 0; i < pVarList->getCount(); ++i) {
		CBagVar *pVar = pVarList->getNodeItem(i);
		if (pVar != nullptr && (pVar->getName().getLength() == sName.getLength()) && !pVar->getName().find(sName)) {
			return pVar;
		}
	}

	return nullptr;
}

void CBagVar::setName(const CBofString &s) {
	_sVarName = s;

	CBagel *pApp = CBagel::getBagApp();
	if (pApp) {
		CBagMasterWin *pMainWin = pApp->getMasterWnd();

		if (!s.isEmpty() && pMainWin && pMainWin->getVariableManager()) {
			char szLocalBuff[256];
			CBofString varStr(szLocalBuff, 256);
			varStr = _sVarName;
			int nHashVal = HASHVAR(szLocalBuff, varStr.getLength());
			g_VarManager->_xVarHashList[nHashVal].addToTail(this);
		}
	}
}

} // namespace Bagel
