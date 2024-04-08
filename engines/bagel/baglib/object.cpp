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

#include "bagel/baglib/object.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/baglib/master_win.h"

#if BOF_MAC
#include <ctype.h>
#endif

namespace Bagel {

extern bool g_bNoMenu;
extern CBofString BofEmptyString;

CBofString GetStringTypeOfObject(BAG_OBJECT_TYPE n) {
	switch (n) {
	case BASEOBJ:
		return "UKN";
	case BMPOBJ:
		return "BMP";
	case SPRITEOBJ:
		return "SPR";
	case BUTTONOBJ:
		return "BUT";
	case SOUNDOBJ:
		return "SND";
	case LINKOBJ:
		return "LNK";
	case RESPRNTOBJ:
		return "RPO";
	case DOSSIEROBJ:
		return "DOS";
	case CHAROBJ:
		return "CHR";
	case AREAOBJ:
		return "ARE";
	case MOVIEOBJ:
		return "MOVIE";
	case VAROBJ:
		return "VAR";
	case COMMANDOBJ:
		return "COMMAND";
	case THNGOBJ:
		return "TNG";
	default:
		return "UKN";
	};
}

CBagObject::CBagObject() {
	m_xObjType = BASEOBJ;
	m_nX = -1;
	m_nY = -1;

	// All objects now default to timeless
	m_nProperties = TIMELESS;

	m_nId = 0;
	m_nOverCursor = 0;

	// Init state
	m_nState = 0;

	m_pMenu = nullptr;
	m_pEvalExpr = nullptr;
	m_bInteractive = true;
	m_bNoMenu = FALSE;

	// Allocate this on an as-needed basis...
	m_psName = nullptr;

	// Object is dirty by default, doesn't break anything else
	// Some objects such as the pda message light are always updated
	SetDirty(true);
	SetAlwaysUpdate(FALSE);

	// All messages start as not message waiting.
	SetMsgWaiting(FALSE);
}

CBagObject::~CBagObject() {
	if (m_pMenu != nullptr) {
		delete m_pMenu;
		m_pMenu = nullptr;
	}

	if (m_psName && (m_psName != &m_sFileName)) {
		delete m_psName;
	}

	m_psName = nullptr;
}

ERROR_CODE CBagObject::Attach() {
	SetVisible();
	return CBagParseObject::Attach();
}

ERROR_CODE CBagObject::Detach() {
	SetVisible(FALSE);
	return CBagParseObject::Detach();
}

ERROR_CODE CBagObject::Update(CBofBitmap * /*pBmp*/, CPoint /*pt*/, CRect * /*pSrcRect*/, int /*nMaskColor*/) {
	return ERR_NONE;
}

int CBagObject::GetProperty(const CBofString &sProp) {
	if (!sProp.Find("STATE"))
		return GetState();
	else if (!sProp.Find("MODAL"))
		return IsModal();
	else
		return 0;
}

void CBagObject::SetProperty(const CBofString &sProp, int nVal) {
	if (!sProp.Find("STATE"))
		SetState(nVal);
	else if (!sProp.Find("TIMELESS")) {
		if (nVal)
			SetTimeless();
		else
			SetTimeless(FALSE);
	} else if (!sProp.Find("MODAL")) {
		if (nVal)
			SetModal();
		else
			SetModal(FALSE);
	}
}

void CBagObject::SetProperty(BAG_OBJECT_PROPERTIES xProp, bool bVal) {
	if (bVal)
		m_nProperties |= xProp;
	else
		m_nProperties &= ~xProp;
}

bool CBagObject::RunObject() {
	if (IsTimeless())
		return true;

	VARMNGR->IncrementTimers();

	return true;
}

PARSE_CODES CBagObject::SetInfo(bof_ifstream &istr) {
	PARSE_CODES rc = UNKNOWN_TOKEN;
	char ch;

	while (!istr.eof()) {
		istr.EatWhite();

		switch ((ch = (char)istr.Get())) {
		//
		//  =filename.ext
		//
		case '=': {
			rc = UPDATED_OBJECT;
			char szLocalBuff[256];
			szLocalBuff[0] = 0;
			CBofString s(szLocalBuff, 256);
			GetAlphaNumFromStream(istr, s);
			MACROREPLACE(s);
			SetFileName(s);
			break;
		}
		//
		//  { menu objects .... }  - Add menu items
		//
		case '{': {
			rc = UPDATED_OBJECT;
			if (!m_pMenu) {
				if ((m_pMenu = new CBagMenu) != nullptr) {

					// Try to cut down the number of Storage Devices by
					// removing these unused ones from the list.
					//
					SDEVMNGR->UnRegisterStorageDev(m_pMenu);
				}
			}

			istr.putback(ch);

			char szBuff[256];
			Common::sprintf_s(szBuff, "Menu:%d", CBagMasterWin::m_lMenuCount++);
			CBofString s(szBuff, 256);

			m_pMenu->LoadFileFromStream(istr, s, FALSE);
		}
		break;
		//
		//  ^id;  - Set id
		//
		case '^': {
			rc = UPDATED_OBJECT;
			char c = (char)istr.peek();
			if (Common::isDigit(c)) {
				int nId;
				GetIntFromStream(istr, nId);
				SetRefId(nId);
			} else {
				char szLocalBuff[256];
				szLocalBuff[0] = 0;
				CBofString s(szLocalBuff, 256);
				GetAlphaNumFromStream(istr, s);
				SetRefName(s);
			}
			break;
		}
		//
		//  *state;  - Set state
		//
		case '*': {
			rc = UPDATED_OBJECT;
			int nState;
			GetIntFromStream(istr, nState);
			SetState(nState);
			break;
		}
		//
		//  %cusror;  - Set cursor
		//
		case '%': {
			rc = UPDATED_OBJECT;
			int nCursor;
			GetIntFromStream(istr, nCursor);
			SetOverCursor(nCursor);
			break;
		}
		//
		//  [left,top,right,bottom]  - Set position
		//
		case '[': {
			rc = UPDATED_OBJECT;
			CBofRect r;
			istr.putback(ch);
			GetRectFromStream(istr, r);
			SetPosition(r.TopLeft());
			if (r.Width() && r.Height())
				SetSize(CSize(r.Width(), r.Height()));
			break;
		}
		//
		//  IS [NOT] [MOVABLE|MODAL|VISIBLE|STRETCHABLE|HIGHLIGHT|ACTIVE|TRANSPARENT|HIDE_ON_CLICK|IMMEDIATE_RUN|LOCAL|CONSTANT_UPDATE|PRELOAD|FOREGROUND]
		//
		case 'I': {
			if (istr.peek() != 'S') {
				istr.putback(ch);
				return rc;
				break;
			}
			char szLocalBuff[256];
			szLocalBuff[0] = 0;
			CBofString s(szLocalBuff, 256);
			bool b = true;
			istr.Get();
			istr.EatWhite();
			GetAlphaNumFromStream(istr, s);
			if (!s.Find("NOT")) {
				GetAlphaNumFromStream(istr, s);
				istr.EatWhite();
				b = FALSE;
			}
			if (!s.Find("MOVABLE")) {
				SetMovable(b);
			} else if (!s.Find("MODAL")) {
				SetModal(b);
			} else if (!s.Find("VISIBLE")) {
				SetVisible(b);
			} else if (!s.Find("STRETCHABLE")) {
				SetStretchable(b);
			} else if (!s.Find("HIGHLIGHT")) {
				SetHighlight(b);
			} else if (!s.Find("ACTIVE")) {
				SetActive(b);
			} else if (!s.Find("TRANSPARENT")) {
				SetTransparent(b);
			} else if (!s.Find("HIDE_ON_CLICK")) {
				SetHideOnClick(b);
			} else if (!s.Find("IMMEDIATE_RUN")) {
				SetImmediateRun(b);
			} else if (!s.Find("TIMELESS")) {
				SetTimeless(b);
			} else if (!s.Find("LOCAL")) {
				SetLocal(b);
			} else if (!s.Find("CONSTANT_UPDATE")) {
				SetConstantUpdate(b);
			} else if (!s.Find("PRELOAD")) {
				SetPreload(b);
			} else if (!s.Find("NOMENU")) {
				m_bNoMenu = true;

			} else if (!s.Find("FOREGROUND")) {
				SetForeGround(b);
			} else {
				PutbackStringOnStream(istr, s);
				if (!b)
					PutbackStringOnStream(istr, " NOT ");
				PutbackStringOnStream(istr, "IS ");
				return rc;
				break;
			}
			rc = UPDATED_OBJECT;
			break;
		}
		//
		//  ; is the end of the line, exit this object
		//
		case ';':
			return PARSING_DONE;
			break;

		//
		//  no match return from funtion
		//
		default:
			istr.putback(ch);
			return rc;
		}
	}

	return rc;
}

void CBagObject::OnLButtonUp(uint32 nFlags, CBofPoint * /*xPoint*/, void *) {
	if (GetMenuPtr()) {

		CBofPoint pt = GetMousePos();
		// CBofWindow *pWnd = CBagel::GetBagApp()->GetMasterWnd();
		CBofWindow *pWnd = CBofApp::GetApp()->GetMainWindow();
		pWnd->ScreenToClient(&pt);
		CRect r = GetRect();
		// GetMenuPtr()->TrackPopupMenu(nFlags,pt.x,pt.y+r.Width(),pWnd,nullptr,&r);
		//  Just send the mouse pos
		GetMenuPtr()->TrackPopupMenu(nFlags, pt.x, pt.y, pWnd, nullptr, &r);

	} else {
		g_bNoMenu = true;
	}

	RunObject();
}

bool CBagObject::OnMouseMove(uint32 /*nFlags*/, CPoint /*xPoint*/, void *) {
	return FALSE;
}

const CBofString &CBagObject::GetRefName() {
	if (m_psName) {
		return *m_psName;
	} else {
		return BofEmptyString;
	}
}

// Since we have an inordinate number of bag objects that replicate the
// filename as the objects name, if this is the case, then use a pointer
// to the filename.
void CBagObject::SetRefName(const CBofString &s) {
	// If we got an old one hanging around then trash it.
	if (m_psName && m_psName != &m_sFileName) {
		delete m_psName;
	}
	m_psName = nullptr;

	// If it's the same as the filename, then point to the filename
	if (!m_sFileName.IsEmpty() && m_sFileName.Compare(s) == 0) {
		m_psName = &m_sFileName;
	} else {
		if (!s.IsEmpty()) {
			m_psName = new CBofString(s);
		}
	}
}

} // namespace Bagel
