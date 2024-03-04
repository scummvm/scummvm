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

CDevDlg::CDevDlg(INT nButtonX, INT nButtonY) {
	m_pTitleText = NULL;
	m_nButtonX = nButtonX;
	m_nButtonY = nButtonY;
	m_achGuess[0] = '\0';
	m_nGuessCount = 0;
	m_bUseExtra = FALSE;
}


CDevDlg::~CDevDlg() {
	if (m_pTitleText != NULL) {
		delete m_pTitleText;
		m_pTitleText = NULL;
	}
}


ERROR_CODE CDevDlg::Create(const CHAR *pszBmp, CBofWindow *pWnd, CBofPalette *pPal, CBofRect *pRect, BOOL bUseEx) {
	Assert(IsValidObject(this));
	Assert(pszBmp != NULL);
	Assert(pWnd != NULL);
	Assert(pPal != NULL);
	Assert(pRect != NULL);

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

	if ((m_pGuessText = new CBofText()) != NULL) {
		m_pGuessText->SetupText(pRect, JUSTIFY_LEFT);
	}
	m_nGuessCount = 0;
	memset(m_achGuess, 0, MAX_CHARS * sizeof(CHAR));         // Null out guess

	CBofBitmap *pBmp;

	pBmp = NULL;

	if (pszBmp != NULL) {

		if ((pBmp = new CBofBitmap(pszBmp, pPal)) != NULL) {

		} else {
			ReportError(ERR_MEMORY);
		}

	}

	// Fall back to original dialog on failure
	//
	if (pBmp == NULL) {

		if ((pBmp = new CBofBitmap(200, 100, pPal)) != NULL) {

			Assert(pPal != NULL);
			pBmp->FillRect(NULL, pPal->GetNearestIndex(RGB(92, 92, 92)) /*RGB(0,0,0)*/);
			CBofRect rect(pBmp->GetRect());
			pBmp->DrawRect(&rect, pPal->GetNearestIndex(RGB(0, 0, 0)) /*RGB(0,0,0)*/);
			pBmp->FillRect(pRect, pPal->GetNearestIndex(RGB(255, 255, 255)));

		} else {
			ReportError(ERR_MEMORY);
		}
	}
	Assert(pBmp != NULL);

	CBofRect rect(pBmp->GetRect());

	CBagStorageDevDlg::Create(str, &rect, pWnd, 0);
	Center();
	SetBackdrop(pBmp);

	return (m_errCode);
}


VOID CDevDlg::OnLButtonUp(UINT /*nFlags*/, CBofPoint * /*pPoint*/) {
}


VOID CDevDlg::OnMouseMove(UINT /*nFlags*/, CBofPoint * /*pPoint*/) {
}


VOID CDevDlg::OnClose() {
	Assert(IsValidObject(this));

	CBagVar *pVar;
	CHAR *p;

	if ((pVar = VARMNGR->GetVariable("DIALOGRETURN")) != NULL) {

		// If we need to parse the input for 2 words (Deven-7 Code words)
		//
		if (m_bUseExtra) {

			// Find the break
			//
			if (((p = strchr(m_achGuess, ',')) != NULL) || ((p = strchr(m_achGuess, ' ')) != NULL)) {

				CBagVar *pVarEx;

				*p = '\0';
				p++;

				// Set variable 2 (DIALOGRETURN2)
				//
				if ((pVarEx = VARMNGR->GetVariable("DIALOGRETURN2")) != NULL) {
					pVarEx->SetValue(p);
				}
			}
		}

		// Set variable 1 (DIALOGRETURN)
		pVar->SetValue(m_achGuess);
	}

	if (m_pGuessText != NULL) {     // last text area
		delete m_pGuessText;
		m_pGuessText = NULL;
	}

	CBagStorageDevDlg::OnClose();
}

VOID CDevDlg::OnKeyHit(ULONG lKeyCode, ULONG nRepCount) {
	Assert(IsValidObject(this));

	BOOL    bPaintGuess = FALSE;

	if (m_nGuessCount < MAX_CHARS) {
		switch (lKeyCode) {
		case BKEY_SPACE:
			m_achGuess[m_nGuessCount] = ' ';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_a:
		case BKEY_A:
			m_achGuess[m_nGuessCount] = 'A';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_b:
		case BKEY_B:
			m_achGuess[m_nGuessCount] = 'B';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_c:
		case BKEY_C:
			m_achGuess[m_nGuessCount] = 'C';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_d:
		case BKEY_D:
			m_achGuess[m_nGuessCount] = 'D';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_e:
		case BKEY_E:
			m_achGuess[m_nGuessCount] = 'E';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_f:
		case BKEY_F:
			m_achGuess[m_nGuessCount] = 'F';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_g:
		case BKEY_G:
			m_achGuess[m_nGuessCount] = 'G';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_h:
		case BKEY_H:
			m_achGuess[m_nGuessCount] = 'H';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_i:
		case BKEY_I:
			m_achGuess[m_nGuessCount] = 'I';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_j:
		case BKEY_J:
			m_achGuess[m_nGuessCount] = 'J';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_k:
		case BKEY_K:
			m_achGuess[m_nGuessCount] = 'K';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_l:
		case BKEY_L:
			m_achGuess[m_nGuessCount] = 'L';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_m:
		case BKEY_M:
			m_achGuess[m_nGuessCount] = 'M';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_n:
		case BKEY_N:
			m_achGuess[m_nGuessCount] = 'N';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_o:
		case BKEY_O:
			m_achGuess[m_nGuessCount] = 'O';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_p:
		case BKEY_P:
			m_achGuess[m_nGuessCount] = 'P';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_q:
		case BKEY_Q:
			m_achGuess[m_nGuessCount] = 'Q';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_r:
		case BKEY_R:
			m_achGuess[m_nGuessCount] = 'R';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_s:
		case BKEY_S:
			m_achGuess[m_nGuessCount] = 'S';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_t:
		case BKEY_T:
			m_achGuess[m_nGuessCount] = 'T';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_u:
		case BKEY_U:
			m_achGuess[m_nGuessCount] = 'U';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_v:
		case BKEY_V:
			m_achGuess[m_nGuessCount] = 'V';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_w:
		case BKEY_W:
			m_achGuess[m_nGuessCount] = 'W';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_x:
		case BKEY_X:
			m_achGuess[m_nGuessCount] = 'X';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_y:
		case BKEY_Y:
			m_achGuess[m_nGuessCount] = 'Y';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_z:
		case BKEY_Z:
			m_achGuess[m_nGuessCount] = 'Z';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_1:
			m_achGuess[m_nGuessCount] = '1';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_2:
			m_achGuess[m_nGuessCount] = '2';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_3:
			m_achGuess[m_nGuessCount] = '3';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_4:
			m_achGuess[m_nGuessCount] = '4';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_5:
			m_achGuess[m_nGuessCount] = '5';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_6:
			m_achGuess[m_nGuessCount] = '6';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_7:
			m_achGuess[m_nGuessCount] = '7';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_8:
			m_achGuess[m_nGuessCount] = '8';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_9:
			m_achGuess[m_nGuessCount] = '9';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;
		case BKEY_0:
			m_achGuess[m_nGuessCount] = '0';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;

		case 0x0027:
			m_achGuess[m_nGuessCount] = '\'';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;

		case 0x0024:
			m_achGuess[m_nGuessCount] = '$';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;

		case 0x002c:
			m_achGuess[m_nGuessCount] = ',';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;

		case 0x002d:
			m_achGuess[m_nGuessCount] = '-';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;

		case 0x0025:
			m_achGuess[m_nGuessCount] = '%';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;

		case 0x002e:
			m_achGuess[m_nGuessCount] = '.';
			m_nGuessCount++;
			bPaintGuess = TRUE;
			break;

		}
		if (lKeyCode == BKEY_BACK) {
			if ((m_nGuessCount - 1) >= 0) {
				m_nGuessCount--;
				m_achGuess[m_nGuessCount] = 0;
				bPaintGuess = TRUE;
			}
		}

		if (bPaintGuess) {
			PaintText();
		}
		// kill dialog box when enter key is hit
		if (lKeyCode ==  BKEY_ENTER) {
			OnClose();
		}
	}
	// if it maxes out, clear it
	if (m_nGuessCount >= MAX_CHARS) {
		memset(m_achGuess, 0, MAX_CHARS * sizeof(CHAR));          // Null out guess
		m_nGuessCount = 0;
	}

	CBagStorageDevDlg::OnKeyHit(lKeyCode, nRepCount);
}


VOID CDevDlg::PaintText() {
	Assert(IsValidObject(this));

	CHAR    achTemp[MAX_CHARS];

	snprintf(achTemp, MAX_CHARS, "%s", m_achGuess);
	m_pGuessText->Display(GetBackdrop(), achTemp, 16, TEXT_MEDIUM);
}

VOID CDevDlg::SetText(CBofString &tStr, CBofRect *pcRect) {
	Assert(IsValidObject(this));

	if ((m_pTitleText = new CBofText) != NULL) {
		m_pTitleText->SetupText(pcRect, JUSTIFY_CENTER, FORMAT_DEFAULT);
		m_pTitleText->SetColor(RGB(255, 255, 255));
		m_pTitleText->SetSize(FONT_14POINT);
		m_pTitleText->SetWeight(TEXT_BOLD);
		m_pTitleText->SetText(tStr);
	}
}

// Override on render to do the painting, but call the default anyway.
ERROR_CODE CDevDlg::OnRender(CBofBitmap *pBmp, CBofRect *pRect) {
	Assert(IsValidObject(this));

	ERROR_CODE err = CBagStorageDevDlg::OnRender(pBmp, pRect);

	if (m_pTitleText != NULL) {
		m_pTitleText->Display(GetBackdrop());
	}

	return (err);
}

} // namespace Bagel
