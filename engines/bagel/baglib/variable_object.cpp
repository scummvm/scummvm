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

#include "bagel/baglib/variable_object.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/var.h"
#include "bagel/boflib/gui/edit_text.h"

#if BOF_MAC
#include <mac.h>
#endif

namespace Bagel {

CBagVariableObject::CBagVariableObject() : CBagObject() {
	m_xObjType = VAROBJ;
	// MDM 9/4/96
	m_nPointSize = 16;
	m_nFGColor = CTEXT_COLOR;
	SetVisible(TRUE);
	SetTimeless(TRUE);
}

CBagVariableObject::~CBagVariableObject() {
	Detach();
}

ERROR_CODE CBagVariableObject::Attach() {
	CBagVar *xVar = VARMNGR->GetVariable(GetFileName());

	if (xVar && !GetRefName().IsEmpty())
		xVar->SetValue(GetRefName());

	return CBagObject::Attach();
}

ERROR_CODE CBagVariableObject::Detach() {
	return CBagObject::Detach();
}

CBofRect CBagVariableObject::GetRect() {
	CBofPoint p = GetPosition();
	CBofSize s = GetSize();
	CBofRect r = CBofRect(p, s);
	return (r);
}

//
//   SetInfo(bof_ifstream& istr)
//   Takes in info and then removes the relative information and returns the info
//   without the relevant info.
//
//      MDM 9/4/96
PARSE_CODES CBagVariableObject::SetInfo(bof_ifstream &istr) {
	INT nChanged;
	BOOL nObjectUpdated = FALSE;
	CHAR ch;

	while (!istr.eof()) {
		nChanged = 0;

		istr.EatWhite();

		switch (ch = (char)istr.peek()) {

		//
		//  SIZE n - n point size of the txt
		//
		case 'S': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256); // jwl 08.28.96 performance improvement

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("SIZE")) {
				istr.EatWhite();
				GetIntFromStream(istr, m_nPointSize);
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

			//
			//  COLOR n - n color index
			//
#if BOF_MAC
#define CTEXT_YELLOW RGB(0xFC, 0xF3, 0x05)
#define CTEXT_WHITE RGB(255, 255, 255)
#else
#define CTEXT_YELLOW RGB(255, 255, 0)
#define CTEXT_WHITE RGB(255, 255, 255)
#endif
		case 'C': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256); // jwl 08.28.96 performance improvement

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("COLOR")) {
				int nColor;
				istr.EatWhite();
				GetIntFromStream(istr, nColor);
				switch (nColor) {
				case 0:
					m_nFGColor = RGB(0, 0, 0);
					break; // black
				case 1:
					m_nFGColor = RGB(255, 0, 0);
					break;
				case 2:
					m_nFGColor = CTEXT_YELLOW;
					break; // yellow
				case 3:
					m_nFGColor = RGB(0, 255, 0);
					break;
				case 4:
					m_nFGColor = RGB(0, 255, 255);
					break;
				case 5:
					m_nFGColor = RGB(0, 0, 255);
					break;
				case 6:
					m_nFGColor = RGB(255, 0, 255);
					break;
				case 7:
					m_nFGColor = CTEXT_WHITE;
					break; // white
				default:
					break;
				}
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;
		//
		//  no match return from funtion
		//
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
		}
		break;
		}
	}

	return PARSING_DONE;
}
INT CBagVariableObject::MapWindowsPointSize(INT pointSize) {
	INT mappedPointSize = pointSize;
#if BOF_MAC
	switch (pointSize) {
	case 8:
		mappedPointSize = 6;
		break;
	case 10:
		mappedPointSize = 8;
		break;
	case 12:
		mappedPointSize = 10;
		break;
	case 14:
		mappedPointSize = 12;
		break;
	case 16:
		mappedPointSize = 12;
		break;
	case 18:
		mappedPointSize = 14;
		break;
	case 20:
		mappedPointSize = 16;
		break;
	default:
#if DEVELOPMENT
		DebugStr("\pMapWindowsPointSize invalid size");
#else
		MacMessageBox("MapWindowsPointSize invalid size", nullptr);
#endif
		break;
	}
#endif
	return mappedPointSize;
}
ERROR_CODE CBagVariableObject::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, INT) {
	ERROR_CODE rc = ERR_NONE;
	CBagVar *xVar = VARMNGR->GetVariable(GetFileName());
	if (IsAttached() && xVar && !(xVar->GetValue().IsEmpty())) {
		CBofRect r(pt, pSrcRect->Size());
		// MDM 9/4/96
		rc = PaintText(pBmp, &r, xVar->GetValue(), MapWindowsPointSize(m_nPointSize), TEXT_NORMAL, m_nFGColor);

		// jwl 10.14.96 Don't need to redraw!
		SetDirty(FALSE);
	}
	return rc;
}

ERROR_CODE CBagVariableObject::Update(CBofWindow *pWnd, CBofPoint pt, CBofRect *pSrcRect, INT) {
	ERROR_CODE rc = ERR_NONE;
	CBagVar *xVar = VARMNGR->GetVariable(GetFileName());
	if (IsAttached() && xVar && !(xVar->GetValue().IsEmpty())) {
		CBofRect r(pt, pSrcRect->Size());
		// MDM 9/4/96
		rc = PaintText(pWnd, &r, xVar->GetValue(), MapWindowsPointSize(m_nPointSize), TEXT_NORMAL, m_nFGColor);
		// rc = PaintText(pWnd, &r, xVar->GetValue(), 20, TEXT_NORMAL);

		// jwl 10.14.96 Don't need to redraw!
		SetDirty(FALSE);
	}
	return rc;
}

} // namespace Bagel
