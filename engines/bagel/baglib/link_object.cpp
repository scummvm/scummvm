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

#include "bagel/baglib/link_object.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/pan_window.h"

namespace Bagel {

extern bool g_bNoMenu;

CBagLinkObject::CBagLinkObject() {
	m_xObjType = LINKOBJ;
	m_xLinkType = LINK;
	m_xDestLocation = CBofPoint(0, 0);
	m_xSrcLocation = CBofPoint(-1, -1);
	m_nFade = 0;
	SetVisible(false);
	SetOverCursor(5);
}

CBagLinkObject::~CBagLinkObject() {
	detach();
}

CBofRect CBagLinkObject::getRect() {
	CBofPoint p = GetPosition();
	CBofSize  s = GetSize();
	CBofRect r = CBofRect(p, s);
	return r;
}

PARSE_CODES CBagLinkObject::SetInfo(bof_ifstream &istr) {
	bool nObjectUpdated = false;

	PARSE_CODES cCode = PARSING_DONE;
	bool bDone = false;

	while (!bDone && !istr.eof()) {
		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  @[x,y]  - destination of a flythru.  start point in next world
		//
		case '@': {
			CBofRect r;
			istr.Get();
			GetRectFromStream(istr, r);
			SetDstLoc(r.TopLeft());
			nObjectUpdated = true;
			break;
		}

		//
		//  #[x,y]  - start location of a flythru.  start point in this world
		//
		case '#': {
			CBofRect r;
			istr.Get();
			GetRectFromStream(istr, r);
			SetSrcLoc(r.TopLeft());
			nObjectUpdated = true;
			break;
		}

		//
		//  AS [LINK|CLOSEUP]  - how to run the link
		//
		case 'A': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("AS")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				if (!sStr.Find("CLOSEUP")) {
					m_xLinkType = CLOSEUP;
					SetOverCursor(2);
					nObjectUpdated = true;
				} else if (!sStr.Find("LINK")) {
					m_xLinkType = LINK;
					nObjectUpdated = true;
				} else {
					PutbackStringOnStream(istr, sStr);
					PutbackStringOnStream(istr, "AS ");
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  FADE n
		//
		case 'F': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("FADE")) {
				istr.EatWhite();
				GetIntFromStream(istr, m_nFade);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  No match return from funtion
		//
		default: {
			PARSE_CODES rc = CBagObject::SetInfo(istr);

			if (rc == PARSING_DONE) {
				cCode = PARSING_DONE;
				bDone = true;

			} else if (rc == UPDATED_OBJECT) {
				nObjectUpdated = true;

			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					cCode = UPDATED_OBJECT;
				else
					cCode = UNKNOWN_TOKEN;

				bDone = true;
			}
			break;
		}
		}
	}

	return cCode;
}


bool CBagLinkObject::RunObject() {
	// Reset Wield
	g_bNoMenu = false;

	char szBuf[256];
	szBuf[0] = '\0';
	CBofString cStr(szBuf, 256);

	cStr = GetFileName();

	// If this is a special link (using the last known sdev stack),
	// then find it's value, and use that instead.
	if (GetFileName().Find("$LASTWORLD") != -1) {
		CBagVar *pVar;

		cStr = GetFileName();

		if ((pVar = VARMNGR->GetVariable("$LASTWORLD")) != nullptr) {
			cStr.ReplaceStr("$LASTWORLD", pVar->GetValue());
		}
	}

	CBagMasterWin::SetActiveCursor(6);

	CBagStorageDevWnd *pSDev1 = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev();

	// Set the link position for the storage device we are about to jump to
	CBagStorageDev *pDestWin;
	if ((pDestWin = SDEVMNGR->GetStorageDevice(cStr)) != nullptr) {
		pDestWin->SetLoadFilePos(GetDstLoc());
	}

	CBagMasterWin *pMasterWin = CBagel::GetBagApp()->GetMasterWnd();
	if (pMasterWin) {
		pMasterWin->SetCurrFadeIn(m_nFade);
		pMasterWin->SetStorageDev(cStr);
	}

	CBagStorageDevWnd *pSDev2 = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev();
	if (!pSDev1->IsCloseup() && !pSDev2->IsCloseup()) {
		VARMNGR->IncrementTimers();
	}

	return CBagObject::RunObject();
}

} // namespace Bagel
