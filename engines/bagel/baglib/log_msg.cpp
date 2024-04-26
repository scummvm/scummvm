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

#include "bagel/baglib/log_msg.h"
#include "bagel/baglib/button_object.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/boflib/sound.h"
#include "bagel/boflib/gui/movie.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/pda.h"
#include "bagel/boflib/gfx/text.h"
#include "bagel/boflib/log.h"

namespace Bagel {

CBagLog *CBagLog::m_bLastFloatPage;

#define LOGBORDER (42)
#define LOGZBORDER (84)

#define OVERRIDESMK             "$SBARDIR\\BAR\\LOG\\OVERRIDE.SMK"
#define OVERRIDEMOVIE           "OVERRIDE_MOVIE"

CBagLog::CBagLog() : CBagStorageDevBmp() {
	m_pQueued_Msgs = new CBofList<CBagObject *>;
	SetCurFltPage(1);
}

CBagLog::~CBagLog() {
	if (m_pQueued_Msgs != nullptr) {
		ReleaseMsg();       // Delete all master sprite objects
		delete m_pQueued_Msgs;
		m_pQueued_Msgs = nullptr;
	}
}

CBofPoint CBagLog::ArrangeFloater(CBofPoint nPos, CBagObject *pObj) {
	CBofPoint   NextPos = nPos;

	// Things are so convoluted now, it is entirely  possible that this method
	// will get called on a storage device that is not the current one,
	// so we will not require a backdrop.

	if (GetBackground() != nullptr) {
		CBofString sDevName = GetName();
		int nBorderSize = 0;

		// Get this from script, allows individual log states to set border.
		CBagVar *pVar = VARMNGR->GetVariable("LOGBORDER");
		if (pVar != nullptr) {
			nBorderSize = pVar->GetNumValue();
		}

		if (nBorderSize == 0) {
			if (sDevName == "LOG_WLD") {
				nBorderSize = LOGBORDER;
			} else if (sDevName == "LOGZ_WLD") {
				nBorderSize = LOGZBORDER;
			}
		}

		// The log window has instructional text at the top and bottom, so
		// create a floater rect to stay in the middle area
		CBofRect xFloatRect = GetBackground()->GetRect();
		xFloatRect.top += nBorderSize;
		xFloatRect.bottom -= (nBorderSize / 2);

		// calculate what page the whole object belongs on
		int nPageNum = ((NextPos.y + pObj->getRect().Height()) / xFloatRect.Height());
		// page numbering is 1-N
		nPageNum++;
		SetNumFloatPages(nPageNum);

		int nTotalPages = GetCurFltPage();
		// Now position this object int the sdev
		// if it fell on this page, show it
		if (nPageNum == nTotalPages) {
			CBofPoint xPagePos = NextPos;
			// Bring the current page into view
			xPagePos.y = xPagePos.y - ((nPageNum - 1) * xFloatRect.Height());
			// Add in the border
			xPagePos.y += nBorderSize;
			pObj->SetPosition(xPagePos);
		} else {
			// Set the position to be off the sdev, so it won't show
			pObj->SetPosition(CBofPoint(NextPos.x, GetBackground()->Height() + 1));
		}

		// Calculate the position for the next floater
		// This will get sent back to the calling func
		NextPos.x += pObj->getRect().Width();

		// Check to see if the whole object can fit in width, if it can't wrap
		if (NextPos.x > (xFloatRect.Width() - pObj->getRect().Width())) {
			NextPos.x = 0;
			NextPos.y += pObj->getRect().Height();
		}
	}

	m_bLastFloatPage = this;
	return NextPos;
}

void CBagLog::ArrangePages() {
	// Don't bother if we don't have a floater worth arranging...
	if (m_bLastFloatPage == nullptr) {
		return;
	}

	CBagObject *pUpObj = nullptr;
	CBagObject *pDownObj = nullptr;

	CBagLog *pLastFloat = m_bLastFloatPage;

	// Get the up button and the down button...
	pUpObj = pLastFloat->GetObject("LOGPAGUP");
	pDownObj = pLastFloat->GetObject("LOGPAGDOWN");

	if (pUpObj == nullptr || pDownObj == nullptr) {
		return;
	}

	// Get current page number and last page number
	int nLastPage = pLastFloat->GetNumFloatPages();
	int nCurPage = pLastFloat->GetCurFltPage();
	int nFirstPage = 1;

	if (nCurPage > nFirstPage && nCurPage < nLastPage) {
		if (pUpObj->IsAttached() == false) {
			pUpObj->SetActive();
			pUpObj->attach();
		}
		if (pDownObj->IsAttached() == false) {
			pDownObj->SetActive();
			pDownObj->attach();
		}
	} else if (nCurPage == nFirstPage && nCurPage == nLastPage) {
		if (pUpObj->IsAttached()) {
			pUpObj->SetActive(false);
			pUpObj->detach();
		}
		if (pDownObj->IsAttached()) {
			pDownObj->SetActive(false);
			pDownObj->detach();
		}
	} else if (nCurPage <= nFirstPage) {
		if (pUpObj->IsAttached()) {
			pUpObj->SetActive(false);
			pUpObj->detach();
		}
		if (pDownObj->IsAttached() == false) {
			pDownObj->SetActive();
			pDownObj->attach();
		}
	} else if (nCurPage >= nLastPage) {
		if (pUpObj->IsAttached() == false) {
			pUpObj->SetActive();
			pUpObj->attach();
		}

		if (pDownObj->IsAttached()) {
			pDownObj->SetActive(false);
			pDownObj->detach();
		}
	}

	// Reinitialize
	m_bLastFloatPage = nullptr;
}

int CBagLog::GetCurFltPage() {
	int nCurFltPage = 0;

	// Read in their total nuggets from game
	CBagVar *pVar = VARMNGR->GetVariable("CUR_BAR_LOG_PAGE");

	if (pVar) {
		nCurFltPage = pVar->GetNumValue();
	}

	return nCurFltPage;
}

void CBagLog::SetCurFltPage(int nFltPage) {
	// Read in their total nuggets from game
	CBagVar *pVar = VARMNGR->GetVariable("CUR_BAR_LOG_PAGE");

	if (pVar)
		pVar->SetValue(nFltPage);
}

ErrorCode CBagLog::ReleaseMsg() {
	ErrorCode errCode = ERR_NONE;
	int nCount = m_pQueued_Msgs->GetCount();

	if (nCount) {
		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = m_pQueued_Msgs->RemoveHead();

			// This is waiting to be played, mark it in memory as such, the fixes
			// get uglier and uglier... since zoomed pda doesn't have a message light,
			// only set this thing as waiting if we are in the regular PDA,
			// otherwise, we get superfluous blinking of the PDA light.
			CBofString  sDevName = GetName();
			if (sDevName == "LOG_WLD") {
				pObj->SetMsgWaiting(true);
			}
		}
	}

	m_pQueued_Msgs->RemoveAll();
	return errCode;
}

CBagObject *CBagLog::OnNewUserObject(const CBofString &sInit) {
	CBagTextObject *LogObj = nullptr;
	CBofRect cSDevRect = getRect();
	CBofString  sDevName = GetName();
	int     nPntSize = 10;

	if (sDevName == "LOG_WLD")
		nPntSize = FONT_8POINT;
	else if (sDevName == "LOGZ_WLD")
		nPntSize = FONT_18POINT;

	if (sInit == "MSG") {
		LogObj = (CBagTextObject *)new CBagLogMsg(cSDevRect.Width());
		LogObj->SetInitInfo(sInit);
		LogObj->SetPointSize(nPntSize);
		LogObj->SetColor(7);
		LogObj->SetFloating();
	} else if (sInit == "SUS") {
		LogObj = (CBagTextObject *)new CBagLogSuspect(cSDevRect.Width());
		LogObj->SetInitInfo(sInit);

		// Reduce point size on zoompda suspect list, make it
		// all fit in the zoompda window.
		if (nPntSize == FONT_18POINT) {
			nPntSize -= 2;
		}
		LogObj->SetPointSize(nPntSize);
		LogObj->SetColor(7);
		LogObj->SetFloating();
	} else if (sInit == "CLU") {
		LogObj = (CBagTextObject *)new CBagLogClue(sInit, cSDevRect.Width(), nPntSize);
	} else if (sInit == "RES") {
		LogObj = (CBagTextObject *)new CBagLogResidue(cSDevRect.Width());
		LogObj->SetInitInfo(sInit);
		LogObj->SetPointSize(nPntSize);
		LogObj->SetColor(7);
		LogObj->SetFloating();
	}

	return LogObj;
}

bool CBagLog::RemoveFromMsgQueue(CBagObject *pRemObj) {
	bool bRemoved = false;
	int nCount = m_pQueued_Msgs->GetCount();

	for (int i = 0; i < nCount; i++) {
		CBagObject *pObj = m_pQueued_Msgs->GetNodeItem(i);

		if (pObj == pRemObj) {
			m_pQueued_Msgs->Remove(i);
			bRemoved = true;
			break;
		}
	}

	return bRemoved;
}

ErrorCode CBagLog::ActivateLocalObject(CBagObject *pObj) {
	ErrorCode errCode = ERR_NONE;

	if (pObj != nullptr) {

		if (pObj->IsMsgWaiting() ||
		        (pObj->GetType() == USEROBJ && (pObj->GetInitInfo() != nullptr) && (*pObj->GetInitInfo() == "MSG"))) {
			m_pQueued_Msgs->AddToTail(pObj);

			// Since zoomed pda doesn't  have a message light, only set this thing
			// as waiting if we are in the  regular PDA, otherwise, we get superfluous
			// blinking of the PDA light.
			CBofString  sDevName = GetName();
			if (sDevName == "LOG_WLD") {
				pObj->SetMsgWaiting(true);
			}

			CBagButtonObject *pMsgLight = nullptr;
			CBagStorageDev *pPda = nullptr;

			pPda = SDEVMNGR->GetStorageDevice("BPDA_WLD");

			if (pPda) {
				pMsgLight = (CBagButtonObject *)pPda->GetObject("MSGLIGHT");

				if (pMsgLight) {
					if (!pMsgLight->IsAttached()) {
						pMsgLight->SetActive();
						pMsgLight->attach();
					}

					// Make sure this guy always gets updated regardless of its
					// dirty bit.
					pMsgLight->SetAlwaysUpdate(true);
					pMsgLight->SetAnimated(true);
				}
			}
		} else {
			errCode = CBagStorageDev::ActivateLocalObject(pObj);
		}
	}

	return errCode;
}

ErrorCode CBagLog::PlayMsgQue() {
	ErrorCode errCode = ERR_NONE;
	int nCount = m_pQueued_Msgs->GetCount();

	// Walk through the message queue and play all the messages
	// Only play one message per click on the pda message light.
	if (nCount) {

		CBagStorageDev *pPda = nullptr;
		pPda = SDEVMNGR->GetStorageDevice("BPDA_WLD");

		// If we're in a closeup, then don't play the message!
		CBagStorageDev *pSDev = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev();
		bool bPlayMsg = true;

		if (pSDev != nullptr) {
			if (pSDev->IsCIC()) {
				bPlayMsg = false;

				char szLocalBuff[256];
				CBofString mStr(szLocalBuff, 256);

				mStr = OVERRIDESMK;
				MACROREPLACE(mStr);

				CBagMovieObject *pMovie = (CBagMovieObject *)GetObject(OVERRIDEMOVIE);
				if (pMovie) {
					if (pMovie->IsAttached() == false) {
						pMovie->attach();
						pMovie->SetVisible();
					}
					pMovie->RunObject();
				}
			}
		}

		// If we're playing a valid message (not the override message) then make sure
		// we remove it from the queue.
		if (bPlayMsg) {
			CBagObject *pObj = m_pQueued_Msgs->RemoveHead();

			if (pObj) {
				CRect  r = getRect();

				errCode = CBagStorageDev::ActivateLocalObject(pObj);
				CBagMenu *pObjMenu = pObj->GetMenuPtr();
				if (pObjMenu)
					pObjMenu->TrackPopupMenu(0, 0, 0, CBofApp::GetApp()->GetMainWindow(), nullptr, &r);
				pObj->RunObject();
				pObj->SetMsgWaiting(false);

				// Mark this guy as played...
				((CBagLogMsg *)pObj)->SetMsgPlayed(true);
			}

			// Although this might seem like a superfluous thing to do, but wait!
			// it is not!  the runobject call above can cause the number of objects in the
			// message queue to be decremented.
			nCount = m_pQueued_Msgs->GetCount();

			// Don't stop message light from blinking unless we're down to zero
			// messages in the queue.

			CBagButtonObject *pMsgLight = nullptr;

			if (pPda) {
				pMsgLight = (CBagButtonObject *)pPda->GetObject("MSGLIGHT");

				if (pMsgLight) {
					if (nCount) {
						pMsgLight->SetAnimated(true);
					} else {
						pMsgLight->SetAnimated(false);
					}
				}
			}
		}
	}

	return errCode;
}

CBagLogResidue::CBagLogResidue(int nSdevWidth) : CBagTextObject() {
	m_xObjType = USEROBJ;
	m_nSdevWidth = nSdevWidth;
	m_bTitle = true;
}

void CBagLogResidue::setSize(const CBofSize &xSize) {
	CBagTextObject::setSize(CBofSize(m_nSdevWidth, xSize.cy));
}

CBagLogMsg::CBagLogMsg(int nSdevWidth) : CBagTextObject() {
	m_xObjType = USEROBJ;
	m_nSdevWidth = nSdevWidth;
	m_bTitle = true;

	// Start all messages off as not played
	SetMsgPlayed(false);
}

void CBagLogMsg::setSize(const CBofSize &xSize) {
	CBagTextObject::setSize(CBofSize(m_nSdevWidth, xSize.cy));
}

PARSE_CODES CBagLogMsg::SetInfo(bof_ifstream &istr) {
	bool nObjectUpdated = false;
	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	char szLocalBuff2[256];
	CBofString s(szLocalBuff2, 256);

	while (!istr.eof()) {
		istr.EatWhite();
		
		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  SENDEE FRANK - Sets the sendee name of the message to FRANK
		//
		case 'S': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("SENDEE")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, s);

				// Replace any underscores with spaces
				s.ReplaceChar('_', ' ');

				SetMsgSendee(s);

				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//   TIME x- Sets the time of the message to xx:xx
		//
		case 'T': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("TIME")) {
				istr.EatWhite();
				char cNext = (char)istr.peek();
				int     nMsgTime = 0;
				if (Common::isDigit(cNext)) {
					GetIntFromStream(istr, nMsgTime);
				} else {
					GetAlphaNumFromStream(istr, m_sMsgTimeStr);
				}

				SetMsgTime(nMsgTime);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		default: {
			PARSE_CODES rc = CBagObject::SetInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
			break;
		}
		}
	}

	return PARSING_DONE;
}

void CBagLogMsg::SetProperty(const CBofString &sProp, int nVal) {
	if (!sProp.Find("TIME")) {
		SetMsgTime(nVal);
	} else if (!sProp.Find("PLAYED")) {
		SetMsgPlayed(nVal);
	}

	CBagObject::SetProperty(sProp, nVal);
}

int CBagLogMsg::GetProperty(const CBofString &sProp) {
	if (!sProp.Find("TIME"))
		return GetMsgTime();

	// Played requires a 1 or a 0 (don't use true or false).
	if (!sProp.Find("PLAYED")) {
		bool bPlayed = GetMsgPlayed();
		return (bPlayed ? 1 : 0);
	}

	return CBagObject::GetProperty(sProp);
}

ErrorCode CBagLogMsg::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int nMaskColor) {
	// We could use a variable here, translate it's value if that's the case.
	if (GetMsgTime() == 0) {
		CBagVar *pVar = VARMNGR->GetVariable(m_sMsgTimeStr);
		int nMsgTime = pVar->GetNumValue();
		SetMsgTime(nMsgTime);
	}

	int nMsgTime = GetMsgTime();
	int nHr = nMsgTime / 100;
	int nMn = nMsgTime - (nHr * 100);

	SetFont(FONT_MONO);
	SetText(BuildString("%-30s%02d:%02d", m_sMsgSendee.GetBuffer(), nHr, nMn));

	return CBagTextObject::Update(pBmp, pt, pSrcRect, nMaskColor);
}

ErrorCode CBagLogResidue::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int nMaskColor) {
	return CBagTextObject::Update(pBmp, pt, pSrcRect, nMaskColor);
}

CBagLogSuspect::CBagLogSuspect(int nSdevWidth) : CBagTextObject() {
	m_xObjType = USEROBJ;
	m_nSdevWidth = nSdevWidth;

	m_bTitle = true;

	// Need to save state info, set all to false.
	SetState(0);
}

PARSE_CODES CBagLogSuspect::SetInfo(bof_ifstream &istr) {
	bool nObjectUpdated = false;
	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	char szLocalBuff2[256];
	CBofString s(szLocalBuff2, 256);

	while (!istr.eof()) {
		istr.EatWhite();

		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  NAME FRANK - Sets the sendee name of the message to FRANK
		//
		case 'N': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("NAME")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, s);

				// Replace any underscores with spaces
				s.ReplaceChar('_', ' ');
				SetSusName(s);

				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		case 'S': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("SPECIES")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, s);

				// Replace any underscores with spaces
				s.ReplaceChar('_', ' ');

				SetSusSpecies(s);

				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		case 'R': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("ROOM")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, s);

				// Replace any underscores with spaces
				s.ReplaceChar('_', ' ');
				SetSusRoom(s);

				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		default: {
			PARSE_CODES rc = CBagObject::SetInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
			break;
		}
		}
	}

	return PARSING_DONE;
}
void CBagLogSuspect::SetProperty(const CBofString &sProp, int nVal) {
	if (!sProp.Find("ROOM")) {
		switch (nVal) {
		case 1:  // BAP
			SetSusRoom("Entry Vestibule");
			break;
		case 2:  // BBP
			SetSusRoom("Howdy Saloon");
			break;
		case 4:  // BDP
			SetSusRoom("Bar Area");
			break;
		case 5:  // BEP
			SetSusRoom("Dance Floor");
			break;
		case 6:  // BFP
			SetSusRoom("Dining Area");
			break;
		case 7:  // BGP
			SetSusRoom("Gambling Hall");
			break;
		case 10: // BJP
			SetSusRoom("Kitchen");
			break;
		case 23:  // BWP
			SetSusRoom("Outside Howdy Saloon");
			break;
		default:
			break;
		}
	} else {
		// Hack alert!  If our value is 2, then this means toggle the boolean!!!
		int bVal = false;
		if (nVal == 1)
			bVal = true;
		if (nVal == 0)
			bVal = false;
		if (nVal == 2)
			bVal = 2;

		if (!sProp.Find("CHECKED"))
			SetSusChecked(bVal == 2 ? !GetSusChecked() : bVal);
		else if (!sProp.Find("VP"))
			SetSusVP(bVal == 2 ? !GetSusVP() : bVal);
		else if (!sProp.Find("RP"))
			SetSusRP(bVal == 2 ? !GetSusRP() : bVal);
		else
			CBagObject::SetProperty(sProp, nVal);
	}
}


int CBagLogSuspect::GetProperty(const CBofString &sProp) {
	if (!sProp.Find("CHECKED"))
		return GetSusChecked();

	if (!sProp.Find("VP"))
		return GetSusVP();

	if (!sProp.Find("RP"))
		return GetSusRP();

	return CBagObject::GetProperty(sProp);
}


void CBagLogSuspect::setSize(const CBofSize &xSize) {
	CBagTextObject::setSize(CBofSize(m_nSdevWidth, xSize.cy));

}


ErrorCode CBagLogSuspect::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int nMaskColor) {
	char szSusChecked[256];
	CBofString sSusChecked(szSusChecked, 256);

	// Remove all the references to the jamming and voice printer state
	char szSusVP[256];
	char szSusRP[256];

	CBofString sSusVP(szSusVP, 256);
	CBofString sSusRP(szSusRP, 256);

	if (GetSusChecked())
		sSusChecked = "Y";
	else
		sSusChecked = "N";

	if (GetSusVP())
		sSusVP = "OK";
	else
		sSusVP = "?";

	if (GetSusRP())
		sSusRP = "Y";
	else
		sSusRP = "N";

	SetFont(FONT_MONO);

	SetText(BuildString(" %-5.5s %-17.17s %-12.12s %-20.20s %-4.4s %-4.4s",
	                    sSusChecked.GetBuffer(),
	                    m_sSusName.GetBuffer(),
	                    m_sSusSpecies.GetBuffer(),
	                    m_sSusRoom.GetBuffer(),
	                    sSusVP.GetBuffer(),
	                    sSusRP.GetBuffer()));

	return CBagTextObject::Update(pBmp, pt, pSrcRect, nMaskColor);
}

// Energy detector objects, this should be pretty straightforward.

CBagEnergyDetectorObject::CBagEnergyDetectorObject() {
	SetFont(FONT_MONO);				// Correct for spacing
	SetColor(7);					// Make it white
	SetFloating();					// Is definitely floating
	SetHighlight();					// Is highlight
	SetTitle();						// As title
	m_bTextInitialized = false;     // Not initialized yet
}

CBagEnergyDetectorObject::~CBagEnergyDetectorObject() {
}

PARSE_CODES CBagEnergyDetectorObject::SetInfo(bof_ifstream &istr) {
	bool nObjectUpdated = false;
	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	char szLocalBuff2[256];
	CBofString s(szLocalBuff2, 256);

	while (!istr.eof()) {
		istr.EatWhite();
		
		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  ZHAPS - NUMBER OF ZHAPS (ENERGY UNITS)
		//
		case 'Z': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("ZHAPS")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, s);

				m_sZhapsStr = s;

				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  CAUSE - REASON FOR ENERGY BURST
		//
		case 'C': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("CAUSE")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, s);

				m_sCauseStr = s;

				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//   TIME x- Sets the time of the message to xx:xx
		//
		case 'T': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("TIME")) {
				istr.EatWhite();
				(void)istr.peek();

				GetAlphaNumFromStream(istr, m_sEnergyTimeStr);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  SIZE n - n point size of the txt
		//
		case 'S': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr2(szLocalStr, 256);

			GetAlphaNumFromStream(istr, sStr2);

			if (!sStr2.Find("SIZE")) {
				istr.EatWhite();
				int n;
				GetIntFromStream(istr, n);
				SetPointSize(n);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr2);
			}
			break;
		}

		default: {
			PARSE_CODES rc = CBagObject::SetInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
			break;
		}
		}
	}

	return PARSING_DONE;
}

ErrorCode CBagEnergyDetectorObject::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int nMaskColor) {
	// Don't draw until we're attached
	if (IsAttached() == false) {
		return ERR_NONE;
	}

	return CBagTextObject::Update(pBmp, pt, pSrcRect, nMaskColor);
}

ErrorCode CBagEnergyDetectorObject::attach() {
	Assert(IsValidObject(this));

	int nMsgTime;
	char szLocalBuff[256];
	CBofString causeStr(szLocalBuff, 256);

	char szZhapsBuff[256];
	CBofString zStr(szZhapsBuff, 256);

	// We could use a variable here, translate it's value if that's the case.
	CBagVar *pVar = VARMNGR->GetVariable(m_sEnergyTimeStr);
	if (pVar) {
		nMsgTime = pVar->GetNumValue();
	} else {
		nMsgTime = atoi(m_sEnergyTimeStr.GetBuffer());
	}

	int nHr = nMsgTime / 100;
	int nMn = nMsgTime - (nHr * 100);

	// Get the number of zhaps.
	pVar = VARMNGR->GetVariable(m_sZhapsStr);
	if (pVar) {
		zStr = pVar->GetValue();
	} else {
		zStr = m_sZhapsStr;
	}

	// Get the cause
	pVar = VARMNGR->GetVariable(m_sCauseStr);
	if (pVar) {
		causeStr = pVar->GetValue();
	} else {
		causeStr = m_sCauseStr;
	}

	// Replace any underscores with spaces
	causeStr.ReplaceChar('_', ' ');

	CBofString cStr;

	SetPSText(&cStr);

	SetText(BuildString("%02d:%02d %6.6s %s  %-35.35s", nHr, nMn, zStr.GetBuffer(), "zhaps", causeStr.GetBuffer()));
	RecalcTextRect(false);

	return CBagObject::attach();
}

CBagLogClue::CBagLogClue(const CBofString &sInit, int nSdevWidth, int nPointSize) : CBagTextObject() {
	m_xObjType = USEROBJ;
	m_nSdevWidth = nSdevWidth;

	m_bTitle = true;

	m_pStringVar1 = nullptr;
	m_pStringVar2 = nullptr;
	m_pStringVar3 = nullptr;
	m_pStringVar4 = nullptr;

	SetFont(FONT_MONO);
	SetInitInfo(sInit);
	SetPointSize(nPointSize);
	SetColor(7);
	SetFloating();
}

ErrorCode CBagLogClue::attach() {
	char szFormatStr[256];
	char szClueStr[256];
	CBofString cFormat(szFormatStr, 256);

	Assert(IsValidObject(this));

	ErrorCode ec = CBagTextObject::attach();

	// Get what is defined in the script.
	cFormat = GetFileName();

	// Replace '$' with '%' (% is an illegal character embedded in a clue string).
	cFormat.ReplaceChar('_', ' ');
	cFormat.ReplaceChar('$', '%');

	// Format the text appropriately.
	Common::sprintf_s(szClueStr, cFormat.GetBuffer(),
	                  (m_pStringVar1 ? (const char *)m_pStringVar1->GetValue() : (const char *)""),
	                  (m_pStringVar2 ? (const char *)m_pStringVar2->GetValue() : (const char *)""),
	                  (m_pStringVar3 ? (const char *)m_pStringVar3->GetValue() : (const char *)""),
	                  (m_pStringVar4 ? (const char *)m_pStringVar4->GetValue() : (const char *)""));

	CBofString cStr(szClueStr);
	SetPSText(&cStr);

	return ec;
}

PARSE_CODES CBagLogClue::SetInfo(bof_ifstream &istr) {
	bool nObjectUpdated = false;

	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	while (!istr.eof()) {
		istr.EatWhite();
		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  STRINGVAR - This will be a variable used to display some information that
		//  is contained in script in a clue statement.
		//
		case 'S': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("STRINGVAR")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				CBagVar *pVar = VARMNGR->GetVariable(sStr);
				// The variable must have been found, if it wasn't, then
				// complain violently.

				if (pVar == nullptr) {
					return UNKNOWN_TOKEN;
				}

				if (m_pStringVar1 == nullptr) {
					m_pStringVar1 = pVar;
				} else if (m_pStringVar2 == nullptr) {
					m_pStringVar2 = pVar;
				} else if (m_pStringVar3 == nullptr) {
					m_pStringVar3 = pVar;
				} else if (m_pStringVar4 == nullptr) {
					m_pStringVar4 = pVar;
				} else {
					return UNKNOWN_TOKEN;
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		default: {
			PARSE_CODES rc = CBagObject::SetInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
			break;
		}
		}

	}

	return PARSING_DONE;
}

ErrorCode CBagLogClue::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int nMaskColor) {
	return CBagTextObject::Update(pBmp, pt, pSrcRect, nMaskColor);
}

} // namespace Bagel
