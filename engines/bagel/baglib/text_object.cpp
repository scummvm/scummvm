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
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/baglib/zoom_pda.h"
#include "bagel/baglib/base_pda.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/rp_object.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {

#define CTEXT_WHITE RGB(255, 255, 255)

extern bool g_bPauseTimer;

CBagTextObject::CBagTextObject() : CBagObject() {
	m_xObjType = TEXTOBJ;
	m_nDX = 80;
	_nDY = 20;
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
	detach();
}

CBofRect CBagTextObject::getRect() {
	CBofPoint p = getPosition();
	CBofSize s = getSize();
	CBofRect r = CBofRect(p, s);
	return r;
}

ErrorCode CBagTextObject::update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);
	Assert(pSrcRect != nullptr);

	// assume no error
	 ErrorCode errCode = ERR_NONE;

	if ((pBmp != nullptr) && isAttached() && !(getText().IsEmpty())) {

		if (pBmp->getRect().PtInRect(pt)) {

			CBofRect r(pt, pSrcRect->Size());

			int nPointSize = m_nPointSize;
			int nFormat = FORMAT_CENTER_LEFT;
			if (!m_bTitle) {

				CBofRect cBevel;
				int i;

				byte c1 = 3;
				byte c2 = 9;

				cBevel.IntersectRect(pBmp->getRect(), r);

				int left = cBevel.left;
				int top = cBevel.top;
				int right = cBevel.right;
				int bottom = cBevel.bottom;

				r.left += 6;
				r.top += 3;
				r.right -= 5;
				r.bottom -= 5;

				for (i = 1; i <= 3; i++) {
					pBmp->line(left + i, bottom - i, right - i, bottom - i, c1);
					pBmp->line(right - i, bottom - i, right - i, top + i - 1, c1);
				}

				for (i = 1; i <= 3; i++) {
					pBmp->line(left + i, bottom - i, left + i, top + i - 1, c2);
					pBmp->line(left + i, top + i - 1, right - i, top + i - 1, c2);
				}
				nPointSize = 16;
				nFormat = FORMAT_TOP_LEFT;

			} else {
				r.left += 1;
			}

			errCode = paintText(pBmp, &r, getText(), MapWindowsPointSize(nPointSize), TEXT_NORMAL, m_nFGColor, JUSTIFY_WRAP, nFormat, m_nTextFont);

			// This object does not need to be updated now...
			SetDirty(false);
		}
	}

	return errCode;
}

ErrorCode CBagTextObject::attach() {
	Assert(IsValidObject(this));

	if (!getFileName().Right(4).Find(".TXT") || !getFileName().Right(4).Find(".txt")) {
		// Prevent memory leak
		if (m_psText != nullptr) {
			delete m_psText;
			m_psText = nullptr;
		}

		// Allocate a new string
		m_psText = new CBofString;
		if (m_psText != nullptr) {
			CBofFile fpTextFile(getFileName());

			if (!fpTextFile.ErrorOccurred()) {
				// Allocate the buffers
				uint32 nFileLen = fpTextFile.GetLength();
				char *pTextBuff = (char *)BofCAlloc(nFileLen + 1, 1);
				if (pTextBuff != nullptr) {
					// Read the text file into buffers
					fpTextFile.Read(pTextBuff, nFileLen);
					fpTextFile.close();

					*m_psText += pTextBuff;

					if (m_psInitInfo != nullptr) {
						CBagVar *pVar = VAR_MANAGER->GetVariable(*m_psInitInfo);

						if (pVar != nullptr) {
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
		// Prevent memory leak
		if (m_psText != nullptr) {
			delete m_psText;
			m_psText = nullptr;
		}

		// Allocate a new string
		if ((m_psText = new CBofString) != nullptr) {
			*m_psText = getFileName();

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

	return CBagObject::attach();
}

ErrorCode CBagTextObject::detach() {
	Assert(IsValidObject(this));

	if (m_psText != nullptr) {
		delete m_psText;
		m_psText = nullptr;
	}

	return CBagObject::detach();
}

const CBofString &CBagTextObject::getText() {
	if (m_psText)
		return *m_psText;

	return getFileName();
}

void CBagTextObject::setText(const CBofString &s) {
	if (m_psText) {
		*m_psText = s;
	} else {
		SetFileName(s);
	}

	RecalcTextRect(!getFileName().Right(4).Find(".TXT") || !getFileName().Right(4).Find(".txt"));
}

//   Takes in info and then removes the relative information and returns the info
//   without the relevant info.
//
PARSE_CODES CBagTextObject::setInfo(CBagIfstream &istr) {
	bool nObjectUpdated = false;

	while (!istr.eof()) {
		istr.eatWhite();

		char ch = (char)istr.peek();
		switch (ch) {

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
				istr.eatWhite();
				GetAlphaNumFromStream(istr, sStr);
				SetInitInfo(sStr);
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
			CBofString sStr(szLocalStr, 256);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("SIZE")) {
				istr.eatWhite();
				int n;
				GetIntFromStream(istr, n);
				m_nPointSize = (byte)n;
				nObjectUpdated = true;

				// WORKAROUND: Reduce the font size of Cilia's full-screen log
				// in ScummVM so that it fits on the screen
				if (m_nPointSize == 28 && istr.getSize() == 359105)
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
				istr.eatWhite();
				int n;
				GetIntFromStream(istr, n);
				m_nTextFont = MapFont(n);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			// FIXME: Missing break?
		}
		// fallthrough

		//
		//  AS [CAPTION]  - how to run the link
		//
		case 'A': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("AS")) {
				istr.eatWhite();
				GetAlphaNumFromStream(istr, sStr);
				if (!sStr.Find("CAPTION")) {
					m_bCaption = true;
					nObjectUpdated = true;

				} else if (!sStr.Find("TITLE")) {
					m_bTitle = true;
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
				istr.eatWhite();
				GetIntFromStream(istr, nColor);
				setColor(nColor);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		// No match return from funtion
		//
		default: {
			PARSE_CODES rc;
			if ((rc = CBagObject::setInfo(istr)) == PARSING_DONE) {
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

void CBagTextObject::setColor(int nColor) {
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

void CBagTextObject::setProperty(const CBofString &sProp, int nVal) {
	if (!sProp.Find("SIZE"))
		setPointSize(nVal);
	else if (!sProp.Find("FONT"))
		setFont(MapFont(nVal));
	else if (!sProp.Find("COLOR"))
		setColor(nVal);
	else
		CBagObject::setProperty(sProp, nVal);
}

int CBagTextObject::getProperty(const CBofString &sProp) {
	if (!sProp.Find("SIZE"))
		return getPointSize();
	if (!sProp.Find("FONT"))
		return getFont();
	if (!sProp.Find("COLOR"))
		return getColor();

	return CBagObject::getProperty(sProp);
}

bool CBagTextObject::runObject() {
	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	if (m_bCaption && IsImmediateRun()) {

		// Re-attach this object to get any change in a variable
		// (Must be using the VAR token).
		if (m_bReAttach) {
			attach();
		}

		CBagel *pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			CBagMasterWin *pWin = pApp->getMasterWnd();
			if (pWin != nullptr) {
				CBagStorageDevWnd *pParent = pWin->getCurrentStorageDev();

				CBofRect cRect(80, 10, 80 + 480 /*- 1 */, 10 + getRect().height() - 1 + 5);
				CBofPoint cPoint(0, 0);

				CBofPalette *pPal = pApp->getPalette();
				CBofBitmap cBmp(cRect.width(), cRect.height(), pPal);

				cBmp.fillRect(nullptr, pPal->getNearestIndex(RGB(92, 92, 92)));

				CBagMenuDlg cDlg;
				cDlg.create(pParent, pPal, &cRect);

				update(cDlg.getBackdrop(), cPoint, &cRect);

				CBagPDA *pPDA = nullptr;
				sStr = "BPDA_WLD";
				pPDA = (CBagPDA *)SDEV_MANAGER->GetStorageDevice(sStr);

				// If we're in the zoom pda then put this box at the
				// bottom of the zoom rect.
				SBZoomPda *pPDAZ = nullptr;
				sStr = "BPDAZ_WLD";
				pPDAZ = (SBZoomPda *)SDEV_MANAGER->GetStorageDevice(sStr);

				if (pPDAZ && pPDAZ->getZoomed() == true) {
					CBofRect zRect = pPDAZ->getViewRect();
					Assert(zRect.height() > 0 && zRect.height() < 480);
					Assert(zRect.width() > 0 && zRect.width() < 640);
					cDlg.move(80, zRect.bottom - cRect.height(), true); // xxx
				} else if ((pPDA != nullptr) && (pPDA->isActivated() || pPDA->isActivating())) {
					cDlg.move(80, 10, true);

				} else {
					int x = 80;
					int y = 360 + 10 - cRect.height();
					cDlg.move(x, y, true);
				}
				g_bPauseTimer = true;
				cDlg.doModal();
				g_bPauseTimer = false;
			}
		}
	}

	return false;
}

int MapFont(int nFont) {
	if (nFont == 0)
		return FONT_MONO;

	return FONT_DEFAULT;
}

void CBagTextObject::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *pv) {
	// If there's a residue printing object, then hand this guy off to
	// him, otherwise, call back to Cbagobj.
	CBagRPObject *pRPObj = (CBagRPObject *)GetRPObject();
	if (pRPObj) {
		pRPObj->onLButtonUp(nFlags, xPoint, pv);
		return;
	}

	CBagObject::onLButtonUp(nFlags, xPoint, pv);
}

void CBagTextObject::RecalcTextRect(bool bTextFromFile) {
	CBofRect ViewRect;      // The rect of the area where objects are displayed
	CBofSize cDisplaySize;  // Size of rect needed to display font
	CBofSize cSize;         // Size of rect needed to display font

	Assert(m_psText != nullptr);

	// The window where the object are displayed
	CBagPanWindow *pPanWin = (CBagPanWindow *)(CBagel::getBagApp()->getMasterWnd()->getCurrentGameWindow());
	if (bTextFromFile) {
		if (pPanWin->GetDeviceType() == SDEV_GAMEWIN) {
			ViewRect = pPanWin->GetViewPort();
		} else {
			ViewRect = pPanWin->getClientRect();
		}
	}

	if (ViewRect.IsRectEmpty()) {
		ViewRect.SetRect(80, 10, 480 + 80 - 1, 360 + 10 - 1);
	}

	// Get the area spanned by the text (i.e. Get the pixel width and
	// height of the text string).
	CBofRect tmpRect = ViewRect;
	if (!m_bTitle) {
		// Exactly match the width used in displayTextEx
		tmpRect.left += 5;
		tmpRect.right = (ViewRect.right == 640 ? PAN_AREA_WIDTH : ViewRect.right) - 5;
	}

	CBofRect textRect = calculateTextRect(tmpRect, m_psText, m_nPointSize, getFont());
	CBofSize stTextSize(textRect.right, textRect.bottom);

	if (bTextFromFile) {
		// Add fudge factor to make sure that all the text will fit, and not
		// get cut off.  This may cause an extra blank line of text in some
		// captions, but tough diddles, it's still better than truncating
		// some text.

		cSize.cx = stTextSize.cx;
		cSize.cy = stTextSize.cy;
	} else {
		cSize.cx = stTextSize.cx + 9;
		cSize.cy = stTextSize.cy + (m_bTitle ? 0 : 7);
		setSize(cSize);
	}

	if (bTextFromFile) {
		cDisplaySize.cx = ViewRect.width();
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

		setSize(cDisplaySize);
	}
}

void CBagTextObject::SetPSText(CBofString *p) {
	Assert(IsValidObject(this));

	delete m_psText;
	m_psText = nullptr;

	if (p != nullptr) {
		m_psText = new CBofString(*p);
	}
}

} // namespace Bagel
