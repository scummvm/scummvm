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

#include "bagel/baglib/text_object.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/baglib/zoom_pda.h"
#include "bagel/baglib/base_pda.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/rp_object.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/boflib/gfx/text.h"

#if BOF_MAC
#include <mac.h>
#endif

namespace Bagel {

#define CTEXT_WHITE RGB(255, 255, 255)

// local prototypes...
int MapFont(int nFont);

extern bool g_bPauseTimer;

CBagTextObject::CBagTextObject() : CBagObject() {
	m_xObjType = TEXTOBJ;
	m_nDX = 80;
	m_nDY = 20;
	m_psText = nullptr;
	SetOverCursor(1); // Switch to cursor 1, 4 doesn't exist.

	m_nPointSize = 16;
	m_nFGColor = CTEXT_COLOR;
	m_psInitInfo = nullptr;
	m_bCaption = false;
	m_bTitle = false;
	m_bReAttach = false;
	m_nTextFont = FONT_DEFAULT;

	SetRPObject(nullptr);
}

CBagTextObject::~CBagTextObject() {
	if (m_psInitInfo != nullptr) {
		delete m_psInitInfo;
		m_psInitInfo = nullptr;
	}
	Detach();
}

CBofRect CBagTextObject::GetRect() {
	CBofPoint p = GetPosition();
	CBofSize s = GetSize();
	CBofRect r = CBofRect(p, s);
	return r;
}

ERROR_CODE CBagTextObject::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);
	Assert(pSrcRect != nullptr);

	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pBmp != nullptr) && IsAttached() && !(GetText().IsEmpty())) {

		if (pBmp->GetRect().PtInRect(pt)) {

			CBofRect r(pt, pSrcRect->Size());
			int nPointSize, nFormat;

			nPointSize = m_nPointSize;

			nFormat = FORMAT_CENTER_LEFT;
			if (!m_bTitle) {

				CBofRect cBevel;
				int i, left, top, right, bottom;
				byte c1, c2;

				c1 = 3;
				c2 = 9;

				cBevel.IntersectRect(pBmp->GetRect(), r);

				left = cBevel.left;
				top = cBevel.top;
				right = cBevel.right;
				bottom = cBevel.bottom;

				r.left += 6;
				r.top += 3;
				r.right -= 5;
				r.bottom -= 5;

				for (i = 1; i <= 3; i++) {
					pBmp->Line(left + i, bottom - i, right - i, bottom - i, c1);
					pBmp->Line(right - i, bottom - i, right - i, top + i - 1, c1);
				}

				for (i = 1; i <= 3; i++) {
					pBmp->Line(left + i, bottom - i, left + i, top + i - 1, c2);
					pBmp->Line(left + i, top + i - 1, right - i, top + i - 1, c2);
				}
				nPointSize = 16;
				nFormat = FORMAT_TOP_LEFT;

			} else {
				r.left += 1;
			}

			errCode = PaintText(pBmp, &r, GetText(), MapWindowsPointSize(nPointSize), TEXT_NORMAL, m_nFGColor, JUSTIFY_WRAP, nFormat, m_nTextFont);

			// This object does not need to be updated now...
			SetDirty(false);
		}
	}

	return errCode;
}

ERROR_CODE CBagTextObject::Attach() {
	Assert(IsValidObject(this));

	if (!GetFileName().Right(4).Find(".TXT") || !GetFileName().Right(4).Find(".txt")) {

		// Prevent memory leak
		if (m_psText != nullptr) {
			delete m_psText;
			m_psText = nullptr;
		}

		// Allocate a new string
		if ((m_psText = new CBofString) != nullptr) {

			CBofFile fpTextFile(GetFileName());
			char *pTextBuff;
			uint32 nFileLen;

			if (!fpTextFile.ErrorOccurred()) {

				// Allocate the buffers
				//
				nFileLen = fpTextFile.GetLength();
				if ((pTextBuff = (char *)BofCAlloc(nFileLen + 1, 1)) != nullptr) {

					// Read the text file into buffers
					fpTextFile.Read(pTextBuff, nFileLen);
					fpTextFile.Close();

					*m_psText += pTextBuff;

					if (m_psInitInfo != nullptr) {
						CBagVar *pVar;

						if ((pVar = VARMNGR->GetVariable(*m_psInitInfo)) != nullptr) {
							m_bReAttach = true;
							m_psText->ReplaceStr("%s", pVar->GetValue());
						}
					}

					BofFree(pTextBuff); // Changed from delete.

				} else {
					ReportError(ERR_MEMORY);
				}

			} else {
				ReportError(ERR_MEMORY);
			}
		}

		if (IsCaption()) {
			RecalcTextRect(true);
		}

	} else {
		// The Text is in the Bagel script, rather than a .txt file
#if 0
		CBagPanWindow  *pPanWin;        // The window where the object are displayed
		CBofSize        cSize;          // Size of rect needed to display font
		HFONT           hFont, hFontOld;                // font that was mapped to the context
		HDC             hDC;
#endif

		// Prevent memory leak
		if (m_psText != nullptr) {
			delete m_psText;
			m_psText = nullptr;
		}

		// Allocate a new string
		if ((m_psText = new CBofString) != nullptr) {
			*m_psText = GetFileName();

			// Replace any underscores with spaces
			m_psText->ReplaceChar('_', ' ');

			RecalcTextRect(false);

		} else {
			ReportError(ERR_MEMORY);
		}
	}

	// If this guy is linked to a residue printing object, make sure he knows
	// we've been attached.
	CBagRPObject *pRPObj = (CBagRPObject *)GetRPObject();

	if (pRPObj != nullptr) {
		pRPObj->SetTimeSet(false);
	}

	return CBagObject::Attach();
}

ERROR_CODE CBagTextObject::Detach() {
	Assert(IsValidObject(this));

	if (m_psText != nullptr) {
		delete m_psText;
		m_psText = nullptr;
	}

	return CBagObject::Detach();
}

const CBofString &CBagTextObject::GetText() {
	if (m_psText)
		return *m_psText;
	else
		return GetFileName();
}

void CBagTextObject::SetText(const CBofString &s) {
	if (m_psText) {
		*m_psText = s;
	} else {
		SetFileName(s);
	}

	RecalcTextRect(!GetFileName().Right(4).Find(".TXT") || !GetFileName().Right(4).Find(".txt"));
}

//
// SetInfo(bof_ifstream& istr)
//   Takes in info and then removes the relative information and returns the info
//   without the relevant info.
//
PARSE_CODES CBagTextObject::SetInfo(bof_ifstream &istr) {
	int nChanged;
	bool nObjectUpdated = false;
	char ch;

	while (!istr.eof()) {
		nChanged = 0;

		istr.EatWhite();

		switch (ch = (char)istr.peek()) {

		//
		//  VAR var - var is a BAGEL CBagVar variable (replaces all %s in text)
		//
		case 'V': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			// Need to use this field, so no one else can
			Assert(m_psInitInfo == nullptr);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("VAR")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				SetInitInfo(sStr);
				nObjectUpdated = true;
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
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("SIZE")) {
				istr.EatWhite();
				int n;
				GetIntFromStream(istr, n);
				m_nPointSize = (byte)n;
				nObjectUpdated = true;
				nChanged++;

				// WORKAROUND: Reduce the font size of Cilia's full-screen log
				// in ScummVM so that it fits on the screen
				if (m_nPointSize == 28 && istr.GetSize() == 359105)
					m_nPointSize = 26;

			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}
		//
		//  FONT MONO or DEFAULT
		//
		case 'F': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("FONT")) {
				istr.EatWhite();
				int n;
				GetIntFromStream(istr, n);
				m_nTextFont = MapFont(n);
				nObjectUpdated = true;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		//
		//  AS [CAPTION]  - how to run the link
		//
		case 'A': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("AS")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				if (!sStr.Find("CAPTION")) {
					m_bCaption = true;
					nChanged++;
					nObjectUpdated = true;

				} else if (!sStr.Find("TITLE")) {
					m_bTitle = true;
					nChanged++;
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
			//  COLOR n - n color index
			//
		case 'C': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("COLOR")) {
				int nColor;
				istr.EatWhite();
				GetIntFromStream(istr, nColor);
				SetColor(nColor);
				nObjectUpdated = true;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}
		//
		//  no match return from funtion
		//
		default: {
			PARSE_CODES rc;
			if ((rc = CBagObject::SetInfo(istr)) == PARSING_DONE) {
				return PARSING_DONE;
			} else if (rc == UPDATED_OBJECT) {
				nObjectUpdated = true;
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

void CBagTextObject::SetColor(int nColor) {
	switch (nColor) {
	case 0:
		m_nFGColor = RGB(0, 0, 0);
		break; // Black
	case 1:
		m_nFGColor = RGB(226, 50, 51);
		break; // Red
	case 2:
		m_nFGColor = RGB(255, 255, 255);
		break; // Yellow
	case 3:
		m_nFGColor = RGB(255, 255, 255);
		break; // Green
	case 4:
		m_nFGColor = RGB(255, 255, 255);
		break; // Green
	case 5:
		m_nFGColor = RGB(0, 0, 255);
		break; // Blue
	case 6:
		m_nFGColor = RGB(255, 0, 255);
		break;
	case 7:
		m_nFGColor = CTEXT_WHITE;
		break; // White
	case 8:
		m_nFGColor = RGB(255, 255, 0);
		break; // Yellow (chat highlight)
	default:
		break;
	}
}

void CBagTextObject::SetProperty(const CBofString &sProp, int nVal) {
	if (!sProp.Find("SIZE"))
		SetPointSize(nVal);
	else if (!sProp.Find("FONT"))
		SetFont(MapFont(nVal));
	else if (!sProp.Find("COLOR"))
		SetColor(nVal);
	else
		CBagObject::SetProperty(sProp, nVal);
}

int CBagTextObject::GetProperty(const CBofString &sProp) {
	if (!sProp.Find("SIZE"))
		return GetPointSize();
	if (!sProp.Find("FONT"))
		return GetFont();
	if (!sProp.Find("COLOR"))
		return GetColor();

	return CBagObject::GetProperty(sProp);
}

/*class CBagTextDialog : public CBofDialog {

    public:
        CBagTextDialog();
};*/

bool CBagTextObject::RunObject() {
	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	if (m_bCaption && IsImmediateRun()) {

		// Re-Attach this object to get any change in a variable
		// (Must be using the VAR token).
		//
		if (m_bReAttach) {
			Attach();
		}

		CBagMenuDlg cDlg;

		CBagel *pApp;
		CBagMasterWin *pWin;
		CBagStorageDevWnd *pParent;

		if ((pApp = CBagel::GetBagApp()) != nullptr) {
			if ((pWin = pApp->GetMasterWnd()) != nullptr) {
				pParent = pWin->GetCurrentStorageDev();

				CBofRect cRect(80, 10, 80 + 480 /*- 1 */, 10 + GetRect().Height() - 1 + 5);
				CBofPoint cPoint(0, 0);

				CBofPalette *pPal;
				pPal = pApp->GetPalette();

				CBofBitmap cBmp(cRect.Width(), cRect.Height(), pPal);

				cBmp.FillRect(nullptr, pPal->GetNearestIndex(RGB(92, 92, 92)));

				cDlg.Create(pParent, pPal, &cRect);
				// cDlg.SetBackdrop(&cBmp);

				Update(cDlg.GetBackdrop(), cPoint, &cRect);

				CBagPDA *pPDA = nullptr;
				sStr = "BPDA_WLD";
				pPDA = (CBagPDA *)SDEVMNGR->GetStorageDevice(sStr);

				// If we're in the zoom pda then put this box at the
				// bottom of the zoom rect.
				SBZoomPda *pPDAZ = nullptr;
				sStr = "BPDAZ_WLD";
				pPDAZ = (SBZoomPda *)SDEVMNGR->GetStorageDevice(sStr);

				if (pPDAZ && pPDAZ->GetZoomed() == true) {
					CBofRect zRect = pPDAZ->GetViewRect();
					Assert(zRect.Height() > 0 && zRect.Height() < 480);
					Assert(zRect.Width() > 0 && zRect.Width() < 640);
					cDlg.Move(80, zRect.bottom - cRect.Height(), true); // xxx
				} else {
					if ((pPDA != nullptr) && (pPDA->IsActivated() || pPDA->IsActivating())) {
						cDlg.Move(80, 10, true);

					} else {
						int x, y;

						x = 80;
						y = 360 + 10 - cRect.Height();
						cDlg.Move(x, y, true);
					}
				}
				g_bPauseTimer = true;
				cDlg.DoModal();
				g_bPauseTimer = false;
			}
		}
	}

	return false;
}

int MapFont(int nFont) {
	switch (nFont) {

	case 0:
		return FONT_MONO;

	case 1:
	default:
		return FONT_DEFAULT;
	}
}

// Really foolish method for handling a mouse event on a text object that
// is associated with a residue printing object in that same sdev.
void CBagTextObject::OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *pv) {
	// If there's a residue printing object, then hand this guy off to
	// him, otherwise, call back to Cbagobj.
	CBagRPObject *pRPObj = (CBagRPObject *)GetRPObject();
	if (pRPObj) {
		pRPObj->OnLButtonUp(nFlags, xPoint, pv);
		return;
	}

	CBagObject::OnLButtonUp(nFlags, xPoint, pv);
}

void CBagTextObject::RecalcTextRect(bool bTextFromFile) {
	CBagPanWindow *pPanWin; // The window where the object are displayed
	CBofRect ViewRect;      // The rect of the area where objects are displayed
	CBofSize cDisplaySize;  // Size of rect needed to display font
	CBofSize cSize;         // Size of rect needed to display font

	Assert(m_psText != nullptr);

	pPanWin = (CBagPanWindow *)(CBagel::GetBagApp()->GetMasterWnd()->GetCurrentGameWindow());
	if (bTextFromFile) {
		if (pPanWin->GetDeviceType() == SDEV_GAMEWIN) {
			ViewRect = pPanWin->GetViewPort();
		} else {
			ViewRect = pPanWin->GetClientRect();
		}
	}

	if (ViewRect.IsRectEmpty()) {
		ViewRect.SetRect(80, 10, 480 + 80 - 1, 360 + 10 - 1);
	}

	// Get the area spanned by the text (i.e. Get the pixel width and
	// height of the text string).
	CBofRect tmpRect = ViewRect;
	if (!m_bTitle) {
		// Exactly match the width used in DisplayTextEx
		tmpRect.left += 5;
		tmpRect.right = (ViewRect.right == 640 ? PAN_AREA_WIDTH : ViewRect.right) - 5;
	}

	CBofRect textRect = CalculateTextRect(tmpRect, m_psText, m_nPointSize, GetFont());
	CBofSize stTextSize(textRect.right, textRect.bottom);

	if (bTextFromFile) {
		// Add fudge factor to make sure that all the text will fit, and not
		// get cut off.  This may cause an extra blank line of text in some
		// captions, but tough diddles, it's still better than truncating
		// some text.
		//stTextSize.cx += 43;

		cSize.cx = stTextSize.cx;
		cSize.cy = stTextSize.cy;
	} else {
		cSize.cx = stTextSize.cx + 9;
		cSize.cy = stTextSize.cy + (m_bTitle ? 0 : 7);
		SetSize(cSize);
	}

	if (bTextFromFile) {
		cDisplaySize.cx = ViewRect.Width();
		cDisplaySize.cy = cSize.cy;

		// If for some reason (CIC, CHAT) we got too large
		// a viewrect, cut it back to the correct width
		if (cDisplaySize.cx > PAN_AREA_WIDTH)
			cDisplaySize.cx = PAN_AREA_WIDTH;

		// Buffer the size a little for spacing etc.
		cDisplaySize.cx -= 5;

		// While the text is wider then the view area
		while (cSize.cx > cDisplaySize.cx) {
			// Increment Display Height to account for another line
			cDisplaySize.cy += cSize.cy;

			// Decrement the size of text by the width of one line
			cSize.cx -= cDisplaySize.cx;
		}

		// Add a little space at the bottom
		cDisplaySize.cy += 5;

		SetSize(cDisplaySize);
	}
}

void CBagTextObject::SetPSText(CBofString *p) {
	Assert(IsValidObject(this));

	if (m_psText != nullptr) {
		delete m_psText;
		m_psText = nullptr;
	}

	if (p != nullptr) {
		m_psText = new CBofString(*p);
	}
}

} // namespace Bagel
