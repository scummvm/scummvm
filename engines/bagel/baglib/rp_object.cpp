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

#include "bagel/baglib/rp_object.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/base_pda.h"
#include "bagel/baglib/log_msg.h"
#include "bagel/baglib/pda.h"
#include "bagel/baglib/zoom_pda.h"
#include "bagel/boflib/log.h"

namespace Bagel {

// constants for save/restore
#define NUM_RP_FIELDS 9
#define mRPCurDossierShift 28

#define mRPReported 0x00010000
#define mResPrinted 0x00020000
#define mTouched 0x00040000
#define mRPRead 0x00080000
#define mRPMoviePlayed 0x00100000
#define mRPTDos1Eval 0x00200000
#define mRPTDos2Eval 0x00400000
#define mRPTDos3Eval 0x00800000
#define mRPUDos1Eval 0x01000000
#define mRPUDos2Eval 0x02000000
#define mRPUDos3Eval 0x04000000
#define mRPCurVisible 0x08000000
#define mRPTimeSet 0x10000000
#define mRPCurDossier 0x60000000
#define mRPTime 0x0000FFFF

// Initialize statics
CBofList<CBagRPObject *> *CBagRPObject::_pRPList = nullptr;
CBagVar *CBagRPObject::_turnCount = nullptr;
CBagVar *CBagRPObject::_pLogStateVar = nullptr;
CBagVar *CBagRPObject::_pPrevLogStateVar = nullptr;
CBagVar *CBagRPObject::_pBarLogPages = nullptr;
CBagVar *CBagRPObject::_pPrevBarPage = nullptr;
CBagVar *CBagRPObject::_pCurBarPage = nullptr;
int32 CBagRPObject::_nLastRPQCheck = 0;
CBagRPObject *CBagRPObject::_pCurRPObject = nullptr;
RPStates CBagRPObject::_eRPMode = RP_NO_MODE;
CBagRPObject *CBagRPObject::_pActivateThisGuy = nullptr;

// Local globals
//
// CBagRPObject -
//  CBagRPObject is an object that can be accessed through script to handle residue
//  printing.
//
CBagRPObject::CBagRPObject() {
	_xObjType = RESPRNT_OBJ;

	// Dossier list
	_pTouchedList = nullptr;
	_pUntouchedList = nullptr;

	// Touched var, not required.
	_pTouchedVar = nullptr;

	_pDescObj = nullptr;

	_nRPTime = 0;
	_bRPReported = false;
	_bResPrinted = false;
	_bTouched = false;
	_bRPRead = false;
	_bRPTimeSet = false;
	_bCurVisible = false;
	_bInitialized = false;
	_bMoviePlayed = false;

	// Have to save the original rectangle since we'll be messing with it later
	// on.
	_cOrigRect.setRect(0, 0, 0, 0);
	_bOrigRectInit = false;

	// the object's name, we pull this out of the wld sdev.
	_pObjectName = nullptr;

	// Initialize movie land
	_sMovieObj = "";
	_pMovieObj = nullptr;

	// Save variable
	_pSaveVar = nullptr;

	// Current dossier
	_nCurDossier = 0;

	_pVarObj = nullptr;
}

CBagRPObject::~CBagRPObject() {
	// If still attached, then detach
	if (CBagParseObject::isAttached()) {
		CBagRPObject::detach();
	}

	// Explicitly delete everything in the list
	bool bSame = (_pTouchedList == _pUntouchedList);
	DossierObj *pDObj;

	// Trash the expression list
	if (_pTouchedList != nullptr) {
		int nCount = _pTouchedList->getCount();
		for (int i = 0; i < nCount; i++) {
			pDObj = _pTouchedList->remove(0);
			if (pDObj) {
				delete pDObj;
			}
		}

		delete _pTouchedList;
		_pTouchedList = nullptr;
	}

	//  Explicitly delete everything in the list
	//  Only trash them if they are not the same list.
	if ((bSame == false) && (_pUntouchedList != nullptr)) {
		int nCount = _pUntouchedList->getCount();
		for (int i = 0; i < nCount; i++) {
			pDObj = _pUntouchedList->remove(0);
			if (pDObj) {
				delete pDObj;
			}
		}
		delete _pUntouchedList;
		_pUntouchedList = nullptr;
	}

	// Delete the description object
	delete _pDescObj;
	_pDescObj = nullptr;

	// We got these vars from the var manager, so just null it out, don't delete it!
	_pVarObj = nullptr;
	_turnCount = nullptr;
	_pTouchedVar = nullptr;
	_pMovieObj = nullptr;
	_pSaveVar = nullptr;

	// Clear our statics, yes, I mean to do that here.
	_pLogStateVar = nullptr;
	_pPrevLogStateVar = nullptr;
	_pBarLogPages = nullptr;
	_pPrevBarPage = nullptr;
	_pCurBarPage = nullptr;
}

ParseCodes CBagRPObject::setInfo(CBagIfstream &istr) {
	bool nObjectUpdated = false;
	char szLocalStr[256];
	CBofString sStr(szLocalStr, 256);

	while (!istr.eof()) {
		istr.eatWhite();

		char ch = (char)istr.peek();
		switch (ch) {
		//
		// This will be the list of results that are to be used if the
		// object is found to have been touched.
		//
		// Touched var can also be in this stream.
		//
		case 'U':
		case 'T': {
			getAlphaNumFromStream(istr, sStr);
			if (sStr.find("TOUCHEDVAR") == 0) {
				getAlphaNumFromStream(istr, sStr);

				nObjectUpdated = true;
				istr.eatWhite();

				// Get the variable name from the definition line, then find it
				// in the global list.
				_pTouchedVar = g_VarManager->getVariable(sStr);

				break;
			}

			bool bTouched = (ch == 'T');

			if ((bTouched && !sStr.find("TOUCHED")) || (bTouched == false && !sStr.find("UNTOUCHED"))) {

				nObjectUpdated = true;

				bool bContinue;
				do {
					CBagExpression *px = nullptr;

					getAlphaNumFromStream(istr, sStr);
					bContinue = false;

					// If the next non-whitespace char is a paren, then we're going
					// to have an expression.
					istr.eatWhite();
					if ((char)istr.peek() == '(') {
						px = new CBagExpression();
						px->setInfo(istr);
					}

					// add this to the correct list.
					if (bTouched) {
						setTouchedDos(sStr, px);
					} else {
						setUntouchedDos(sStr, px);
					}

					if ((char)istr.peek() == ',') {
						istr.getCh();
						istr.eatWhite();
						bContinue = true;
					}
				} while (bContinue);
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		//
		// We can hit a continuation character, a plus sign tells us that there
		// is more on the next line.
		case '+': {
			// Skip over this character
			istr.getCh();
			istr.eatWhite();

			// Now keep going until we get our next alpha num.
			while (!Common::isAlnum((char)istr.peek())) {
				istr.getCh();
			}
		}
		break;

		case 'N': {
			getAlphaNumFromStream(istr, sStr);
			if (!sStr.find("NAME")) {
				nObjectUpdated = true;

				istr.eatWhite();
				getAlphaNumFromStream(istr, sStr);

				_sObjectName = sStr;

			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		// Same and Save can be in the stream.
		case 'S': {
			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("SAVE")) {
				nObjectUpdated = true;
				istr.eatWhite();

				// Get the variable name from the definition line, then find it
				// in the global list.
				getAlphaNumFromStream(istr, sStr);
				_pSaveVar = g_VarManager->getVariable(sStr);

				// the variable must have been found, if it wasn't, then
				// complain violently.
				if (_pSaveVar == nullptr) {
					reportError(ERR_UNKNOWN, "Invalid Residue Print SAVE Variable=%s",
					            sStr.getBuffer());
					return UNKNOWN_TOKEN;
				}
				break;
			}

			if (!sStr.find("SAME")) {
				nObjectUpdated = true;

				_pUntouchedList = _pTouchedList;
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		//
		// Might have an associated movie
		//
		case 'M': {
			getAlphaNumFromStream(istr, sStr);
			if (!sStr.find("MOVIE")) {
				nObjectUpdated = true;

				getAlphaNumFromStream(istr, sStr);
				_sMovieObj = sStr;
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		//
		// Must have a description file, this contains the description of the object
		// that is being residue printed.
		//
		case 'D': {
			getAlphaNumFromStream(istr, sStr);
			if (!sStr.find("DESC")) {
				nObjectUpdated = true;

				_pDescObj = new CBagTextObject();
				if (_pDescObj->setInfo(istr) == PARSING_DONE) {
					return PARSING_DONE;
				}
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		// The variable, which is required, will be the var that is set when
		// residue printing of that object has occurred.
		case 'V': {
			getAlphaNumFromStream(istr, sStr);
			if (!sStr.find("VAR")) {
				nObjectUpdated = true;
				istr.eatWhite();

				// Get the variable name from the definition line, then find it
				// in the global list.
				getAlphaNumFromStream(istr, sStr);
				_pVarObj = g_VarManager->getVariable(sStr);

				// the variable must have been found, if it wasn't, then
				// complain violently.
				if (_pVarObj == nullptr) {
					reportError(ERR_UNKNOWN, "Invalid Residue Print Variable=%s",
								sStr.getBuffer());
					return UNKNOWN_TOKEN;
				}
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		default: {
			ParseCodes parseCode = CBagObject::setInfo(istr);
			if (parseCode == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (parseCode == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
		}
		break;
		}
	}

	return PARSING_DONE;
}

ErrorCode CBagRPObject::attach() {
	// Add this object to the list of objects in the RPO queue.

	if (_pRPList == nullptr) {
		_pRPList = new CBofList<CBagRPObject *>;
	}

	assert(_pRPList != nullptr);

	_pRPList->addToTail(this);

	ErrorCode errorCode = CBagObject::attach();

	// If we haven't initialized this guys original rectangle, then do it
	// here.
	if (_bOrigRectInit == false) {
		_cOrigRect = getRect();
	}

	// Start this object off as invisible (so that we don't receive update events).
	setVisible(false);

	restoreResiduePrintedVars();

	if (_bInitialized == false) {
		_bInitialized = initialize();
	}

	RPStates rpState = getLogState();
	switch (rpState) {
	case RP_NO_MODE:
		break;
	case RP_RESULTS:
	case RP_READ_DOSSIER:
		// If this guy was being shown before, bring it up now.
		if (_bCurVisible) {
			_pActivateThisGuy = this;
		}
		break;
	case RP_REVIEW:
		if (_bRPRead) {
			activateResiduePrintedReview();
		}
		break;

	default:
		break;
	}

	return errorCode;
}

ErrorCode CBagRPObject::detach() {
	// Turn off our current object
	if (_pCurRPObject == this) {
		_pCurRPObject = nullptr;
	}

	// If we're in the current list, then remove us!
	if (_pRPList) {
		int nCount = _pRPList->getCount();
		for (int i = 0; i < nCount; i++) {
			CBagObject *pObj = _pRPList->getNodeItem(i);

			if (pObj == this) {
				_pRPList->remove(i);
				break;
			}
		}

		if (_pRPList->getCount() == 0) {
			delete _pRPList;
			_pRPList = nullptr;
		}
	}

	_bInitialized = false;

	// Don't really need to do anything here since the destructor will trash the
	// entire list.
	return CBagObject::detach();
}

// Have a dossier name and expression for a touched object
void CBagRPObject::setTouchedDos(CBofString &s, CBagExpression *x) {
	//  Make sure the list has been allocated
	if (_pTouchedList == nullptr) {
		_pTouchedList = new CBofList<DossierObj *>;
	}

	DossierObj *pDosObj = new DossierObj();

	// Just store the name for now, we'll get the pointer to the dossier in
	// the attach code.
	pDosObj->_sDossier = s;
	pDosObj->_xDosExp = x;

	_pTouchedList->addToTail(pDosObj);
}

// Store a dossier name and associated expression for an untouched object
void CBagRPObject::setUntouchedDos(CBofString &s, CBagExpression *x) {
	//  Make sure the list has been allocated
	if (_pUntouchedList == nullptr) {
		_pUntouchedList = new CBofList<DossierObj *>;
	}

	DossierObj *pDosObj = new DossierObj();

	// Store the expression and the dossier string.
	pDosObj->_sDossier = s;
	pDosObj->_xDosExp = x;

	_pUntouchedList->addToTail(pDosObj);
}

// This static is the tough guy that is in charge of checking the rp queue for
// any objects that have results that should be returned.
int CBagRPObject::runResiduePrintedQueue() {
	// Might get called with no residue printing list
	if (_pRPList == nullptr) {
		return 0;
	}

	// The first part of this static should only run if the log is frontmost.
	if (SBBasePda::getPdaMode() == PDA_LOG_MODE) {
		// If our script switched us to the main menu, then make sure that we
		// have a chance to deactivate anything that we have active.
		RPStates prevRPState = _eRPMode;
		RPStates curRPState = getLogState();
		if (prevRPState != curRPState && curRPState == RP_MAIN_MENU) {
			if (_pCurRPObject) {
				_pCurRPObject->deactivateRPObject();
				_pCurRPObject->_bCurVisible = false;
				_pCurRPObject->saveResiduePrintedVars();
			}
		}

		// Check the first guy in the queue to see if he's been initialized, if not,
		// then cruise the whole thing.
		int nCount = _pRPList->getCount();
		if (nCount == 0) {
			return 0;
		}

		for (int i = 0; i < nCount; i++) {
			CBagRPObject *pRPObj = _pRPList->getNodeItem(i);

			// This could fail if we are not initialized properly
			if (pRPObj->_bInitialized == false) {
				pRPObj->_bInitialized = pRPObj->initialize();
			}
		}

		// If we have an activate set, then we want to get this guy up as soon as possible,
		// he has been completely constructed and attached and all that cool stuff, this was
		// the object that was frontmost when we went into a flashback, saved a game, etc...
		//
		// Don't activate until we've been properly initialized.
		if (_pActivateThisGuy) {
			if (_pActivateThisGuy->_bInitialized) {
				RPStates rpState = getLogState();
				switch (rpState) {
				case RP_REVIEW:
					if (_pActivateThisGuy->_bRPRead) {
						_pActivateThisGuy->activateResiduePrintedReview();
					}
					break;

				case RP_RESULTS:
					_pActivateThisGuy->activateRPObject();
					break;

				case RP_READ_DOSSIER: {
					CBagDossierObject *pDObj = _pActivateThisGuy->getActiveDossier();
					if (pDObj) {
						pDObj->showDossierText();
						// Special case, make sure the trail back to the rp obj is clearly marked
						pDObj->setResiduePrintedObject(_pActivateThisGuy);
					}
					break;
				}

				default:
					break;
				}
				_pActivateThisGuy = nullptr;
			}
		}

		// If our last update is zero, then we have to parouse our list and fill in all the
		// missing vars.  This is considered our initialization trigger.
		if (_nLastRPQCheck == 0) {
			deactivateResiduePrintedReview();
		}
	}

	// Be semi-efficient about this and only execute every 5 seconds or so.

	int32 nCurTime = getTimer();
	if (nCurTime > _nLastRPQCheck + 5000) {

		// Get the turncount variable.
		if (_turnCount == nullptr) {
			_turnCount = g_VarManager->getVariable("TURNCOUNT");
		}
		//
		assert(_turnCount != nullptr);

		// Get the current time
		int nCurSBTime = _turnCount->getNumValue();
		int nCount = _pRPList->getCount();

		for (int i = 0; i < nCount; i++) {
			CBagRPObject *pRPObj = _pRPList->getNodeItem(i);

			// Find out if there are any events worth reporting.
			//
			// Remove the addition of 20, that's already been added in the script.
			if (pRPObj->_bRPReported == false && pRPObj->_nRPTime != 0 && nCurSBTime >= pRPObj->_nRPTime) {
				addToMsgQueue(pRPObj);
			}
		}

		if (getLogState() == RP_REVIEW) {
			setLogPages(99);
		}

		_nLastRPQCheck = nCurTime;
	}

	return 0;
}

// This static will cruise through the whole queue, check the value of each
// associated variable, find a non-null one and activate the return time associated
// with that residue print request.
int CBagRPObject::updateResiduePrintedQueue() {
	assert(_pRPList != nullptr);

	int nCount = _pRPList->getCount();
	char szLocalBuff[256];
	CBofString cStr(szLocalBuff, 256);

	// Cruise through and find if we have one that has had it's value changed.
	for (int i = 0; i < nCount; i++) {
		CBagRPObject *pRPObj = _pRPList->getNodeItem(i);
		if (pRPObj) {
			cStr = pRPObj->_pVarObj->getValue();

			// If it's value is NOT 3000, then store its value in the associative
			// rp object.
			if (cStr.find("3000") == -1) {
				int nTimecount = atoi(szLocalBuff);
				pRPObj->_nRPTime = nTimecount;
				pRPObj->_bRPReported = false;
				pRPObj->_bResPrinted = true;

				// Evaluate the expression as we submit it for printing, this will
				// guarantee that the expressions are reported as a function of the
				// time that they were submitted.  Trust me, this statement makes
				// perfect sense.
				pRPObj->evaluateDossiers();
				pRPObj->saveResiduePrintedVars();
				// Reset back to "3000"

				cStr = "3000";
				pRPObj->_pVarObj->setValue(cStr);
			}
		}
	}

	return 0;
}

// This static runs through the entire chain and makes sure that everything
// has been deactivated and is not displayed to the PDA.  Called from
// script.
void CBagRPObject::deactivateResiduePrintedQueue() {
	assert(_pRPList != nullptr);

	int nCount = _pRPList->getCount();

	// Cruise through and find if we have one that has had it's value changed.
	for (int i = 0; i < nCount; i++) {
		CBagRPObject *pRPObj = _pRPList->getNodeItem(i);
		if (pRPObj) {
			pRPObj->deactivateRPObject();
		}
	}

	deactivateResiduePrintedReview();
}

// Add this guy to the message queue and make that little message
// light blink away!
bool CBagRPObject::addToMsgQueue(CBagRPObject *pRPObj) {
	bool bAddedToQueue = false;

	// If zoomed, then don't add to the message queue!!!!
	if (zoomed()) {
		return false;
	}

	CBagLog *pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOG_WLD);

	if (pLogWld) {
		pRPObj->setMsgWaiting(true); // mark this guy as waiting
		pLogWld->activateLocalObject(pRPObj);

		pRPObj->setVisible(true);     // make sure it gets updated.
		pRPObj->_bRPRead = false;    // hasn't been read yet
		pRPObj->_bRPReported = true; // been reported though
		pRPObj->saveResiduePrintedVars();         // Update the variable store
		bAddedToQueue = true;
	}

	return bAddedToQueue;
}

// If we get called to run this guy, then run the movie and post the results,
// if any.
bool CBagRPObject::runObject() {
	bool bMoviePlayed = false;

	// If we have a movie, which we should, run it!
	//only play the movie the first time through!
	bool bLocalMoviePlayed = false;
	if (_pMovieObj && _bMoviePlayed == false) {
		bMoviePlayed = _pMovieObj->runObject();
		_bMoviePlayed = true;
		bLocalMoviePlayed = true;
	}

	// Order is important here, if we end up activating the RP review, then
	// we're going to need this guy marked as read.

	_bRPRead = true;
	saveResiduePrintedVars(); // Update the variable store

	// Make sure that nothing from a previous res print is showing
	if (_pCurRPObject) {
		_pCurRPObject->deactivateRPObject();
		_pCurRPObject->_bCurVisible = false;
		_pCurRPObject->saveResiduePrintedVars();
		_pCurRPObject = nullptr;
	}

	// Parouse the rp list, if we have more than one waiting for the
	// user, then go to the residue print results.  Be a good guy and
	// go to the page that this guy will show up on.
	//
	// results waiting will only return 1 if there are two in
	// the queue (because this one has already been marked as read).
	//
	// only want to remove everything waiting if this movie
	// was actually played.
	if (bLocalMoviePlayed == true && !zoomed() && residuePrintedResultsWaiting() >= 1) {

		// Remove all these from the message waiting queue.
		removeAllFromMsgQueue(this);

		activateResiduePrintedReview();
	} else {
		activateRPObject();
	}

	return bMoviePlayed;
}

// This routine just displays results for a single object.  We still need to
// implement cascading RP results.
#define REVIEWDOSZ "REVIEWDOSZ"
#define REVIEWDOS "REVIEWDOS"

ErrorCode CBagRPObject::update(CBofBitmap *pBmp, CBofPoint /*pt*/, CBofRect * /*pSrcRect*/, int) {
	if (getLogState() == RP_RESULTS) {
		if (_pDescObj) {
			CBofRect txRect = _pDescObj->getRect();
			CBofPoint txPt = txRect.topLeft();
			_pDescObj->update(pBmp, txPt, &txRect, 0);
		}
	}

	return ERR_NONE;
}

bool CBagRPObject::activateRPObject() {
	// If there's one already activated, then deactivate it.  Don't want
	// two of these drawn on top of each other.
	if (_pCurRPObject && _pCurRPObject != this) {
		_pCurRPObject->deactivateRPObject();
		_pCurRPObject->_bCurVisible = false;
		_pCurRPObject->saveResiduePrintedVars();
	}

	// Set current and make sure our variable knows that this one is being shown,
	// this is important for snapping from zoom to regular and vice-versa.
	_pCurRPObject = this;
	_bCurVisible = true;
	saveResiduePrintedVars();

	// Make sure this guy is active and ready to get drawn.
	setVisible(); // show this guy
	setActive();  // accept mouse downs

	CBagLog *pLogWld;
	if (zoomed()) {
		pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOGZ_WLD);
	} else {
		pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOG_WLD);
	}

	if (pLogWld == nullptr) {
		return false;
	}

	// If we're coming from residue print review, make sure none of those objects
	// are visible.
	if (getLogState() == RP_REVIEW) {
		hideResiduePrintedReview();
	}

	// We're ready to display a RP result, so switch to results mode
	if (getLogState() != RP_RESULTS) {
		setLogState(RP_RESULTS);
	}
	setLogPages(1);

	// Now go through each one and make sure we have the backp
	CBofList<DossierObj *> *pDosList = (_bTouched ? _pTouchedList : _pUntouchedList);
	int nCount = pDosList->getCount();

	for (int i = 0; i < nCount; i++) {
		DossierObj *pDosObj = pDosList->getNodeItem(i);

		// By default, include the dossier in the list
		if (pDosObj->_bDisplayDossier) {
			pDosObj->_pDossier->activateDossierObject(pLogWld);
		}

		// Give it a back pointer so that it can find the parent res print object
		//
		// We must do this here as this will point to the object that just got
		// residue printed.
		pDosObj->_pDossier->setResiduePrintedObject(this);
	}

	showPdaLog();

	// Attach the description object.
	assert(_pDescObj != nullptr);

	// This object might not be attached since it is not a local object in the
	// log storage device
	if (!_pDescObj->isAttached()) {
		_pDescObj->attach();
	}
	_pDescObj->setVisible(); // Show this guy

	return true;
}

void CBagRPObject::deactivateRPObject() {
	CBagLog *pLogWld;

	// Get the appropriate storage device
	if (zoomed()) {
		pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOGZ_WLD);
	} else {
		pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOG_WLD);
	}

	// Can't bloody well do much without our storage device
	if (pLogWld == nullptr) {
		return;
	}

	// Make sure that all our objects that we care about are no longer active
	int nCount = _pTouchedList->getCount();
	DossierObj *pDosLObj;

	for (int i = 0; i < nCount; i++) {
		pDosLObj = _pTouchedList->getNodeItem(i);
		pDosLObj->_pDossier->deactivateDossierObject(pLogWld);
	}

	if (_pTouchedList != _pUntouchedList) {
		nCount = _pUntouchedList->getCount();

		for (int i = 0; i < nCount; i++) {
			pDosLObj = _pUntouchedList->getNodeItem(i);
			pDosLObj->_pDossier->deactivateDossierObject(pLogWld);
		}
	}

	// Attach the description object.
	assert(_pDescObj != nullptr);

	if (_pDescObj) {
		_pDescObj->setVisible(false);  // Hide this guy
		_pDescObj->setActive(false);   // Don't take mousedowns
		_pDescObj->setFloating(false); // Don't calculate floating rects
	}
	setVisible(false); // Hide this guy

	return;
}

// This static will cruise the entire rpo object list, mark each one that has
// had a result returned as active and floater.  also, update the mode var
void CBagRPObject::activateResiduePrintedReview() {
	// Make sure the log is frontmost
	if (getLogState() != RP_REVIEW) {
		showPdaLog();

		// Now in review mode, this is used in our update code to determine what to
		// show.
		setLogState(RP_REVIEW);
	}

	setLogPages(99);
	showResiduePrintedReview();
}

void CBagRPObject::deactivateResiduePrintedReview() {
	CBagLog *pLogWld;

	// Get the appropriate storage device
	if (zoomed()) {
		pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOGZ_WLD);
	} else {
		pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOG_WLD);
	}

	// can't bloody well get anywhere without this info.
	assert(pLogWld != nullptr);
	if (pLogWld == nullptr) {
		return;
	}

	// Now in review mode, this is used in our update code to determine what to show.
	CBagRPObject *pRPObj = _pRPList->getNodeItem(0);
	if (pRPObj == nullptr) {
		return;
	}

	// Cruise the whole list, fill in missing vars, set all our text
	// objects to inactive so they don't get drawn.
	int nCount = _pRPList->getCount();

	for (int i = 0; i < nCount; i++) {
		pRPObj = _pRPList->getNodeItem(i);

		// We know this object is attached, but it might not be visible.
		if (pRPObj->_pObjectName) {
			pRPObj->_pObjectName->setFloating(false);
			pRPObj->_pObjectName->setVisible(false);
			pRPObj->_pObjectName->setActive(false);
		}
	}
}

void CBagRPObject::onLButtonUp(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * /*pv*/) {
	// Deactivate everything in the rp list
	hideResiduePrintedReview();

	// We're ready to display a RP result, so switch to results mode
	setLogState(RP_RESULTS);
	setLogPages(1);

	runObject();
}

void CBagRPObject::evaluateDossiers() {

	// If we have a touched variable, then evaluate and if it is not 3000, then
	// we know it's been touched.
	if (_pTouchedVar) {
		if (_pTouchedVar->getValue() != "3000") {
			_bTouched = true;
		}
	}

	// Just cruise through each of our dossier's and decide which ones
	// to include in our list, this is based on the expression objects
	// that are in the script.
	//
	// A note on expression objects in script.  Note that all the expressions
	// in the script that have mathematical expressions in them take place
	// on the left of the >/=/< sign.  This is because the eval left to
	// right routine used below has been specifically tweaked to handle
	// left to right evaluation.
	CBofList<DossierObj *> *pDosList = (_bTouched ? _pTouchedList : _pUntouchedList);
	assert(pDosList != nullptr);

	int nCount = pDosList->getCount();
	CBagLog *pLogZWld = nullptr;

	for (int i = 0; i < nCount; i++) {
		DossierObj *pDosLObj = pDosList->getNodeItem(i);

		// By default, include the dossier in the list
		pDosLObj->_bDisplayDossier = true;
		if (pDosLObj->_xDosExp != nullptr) {
			pDosLObj->_bDisplayDossier = pDosLObj->_xDosExp->evalLeftToRight(false);
		}

		// If we are displaying this dossier and the suspect var is
		// there, then lookup the suspect var and let the voice printing code know
		// that this guy's DNA has shown up on some object.
		if (pDosLObj->_bDisplayDossier &&
		        pDosLObj->_pDossier &&
		        pDosLObj->_pDossier->_sSuspectVar.isEmpty() == false) {

			if (pLogZWld == nullptr) {
				pLogZWld = (CBagLog *)g_SDevManager->getStorageDevice(LOGZ_WLD);
			}

			if (pLogZWld != nullptr) {
				CBagLogSuspect *pSusObj = (CBagLogSuspect *)pLogZWld->getObject(pDosLObj->_pDossier->_sSuspectVar);

				if (pSusObj != nullptr) {
					pSusObj->setSusResiduePrinted(true);
				}
			}
		}
	}
	saveResiduePrintedVars(); // Update the variable store
}

// This is really ugly, so put it all in the same place
void CBagRPObject::setLogState(RPStates eLogMode) {
	// This is very, very, very hacky, but make sure that our script has
	// no power over what is going on, so set our log state to "CODE_RP_RESULTS".
	// Also, so we know where to return to, make sure we set the previous log
	// state to whatever LOG_STATE is right now.
	if (_pLogStateVar == nullptr) {
		_pLogStateVar = g_VarManager->getVariable("LOG_STATE");
	}

	if (_pPrevLogStateVar == nullptr) {
		_pPrevLogStateVar = g_VarManager->getVariable("PREV_LOG_STATE");
	}

	assert(_pLogStateVar != nullptr && _pPrevLogStateVar != nullptr);

	if (_pLogStateVar != nullptr && _pPrevLogStateVar != nullptr) {
		char szLocalBuff[256];
		CBofString cStr(szLocalBuff, 256);
		bool bRemember = false;
		bool bSavePage = false;
		bool bRestorePage = false;

		switch (eLogMode) {
		case RP_RESULTS:
			cStr = "CODE_RP_RESULTS";
			bSavePage = true;
			bRemember = true;
			break;

		case RP_READ_DOSSIER:
			cStr = "CODE_RP_DOSSIER";
			break;

		case RP_REVIEW:
			bRemember = true;
			bRestorePage = true;
			cStr = "RES_PRINT_REVIEW";
			break;

		default:
			break;
		}

		if (_eRPMode != RP_READ_DOSSIER) {
			if (bRemember) {
				_pPrevLogStateVar->setValue(_pLogStateVar->getValue());
			}
		}
		_pLogStateVar->setValue(cStr);

		if (bSavePage || bRestorePage) {
			if (_pPrevBarPage == nullptr) {
				_pPrevBarPage = g_VarManager->getVariable("PREV_BAR_LOG_PAGE");
			}

			if (_pCurBarPage == nullptr) {
				_pCurBarPage = g_VarManager->getVariable("CUR_BAR_LOG_PAGE");
			}

			if (_pPrevBarPage && _pCurBarPage) {
				if (bSavePage) {
					_pPrevBarPage->setValue(_pCurBarPage->getValue());
				}

				if (bRestorePage) {
					_pCurBarPage->setValue(_pPrevBarPage->getValue());
				}
			}
		}
	}

	_eRPMode = eLogMode;
}

// Return the current log state
RPStates CBagRPObject::getLogState() {
	if (_pLogStateVar == nullptr) {
		_pLogStateVar = g_VarManager->getVariable("LOG_STATE");
	}

	_eRPMode = RP_NO_MODE;

	assert(_pLogStateVar != nullptr);
	if (_pLogStateVar) {
		char szLocalBuff[256];
		CBofString cStr(szLocalBuff, 256);

		cStr = _pLogStateVar->getValue();
		if (cStr == "CODE_RP_RESULTS") {
			_eRPMode = RP_RESULTS;
		} else if (cStr == "CODE_RP_DOSSIER") {
			_eRPMode = RP_READ_DOSSIER;
		} else if (cStr == "RES_PRINT_REVIEW") {
			_eRPMode = RP_REVIEW;
		} else if (cStr == "MAINMENU") {
			_eRPMode = RP_MAIN_MENU;
		}
	}

	return _eRPMode;
}

// Set the current number of pages in both script and code.
void CBagRPObject::setLogPages(int nPages) {
	if (_pBarLogPages == nullptr) {
		_pBarLogPages = g_VarManager->getVariable("CODE_TOTAL_LOG_PAGES");
	}

	assert(_pBarLogPages != nullptr);

	if (_pBarLogPages) {
		_pBarLogPages->setValue(nPages);
	}

	// Let the float code know how many pages we have.
	CBagLog *pLogWld;
	if (zoomed()) {
		pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOGZ_WLD);
	} else {
		pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOG_WLD);
	}

	assert(pLogWld != nullptr);
	if (pLogWld) {
		// Let the float code know how many pages we have.
		pLogWld->setNumFloatPages(nPages);
		if (nPages == 1) {
			pLogWld->setCurFltPage(1);
		}
	}
}

// Save the residue print variables

void CBagRPObject::saveResiduePrintedVars() {
	if (_pSaveVar == nullptr) {
		return;
	}

	uint32 nVars = 0;

	for (int i = 0; i < NUM_RP_FIELDS; i++) {
		switch (i) {
		case 0:
			nVars = _nRPTime & mRPTime;
			break;
		case 1:
			nVars |= (_bRPReported ? mRPReported : 0);
			break;
		case 2:
			nVars |= (_bResPrinted ? mResPrinted : 0);
			break;
		case 3:
			nVars |= (_bTouched ? mTouched : 0);
			break;
		case 4:
			nVars |= (_bRPRead ? mRPRead : 0);
			break;
		case 5:
			nVars |= (_bCurVisible ? mRPCurVisible : 0);
			break;
		case 6:
			nVars |= ((_nCurDossier & 0x3) << mRPCurDossierShift);
			break;
		case 7:
			nVars |= (_bMoviePlayed ? mRPMoviePlayed : 0);
			break;
		case 8:
			nVars |= (_bRPTimeSet ? mRPTimeSet : 0);
			break;
		}
	}

	DossierObj *pDosObj;

	if (_pTouchedList) {
		// Now handle touched dossier's
		int nCount = _pTouchedList->getCount();
		assert(nCount <= 3);

		for (int i = 0; i < nCount; i++) {
			pDosObj = _pTouchedList->getNodeItem(i);
			switch (i) {
			case 0:
				nVars |= (pDosObj->_bDisplayDossier ? mRPTDos1Eval : 0);
				break;
			case 1:
				nVars |= (pDosObj->_bDisplayDossier ? mRPTDos2Eval : 0);
				break;
			case 2:
				nVars |= (pDosObj->_bDisplayDossier ? mRPTDos3Eval : 0);
				break;
			}
		}
	}

	if (_pUntouchedList) {
		// Now handle untouched dossier's
		int nCount = _pUntouchedList->getCount();
		assert(nCount <= 3);

		for (int i = 0; i < nCount; i++) {
			pDosObj = _pUntouchedList->getNodeItem(i);
			switch (i) {
			case 0:
				nVars |= (pDosObj->_bDisplayDossier ? mRPUDos1Eval : 0);
				break;
			case 1:
				nVars |= (pDosObj->_bDisplayDossier ? mRPUDos2Eval : 0);
				break;
			case 2:
				nVars |= (pDosObj->_bDisplayDossier ? mRPUDos3Eval : 0);
				break;
			}
		}
	}

	_pSaveVar->setValue(nVars);
}

// Restore the residue print variable from memory
void CBagRPObject::restoreResiduePrintedVars() {
	if (_pSaveVar == nullptr) {
		return;
	}

	uint32 nVars = _pSaveVar->getNumValue();

	for (int i = 0; i < NUM_RP_FIELDS; i++) {
		switch (i) {
		case 0:
			_nRPTime = nVars & mRPTime;
			break;
		case 1:
			_bRPReported = (nVars & mRPReported ? true : false);
			break;
		case 2:
			_bResPrinted = (nVars & mResPrinted ? true : false);
			break;
		case 3:
			_bTouched = (nVars & mTouched ? true : false);
			break;
		case 4:
			_bRPRead = (nVars & mRPRead ? true : false);
			break;
		case 5:
			_bCurVisible = (nVars & mRPCurVisible ? true : false);
			break;
		case 6:
			(_nCurDossier = (nVars & mRPCurDossier) >> mRPCurDossierShift);
			break;
		case 7:
			_bMoviePlayed = (nVars & mRPMoviePlayed ? true : false);
			break;
		case 8:
			_bRPTimeSet = (nVars & mRPTimeSet ? true : false);
			break;
		}
	}

	DossierObj *pDosObj;

	if (_pTouchedList) {
		// Now handle touched dossier's
		int nCount = _pTouchedList->getCount();
		assert(nCount <= 3);

		for (int i = 0; i < nCount; i++) {
			pDosObj = _pTouchedList->getNodeItem(i);
			switch (i) {
			case 0:
				pDosObj->_bDisplayDossier = (nVars & mRPTDos1Eval ? true : false);
				break;
			case 1:
				pDosObj->_bDisplayDossier = (nVars & mRPTDos2Eval ? true : false);
				break;
			case 2:
				pDosObj->_bDisplayDossier = (nVars & mRPTDos3Eval ? true : false);
				break;
			}
		}
	}

	if (_pUntouchedList) {
		// Now handle untouched dossier's
		int nCount = _pUntouchedList->getCount();
		assert(nCount <= 3);

		for (int i = 0; i < nCount; i++) {
			pDosObj = _pUntouchedList->getNodeItem(i);
			switch (i) {
			case 0:
				pDosObj->_bDisplayDossier = (nVars & mRPUDos1Eval ? true : false);
				break;
			case 1:
				pDosObj->_bDisplayDossier = (nVars & mRPUDos2Eval ? true : false);
				break;
			case 2:
				pDosObj->_bDisplayDossier = (nVars & mRPUDos3Eval ? true : false);
				break;
			}
		}
	}
}

// Hide the list of rp results, don't purge them from memory, just set to not visible.
void CBagRPObject::hideResiduePrintedReview() {
	CBagLog *pLogWld;

	if (zoomed()) {
		pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOGZ_WLD);
	} else {
		pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOG_WLD);
	}

	CBagRPObject *pRPObj = _pRPList->getNodeItem(0);
	if (pRPObj == nullptr) {
		return;
	}

	// Count items in this queue.
	int nCount = _pRPList->getCount();

	for (int i = 0; i < nCount; i++) {
		pRPObj = _pRPList->getNodeItem(i);

		if (!pRPObj)
			error("hideResiduePrintedReview() - Unexpected null pRPObj");
		
		// If the object txt var is not found yet, then get it.
		if (pRPObj->_pObjectName == nullptr) {
			if (!pLogWld)
				error("hideResiduePrintedReview() - Unexpected null pLogWld");

			pRPObj->_pObjectName = (CBagTextObject *)pLogWld->getObject(pRPObj->_sObjectName);
		}

		// We know this object is attached, but it might not be visible.
		if (pRPObj->_pObjectName) {
			pRPObj->_pObjectName->setVisible(false);  // hide this guy
			pRPObj->_pObjectName->setFloating(false); // don't arrange floating pages
			pRPObj->_pObjectName->setActive(false);   // don't accept mouse downs
		}
	}
}

void CBagRPObject::showResiduePrintedReview() {
	CBagLog *pLogWld;

	if (zoomed()) {
		pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOGZ_WLD);
	} else {
		pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOG_WLD);
	}

	if (!pLogWld) {
		warning("showResiduePrintedReview() - Unexpected null logWld");
		return;
	}
	
	// Count items in this queue.
	int nCount = pLogWld->getObjectCount();

	// Check the first guy in the queue to see if he's been initialized, if not,
	// then cruise the whole thing.
	for (int i = 0; i < nCount; i++) {
		CBagObject *pObj = pLogWld->getObjectByPos(i);
		if (pObj && pObj->getType() == RESPRNT_OBJ) {

			CBagRPObject *pRPObj = (CBagRPObject *)pObj;
			// Find out if there are any events worth reporting.
			if (pRPObj->_bResPrinted && pRPObj->_bRPRead) {

				// We know this object is attached, but it might not be visible.
				if (pRPObj->_pObjectName) {

					bool bIsAttached = pRPObj->_pObjectName->isAttached();
					if (bIsAttached == false) {
						pLogWld->activateLocalObject(pRPObj->_pObjectName);
					}
					pRPObj->_pObjectName->setVisible();
					pRPObj->_pObjectName->setFloating();
					pRPObj->_pObjectName->setActive();

					// If the time on this object has not yet been set, then force it to
					// be re-read into memory and append the current time to it.
					if (pRPObj->_bRPTimeSet == false) {
						CBofString s = pRPObj->_pObjectName->getText();

						int nHr = pRPObj->_nRPTime / 100;
						int nMn = pRPObj->_nRPTime - (nHr * 100);

						s += buildString("%02d:%02d", nHr, nMn);

						pRPObj->_pObjectName->setText(s);
						pRPObj->_bRPTimeSet = true;
						pRPObj->saveResiduePrintedVars();
					}
				}
			}
		}
	}
}

// Count the number of residue print objects waiting to be reported;
int CBagRPObject::residuePrintedResultsWaiting() {
	int nCount = _pRPList->getCount();
	int nWaiting = 0;

	for (int i = 0; i < nCount; i++) {
		CBagRPObject *pRPObj = _pRPList->getNodeItem(i);
		if (pRPObj->_bRPReported && pRPObj->_bRPRead == false) {
			nWaiting++;
		}
	}

	return nWaiting;
}

// Remove all residue print results from the message queue
void CBagRPObject::removeAllFromMsgQueue(CBagRPObject *pCurRPObj) {
	// we really only care about the log world, not the logz.
	CBagLog *pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOG_WLD);
	assert(pLogWld != nullptr);
	int nCount = _pRPList->getCount();

	for (int i = 0; i < nCount; i++) {
		CBagRPObject *pRPObj = _pRPList->getNodeItem(i);

		if (pRPObj->_bRPReported && pRPObj->_bRPRead == false && pRPObj != pCurRPObj) {
			pLogWld->removeFromMsgQueue(pRPObj);

			pRPObj->_bRPRead = true;
			pRPObj->_bMoviePlayed = true; // Don't want this guys movie again
			pRPObj->saveResiduePrintedVars();          // Update the variable store
		}
	}
}

//  We're going to need to switch the PDA to log mode.
void CBagRPObject::showPdaLog() {
	if (zoomed()) {
		SBZoomPda *pZoomPDA = (SBZoomPda *)g_SDevManager->getStorageDevice(PDAZ_WLD);
		if (pZoomPDA) {
			pZoomPDA->showLog();
		}
	} else {
		CBagPDA *pPDA = (CBagPDA *)g_SDevManager->getStorageDevice(PDA_WLD);
		if (pPDA) {
			pPDA->showLog();
		}
	}
}

bool CBagRPObject::zoomed() {
	SBZoomPda *pPDA = (SBZoomPda *)g_SDevManager->getStorageDevice(PDAZ_WLD);
	if (pPDA == nullptr) {
		return false;
	}

	return pPDA->getZoomed();
}

bool CBagRPObject::initialize() {
	// Cruise the dossier's for both lists and get pointers to the actual bagdoobj's.
	// Search the current storage device for this object.
	CBagStorageDev *pSDev;

	if (zoomed()) {
		pSDev = g_SDevManager->getStorageDevice(LOGZ_WLD);
	} else {
		pSDev = g_SDevManager->getStorageDevice(LOG_WLD);
	}
	assert(pSDev != nullptr);

	// Scoff the dossier out of the LOG_WLD SDEV.  If it's not there then hurl.
	bool bDoUntouched = (_pTouchedList != _pUntouchedList);
	int nCount = _pTouchedList->getCount();

	for (int i = 0; i < nCount; i++) {
		DossierObj *pDosObj = _pTouchedList->getNodeItem(i);
		pDosObj->_pDossier = (CBagDossierObject *)pSDev->getObject(pDosObj->_sDossier);
		if (pDosObj->_pDossier == nullptr) {
			return false;
		}
		// Give it a back pointer so that it can find the parent res print object
		//
		// We must do this here as this will point to the object that just got
		// residue printed.
		pDosObj->_pDossier->setResiduePrintedObject(this);
	}

	if (bDoUntouched) {
		nCount = _pUntouchedList->getCount();
		for (int i = 0; i < nCount; i++) {
			DossierObj *pDosObj = _pUntouchedList->getNodeItem(i);
			pDosObj->_pDossier = (CBagDossierObject *)pSDev->getObject(pDosObj->_sDossier);
			if (pDosObj->_pDossier == nullptr) {
				return false;
			}

			// Give it a back pointer so that it can find the parent res print object
			//
			// We must do this here as this will point to the object that just got
			// residue printed.
			pDosObj->_pDossier->setResiduePrintedObject(this);
		}
	}

	// Fill in the movie object if we have one.
	if (!_sMovieObj.isEmpty()) {
		_pMovieObj = (CBagMovieObject *)pSDev->getObject(_sMovieObj);
		if (_pMovieObj == nullptr) {
			return false;
		}
	}

	// Fill in the object name if we have one.
	if (_pObjectName == nullptr) {
		_pObjectName = (CBagTextObject *)pSDev->getObject(_sObjectName);
		if (_pObjectName == nullptr) {
			return false;
		}
	}

	// Make sure the object is not active. Object name is the
	// line that shows up in the RP Review screen (such as "Voice Printer")

	// Give the dossier a back pointer so that it can respond to
	// mouse down events
	_pObjectName->setRPObject(this);

	_pObjectName->setVisible(false);
	_pObjectName->setActive(false);
	_pObjectName->setFloating(false);

	return true;
}

// This hack is used to make sure that any variable values that were altered by
// the zoom pda are propagated down to the regular PDA.
void CBagRPObject::synchronizeResiduePrintedObjects(bool bLogFrontmost) {
	// only synchronize in the bar
	CBagVar *pVar = g_VarManager->getVariable("INBAR");
	if (pVar == nullptr) {
		return;
	}

	CBagLog *pLogWld = (CBagLog *)g_SDevManager->getStorageDevice(LOG_WLD);
	if (pLogWld == nullptr) {
		return;
	}

	RPStates rpState = getLogState();
	int nCount = pLogWld->getObjectCount();

	// Check the first guy in the queue to see if he's been initialized, if not,
	// then cruise the whole thing.
	for (int i = 0; i < nCount; i++) {
		CBagObject *pObj = pLogWld->getObjectByPos(i);
		if (pObj->getType() == RESPRNT_OBJ) {
			CBagRPObject *pRPObj = (CBagRPObject *)pObj;
			pRPObj->restoreResiduePrintedVars();

			if (bLogFrontmost) {
				switch (rpState) {
				case RP_NO_MODE:
					break;
				case RP_RESULTS:
					// If this guy was being shown before, bring it up now.
					if (pRPObj->_bCurVisible) {
						_pActivateThisGuy = pRPObj;
					}
					break;
				case RP_READ_DOSSIER:
					if (pRPObj->_bCurVisible) {
						_pActivateThisGuy = pRPObj;
					}
					break;
				case RP_REVIEW:
					if (pRPObj->_bRPRead) {
						pRPObj->activateResiduePrintedReview();
					}
					break;
				default:
					break;
				}
			}
		}
	}
}

// Used to fetch the currently active dossier, good for coming out of save/
// restore, flashbacks and zoom/unzoom.
CBagDossierObject *CBagRPObject::getActiveDossier() {
	CBofList<DossierObj *> *pDosList = (_bTouched ? _pTouchedList : _pUntouchedList);

	return pDosList->getNodeItem(_nCurDossier)->_pDossier;
}

// Used to set the currently active dossier when one is displayed to the user.
void CBagRPObject::setActiveDossier(CBagDossierObject *pDosObj) {
	CBofList<DossierObj *> *pDosList = (_bTouched ? _pTouchedList : _pUntouchedList);

	int nCount = pDosList->getCount();
	for (int i = 0; i < nCount; i++) {
		DossierObj *p = pDosList->getNodeItem(i);
		if (p->_pDossier == pDosObj) {
			_nCurDossier = i;
			saveResiduePrintedVars();
			break;
		}
	}
}

// Initialize all dossier fields
DossierObj::DossierObj() {
	_pDossier = nullptr;
	_sDossier = "";
	_xDosExp = nullptr;
	_bDisplayDossier = false;
}

// Remove all allocated dossier objects
DossierObj::~DossierObj() {
	// Don't delete dossier's, we got those from the storage dev manager.
	_pDossier = nullptr;

	// Expressions, however, we do own
	delete _xDosExp;
	_xDosExp = nullptr;
}

} // namespace Bagel
