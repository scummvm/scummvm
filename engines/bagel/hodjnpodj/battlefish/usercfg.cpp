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

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/battlefish/usercfg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Battlefish {

#define ID_RESET    104
#define ID_LIMIT    105

#define ID_PLAYER    106
#define ID_COMPUTER  107

#define PAGE_SIZE   1

static const char *pszDiffLevel[DIFF_MAX + 1] = {
	"Wimpy",
	"Average",
	"Hefty"
};

extern const char *INI_SECTION;


CUserCfgDlg::CUserCfgDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
	: CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP") {
	m_pOKButton = nullptr;
	m_pCancelButton = nullptr;
	m_pDefaultsButton = nullptr;
	m_pUserButton = nullptr;
	m_pCompButton = nullptr;
	DoModal();
}

bool CUserCfgDlg::OnInitDialog() {
	CRect tmpRect;
	CDC *pDC;

	CBmpDialog::OnInitDialog();

	m_pTxtLevel = nullptr;
	m_pTxtDifficulty = nullptr;

	tmpRect.SetRect(22, 135, 122, 155);
	if ((m_pScrollBar = new CScrollBar) != nullptr) {
		m_pScrollBar->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_LIMIT);
		m_pScrollBar->SetScrollRange(DIFF_MIN, DIFF_MAX, true);
	}

	if ((pDC = GetDC()) != nullptr) {

		tmpRect.SetRect(25, 111, 80, 131);
		if ((m_pTxtLevel = new CText) != nullptr) {
			m_pTxtLevel->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
		}

		tmpRect.SetRect(81, 111, 146, 131);
		if ((m_pTxtDifficulty = new CText) != nullptr) {
			m_pTxtDifficulty->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
		}

		tmpRect.SetRect(25, 20, 150, 40);
		if ((m_pTxtOrder = new CText) != nullptr) {
			m_pTxtOrder->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
		}

		ReleaseDC(pDC);
	}

	if ((m_pOKButton = new CColorButton) != nullptr) {       // build a color QUIT button to let us exit
		(*m_pOKButton).SetPalette(m_pPalette);            // set the palette to use
		(*m_pOKButton).SetControl(IDOK, this);            // tie to the dialog control
	}

	if ((m_pCancelButton = new CColorButton) != nullptr) {   // build a color QUIT button to let us exit
		(*m_pCancelButton).SetPalette(m_pPalette);        // set the palette to use
		(*m_pCancelButton).SetControl(IDCANCEL, this);    // tie to the dialog control
	}

	if ((m_pDefaultsButton = new CColorButton) != nullptr) { // build a color QUIT button to let us exit
		(*m_pDefaultsButton).SetPalette(m_pPalette);      // set the palette to use
		(*m_pDefaultsButton).SetControl(ID_RESET, this);  // tie to the dialog control
	}

	if ((m_pUserButton = new CRadioButton) != nullptr) {
		tmpRect.SetRect(21, 32, 75, 45);
		//m_pUserButton->Create("Human", BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP | BS_OWNERDRAW, tmpRect, this, ID_PLAYER);
		m_pUserButton->SetPalette(m_pPalette);
		m_pUserButton->SetControl(ID_PLAYER, this);
	}

	if ((m_pCompButton = new CRadioButton) != nullptr) {
		tmpRect.SetRect(21, 45, 75, 58);
		//m_pUserButton->Create("Computer", BS_AUTORADIOBUTTON | WS_TABSTOP | BS_OWNERDRAW, tmpRect, this, ID_COMPUTER);
		m_pCompButton->SetPalette(m_pPalette);
		m_pCompButton->SetControl(ID_COMPUTER, this);
	}

	m_bSave = false;

	LoadIniSettings();

	PutDlgData();
	return true;
}


void CUserCfgDlg::PutDlgData() {
	m_pScrollBar->SetScrollPos(m_nDifficultyLevel);

	m_pUserButton->SetCheck(m_bUserGoesFirst);
	m_pCompButton->SetCheck(!m_bUserGoesFirst);
}

void CUserCfgDlg::GetDlgData() {
	m_nDifficultyLevel = m_pScrollBar->GetScrollPos();

	m_bUserGoesFirst = false;
	if (m_pUserButton->GetCheck() == 1)
		m_bUserGoesFirst = true;
}


bool CUserCfgDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
	//
	// respond to user
	//
	if (HIWORD(lParam) == BN_CLICKED) {

		switch (wParam) {

		case IDOK:
			m_bSave = true;
			ClearDialogImage();
			EndDialog(IDOK);
			return false;

		case IDCANCEL:
			ClearDialogImage();
			EndDialog(IDCANCEL);
			return false;

		case ID_PLAYER:
			m_bUserGoesFirst = true;
			PutDlgData();
			break;

		case ID_COMPUTER:
			m_bUserGoesFirst = false;
			PutDlgData();
			break;

		/*
		* reset params to default
		*/
		case ID_RESET:

			m_nDifficultyLevel = DIFF_DEF;
			m_bUserGoesFirst = TURN_DEF;

			PutDlgData();
			DispLimit();
			break;

		default:
			break;
		}
	}

	return CBmpDialog::OnCommand(wParam, lParam);
}

void CUserCfgDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar *pScroll) {
	switch (nSBCode) {

	case SB_LEFT:
		m_nDifficultyLevel = DIFF_MIN;
		break;

	case SB_PAGELEFT:
		m_nDifficultyLevel -= PAGE_SIZE;
		break;

	case SB_LINELEFT:
		if (m_nDifficultyLevel > DIFF_MIN)
			m_nDifficultyLevel--;
		break;

	case SB_RIGHT:
		m_nDifficultyLevel = DIFF_MAX;
		break;

	case SB_PAGERIGHT:
		m_nDifficultyLevel += PAGE_SIZE;
		break;

	case SB_LINERIGHT:
		if (m_nDifficultyLevel < DIFF_MAX)
			m_nDifficultyLevel++;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_nDifficultyLevel = nPos;
		break;

	default:
		break;
	}

	if (m_nDifficultyLevel <= DIFF_MIN)
		m_nDifficultyLevel = DIFF_MIN;
	if (m_nDifficultyLevel > DIFF_MAX)
		m_nDifficultyLevel = DIFF_MAX;

	// can't access a null pointer
	assert(pScroll != nullptr);

	pScroll->SetScrollPos(m_nDifficultyLevel);

	DispLimit();
}


/*****************************************************************
*
*  LoadIniSettings
*
*  FUNCTIONAL DESCRIPTION:
*
*       Loads this game's parameters from .INI file
*
*  FORMAL PARAMETERS:
*
*       None
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
void CUserCfgDlg::LoadIniSettings() {
	int nVal;

	// Get the Difficulty level  (0..2)
	//
	nVal = GetPrivateProfileInt(INI_SECTION, "DifficultyLevel", DIFF_DEF, INI_FILENAME);
	m_nDifficultyLevel = nVal;
	if (nVal < DIFF_MIN || nVal > DIFF_MAX)
		m_nDifficultyLevel = DIFF_DEF;

	// Get the UserGoesFirst option setting
	//
	nVal = GetPrivateProfileInt(INI_SECTION, "UserGoesFirst", TURN_DEF, INI_FILENAME);
	m_bUserGoesFirst = (nVal == 0 ? false : true);
}

void CUserCfgDlg::SaveIniSettings() {
	WritePrivateProfileString(INI_SECTION,
	                          "DifficultyLevel",
	                          Common::String::format("%d", m_nDifficultyLevel).c_str(),
	                          INI_FILENAME);

	WritePrivateProfileString(INI_SECTION,
	                          "UserGoesFirst",
	                          Common::String::format("%d", m_bUserGoesFirst).c_str(),
	                          INI_FILENAME);
}

void CUserCfgDlg::OnPaint() {
	CDC *pDC;

	CBmpDialog::OnPaint();

	if ((pDC = GetDC()) != nullptr) {
		m_pTxtLevel->DisplayString(pDC, "Difficulty:", 14, TEXT_BOLD, RGB(0, 0, 0));
		m_pTxtOrder->DisplayString(pDC, "Who goes first?", 14, TEXT_BOLD, RGB(0, 0, 0));
		ReleaseDC(pDC);
	}

	DispLimit();
}

void CUserCfgDlg::OnDestroy() {
	if (m_pOKButton != nullptr) {                          // release the button
		delete m_pOKButton;
		m_pOKButton = nullptr;
	}

	if (m_pCancelButton != nullptr) {                        // release the button
		delete m_pCancelButton;
		m_pCancelButton = nullptr;
	}

	if (m_pDefaultsButton != nullptr) {                    // release the button
		delete m_pDefaultsButton;
		m_pDefaultsButton = nullptr;
	}

	assert(m_pTxtDifficulty != nullptr);
	if (m_pTxtDifficulty != nullptr) {
		delete m_pTxtDifficulty;
		m_pTxtDifficulty = nullptr;
	}

	assert(m_pTxtLevel != nullptr);
	if (m_pTxtLevel != nullptr) {
		delete m_pTxtLevel;
		m_pTxtLevel = nullptr;
	}

	if (m_pTxtOrder != nullptr) {
		delete m_pTxtOrder;
		m_pTxtOrder = nullptr;
	}

	//
	// de-allocate the scroll bar
	//
	assert(m_pScrollBar != nullptr);
	if (m_pScrollBar != nullptr) {
		delete m_pScrollBar;
		m_pScrollBar = nullptr;
	}

	CBmpDialog::OnDestroy();
}


void CUserCfgDlg::ClearDialogImage() {
	if (m_bSave) {
		GetDlgData();
		SaveIniSettings();
	}

	if (m_pCompButton != nullptr) {
		delete m_pCompButton;
		m_pCompButton = nullptr;
	}

	if (m_pUserButton != nullptr) {
		delete m_pUserButton;
		m_pUserButton = nullptr;
	}

	if (m_pOKButton != nullptr) {                          // release the button
		delete m_pOKButton;
		m_pOKButton = nullptr;
	}

	if (m_pCancelButton != nullptr) {                        // release the button
		delete m_pCancelButton;
		m_pCancelButton = nullptr;
	}

	if (m_pDefaultsButton != nullptr) {                    // release the button
		delete m_pDefaultsButton;
		m_pDefaultsButton = nullptr;
	}

	ValidateRect(nullptr);
}

void CUserCfgDlg::DispLimit() {
	CDC *pDC;

	if ((pDC = GetDC()) != nullptr) {

		m_pTxtDifficulty->DisplayString(pDC, pszDiffLevel[m_nDifficultyLevel], 14, TEXT_BOLD, RGB(0, 0, 0));
		ReleaseDC(pDC);
	}
}

BEGIN_MESSAGE_MAP(CUserCfgDlg, CBmpDialog)
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_PAINT()
END_MESSAGE_MAP()

} // namespace Battlefish
} // namespace HodjNPodj
} // namespace Bagel
