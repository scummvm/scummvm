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

#include "bagel/boflib/boffo.h"
#include "bagel/baglib/dev_dlg.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/baglib/bagel.h"

namespace Bagel {

#define MENU_DFLT_HEIGHT 20

CDevDlg::CDevDlg(int nButtonX, int nButtonY) {
	m_pTitleText = nullptr;
	m_nButtonX = nButtonX;
	m_nButtonY = nButtonY;
	m_achGuess[0] = '\0';
	m_nGuessCount = 0;
	m_bUseExtra = false;
}


CDevDlg::~CDevDlg() {
	if (m_pTitleText != nullptr) {
		delete m_pTitleText;
		m_pTitleText = nullptr;
	}
}


ErrorCode CDevDlg::Create(const char *pszBmp, CBofWindow *pWnd, CBofPalette *pPal, CBofRect *pRect, bool bUseEx) {
	Assert(IsValidObject(this));
	Assert(pszBmp != nullptr);
	Assert(pWnd != nullptr);
	Assert(pPal != nullptr);
	Assert(pRect != nullptr);

	CBofString str = "CDevDlg";
	CBofRect cDlgRect, r, txtRect;

	m_bUseExtra = bUseEx;

	if (!pRect) {
		r = pWnd->GetWindowRect();
		r.OffsetRect(-r.left, -r.top);
		r.bottom = r.top + MENU_DFLT_HEIGHT;
	} else {
		r = *pRect;
	}

	if ((m_pGuessText = new CBofText()) != nullptr) {
		m_pGuessText->SetupText(pRect, JUSTIFY_LEFT);
	}

	m_nGuessCount = 0;
	Common::fill(m_achGuess, m_achGuess + MAX_CHARS, 0);

	CBofBitmap *pBmp = nullptr;

	if (pszBmp != nullptr) {

		if ((pBmp = new CBofBitmap(pszBmp, pPal)) != nullptr) {

		} else {
			ReportError(ERR_MEMORY);
		}

	}

	// Fall back to original dialog on failure
	if (pBmp == nullptr) {
		if ((pBmp = new CBofBitmap(200, 100, pPal)) != nullptr) {
			Assert(pPal != nullptr);

			pBmp->FillRect(nullptr, pPal->GetNearestIndex(RGB(92, 92, 92)) /*RGB(0,0,0)*/);

			CBofRect rect(pBmp->GetRect());
			pBmp->DrawRect(&rect, pPal->GetNearestIndex(RGB(0, 0, 0)) /*RGB(0,0,0)*/);
			pBmp->FillRect(pRect, pPal->GetNearestIndex(RGB(255, 255, 255)));

		} else {
			ReportError(ERR_MEMORY);
		}
	}

	Assert(pBmp != nullptr);
	CBofRect rect(pBmp->GetRect());

	CBagStorageDevDlg::Create(str, &rect, pWnd, 0);
	Center();
	SetBackdrop(pBmp);

	return m_errCode;
}


void CDevDlg::OnLButtonUp(uint32 /*nFlags*/, CBofPoint * /*pPoint*/, void *) {
}


void CDevDlg::OnMouseMove(uint32 /*nFlags*/, CBofPoint * /*pPoint*/, void *) {
}


void CDevDlg::OnClose() {
	Assert(IsValidObject(this));

	CBagVar *pVar;
	if ((pVar = VARMNGR->GetVariable("DIALOGRETURN")) != nullptr) {
		// If we need to parse the input for 2 words (Deven-7 Code words)
		if (m_bUseExtra) {
			// Find the break
			char *p;
			if (((p = strchr(m_achGuess, ',')) != nullptr) || ((p = strchr(m_achGuess, ' ')) != nullptr)) {
				CBagVar *pVarEx;

				*p = '\0';
				p++;

				// Set variable 2 (DIALOGRETURN2)
				if ((pVarEx = VARMNGR->GetVariable("DIALOGRETURN2")) != nullptr) {
					pVarEx->SetValue(p);
				}
			}
		}

		// Set variable 1 (DIALOGRETURN)
		pVar->SetValue(m_achGuess);
	}

	if (m_pGuessText != nullptr) {     // last text area
		delete m_pGuessText;
		m_pGuessText = nullptr;
	}

	CBagStorageDevDlg::OnClose();
}

void CDevDlg::OnKeyHit(uint32 lKeyCode, uint32 nRepCount) {
	Assert(IsValidObject(this));
	bool bPaintGuess = false;

	if (m_nGuessCount < MAX_CHARS) {
		const char *const MISC_KEYS = " \'$,-%.";

		if (Common::isAlnum(lKeyCode) || strchr(MISC_KEYS, lKeyCode)) {
			m_achGuess[m_nGuessCount] = toupper(lKeyCode);
			m_nGuessCount++;
			bPaintGuess = true;

		} else if (lKeyCode == BKEY_BACK && (m_nGuessCount - 1) >= 0) {
			m_nGuessCount--;
			m_achGuess[m_nGuessCount] = 0;
			bPaintGuess = true;
		}

		if (bPaintGuess) {
			PaintText();
		}

		// Close dialog box when enter key is hit
		if (lKeyCode ==  BKEY_ENTER) {
			OnClose();
		}
	}

	// If it maxes out, clear it
	if (m_nGuessCount >= MAX_CHARS) {
		Common::fill(m_achGuess, m_achGuess + MAX_CHARS, 0);
		m_nGuessCount = 0;
	}

	CBagStorageDevDlg::OnKeyHit(lKeyCode, nRepCount);
}


void CDevDlg::PaintText() {
	Assert(IsValidObject(this));

	char achTemp[MAX_CHARS];
	snprintf(achTemp, MAX_CHARS, "%s", m_achGuess);

	m_pGuessText->Display(GetBackdrop(), achTemp, 16, TEXT_MEDIUM);
}

void CDevDlg::SetText(CBofString &tStr, CBofRect *pcRect) {
	Assert(IsValidObject(this));

	if ((m_pTitleText = new CBofText) != nullptr) {
		m_pTitleText->SetupText(pcRect, JUSTIFY_CENTER, FORMAT_DEFAULT);
		m_pTitleText->SetColor(RGB(255, 255, 255));
		m_pTitleText->SetSize(FONT_14POINT);
		m_pTitleText->SetWeight(TEXT_BOLD);
		m_pTitleText->SetText(tStr);
	}
}

// Override on render to do the painting, but call the default anyway.
ErrorCode CDevDlg::OnRender(CBofBitmap *pBmp, CBofRect *pRect) {
	Assert(IsValidObject(this));

	ErrorCode err = CBagStorageDevDlg::OnRender(pBmp, pRect);

	if (m_pTitleText != nullptr) {
		m_pTitleText->Display(GetBackdrop());
	}

	return err;
}

} // namespace Bagel
