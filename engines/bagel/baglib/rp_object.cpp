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
CBofList<CBagRPObject *> *CBagRPObject::m_pRPList = nullptr;
CBagVar *CBagRPObject::m_pTurncount = nullptr;
CBagVar *CBagRPObject::m_pLogStateVar = nullptr;
CBagVar *CBagRPObject::m_pPrevLogStateVar = nullptr;
CBagVar *CBagRPObject::m_pBarLogPages = nullptr;
CBagVar *CBagRPObject::m_pPrevBarPage = nullptr;
CBagVar *CBagRPObject::m_pCurBarPage = nullptr;
int32 CBagRPObject::m_nLastRPQCheck = 0;
CBagRPObject *CBagRPObject::m_pCurRPObject = nullptr;
RPSTATES CBagRPObject::m_eRPMode = RP_NOMODE;
CBagRPObject *CBagRPObject::m_pActivateThisGuy = nullptr;

// Local globals
//
// CBagRPObject -
//  CBagRPObject is an object that can be accessed through script to handle residue
//  printing.
//
CBagRPObject::CBagRPObject() {
	m_xObjType = RESPRNTOBJ;

	// Dossier list
	m_pTouchedList = nullptr;
	m_pUntouchedList = nullptr;

	// Touched var, not required.
	m_pTouchedVar = nullptr;

	m_pDescObj = nullptr;

	m_nRPTime = 0;
	m_bRPReported = false;
	m_bResPrinted = false;
	m_bResPrinted = false;
	m_bTouched = false;
	m_bRPRead = false;
	m_bRPTimeSet = false;
	m_bCurVisible = false;
	m_bInitialized = false;
	m_bMoviePlayed = false;

	// Have to save the original rectangle since we'll be messing with it later
	// on.
	m_cOrigRect.SetRect(0, 0, 0, 0);
	m_bOrigRectInit = false;

	// the object's name, we pull this out of the wld sdev.
	m_pObjectName = nullptr;

	// Initialize movie land
	m_sMovieObj = "";
	m_pMovieObj = nullptr;

	// Save variable
	m_pSaveVar = nullptr;

	// Current dossier
	m_nCurDossier = 0;
}

CBagRPObject::~CBagRPObject() {
	// If still attached, then detach
	if (IsAttached()) {
		Detach();
	}

	// Explicitly delete everything in the list
	bool bSame = (m_pTouchedList == m_pUntouchedList);
	DossierObj *pDObj;

	// Trash the expression list
	if (m_pTouchedList != nullptr) {
		int nCount = m_pTouchedList->GetCount();
		for (int i = 0; i < nCount; i++) {
			pDObj = m_pTouchedList->Remove(0);
			if (pDObj) {
				delete pDObj;
			}
		}

		delete m_pTouchedList;
		m_pTouchedList = nullptr;
	}

	//  Explicitly delete everything in the list
	//  Only trash them if they are not the same list.
	if (bSame == false) {
		if (m_pUntouchedList != nullptr) {
			int nCount = m_pUntouchedList->GetCount();
			for (int i = 0; i < nCount; i++) {
				pDObj = m_pUntouchedList->Remove(0);
				if (pDObj) {
					delete pDObj;
				}
			}
			delete m_pUntouchedList;
			m_pUntouchedList = nullptr;
		}
	}

	// Delete the description object
	if (m_pDescObj) {
		delete m_pDescObj;
		m_pDescObj = nullptr;
	}

	// We got these vars from the var manager, so just null it out, don't delete it!
	m_pVarObj = nullptr;
	m_pTurncount = nullptr;
	m_pTouchedVar = nullptr;
	m_pMovieObj = nullptr;
	m_pSaveVar = nullptr;

	// Clear our statics, yes, I mean to do that here.
	m_pTurncount = nullptr;
	m_pLogStateVar = nullptr;
	m_pPrevLogStateVar = nullptr;
	m_pBarLogPages = nullptr;
	m_pPrevBarPage = nullptr;
	m_pCurBarPage = nullptr;
}

PARSE_CODES CBagRPObject::SetInfo(bof_ifstream &istr) {
	bool nObjectUpdated = false;
	char szLocalStr[256];
	CBofString sStr(szLocalStr, 256);

	while (!istr.eof()) {
		istr.EatWhite();

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
			GetAlphaNumFromStream(istr, sStr);
			if (sStr.Find("TOUCHEDVAR") == 0) {
				GetAlphaNumFromStream(istr, sStr);

				nObjectUpdated = true;
				istr.EatWhite();

				// Get the variable name from the definition line, then find it
				// in the global list.
				m_pTouchedVar = VARMNGR->GetVariable(sStr);

				break;
			}

			bool bTouched = (ch == 'T');

			if ((bTouched && !sStr.Find("TOUCHED")) || (bTouched == false && !sStr.Find("UNTOUCHED"))) {

				nObjectUpdated = true;

				bool bContinue;
				do {
					CBagExpression *px = nullptr;

					GetAlphaNumFromStream(istr, sStr);
					bContinue = false;

					// If the next non-whitespace char is a paren, then we're going
					// to have an expression.
					istr.EatWhite();
					if ((char)istr.peek() == '(') {
						px = new CBagExpression();
						if (px) {
							px->SetInfo(istr);
						}
					}

					// add this to the correct list.
					if (bTouched) {
						SetTouchedDos(sStr, px);
					} else {
						SetUntouchedDos(sStr, px);
					}

					if ((char)istr.peek() == ',') {
						istr.Get();
						istr.EatWhite();
						bContinue = true;
					}
				} while (bContinue);
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		//
		// We can hit a continuation character, a plus sign tells us that there
		// is more on the next line.
		case '+': {
			// Skip over this character
			istr.Get();
			istr.EatWhite();

			// Now keep going until we get our next alpha num.
			while (!Common::isAlnum((char)istr.peek())) {
				istr.Get();
			}
		}
		break;

		case 'N': {
			GetAlphaNumFromStream(istr, sStr);
			if (!sStr.Find("NAME")) {
				nObjectUpdated = true;

				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);

				m_sObjectName = sStr;

			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		// Same and Save can be in the stream.
		case 'S': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("SAVE")) {
				nObjectUpdated = true;
				istr.EatWhite();

				// Get the variable name from the definition line, then find it
				// in the global list.
				GetAlphaNumFromStream(istr, sStr);
				m_pSaveVar = VARMNGR->GetVariable(sStr);

				// the variable must have been found, if it wasn't, then
				// complain violently.
				if (m_pSaveVar == nullptr) {
					ReportError(ERR_UNKNOWN, "Invalid Residue Print SAVE Variable=%s",
					            sStr.GetBuffer());
					return UNKNOWN_TOKEN;
				}
				break;
			}

			if (!sStr.Find("SAME")) {
				nObjectUpdated = true;

				m_pUntouchedList = m_pTouchedList;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		//
		// Might have an associated movie
		//
		case 'M': {
			GetAlphaNumFromStream(istr, sStr);
			if (!sStr.Find("MOVIE")) {
				nObjectUpdated = true;

				GetAlphaNumFromStream(istr, sStr);
				m_sMovieObj = sStr;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		//
		// Must have a description file, this contains the description of the object
		// that is being residue printed.
		//
		case 'D': {
			GetAlphaNumFromStream(istr, sStr);
			if (!sStr.Find("DESC")) {
				nObjectUpdated = true;

				m_pDescObj = new CBagTextObject();
				if (m_pDescObj && m_pDescObj->SetInfo(istr) == PARSING_DONE) {
					return PARSING_DONE;
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		// The variable, which is required, will be the var that is set when
		// residue printing of that object has occurred.
		case 'V': {
			GetAlphaNumFromStream(istr, sStr);
			if (!sStr.Find("VAR")) {
				nObjectUpdated = true;
				istr.EatWhite();

				// Get the variable name from the definition line, then find it
				// in the global list.
				GetAlphaNumFromStream(istr, sStr);
				m_pVarObj = VARMNGR->GetVariable(sStr);

				// the variable must have been found, if it wasn't, then
				// complain violently.
				if (m_pVarObj == nullptr) {
					ReportError(ERR_UNKNOWN, "Invalid Residue Print Variable=%s",
					            sStr.GetBuffer());
					return UNKNOWN_TOKEN;
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		default: {
			PARSE_CODES rc;
			if ((rc = CBagObject::SetInfo(istr)) == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
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

ErrorCode CBagRPObject::Attach() {
	// Add this object to the list of objects in the RPO queue.

	if (m_pRPList == nullptr) {
		m_pRPList = new CBofList<CBagRPObject *>;
	}

	Assert(m_pRPList != nullptr);

	m_pRPList->AddToTail(this);

	ErrorCode ec = CBagObject::Attach();

	// If we haven't initialized this guys original rectangle, then do it
	// here.
	if (m_bOrigRectInit == false) {
		m_cOrigRect = GetRect();
	}

	// Start this object off as invisible (so that we don't receive update events).
	SetVisible(false);

	RestoreRPVars();

	if (m_bInitialized == false) {
		m_bInitialized = initialize();
	}

	RPSTATES rpState = GetLogState();
	switch (rpState) {
	case RP_NOMODE:
		break;
	case RP_RESULTS:
	case RP_READ_DOSSIER:
		// If this guy was being shown before, bring it up now.
		if (m_bCurVisible) {
			m_pActivateThisGuy = this;
		}
		break;
	case RP_REVIEW:
		if (m_bRPRead) {
			ActivateRPReview();
		}
		break;

	default:
		break;
	}

	return ec;
}

ErrorCode CBagRPObject::Detach() {
	// Turn off our current object
	if (m_pCurRPObject == this) {
		m_pCurRPObject = nullptr;
	}

	// If we're in the current list, then remove us!
	if (m_pRPList) {
		int nCount = m_pRPList->GetCount();
		for (int i = 0; i < nCount; i++) {
			CBagObject *pObj = m_pRPList->GetNodeItem(i);

			if (pObj == this) {
				m_pRPList->Remove(i);
				break;
			}
		}

		if (m_pRPList->GetCount() == 0) {
			delete m_pRPList;
			m_pRPList = nullptr;
		}
	}

	m_bInitialized = false;

	// Don't really need to do anything here since the destructor will trash the
	// entire list.
	return CBagObject::Detach();
}

// Have a dossier name and expression for a touched object
void CBagRPObject::SetTouchedDos(CBofString &s, CBagExpression *x) {
	//  Make sure the list has been allocated
	if (m_pTouchedList == nullptr) {
		m_pTouchedList = new CBofList<DossierObj *>;
	}

	Assert(m_pTouchedList != nullptr);

	DossierObj *pDosObj = new DossierObj();
	Assert(pDosObj != nullptr);

	// Just store the name for now, we'll get the pointer to the dossier in
	// the attach code.
	pDosObj->m_sDossier = s;
	pDosObj->m_xDosExp = x;

	m_pTouchedList->AddToTail(pDosObj);
}

// Store a dossier name and associated expression for an untouched object
void CBagRPObject::SetUntouchedDos(CBofString &s, CBagExpression *x) {
	//  Make sure the list has been allocated
	if (m_pUntouchedList == nullptr) {
		m_pUntouchedList = new CBofList<DossierObj *>;
	}

	Assert(m_pUntouchedList != nullptr);

	DossierObj *pDosObj = new DossierObj();
	Assert(pDosObj != nullptr);

	// Store the expression and the dossier string.
	pDosObj->m_sDossier = s;
	pDosObj->m_xDosExp = x;

	m_pUntouchedList->AddToTail(pDosObj);
}

// This static is the tough guy that is in charge of checking the rp queue for
// any objects that have results that should be returned.
int CBagRPObject::RunRPQueue() {
	// Might get called with no residue printing list
	if (m_pRPList == nullptr) {
		return 0;
	}

	// The first part of this static should only run if the log is frontmost.
	if (SBBasePda::GetPDAMode() == LOGMODE) {
		// If our script switched us to the main menu, then make sure that we
		// have a chance to deactivate anything that we have active.
		RPSTATES prevRPState = m_eRPMode;
		RPSTATES curRPState = GetLogState();
		if (prevRPState != curRPState && curRPState == RP_MAINMENU) {
			if (m_pCurRPObject) {
				m_pCurRPObject->DeactivateRPObject();
				m_pCurRPObject->m_bCurVisible = false;
				m_pCurRPObject->SaveRPVars();
			}
		}

		// Check the first guy in the queue to see if he's been initialized, if not,
		// then cruise the whole thing.
		int nCount = m_pRPList->GetCount();
		if (nCount == 0) {
			return 0;
		}

		CBagRPObject *pRPObj = m_pRPList->GetNodeItem(0);
		for (int i = 0; i < nCount; i++) {
			pRPObj = m_pRPList->GetNodeItem(i);

			// This could fail if we are not initialized properly
			if (pRPObj->m_bInitialized == false) {
				pRPObj->m_bInitialized = pRPObj->initialize();
			}
		}

		// If we have an activate set, then we want to get this guy up as soon as possible,
		// he has been completely constructed and attached and all that cool stuff, this was
		// the object that was frontmost when we went into a flashback, saved a game, etc...
		//
		// Don't activate until we've been properly initialized.
		if (m_pActivateThisGuy) {
			if (m_pActivateThisGuy->m_bInitialized) {
				RPSTATES rpState = GetLogState();
				switch (rpState) {
				case RP_REVIEW:
					if (m_pActivateThisGuy->m_bRPRead) {
						m_pActivateThisGuy->ActivateRPReview();
					}
					break;

				case RP_RESULTS:
					m_pActivateThisGuy->ActivateRPObject();
					break;

				case RP_READ_DOSSIER: {
					CBagDossierObject *pDObj = m_pActivateThisGuy->GetActiveDossier();
					if (pDObj) {
						pDObj->ShowDosText();
						// Special case, make sure the trail back to the rp obj is clearly marked
						pDObj->SetRPObj(m_pActivateThisGuy);
					}
					break;
				}

				default:
					break;
				}
				m_pActivateThisGuy = nullptr;
			}
		}

		// If our last update is zero, then we have to parouse our list and fill in all the
		// missing vars.  This is considered our initialization trigger.
		if (m_nLastRPQCheck == 0) {
			DeactivateRPReview();
		}
	}

	// Be semi-efficient about this and only execute every 5 seconds or so.

	int32 nCurTime = GetTimer();
	if (nCurTime > m_nLastRPQCheck + 5000) {

		// Get the turncount variable.
		if (m_pTurncount == nullptr) {
			m_pTurncount = VARMNGR->GetVariable("TURNCOUNT");
		}
		//
		Assert(m_pTurncount != nullptr);

		// Get the current time
		int nCurSBTime = m_pTurncount->GetNumValue();
		int nCount = m_pRPList->GetCount();

		for (int i = 0; i < nCount; i++) {
			CBagRPObject *pRPObj = m_pRPList->GetNodeItem(i);

			// Find out if there are any events worth reporting.
			//
			// Remove the addition of 20, that's already been added in the script.
			if (pRPObj->m_bRPReported == false && pRPObj->m_nRPTime != 0 && nCurSBTime >= pRPObj->m_nRPTime) {
				AddToMsgQueue(pRPObj);
			}
		}

		if (GetLogState() == RP_REVIEW) {
			SetLogPages(99);
		}

		m_nLastRPQCheck = nCurTime;
	}

	return 0;
}

// This static will cruise through the whole queue, check the value of each
// associated variable, find a non-null one and activate the return time associated
// with that residue print request.
int CBagRPObject::UpdateRPQueue() {
	Assert(m_pRPList != nullptr);

	int nCount = m_pRPList->GetCount();
	char szLocalBuff[256];
	CBofString cStr(szLocalBuff, 256);

	// Cruise through and find if we have one that has had it's value changed.
	for (int i = 0; i < nCount; i++) {
		CBagRPObject *pRPObj = m_pRPList->GetNodeItem(i);
		if (pRPObj) {
			cStr = pRPObj->m_pVarObj->GetValue();

			// If it's value is NOT 3000, then store its value in the associative
			// rp object.
			if (cStr.Find("3000") == -1) {
				int nTimecount = atoi(szLocalBuff);
				pRPObj->m_nRPTime = nTimecount;
				pRPObj->m_bRPReported = false;
				pRPObj->m_bResPrinted = true;

				// Evaluate the expression as we submit it for printing, this will
				// guarantee that the expressions are reported as a function of the
				// time that they were submitted.  Trust me, this statement makes
				// perfect sense.
				pRPObj->EvaluateDossiers();
				pRPObj->SaveRPVars();
				// Reset back to "3000"

				cStr = "3000";
				pRPObj->m_pVarObj->SetValue(cStr);
			}
		}
	}

	return 0;
}

// This static runs through the entire chain and makes sure that everything
// has been deactivated and is not displayed to the PDA.  Called from
// script.
void CBagRPObject::DeactivateRPQueue() {
	Assert(m_pRPList != nullptr);

	int nCount = m_pRPList->GetCount();

	// Cruise through and find if we have one that has had it's value changed.
	for (int i = 0; i < nCount; i++) {
		CBagRPObject *pRPObj = m_pRPList->GetNodeItem(i);
		if (pRPObj) {
			pRPObj->DeactivateRPObject();
		}
	}

	DeactivateRPReview();
}

// Add this guy to the message queue and make that little message
// light blink away!
bool CBagRPObject::AddToMsgQueue(CBagRPObject *pRPObj) {
	bool bAddedToQueue = false;

	// If zoomed, then don't add to the message queue!!!!
	if (Zoomed()) {
		return false;
	}

	CBagLog *pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);

	if (pLogWld) {
		pRPObj->SetMsgWaiting(true); // mark this guy as waiting
		pLogWld->ActivateLocalObject(pRPObj);

		pRPObj->SetVisible(true);     // make sure it gets updated.
		pRPObj->m_bRPRead = false;    // hasn't been read yet
		pRPObj->m_bRPReported = true; // been reported though
		pRPObj->SaveRPVars();         // Update the variable store
		bAddedToQueue = true;
	}

	return bAddedToQueue;
}

// If we get called to run this guy, then run the movie and post the results,
// if any.
bool CBagRPObject::RunObject() {
	bool bMoviePlayed = false;

	// If we have a movie, which we should, run it!
	//only play the movie the first time through!
	bool bLocalMoviePlayed = false;
	if (m_pMovieObj && m_bMoviePlayed == false) {
		bMoviePlayed = m_pMovieObj->RunObject();
		m_bMoviePlayed = true;
		bLocalMoviePlayed = true;
	}

	// Order is important here, if we end up activating the RP review, then
	// we're going to need this guy marked as read.

	m_bRPRead = true;
	SaveRPVars(); // Update the variable store

	// Make sure that nothing from a previous res print is showing
	if (m_pCurRPObject) {
		m_pCurRPObject->DeactivateRPObject();
		m_pCurRPObject->m_bCurVisible = false;
		m_pCurRPObject->SaveRPVars();
		m_pCurRPObject = nullptr;
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
	if (bLocalMoviePlayed == true && !Zoomed() && RPResultsWaiting() >= 1) {

		// Remove all these from the message waiting queue.
		RemoveAllFromMsgQueue(this);

		ActivateRPReview();
	} else {
		ActivateRPObject();
	}

	return bMoviePlayed;
}

// This routine just displays results for a single object.  We still need to
// implement cascading RP results.
#define REVIEWDOSZ "REVIEWDOSZ"
#define REVIEWDOS "REVIEWDOS"

ErrorCode CBagRPObject::Update(CBofBitmap *pBmp, CBofPoint /*pt*/, CBofRect * /*pSrcRect*/, int) {
	if (GetLogState() == RP_RESULTS) {
		if (m_pDescObj) {
			CBofRect txRect = m_pDescObj->GetRect();
			CBofPoint txPt = txRect.TopLeft();
			m_pDescObj->Update(pBmp, txPt, &txRect, 0);
		}
	}

	return ERR_NONE;
}

bool CBagRPObject::ActivateRPObject() {
	// If there's one already activated, then deactivate it.  Don't want
	// two of these drawn on top of each other.
	if (m_pCurRPObject && m_pCurRPObject != this) {
		m_pCurRPObject->DeactivateRPObject();
		m_pCurRPObject->m_bCurVisible = false;
		m_pCurRPObject->SaveRPVars();
	}

	// Set current and make sure our variable knows that this one is being shown,
	// this is important for snapping from zoom to regular and vice-versa.
	m_pCurRPObject = this;
	m_bCurVisible = true;
	SaveRPVars();

	// Make sure this guy is active and ready to get drawn.
	SetVisible(); // show this guy
	SetActive();  // accept mouse downs

	CBagLog *pLogWld;
	if (Zoomed()) {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGZWLD);
	} else {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);
	}

	if (pLogWld == nullptr) {
		return false;
	}

	// If we're coming from residue print review, make sure none of those objects
	// are visible.
	if (GetLogState() == RP_REVIEW) {
		HideRPReview();
	}

	// We're ready to display a RP result, so switch to results mode
	if (GetLogState() != RP_RESULTS) {
		SetLogState(RP_RESULTS);
	}
	SetLogPages(1);

	// Now go through each one and make sure we have the backp
	CBofList<DossierObj *> *pDosList = (m_bTouched ? m_pTouchedList : m_pUntouchedList);
	int nCount = pDosList->GetCount();

	for (int i = 0; i < nCount; i++) {
		DossierObj *pDosObj = pDosList->GetNodeItem(i);

		// By default, include the dossier in the list
		if (pDosObj->m_bDisplayDossier) {
			pDosObj->m_pDossier->ActivateDosObject(pLogWld);
		}

		// Give it a back pointer so that it can find the parent res print object
		//
		// We must do this here as this will point to the object that just got
		// residue printed.
		pDosObj->m_pDossier->SetRPObj(this);
	}

	ShowPDALog();

	// Attach the description object.
	Assert(m_pDescObj != nullptr);

	// This object might not be attached since it is not a local object in the
	// log storage device
	if (!m_pDescObj->IsAttached()) {
		m_pDescObj->Attach();
	}
	m_pDescObj->SetVisible(); // Show this guy

	return true;
}

void CBagRPObject::DeactivateRPObject() {
	CBagLog *pLogWld;

	// Get the appropriate storage device
	if (Zoomed()) {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGZWLD);
	} else {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);
	}

	// Can't bloody well do much without our storage device
	if (pLogWld == nullptr) {
		return;
	}

	// Make sure that all our objects that we care about are no longer active
	int nCount = m_pTouchedList->GetCount();
	DossierObj *pDosLObj;

	for (int i = 0; i < nCount; i++) {
		pDosLObj = m_pTouchedList->GetNodeItem(i);
		pDosLObj->m_pDossier->DeactivateDosObject(pLogWld);
	}

	if (m_pTouchedList != m_pUntouchedList) {
		nCount = m_pUntouchedList->GetCount();

		for (int i = 0; i < nCount; i++) {
			pDosLObj = m_pUntouchedList->GetNodeItem(i);
			pDosLObj->m_pDossier->DeactivateDosObject(pLogWld);
		}
	}

	// Attach the description object.
	Assert(m_pDescObj != nullptr);

	if (m_pDescObj) {
		m_pDescObj->SetVisible(false);  // Hide this guy
		m_pDescObj->SetActive(false);   // Don't take mousedowns
		m_pDescObj->SetFloating(false); // Don't calculate floating rects
	}
	SetVisible(false); // Hide this guy

	return;
}

// This static will cruise the entire rpo object list, mark each one that has
// had a result returned as active and floater.  also, update the mode var
void CBagRPObject::ActivateRPReview() {
	// Make sure the log is frontmost
	if (GetLogState() != RP_REVIEW) {
		ShowPDALog();

		// Now in review mode, this is used in our update code to determine what to
		// show.
		SetLogState(RP_REVIEW);
	}

	SetLogPages(99);
	ShowRPReview();
}

void CBagRPObject::DeactivateRPReview() {
	CBagLog *pLogWld;

	// Get the appropriate storage device
	if (Zoomed()) {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGZWLD);
	} else {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);
	}

	// can't bloody well get anywhere without this info.
	Assert(pLogWld != nullptr);
	if (pLogWld == nullptr) {
		return;
	}

	// Now in review mode, this is used in our update code to determine what to show.
	CBagRPObject *pRPObj = m_pRPList->GetNodeItem(0);
	if (pRPObj == nullptr) {
		return;
	}

	// Cruise the whole list, fill in missing vars, set all our text
	// objects to inactive so they don't get drawn.
	int nCount = m_pRPList->GetCount();

	for (int i = 0; i < nCount; i++) {
		pRPObj = m_pRPList->GetNodeItem(i);

		// We know this object is attached, but it might not be visible.
		if (pRPObj->m_pObjectName) {
			pRPObj->m_pObjectName->SetFloating(false);
			pRPObj->m_pObjectName->SetVisible(false);
			pRPObj->m_pObjectName->SetActive(false);
		}
	}
}

void CBagRPObject::OnLButtonUp(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void * /*pv*/) {
	// Deactivate everything in the rp list
	HideRPReview();

	// We're ready to display a RP result, so switch to results mode
	SetLogState(RP_RESULTS);
	SetLogPages(1);

	RunObject();
}

void CBagRPObject::EvaluateDossiers() {

	// If we have a touched variable, then evaluate and if it is not 3000, then
	// we know it's been touched.
	if (m_pTouchedVar) {
		if (m_pTouchedVar->GetValue() != "3000") {
			m_bTouched = true;
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
	CBofList<DossierObj *> *pDosList = (m_bTouched ? m_pTouchedList : m_pUntouchedList);
	Assert(pDosList != nullptr);

	int nCount = pDosList->GetCount();
	CBagLog *pLogZWld = nullptr;

	for (int i = 0; i < nCount; i++) {
		DossierObj *pDosLObj = pDosList->GetNodeItem(i);

		// By default, include the dossier in the list
		pDosLObj->m_bDisplayDossier = true;
		if (pDosLObj->m_xDosExp != nullptr) {
			pDosLObj->m_bDisplayDossier = pDosLObj->m_xDosExp->EvalLeftToRight(false);
		}

		// If we are displaying this dossier and the suspect var is
		// there, then lookup the suspect var and let the voice printing code know
		// that this guy's DNA has shown up on some object.
		if (pDosLObj->m_bDisplayDossier &&
		        pDosLObj->m_pDossier &&
		        pDosLObj->m_pDossier->m_sSuspectVar.IsEmpty() == false) {

			if (pLogZWld == nullptr) {
				pLogZWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGZWLD);
			}

			if (pLogZWld != nullptr) {
				CBagLogSuspect *pSusObj = (CBagLogSuspect *)pLogZWld->GetObject(pDosLObj->m_pDossier->m_sSuspectVar);

				if (pSusObj != nullptr) {
					pSusObj->SetSusRP(true);
				}
			}
		}
	}
	SaveRPVars(); // Update the variable store
}

// This is really ugly, so put it all in the same place
void CBagRPObject::SetLogState(RPSTATES eLogMode) {
	// This is very, very, very hacky, but make sure that our script has
	// no power over what is going on, so set our log state to "CODE_RP_RESULTS".
	// Also, so we know where to return to, make sure we set the previous log
	// state to whatever LOG_STATE is right now.
	if (m_pLogStateVar == nullptr) {
		m_pLogStateVar = VARMNGR->GetVariable("LOG_STATE");
	}

	if (m_pPrevLogStateVar == nullptr) {
		m_pPrevLogStateVar = VARMNGR->GetVariable("PREV_LOG_STATE");
	}

	Assert(m_pLogStateVar != nullptr && m_pPrevLogStateVar != nullptr);

	if (m_pLogStateVar != nullptr && m_pPrevLogStateVar != nullptr) {
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

		if (m_eRPMode != RP_READ_DOSSIER) {
			if (bRemember) {
				m_pPrevLogStateVar->SetValue(m_pLogStateVar->GetValue());
			}
		}
		m_pLogStateVar->SetValue(cStr);

		if (bSavePage || bRestorePage) {
			if (m_pPrevBarPage == nullptr) {
				m_pPrevBarPage = VARMNGR->GetVariable("PREV_BAR_LOG_PAGE");
			}

			if (m_pCurBarPage == nullptr) {
				m_pCurBarPage = VARMNGR->GetVariable("CUR_BAR_LOG_PAGE");
			}

			if (m_pPrevBarPage && m_pCurBarPage) {
				if (bSavePage) {
					m_pPrevBarPage->SetValue(m_pCurBarPage->GetValue());
				}

				if (bRestorePage) {
					m_pCurBarPage->SetValue(m_pPrevBarPage->GetValue());
				}
			}
		}
	}

	m_eRPMode = eLogMode;
}

// Return the current log state
RPSTATES CBagRPObject::GetLogState() {
	if (m_pLogStateVar == nullptr) {
		m_pLogStateVar = VARMNGR->GetVariable("LOG_STATE");
	}

	m_eRPMode = RP_NOMODE;

	Assert(m_pLogStateVar != nullptr);
	if (m_pLogStateVar) {
		char szLocalBuff[256];
		CBofString cStr(szLocalBuff, 256);

		cStr = m_pLogStateVar->GetValue();
		if (cStr == "CODE_RP_RESULTS") {
			m_eRPMode = RP_RESULTS;
		} else if (cStr == "CODE_RP_DOSSIER") {
			m_eRPMode = RP_READ_DOSSIER;
		} else if (cStr == "RES_PRINT_REVIEW") {
			m_eRPMode = RP_REVIEW;
		} else if (cStr == "MAINMENU") {
			m_eRPMode = RP_MAINMENU;
		}
	}

	return m_eRPMode;
}

// Set the current number of pages in both script and code.
void CBagRPObject::SetLogPages(int nPages) {
	if (m_pBarLogPages == nullptr) {
		m_pBarLogPages = VARMNGR->GetVariable("CODE_TOTAL_LOG_PAGES");
	}

	Assert(m_pBarLogPages != nullptr);

	if (m_pBarLogPages) {
		m_pBarLogPages->SetValue(nPages);
	}

	// Let the float code know how many pages we have.
	CBagLog *pLogWld;
	if (Zoomed()) {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGZWLD);
	} else {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);
	}

	Assert(pLogWld != nullptr);
	if (pLogWld) {
		// Let the float code know how many pages we have.
		pLogWld->SetNumFloatPages(nPages);
		if (nPages == 1) {
			pLogWld->SetCurFltPage(1);
		}
	}
}

// Save the residue print variables

void CBagRPObject::SaveRPVars() {
	if (m_pSaveVar == nullptr) {
		return;
	}

	uint32 nVars = 0;

	for (int i = 0; i < NUM_RP_FIELDS; i++) {
		switch (i) {
		case 0:
			nVars = m_nRPTime & mRPTime;
			break;
		case 1:
			nVars |= (m_bRPReported ? mRPReported : 0);
			break;
		case 2:
			nVars |= (m_bResPrinted ? mResPrinted : 0);
			break;
		case 3:
			nVars |= (m_bTouched ? mTouched : 0);
			break;
		case 4:
			nVars |= (m_bRPRead ? mRPRead : 0);
			break;
		case 5:
			nVars |= (m_bCurVisible ? mRPCurVisible : 0);
			break;
		case 6:
			nVars |= ((m_nCurDossier & 0x3) << mRPCurDossierShift);
			break;
		case 7:
			nVars |= (m_bMoviePlayed ? mRPMoviePlayed : 0);
			break;
		case 8:
			nVars |= (m_bRPTimeSet ? mRPTimeSet : 0);
			break;
		}
	}

	// Now handle touched dossier's
	int nCount = m_pTouchedList->GetCount();
	Assert(nCount <= 3);
	DossierObj *pDosObj;

	if (m_pTouchedList) {
		for (int i = 0; i < nCount; i++) {
			pDosObj = m_pTouchedList->GetNodeItem(i);
			switch (i) {
			case 0:
				nVars |= (pDosObj->m_bDisplayDossier ? mRPTDos1Eval : 0);
				break;
			case 1:
				nVars |= (pDosObj->m_bDisplayDossier ? mRPTDos2Eval : 0);
				break;
			case 2:
				nVars |= (pDosObj->m_bDisplayDossier ? mRPTDos3Eval : 0);
				break;
			}
		}
	}

	// Now handle untouched dossier's
	nCount = m_pUntouchedList->GetCount();
	Assert(nCount <= 3);

	if (m_pUntouchedList) {
		for (int i = 0; i < nCount; i++) {
			pDosObj = m_pUntouchedList->GetNodeItem(i);
			switch (i) {
			case 0:
				nVars |= (pDosObj->m_bDisplayDossier ? mRPUDos1Eval : 0);
				break;
			case 1:
				nVars |= (pDosObj->m_bDisplayDossier ? mRPUDos2Eval : 0);
				break;
			case 2:
				nVars |= (pDosObj->m_bDisplayDossier ? mRPUDos3Eval : 0);
				break;
			}
		}
	}

	m_pSaveVar->SetValue(nVars);
}

// Restore the residue print variable from memory
void CBagRPObject::RestoreRPVars() {
	if (m_pSaveVar == nullptr) {
		return;
	}

	uint32 nVars = m_pSaveVar->GetNumValue();

	for (int i = 0; i < NUM_RP_FIELDS; i++) {
		switch (i) {
		case 0:
			m_nRPTime = nVars & mRPTime;
			break;
		case 1:
			m_bRPReported = (nVars & mRPReported ? true : false);
			break;
		case 2:
			m_bResPrinted = (nVars & mResPrinted ? true : false);
			break;
		case 3:
			m_bTouched = (nVars & mTouched ? true : false);
			break;
		case 4:
			m_bRPRead = (nVars & mRPRead ? true : false);
			break;
		case 5:
			m_bCurVisible = (nVars & mRPCurVisible ? true : false);
			break;
		case 6:
			(m_nCurDossier = (nVars & mRPCurDossier) >> mRPCurDossierShift);
			break;
		case 7:
			m_bMoviePlayed = (nVars & mRPMoviePlayed ? true : false);
			break;
		case 8:
			m_bRPTimeSet = (nVars & mRPTimeSet ? true : false);
			break;
		}
	}

	// Now handle touched dossier's
	int nCount = m_pTouchedList->GetCount();
	Assert(nCount <= 3);
	DossierObj *pDosObj;

	if (m_pTouchedList) {
		for (int i = 0; i < nCount; i++) {
			pDosObj = m_pTouchedList->GetNodeItem(i);
			switch (i) {
			case 0:
				pDosObj->m_bDisplayDossier = (nVars & mRPTDos1Eval ? true : false);
				break;
			case 1:
				pDosObj->m_bDisplayDossier = (nVars & mRPTDos2Eval ? true : false);
				break;
			case 2:
				pDosObj->m_bDisplayDossier = (nVars & mRPTDos3Eval ? true : false);
				break;
			}
		}
	}

	// Now handle untouched dossier's
	nCount = m_pUntouchedList->GetCount();
	Assert(nCount <= 3);

	if (m_pUntouchedList) {
		for (int i = 0; i < nCount; i++) {
			pDosObj = m_pUntouchedList->GetNodeItem(i);
			switch (i) {
			case 0:
				pDosObj->m_bDisplayDossier = (nVars & mRPUDos1Eval ? true : false);
				break;
			case 1:
				pDosObj->m_bDisplayDossier = (nVars & mRPUDos2Eval ? true : false);
				break;
			case 2:
				pDosObj->m_bDisplayDossier = (nVars & mRPUDos3Eval ? true : false);
				break;
			}
		}
	}
}

// Hide the list of rp results, don't purge them from memory, just set to not visible.
void CBagRPObject::HideRPReview() {
	CBagRPObject *pRPObj;
	CBagLog *pLogWld;

	if (Zoomed()) {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGZWLD);
	} else {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);
	}

	pRPObj = m_pRPList->GetNodeItem(0);
	if (pRPObj == nullptr) {
		return;
	}

	// Count items in this queue.
	int nCount = m_pRPList->GetCount();

	for (int i = 0; i < nCount; i++) {
		pRPObj = m_pRPList->GetNodeItem(i);

		// If the object txt var is not found yet, then get it.
		if (pRPObj->m_pObjectName == nullptr) {
			pRPObj->m_pObjectName = (CBagTextObject *)pLogWld->GetObject(pRPObj->m_sObjectName);
		}

		// We know this object is attached, but it might not be visible.
		if (pRPObj->m_pObjectName) {
			pRPObj->m_pObjectName->SetVisible(false);  // hide this guy
			pRPObj->m_pObjectName->SetFloating(false); // don't arrange floating pages
			pRPObj->m_pObjectName->SetActive(false);   // don't accept mouse downs
		}
	}
}

void CBagRPObject::ShowRPReview() {
	CBagLog *pLogWld;

	if (Zoomed()) {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGZWLD);
	} else {
		pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);
	}

	// Count items in this queue.
	int nCount = pLogWld->GetObjectCount();

	// Check the first guy in the queue to see if he's been initialized, if not,
	// then cruise the whole thing.
	for (int i = 0; i < nCount; i++) {
		CBagObject *pObj = pLogWld->GetObjectByPos(i);
		if (pObj && pObj->GetType() == RESPRNTOBJ) {

			CBagRPObject *pRPObj = (CBagRPObject *)pObj;
			// Find out if there are any events worth reporting.
			if (pRPObj->m_bResPrinted && pRPObj->m_bRPRead) {

				// We know this object is attached, but it might not be visible.
				if (pRPObj->m_pObjectName) {

					bool bIsAttached = pRPObj->m_pObjectName->IsAttached();
					if (bIsAttached == false) {
						pLogWld->ActivateLocalObject(pRPObj->m_pObjectName);
					}
					pRPObj->m_pObjectName->SetVisible();
					pRPObj->m_pObjectName->SetFloating();
					pRPObj->m_pObjectName->SetActive();

					// If the time on this object has not yet been set, then force it to
					// be re-read into memory and append the current time to it.
					if (pRPObj->m_bRPTimeSet == false) {
						CBofString s = pRPObj->m_pObjectName->GetText();

						int nHr = pRPObj->m_nRPTime / 100;
						int nMn = pRPObj->m_nRPTime - (nHr * 100);

						s += BuildString("%02d:%02d", nHr, nMn);

						pRPObj->m_pObjectName->SetText(s);
						pRPObj->m_bRPTimeSet = true;
						pRPObj->SaveRPVars();
					}
				}
			}
		}
	}
}

// Count the number of residue print objects waiting to be reported;
int CBagRPObject::RPResultsWaiting() {
	int nCount = m_pRPList->GetCount();
	int nWaiting = 0;

	for (int i = 0; i < nCount; i++) {
		CBagRPObject *pRPObj = m_pRPList->GetNodeItem(i);
		if (pRPObj->m_bRPReported && pRPObj->m_bRPRead == false) {
			nWaiting++;
		}
	}

	return nWaiting;
}

// Remove all residue print results from the message queue
void CBagRPObject::RemoveAllFromMsgQueue(CBagRPObject *pCurRPObj) {
	// we really only care about the log world, not the logz.
	CBagLog *pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);
	Assert(pLogWld != nullptr);
	int nCount = m_pRPList->GetCount();

	for (int i = 0; i < nCount; i++) {
		CBagRPObject *pRPObj = m_pRPList->GetNodeItem(i);

		if (pRPObj->m_bRPReported && pRPObj->m_bRPRead == false && pRPObj != pCurRPObj) {
			pLogWld->RemoveFromMsgQueue(pRPObj);

			pRPObj->m_bRPRead = true;
			pRPObj->m_bMoviePlayed = true; // Don't want this guys movie again
			pRPObj->SaveRPVars();          // Update the variable store
		}
	}
}

//  We're going to need to switch the PDA to log mode.
void CBagRPObject::ShowPDALog() {
	if (Zoomed()) {
		SBZoomPda *pZoomPDA = (SBZoomPda *)SDEVMNGR->GetStorageDevice(PDAZWLD);
		if (pZoomPDA) {
			pZoomPDA->ShowLog();
		}
	} else {
		CBagPDA *pPDA = (CBagPDA *)SDEVMNGR->GetStorageDevice(PDAWLD);
		if (pPDA) {
			pPDA->ShowLog();
		}
	}
}

bool CBagRPObject::Zoomed() {
	SBZoomPda *pPDA = (SBZoomPda *)SDEVMNGR->GetStorageDevice(PDAZWLD);
	if (pPDA == nullptr) {
		return false;
	}

	return pPDA->GetZoomed();
}

bool CBagRPObject::initialize() {
	// Cruise the dossier's for both lists and get pointers to the actual bagdoobj's.
	// Search the current storage device for this object.
	CBagStorageDev *pSDev;
	DossierObj *pDosObj;

	if (Zoomed()) {
		pSDev = SDEVMNGR->GetStorageDevice(LOGZWLD);
	} else {
		pSDev = SDEVMNGR->GetStorageDevice(LOGWLD);
	}
	Assert(pSDev != nullptr);

	// Scoff the dossier out of the LOG_WLD SDEV.  If it's not there then hurl.
	bool bDoUntouched = (m_pTouchedList != m_pUntouchedList);
	int nCount = m_pTouchedList->GetCount();

	for (int i = 0; i < nCount; i++) {
		pDosObj = m_pTouchedList->GetNodeItem(i);
		pDosObj->m_pDossier = (CBagDossierObject *)pSDev->GetObject(pDosObj->m_sDossier);
		if (pDosObj->m_pDossier == nullptr) {
			return false;
		}
		// Give it a back pointer so that it can find the parent res print object
		//
		// We must do this here as this will point to the object that just got
		// residue printed.
		pDosObj->m_pDossier->SetRPObj(this);
	}

	if (bDoUntouched) {
		nCount = m_pUntouchedList->GetCount();
		for (int i = 0; i < nCount; i++) {
			pDosObj = m_pUntouchedList->GetNodeItem(i);
			pDosObj->m_pDossier = (CBagDossierObject *)pSDev->GetObject(pDosObj->m_sDossier);
			if (pDosObj->m_pDossier == nullptr) {
				return false;
			}

			// Give it a back pointer so that it can find the parent res print object
			//
			// We must do this here as this will point to the object that just got
			// residue printed.
			pDosObj->m_pDossier->SetRPObj(this);
		}
	}

	// Fill in the movie object if we have one.
	if (!m_sMovieObj.IsEmpty()) {
		m_pMovieObj = (CBagMovieObject *)pSDev->GetObject(m_sMovieObj);
		if (m_pMovieObj == nullptr) {
			return false;
		}
	}

	// Fill in the object name if we have one.
	if (m_pObjectName == nullptr) {
		m_pObjectName = (CBagTextObject *)pSDev->GetObject(m_sObjectName);
		if (m_pObjectName == nullptr) {
			return false;
		}
	}

	// If we have a object name, make sure it is not active.  Object name is the
	// line that shows up in the RP Review screen (such as "Voice Printer")
	if (m_pObjectName) {
		// Give the dossier a back pointer so that it can respond to
		// mouse down events
		m_pObjectName->SetRPObject(this);

		m_pObjectName->SetVisible(false);
		m_pObjectName->SetActive(false);
		m_pObjectName->SetFloating(false);
	}

	return true;
}

// This hack is used to make sure that any variable values that were altered by
// the zoom pda are propagated down to the regular PDA.
void CBagRPObject::SynchronizeRPObjects(bool bLogFrontmost) {
	// only synchronize in the bar
	CBagVar *pVar = VARMNGR->GetVariable("INBAR");
	if (pVar == nullptr) {
		return;
	}

	CBagLog *pLogWld = (CBagLog *)SDEVMNGR->GetStorageDevice(LOGWLD);
	if (pLogWld == nullptr) {
		return;
	}

	RPSTATES rpState = GetLogState();
	int nCount = pLogWld->GetObjectCount();

	// Check the first guy in the queue to see if he's been initialized, if not,
	// then cruise the whole thing.
	for (int i = 0; i < nCount; i++) {
		CBagObject *pObj = pLogWld->GetObjectByPos(i);
		if (pObj->GetType() == RESPRNTOBJ) {
			CBagRPObject *pRPObj = (CBagRPObject *)pObj;
			pRPObj->RestoreRPVars();

			if (bLogFrontmost) {
				switch (rpState) {
				case RP_NOMODE:
					break;
				case RP_RESULTS:
					// If this guy was being shown before, bring it up now.
					if (pRPObj->m_bCurVisible) {
						m_pActivateThisGuy = pRPObj;
					}
					break;
				case RP_READ_DOSSIER:
					if (pRPObj->m_bCurVisible) {
						m_pActivateThisGuy = pRPObj;
					}
					break;
				case RP_REVIEW:
					if (pRPObj->m_bRPRead) {
						pRPObj->ActivateRPReview();
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
CBagDossierObject *CBagRPObject::GetActiveDossier() {
	CBofList<DossierObj *> *pDosList = (m_bTouched ? m_pTouchedList : m_pUntouchedList);

	return pDosList->GetNodeItem(m_nCurDossier)->m_pDossier;
}

// Used to set the currently active dossier when one is displayed to the user.
void CBagRPObject::SetActiveDossier(CBagDossierObject *pDosObj) {
	CBofList<DossierObj *> *pDosList = (m_bTouched ? m_pTouchedList : m_pUntouchedList);
	DossierObj *p = nullptr;

	int nCount = pDosList->GetCount();

	for (int i = 0; i < nCount; i++) {
		p = pDosList->GetNodeItem(i);
		if (p->m_pDossier == pDosObj) {
			m_nCurDossier = i;
			SaveRPVars();
			break;
		}
	}
}

// Initialize all dossier fields
DossierObj::DossierObj() {
	m_pDossier = nullptr;
	m_sDossier = "";
	m_xDosExp = nullptr;
	m_bDisplayDossier = false;
}

// Remove all allocated dossier objects
DossierObj::~DossierObj() {
	// Don't delete dossier's, we got those from the storage dev manager.
	m_pDossier = nullptr;

	// Expressions, however, we do own
	if (m_xDosExp) {
		delete m_xDosExp;
		m_xDosExp = nullptr;
	}
}

} // namespace Bagel
