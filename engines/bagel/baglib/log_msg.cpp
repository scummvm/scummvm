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
#include "bagel/baglib/pan_window.h"
#include "bagel/boflib/gfx/text.h"

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
	CBofRect    xFloatRect;
	int         nBorderSize = 0;

	// Things are so convoluted now, it is entirely  possible that this method
	// will get called on a storage device that is not the current one,
	// so we will not require a backdrop.

	if (GetBackground() != nullptr) {
		CBofString sDevName = GetName();

		// Get this from script, allows individual log states to set border.
		CBagVar *pVar = VARMNGR->GetVariable("LOGBORDER");
		if (pVar != nullptr) {
			nBorderSize = pVar->GetNumValue();
		}

		if (nBorderSize == 0) {
			if (sDevName == "LOG_WLD") {
				nBorderSize = LOGBORDER;
			} else {
				if (sDevName == "LOGZ_WLD") {
					nBorderSize = LOGZBORDER;
				}
			}
		}

		// The log window has instructional text at the top and bottom, so
		// create a floater rect to stay in the middle area
		xFloatRect = GetBackground()->GetRect();
		xFloatRect.top += nBorderSize;
		xFloatRect.bottom -= (nBorderSize / 2);

		// calculate what page the whole object belongs on
		int nPageNum = ((NextPos.y + pObj->GetRect().Height()) / xFloatRect.Height());
		// page numbering is 1-N
		nPageNum++;
		SetNumFloatPages(nPageNum);

		INT nTotalPages = GetCurFltPage();
		// Now position this object int the sdev
		// if it fell on this page, show it
		if (nPageNum == nTotalPages) {
			CBofPoint xPagePos = NextPos;
			// bring the current page into view
			xPagePos.y = xPagePos.y - ((nPageNum - 1) * xFloatRect.Height());
			// add in the border
			xPagePos.y += nBorderSize;
			pObj->SetPosition(xPagePos);
		} else {
			// set the postion to be off the sdev, so it won't show
			pObj->SetPosition(CBofPoint(NextPos.x, GetBackground()->Height() + 1));
		}

		// Calculate the position for the next floater
		// this will get sent back to the calling func
		NextPos.x += pObj->GetRect().Width();

		// Check to see if the whole object can fit in width, if it can't wrap
		if (NextPos.x > (xFloatRect.Width() - pObj->GetRect().Width())) {
			NextPos.x = 0;
			NextPos.y += pObj->GetRect().Height();
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

	Assert(pUpObj != nullptr);
	Assert(pDownObj != nullptr);

	// get current page number and last page number
	INT nLastPage = pLastFloat->GetNumFloatPages();
	INT nCurPage = pLastFloat->GetCurFltPage();
	INT nFirstPage = 1;

	if (nCurPage > nFirstPage && nCurPage < nLastPage) {
		if (pUpObj->IsAttached() == FALSE) {
			pUpObj->SetActive();
			pUpObj->Attach();
		}
		if (pDownObj->IsAttached() == FALSE) {
			pDownObj->SetActive();
			pDownObj->Attach();
		}
	} else {
		if (nCurPage == nFirstPage && nCurPage == nLastPage) {
			if (pUpObj->IsAttached()) {
				pUpObj->SetActive(FALSE);
				pUpObj->Detach();
			}
			if (pDownObj->IsAttached()) {
				pDownObj->SetActive(FALSE);
				pDownObj->Detach();
			}
		} else {
			if (nCurPage <= nFirstPage) {
				if (pUpObj->IsAttached()) {
					pUpObj->SetActive(FALSE);
					pUpObj->Detach();
				}
				if (pDownObj->IsAttached() == FALSE) {
					pDownObj->SetActive();
					pDownObj->Attach();
				}
			} else {
				if (nCurPage >= nLastPage) {
					if (pUpObj->IsAttached() == FALSE) {
						pUpObj->SetActive();
						pUpObj->Attach();
					}

					if (pDownObj->IsAttached()) {
						pDownObj->SetActive(FALSE);
						pDownObj->Detach();
					}
				}
			}
		}
	}

	// Reinitialize
	m_bLastFloatPage = nullptr;
}

int CBagLog::GetCurFltPage() {
	int nCurFltPage;

	// Read in their total nuggets from game
	CBagVar *pVar = nullptr;

	pVar = VARMNGR->GetVariable("CUR_BAR_LOG_PAGE");

	nCurFltPage = 0;
	if (pVar) {
		nCurFltPage = pVar->GetNumValue();
	}

	return nCurFltPage;
}

void CBagLog::SetCurFltPage(int nFltPage) {
	// Read in their total nuggets from game
	CBagVar *pVar = nullptr;

	pVar = VARMNGR->GetVariable("CUR_BAR_LOG_PAGE");

	if (pVar)
		pVar->SetValue(nFltPage);
}

ERROR_CODE CBagLog::ReleaseMsg() {
	ERROR_CODE errCode = ERR_NONE;
	CBagObject *pObj;
	int nCount = m_pQueued_Msgs->GetCount();

	if (nCount) {
		for (int i = 0; i < nCount; ++i) {
			pObj = m_pQueued_Msgs->RemoveHead();

			// This is waiting to be played, mark it in memory as such, the fixes
			// get uglier and uglier... since zoomed pda doesn't have a message light,
			// only set this thing as waiting if we are in the regular PDA,
			// otherwise, we get superflous blinking of the PDA light.
			CBofString  sDevName = GetName();
			if (sDevName == "LOG_WLD") {
				pObj->SetMsgWaiting(TRUE);
			}
		}
	}

	m_pQueued_Msgs->RemoveAll();
	return errCode;
}

CBagObject *CBagLog::OnNewUserObject(const CBofString &sInit) {
	CBagTextObject *LogObj = nullptr;
	CBofRect cSDevRect = GetRect();
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

BOOL CBagLog::RemoveFromMsgQueue(CBagObject *pRemObj) {
	INT nCount = m_pQueued_Msgs->GetCount();
	CBagObject *pObj;
	BOOL        bRemoved = FALSE;

	for (INT i = 0; i < nCount; i++) {
		pObj = m_pQueued_Msgs->GetNodeItem(i);

		if (pObj == pRemObj) {
			m_pQueued_Msgs->Remove(i);
			nCount--;
			bRemoved = TRUE;
			break;
		}
	}

	return bRemoved;
}

ERROR_CODE CBagLog::ActivateLocalObject(CBagObject *pObj) {
	ERROR_CODE errCode = ERR_NONE;

	if (pObj != nullptr) {

		if (pObj->IsMsgWaiting() ||
		        (pObj->GetType() == USEROBJ && (pObj->GetInitInfo() != nullptr) && (*pObj->GetInitInfo() == "MSG"))) {
			m_pQueued_Msgs->AddToTail(pObj);

			// Since zoomed pda doesn't  have a message light, only set this thing
			// as waiting if we are in the  regular PDA, otherwise, we get superflous
			// blinking of the PDA light.
			CBofString  sDevName = GetName();
			if (sDevName == "LOG_WLD") {
				pObj->SetMsgWaiting(TRUE);
			}

			CBagButtonObject *pMsgLight = nullptr;
			CBagStorageDev *pPda = nullptr;

			pPda = SDEVMNGR->GetStorageDevice("BPDA_WLD");

			if (pPda) {
				pMsgLight = (CBagButtonObject *)pPda->GetObject("MSGLIGHT");

				if (pMsgLight) {
					if (!pMsgLight->IsAttached()) {
						pMsgLight->SetActive();
						pMsgLight->Attach();
					}

					// Make sure this guy always gets updated regardless of its
					// dirty bit.
					pMsgLight->SetAlwaysUpdate(TRUE);
					pMsgLight->SetAnimated(TRUE);
				}
			}
		} else {
			errCode = CBagStorageDev::ActivateLocalObject(pObj);
		}
	}

	return errCode;
}

ERROR_CODE CBagLog::PlayMsgQue() {
	ERROR_CODE errCode = ERR_NONE;
	CBagObject *pObj = nullptr;
	INT nCount = m_pQueued_Msgs->GetCount();
	CBagMenu *pObjMenu = nullptr;
	BOOL        bPlayMsg = TRUE;

	// Walk through the message queue and play all the messages
	// Only play one message per click on the pda message light.
	if (nCount) {

		CBagStorageDev *pPda = nullptr;
		//CBagPDA *pPDAReally = nullptr;
		pPda = SDEVMNGR->GetStorageDevice("BPDA_WLD");
		//pPDAReally = (CBagPDA *)pPda;

		// If we're in a closeup, then don't play the message!
		CBagStorageDev *pSDev;

		if ((pSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()) != nullptr) {
			if (pSDev->IsCIC()) {
				bPlayMsg = FALSE;

				CHAR szLocalBuff[256];
				CBofString mStr(szLocalBuff, 256);

				// Global coordinates of pda view rect
				//CBofRect r = pPDAReally->GetViewRect() + pPDAReally->GetRect().TopLeft();

				mStr = OVERRIDESMK;
				MACROREPLACE(mStr);

				CBagMovieObject *pMovie = (CBagMovieObject *)GetObject(OVERRIDEMOVIE);
				if (pMovie) {
					if (pMovie->IsAttached() == FALSE) {
						pMovie->Attach();
						pMovie->SetVisible();
					}
					pMovie->RunObject();
				}
			}
		}

		// If we're playing a valid message (not the override message) then make sure
		// we remove it from the queue.
		if (bPlayMsg) {
			pObj = m_pQueued_Msgs->RemoveHead();

			if (pObj) {
				CRect  r = GetRect();

				errCode = CBagStorageDev::ActivateLocalObject(pObj);
				pObjMenu = pObj->GetMenuPtr();
				if (pObjMenu)
					pObjMenu->TrackPopupMenu(0, 0, 0, CBofApp::GetApp()->GetMainWindow(), nullptr, &r);
				pObj->RunObject();
				pObj->SetMsgWaiting(FALSE);

				// Mark this guy as played...
				((CBagLogMsg *)pObj)->SetMsgPlayed(TRUE);
			}

			// Although this might seem like a superflous thing to do, but wait!
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
						pMsgLight->SetAnimated(TRUE);
					} else {
						pMsgLight->SetAnimated(FALSE);
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
	m_bTitle = TRUE;
}

void CBagLogResidue::SetSize(const CBofSize &xSize) {
	CBagTextObject::SetSize(CBofSize(m_nSdevWidth, xSize.cy));
}

CBagLogMsg::CBagLogMsg(int nSdevWidth) : CBagTextObject() {
	m_xObjType = USEROBJ;
	m_nSdevWidth = nSdevWidth;
	m_bTitle = TRUE;

	// Start all messages off as not played
	SetMsgPlayed(FALSE);
}

void CBagLogMsg::SetSize(const CBofSize &xSize) {
	CBagTextObject::SetSize(CBofSize(m_nSdevWidth, xSize.cy));
}

PARSE_CODES CBagLogMsg::SetInfo(bof_ifstream &istr) {
	int nChanged;
	BOOL nObjectUpdated = FALSE;
	char ch;
	CHAR szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	CHAR szLocalBuff2[256];
	CBofString s(szLocalBuff2, 256);

	while (!istr.eof()) {
		nChanged = 0;
		istr.EatWhite();

		switch (ch = (char)istr.peek()) {
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

				nObjectUpdated = TRUE;
				nChanged++;
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
				CHAR cNext = (char)istr.peek();
				INT     nMsgTime = 0;
				if (Common::isDigit(cNext)) {
					GetIntFromStream(istr, nMsgTime);
				} else {
					GetAlphaNumFromStream(istr, m_sMsgTimeStr);
				}

				SetMsgTime(nMsgTime);
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		default: {
			PARSE_CODES rc;
			if ((rc = CBagObject::SetInfo(istr)) == PARSING_DONE) {
				return PARSING_DONE;
			} else if (rc == UPDATED_OBJECT) {
				nObjectUpdated = TRUE;
			} else if (!nChanged) { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;
				else
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
	} else {
		if (!sProp.Find("PLAYED")) {
			SetMsgPlayed(nVal);
		}
	}

	CBagObject::SetProperty(sProp, nVal);
}

INT CBagLogMsg::GetProperty(const CBofString &sProp) {
	if (!sProp.Find("TIME"))
		return GetMsgTime();

	// Played requires a 1 or a 0 (don't use true or false).
	if (!sProp.Find("PLAYED")) {
		BOOL bPlayed = GetMsgPlayed();
		return (bPlayed ? 1 : 0);
	}

	return CBagObject::GetProperty(sProp);
}

ERROR_CODE CBagLogMsg::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, INT nMaskColor) {
	int nHr, nMn;

	// We could use a variable here, translate it's value if that's the case.
	if (GetMsgTime() == 0) {
		CBagVar *pVar = VARMNGR->GetVariable(m_sMsgTimeStr);
		INT nMsgTime = pVar->GetNumValue();
		SetMsgTime(nMsgTime);
	}

	INT     nMsgTime = GetMsgTime();
	nHr = nMsgTime / 100;
	nMn = nMsgTime - (nHr * 100);

	SetFont(FONT_MONO);

	SetText(BuildString("%-30s%02d:%02d", m_sMsgSendee.GetBuffer(), nHr, nMn));

	return CBagTextObject::Update(pBmp, pt, pSrcRect, nMaskColor);
}

ERROR_CODE CBagLogResidue::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, INT nMaskColor) {
	return CBagTextObject::Update(pBmp, pt, pSrcRect, nMaskColor);
}

CBagLogSuspect::CBagLogSuspect(int nSdevWidth) : CBagTextObject() {
	m_xObjType = USEROBJ;
	m_nSdevWidth = nSdevWidth;

	m_bTitle = TRUE;

	// Need to save state info, set all to false.
	SetState(0);
}

PARSE_CODES CBagLogSuspect::SetInfo(bof_ifstream &istr) {
	int nChanged;
	BOOL nObjectUpdated = FALSE;
	char ch;

	CHAR szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	CHAR szLocalBuff2[256];
	CBofString s(szLocalBuff2, 256);

	while (!istr.eof()) {
		nChanged = 0;
		istr.EatWhite();

		switch (ch = (char)istr.peek()) {
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

				nObjectUpdated = TRUE;
				nChanged++;
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

				nObjectUpdated = TRUE;
				nChanged++;
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

				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		default: {
			PARSE_CODES rc;
			if ((rc = CBagObject::SetInfo(istr)) == PARSING_DONE) {
				return PARSING_DONE;
			} else if (rc == UPDATED_OBJECT) {
				nObjectUpdated = TRUE;
			} else if (!nChanged) { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;
				else
					return UNKNOWN_TOKEN;
			}
			break;
		}
		}
	}

	return PARSING_DONE;
}
void CBagLogSuspect::SetProperty(const CBofString &sProp, int nVal) {
	int bVal;

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
		bVal = FALSE;
		if (nVal == 1)
			bVal = TRUE;
		if (nVal == 0)
			bVal = FALSE;
		if (nVal == 2)
			bVal = 2;

		if (!sProp.Find("CHECKED"))
			SetSusChecked(bVal == 2 ? !GetSusChecked() : bVal);
#if INCLUDE_RP_AND_VP
		else if (!sProp.Find("VP"))
			SetSusVP(bVal == 2 ? !GetSusVP() : bVal);
		else if (!sProp.Find("RP"))
			SetSusRP(bVal == 2 ? !GetSusRP() : bVal);
#endif
		else
			CBagObject::SetProperty(sProp, nVal);
	}
}


INT CBagLogSuspect::GetProperty(const CBofString &sProp) {
	if (!sProp.Find("CHECKED"))
		return GetSusChecked();
#if INCLUDE_RP_AND_VP
	else if (!sProp.Find("VP"))
		return GetSusVP();
	else if (!sProp.Find("RP"))
		return GetSusRP();
#endif
	else
		return CBagObject::GetProperty(sProp);
}


void CBagLogSuspect::SetSize(const CBofSize &xSize) {
	CBagTextObject::SetSize(CBofSize(m_nSdevWidth, xSize.cy));

}


ERROR_CODE CBagLogSuspect::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, INT nMaskColor) {
	CHAR szSusChecked[256];
	CBofString sSusChecked(szSusChecked, 256);

	// Remove all the references to the jamming and voice printer state

#if INCLUDE_RP_AND_VP
	CHAR szSusVP[256];
	CHAR szSusRP[256];

	CBofString sSusVP(szSusVP, 256);
	CBofString sSusRP(szSusRP, 256);
#endif

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

#if INCLUDE_RP_AND_VP
	SetText(BuildString(" %-5.5s %-17.17s %-12.12s %-20.20s %-4.4s %-4.4s",
	                    sSusChecked.GetBuffer(),
	                    m_sSusName.GetBuffer(),
	                    m_sSusSpecies.GetBuffer(),
	                    m_sSusRoom.GetBuffer(),
	                    sSusVP.GetBuffer(),
	                    sSusRP.GetBuffer()));
#else
	SetText(BuildString(" %-5.5s %-17.17s %-12.12s %-20.20s",
	                    sSusChecked.GetBuffer(),
	                    m_sSusName.GetBuffer(),
	                    m_sSusSpecies.GetBuffer(),
	                    m_sSusRoom.GetBuffer()));
#endif

#if 0
	ERROR_CODE rc;

	if (IsAttached() && !(GetText().IsEmpty())) {
		CBofRect r(pt, pSrcRect->Size());

		rc = PaintText(pBmp, &r, GetText(), GetPointSize(), TEXT_NORMAL, RGB(255, 255, 255), JUSTIFY_LEFT);
	}
	return rc;
#else

	return CBagTextObject::Update(pBmp, pt, pSrcRect, nMaskColor);

#endif
}

// Energy detector objects, this should be pretty straightforward.

CBagEnergyDetectorObject::CBagEnergyDetectorObject() {
	SetFont(FONT_MONO);         // correct for spacing
	SetColor(7);                    // make it white
	SetFloating();                  // is definitely floating
	SetHighlight();                 // Is highlight
	SetTitle();                     // As title
	m_bTextInitialized = FALSE;     // Not initialized yet
}

CBagEnergyDetectorObject::~CBagEnergyDetectorObject() {
}

PARSE_CODES CBagEnergyDetectorObject::SetInfo(bof_ifstream &istr) {
	int nChanged;
	BOOL nObjectUpdated = FALSE;
	char ch;
	CHAR szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	CHAR szLocalBuff2[256];
	CBofString s(szLocalBuff2, 256);

	while (!istr.eof()) {
		nChanged = 0;
		istr.EatWhite();

		switch (ch = (char)istr.peek()) {
		//
		//  ZHAPS - NUMBER OF ZHAPS (ENERGY UNITS)
		//
		case 'Z': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("ZHAPS")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, s);

				m_sZhapsStr = s;

				nObjectUpdated = TRUE;
				nChanged++;
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

				nObjectUpdated = TRUE;
				nChanged++;
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
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  SIZE n - n point size of the txt
		//
		case 'S': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr2(szLocalStr, 256);

			GetAlphaNumFromStream(istr, sStr2);

			if (!sStr2.Find("SIZE")) {
				istr.EatWhite();
				INT n;
				GetIntFromStream(istr, n);
				SetPointSize(n);
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr2);
			}
			break;
		}

		default: {
			PARSE_CODES rc;
			if ((rc = CBagObject::SetInfo(istr)) == PARSING_DONE) {
				return PARSING_DONE;
			} else if (rc == UPDATED_OBJECT) {
				nObjectUpdated = TRUE;
			} else if (!nChanged) { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;
				else
					return UNKNOWN_TOKEN;
			}
			break;
		}
		}
	}

	return PARSING_DONE;
}

ERROR_CODE CBagEnergyDetectorObject::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, INT nMaskColor) {
	// Don't draw until we're attached
	if (IsAttached() == FALSE) {
		return ERR_NONE;
	}

	return CBagTextObject::Update(pBmp, pt, pSrcRect, nMaskColor);
}

ERROR_CODE CBagEnergyDetectorObject::Attach() {
	Assert(IsValidObject(this));

	CBagVar *pVar;
	INT nMsgTime;
	INT nHr, nMn;
	CHAR szLocalBuff[256];
	CBofString causeStr(szLocalBuff, 256);

	CHAR szZhapsBuff[256];
	CBofString zStr(szZhapsBuff, 256);

	// We could use a variable here, translate it's value if that's the case.
	pVar = VARMNGR->GetVariable(m_sEnergyTimeStr);
	if (pVar) {
		nMsgTime = pVar->GetNumValue();
	} else {
		nMsgTime = atoi(m_sEnergyTimeStr.GetBuffer());
	}

	nHr = nMsgTime / 100;
	nMn = nMsgTime - (nHr * 100);

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
	RecalcTextRect(FALSE);

	return CBagObject::Attach();
}

CBagLogClue::CBagLogClue(const CBofString &sInit, INT nSdevWidth, INT nPointSize) : CBagTextObject() {
	m_xObjType = USEROBJ;
	m_nSdevWidth = nSdevWidth;

	m_bTitle = TRUE;

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

ERROR_CODE CBagLogClue::Attach() {
	CHAR        szFormatStr[256];
	CHAR        szClueStr[256];
	CBofString  cFormat(szFormatStr, 256);

	Assert(IsValidObject(this));

	ERROR_CODE ec = CBagTextObject::Attach();

	// Get what is defined in the script.
	cFormat = GetFileName();

	// Replace '$' with '%' (% is an illegal character embedded in a clue string).
	cFormat.ReplaceChar('_', ' ');
	cFormat.ReplaceChar('$', '%');

	// Format the text appropriately.
	Common::sprintf_s(szClueStr, cFormat.GetBuffer(),
	                  (m_pStringVar1 ? (const CHAR *)m_pStringVar1->GetValue() : (const CHAR *)""),
	                  (m_pStringVar2 ? (const CHAR *)m_pStringVar2->GetValue() : (const CHAR *)""),
	                  (m_pStringVar3 ? (const CHAR *)m_pStringVar3->GetValue() : (const CHAR *)""),
	                  (m_pStringVar4 ? (const CHAR *)m_pStringVar4->GetValue() : (const CHAR *)""));

	CBofString cStr(szClueStr);
	SetPSText(&cStr);

	return ec;
}

PARSE_CODES CBagLogClue::SetInfo(bof_ifstream &istr) {
	int nChanged;
	BOOL nObjectUpdated = FALSE;
	char ch;

	CHAR szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	while (!istr.eof()) {
		nChanged = 0;
		istr.EatWhite();

		switch (ch = (char)istr.peek()) {
		//
		//  STRINGVAR - This will be a variable used to display some information that
		//  is contained in script in a clue statement.
		//
		case 'S': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("STRINGVAR")) {
				CBagVar *pVar;
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				pVar = VARMNGR->GetVariable(sStr);
				// the variable must have been found, if it wasn't, then
				// complain violently.

				if (pVar == nullptr) {
					return UNKNOWN_TOKEN;
				}

				if (m_pStringVar1 == nullptr) {
					m_pStringVar1 = pVar;
				} else {
					if (m_pStringVar2 == nullptr) {
						m_pStringVar2 = pVar;
					} else {
						if (m_pStringVar3 == nullptr) {
							m_pStringVar3 = pVar;
						} else {
							if (m_pStringVar4 == nullptr) {
								m_pStringVar4 = pVar;
							} else {
								return UNKNOWN_TOKEN;
							}
						}
					}
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		default: {
			PARSE_CODES rc;
			if ((rc = CBagObject::SetInfo(istr)) == PARSING_DONE) {
				return PARSING_DONE;
			} else if (rc == UPDATED_OBJECT) {
				nObjectUpdated = TRUE;
			} else if (!nChanged) {
				// rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;
				else
					return UNKNOWN_TOKEN;
			}
			break;
		}
		}

	}

	return PARSING_DONE;
}

ERROR_CODE CBagLogClue::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, INT nMaskColor) {
	return CBagTextObject::Update(pBmp, pt, pSrcRect, nMaskColor);
}

} // namespace Bagel
