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

#if BOF_MAC // jwl 08.27.96
#include <mac.h>
#endif


namespace Bagel {

// local prototypes...
INT MapFont(INT nFont);

extern BOOL g_bPauseTimer;

//
// CBagTextObject -
//  CBagTextObject is an object that can be place within the slide window.
//
CBagTextObject::CBagTextObject() : CBagObject() {
	m_xObjType = TEXTOBJ;
	m_nDX = 80;
	m_nDY = 20;
	m_psText = NULL;
	SetOverCursor(1); // jwl 11.26.96 switch to cursor 1, 4 doesn't exist.

	m_nPointSize = 16;
	m_nFGColor = CTEXT_COLOR;
	m_psInitInfo = NULL;
	// MDM 6/12
	m_bCaption = FALSE;
	m_bTitle = FALSE;
	m_bReAttach = FALSE;

	// jwl 10.18.96
	m_nTextFont = FONT_DEFAULT;

	// jwl 11.11.96 no rp object by default
	SetRPObject(NULL);
}

CBagTextObject::~CBagTextObject() {
	if (m_psInitInfo != NULL) {
		delete m_psInitInfo;
		m_psInitInfo = NULL;
	}
	Detach();
}

CBofRect CBagTextObject::GetRect(VOID) {
	CBofPoint p = GetPosition();
	CBofSize s = GetSize();
	CBofRect r = CBofRect(p, s);
	return (r);
}

ERROR_CODE CBagTextObject::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, INT) {
	Assert(IsValidObject(this));
	Assert(pBmp != NULL);
	Assert(pSrcRect != NULL);

	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	if ((pBmp != NULL) && IsAttached() && !(GetText().IsEmpty())) {

		if (pBmp->GetRect().PtInRect(pt)) {

			CBofRect r(pt, pSrcRect->Size());
			INT nPointSize, nFormat;

			nPointSize = m_nPointSize;

			nFormat = FORMAT_CENTER_LEFT;
			if (!m_bTitle) {

				CBofRect cBevel;
				INT i, left, top, right, bottom;
				UBYTE c1, c2;

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

			// jwl 10.11.96 this object does not need to be updated now...
			SetDirty(FALSE);
		}
	}

	return (errCode);
}

ERROR_CODE CBagTextObject::Attach() {
	Assert(IsValidObject(this));

	if (!GetFileName().Right(4).Find(".TXT") || !GetFileName().Right(4).Find(".txt")) {

		// Prevent memory leak
		if (m_psText != NULL) {
			delete m_psText;
			m_psText = NULL;
		}

		// Allocate a new string
		if ((m_psText = new CBofString) != NULL) {

			CBofFile fpTextFile(GetFileName());
			CHAR *pTextBuff;
			ULONG nFileLen;

			if (!fpTextFile.ErrorOccurred()) {

				// Allocate the buffers
				//
				nFileLen = fpTextFile.GetLength();
				if ((pTextBuff = (CHAR *)BofCAlloc(nFileLen + 1, 1)) != NULL) {

					// Read the text file into buffers
					fpTextFile.Read(pTextBuff, nFileLen);
					fpTextFile.Close();

					*m_psText += pTextBuff;

					if (m_psInitInfo != NULL) {
						CBagVar *pVar;

						if ((pVar = VARMNGR->GetVariable(*m_psInitInfo)) != NULL) {
							m_bReAttach = TRUE;
							m_psText->ReplaceStr("%s", pVar->GetValue());
						}
					}

					BofFree(pTextBuff); // jwl 10.22.96 changed from delete.

				} else {
					ReportError(ERR_MEMORY);
				}

			} else {
				ReportError(ERR_MEMORY);
			}
		}

		if (IsCaption()) {
			RecalcTextRect(TRUE);
		}

	} else {
		// The Text is in the Bagel script, rather than a .txt file
#if 0
        CBagPanWindow*  pPanWin;        // The window where the object are displayed
        CBofSize        cSize;          // Size of rect needed to display font
		HFONT       	hFont, hFontOld;                // font that was mapped to the context
		HDC 			hDC;
#endif

		// Prevent memory leak
		if (m_psText != NULL) {
			delete m_psText;
			m_psText = NULL;
		}

		// Allocate a new string
		if ((m_psText = new CBofString) != NULL) {
			*m_psText = GetFileName();

			// Replace any underscores with spaces
			m_psText->ReplaceChar('_', ' ');

			RecalcTextRect(FALSE);

		} else {
			ReportError(ERR_MEMORY);
		}
	}

	// If this guy is linked to a residue printing object, make sure he knows
	// we've been attached.
	CBagRPObject *pRPObj = (CBagRPObject *)GetRPObject();

	if (pRPObj != NULL) {
		pRPObj->SetTimeSet(FALSE);
	}

	return CBagObject::Attach();
}

ERROR_CODE CBagTextObject::Detach() {
	Assert(IsValidObject(this));

	if (m_psText != NULL) {
		delete m_psText;
		m_psText = NULL;
	}

	return CBagObject::Detach();
}

const CBofString &CBagTextObject::GetText() {
	if (m_psText)
		return *m_psText;
	else
		return GetFileName();
}

VOID CBagTextObject::SetText(const CBofString &s) {
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
	INT nChanged;
	BOOL nObjectUpdated = FALSE;
	CHAR ch;

	while (!istr.eof()) {
		nChanged = 0;

		istr.EatWhite();

		switch (ch = (char)istr.peek()) {

		//
		//  VAR var - var is a BAGEL CBagVar variable (replaces all %s in text)
		//
		case 'V': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);

			// Need to use this field, so no one else can
			Assert(m_psInitInfo == NULL);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("VAR")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				SetInitInfo(sStr);
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
			CBofString sStr(szLocalStr, 256); // jwl 08.28.96 performance improvement

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("SIZE")) {
				istr.EatWhite();
				INT n;
				GetIntFromStream(istr, n);
				m_nPointSize = (UBYTE)n;
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		} break;
		//
		//  FONT MONO or DEFAULT
		//
		case 'F': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256); // jwl 08.28.96 performance improvement

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("FONT")) {
				istr.EatWhite();
				INT n;
				GetIntFromStream(istr, n);
				m_nTextFont = MapFont(n);
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		} break;
			//
			//  AS [CAPTION]  - how to run the link
			//
			//  MDM 6/12
		case 'A': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256); // jwl 08.28.96 performance improvement
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("AS")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				if (!sStr.Find("CAPTION")) {
					m_bCaption = TRUE;
					nChanged++;
					nObjectUpdated = TRUE;

				} else if (!sStr.Find("TITLE")) {
					m_bTitle = TRUE;
					nChanged++;
					nObjectUpdated = TRUE;

				} else {
					PutbackStringOnStream(istr, sStr);
					PutbackStringOnStream(istr, "AS ");
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		} break;
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
				SetColor(nColor);
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		} break;
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
		} break;
		}
	}

	return PARSING_DONE;
}

VOID CBagTextObject::SetColor(INT nColor) {
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

VOID CBagTextObject::SetProperty(const CBofString &sProp, int nVal) {
	if (!sProp.Find("SIZE"))
		SetPointSize(nVal);
	else if (!sProp.Find("FONT"))
		SetFont(MapFont(nVal));
	else if (!sProp.Find("COLOR"))
		SetColor(nVal);
	else
		CBagObject::SetProperty(sProp, nVal);
}

INT CBagTextObject::GetProperty(const CBofString &sProp) {
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

BOOL CBagTextObject::RunObject() {
	CHAR szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	if (m_bCaption && IsImmediateRun()) {

		// BCW - 12/17/96 02:01 pm
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

		if ((pApp = CBagel::GetBagApp()) != NULL) {
			if ((pWin = pApp->GetMasterWnd()) != NULL) {
				pParent = pWin->GetCurrentStorageDev();

				CBofRect cRect(80, 10, 80 + 480 /*- 1 */, 10 + GetRect().Height() - 1 + 5);
				CBofPoint cPoint(0, 0);

				CBofPalette *pPal;
				pPal = pApp->GetPalette();

				CBofBitmap cBmp(cRect.Width(), cRect.Height(), pPal);

				cBmp.FillRect(NULL, pPal->GetNearestIndex(RGB(92, 92, 92)));

				cDlg.Create(pParent, pPal, &cRect);
				// cDlg.SetBackdrop(&cBmp);

				Update(cDlg.GetBackdrop(), cPoint, &cRect);

				CBagPDA *pPDA = NULL;
				sStr = "BPDA_WLD";
				pPDA = (CBagPDA *)SDEVMNGR->GetStorageDevice(sStr);

				// jwl 12.12.96 if we're in the zoom pda then put this box at the
				// bottom of the zoom rect.
				SBZoomPda *pPDAZ = NULL;
				sStr = "BPDAZ_WLD";
				pPDAZ = (SBZoomPda *)SDEVMNGR->GetStorageDevice(sStr);

				if (pPDAZ && pPDAZ->GetZoomed() == TRUE) {
					CBofRect zRect = pPDAZ->GetViewRect();
					Assert(zRect.Height() > 0 && zRect.Height() < 480);
					Assert(zRect.Width() > 0 && zRect.Width() < 640);
					cDlg.Move(80, zRect.bottom - cRect.Height(), TRUE); // xxx
				} else {
					if ((pPDA != NULL) && (pPDA->IsActivated() || pPDA->IsActivating())) {
						cDlg.Move(80, 10, TRUE);

					} else {
						INT x, y;

						x = 80;
						y = 360 + 10 - cRect.Height();
						cDlg.Move(x, y, TRUE);
					}
				}
				g_bPauseTimer = TRUE;
				cDlg.DoModal();
				g_bPauseTimer = FALSE;
			}
		}
	}

	return (FALSE);
}

INT MapFont(INT nFont) {
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
BOOL CBagTextObject::OnLButtonUp(UINT nFlags, CBofPoint xPoint, void *pv) {
	// If there's a residue printing object, then hand this guy off to
	// him, otherwise, call back to Cbagobj.
	CBagRPObject *pRPObj = (CBagRPObject *)GetRPObject();
	if (pRPObj) {
		return (pRPObj->OnLButtonUp(nFlags, xPoint, pv));
	}

	return CBagObject::OnLButtonUp(nFlags, xPoint, pv);
}

// We use this code in four places, so isolate it.
VOID CBagTextObject::RecalcTextRect(BOOL bTextFromFile) {
	CBagPanWindow *pPanWin; // The window where the object are displayed
	CBofRect ViewRect;      // The rect of the area where objects are displayed
	CBofSize cDisplaySize;  // Size of rect needed to display font
	CBofSize cSize;         // Size of rect needed to display font

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

	//
	// Get the area spanned by the text (i.e. Get the pixel width and
	// height of the text string).
	//
#if BOF_WINDOWS
	SIZE stTextSize; // font info about the text to be displayed
	HDC hDC;
	HFONT hFont, hFontOld; // font that was mapped to the context

	if ((hDC = pPanWin->GetDC()) != NULL) {

		// jwl 12.20.96 if we have a mono space font, use our utility routine to get our font handle
		if (GetFont() == FONT_MONO) {
			hFont = CBofText::GetMonoFont(m_nPointSize, 0);
		} else {
			hFont = CreateFont(m_nPointSize, 0, 0, 0, 0, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
		}
		hFontOld = (HFONT)SelectObject(hDC, hFont); // select it into our context

		Assert(m_psText != NULL);

#if BOF_WIN16 || BOF_WINMAC
		GetTextExtentPoint(hDC, m_psText->GetBuffer(), strlen(m_psText->GetBuffer()), &stTextSize);
#else
		GetTextExtentPoint32(hDC, m_psText->GetBuffer(), strlen(m_psText->GetBuffer()), &stTextSize);
#endif

		::SelectObject(hDC, hFontOld);
		pPanWin->ReleaseDC(hDC);
		::DeleteObject(hFont);
	}

	if (bTextFromFile) {

		// BCW - 11/29/96 12:43 pm
		// Add fudge factor to make sure that all the text will fit, and not
		// get cut off.  This may cause an extra blank line of text in some
		// captions, but tough diddles, it's still better than truncating
		// some text.
		//
		stTextSize.cx += 43;

		cSize.cx = stTextSize.cx;
		cSize.cy = stTextSize.cy;
	} else {
		cSize.cx = stTextSize.cx + 9;
		cSize.cy = stTextSize.cy + (m_bTitle ? 0 : 7);
		SetSize(cSize);
	}

#elif BOF_MAC
	FontInfo fInfo;
	GrafPtr curPort;

	::GetPort(&curPort);

	// jwl 08.05.96 set the text characteristics before calling textwidth.

	short saveTxSize = curPort->txSize;
	short saveTxFont = curPort->txFont;

	::TextFont(GetFont());
	::TextSize(ABS(MapWindowsPointSize(m_nPointSize)));

	// jwl 07.30.96 use toolbox to find text width.

	::GetFontInfo(&fInfo);

	Assert(m_psText != NULL);

	cSize.cx = ::TextWidth(m_psText->GetBuffer(), 0, strlen(m_psText->GetBuffer())) + 5;
	cSize.cy = fInfo.ascent + fInfo.descent + fInfo.leading;

	// jwl 11.27.97 add some to the height, we were not providing enough here.
	if (bTextFromFile == FALSE) {
		cSize.cy += (m_bTitle ? 0 : 5);
	}

	::TextFont(saveTxFont);
	::TextSize(saveTxSize);

	if (bTextFromFile == FALSE) {
		SetSize(cSize);
	}
#else
#endif
	if (bTextFromFile) {
		cDisplaySize.cx = ViewRect.Width();
		cDisplaySize.cy = cSize.cy;

		// If for some reason(CIC, CHAT) we got too large
		// a viewrect, cut it back to the correct width
		//
		if (cDisplaySize.cx > PAN_AREA_WIDTH)
			cDisplaySize.cx = PAN_AREA_WIDTH;

		// Buffer the size a little for spacing etc.
		// cSize.cy += 5;
		cDisplaySize.cx -= 5;

		// While the text is wider then the view area
		//
		while (cSize.cx > cDisplaySize.cx) {

			// Increment Display Height to account for another line
			cDisplaySize.cy += cSize.cy;

			// Decrement the size of text by the width of one line
			cSize.cx -= cDisplaySize.cx;
		}
		// add a little space at the bottom
		cDisplaySize.cy += 5;

		SetSize(cDisplaySize);
	}
}

VOID CBagTextObject::SetPSText(CBofString *p) {
	Assert(IsValidObject(this));

	if (m_psText != NULL) {
		delete m_psText;
		m_psText = NULL;
	}

	if (p != NULL) {
		m_psText = new CBofString(*p);
	}
}

} // namespace Bagel
