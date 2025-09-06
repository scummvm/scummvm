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
#include "bagel/hodjnpodj/hnplibs/menures.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/pdq/usercfg.h"

namespace Bagel {
namespace HodjNPodj {
namespace PDQ {

#define ID_RESET     104
#define ID_GAMESPEED 105
#define ID_FIXED     106
#define ID_SPEED     112
#define ID_SHOWN     114
#define ID_NAMES     111

extern const char *INI_SECTION;

static const char *apszSpeeds[10] = {
	"Molasses in Stasis",
	"Frozen Molasses",
	"Molasses",
	"Maple Syrup",
	"Oil",
	"Watery Oil",
	"Oily Water",
	"Water",
	"Quicksilver",
	"Quick Quicksilver"
};

static CText *txtGameSpeed;
static CText *txtRevealed;
static CText *txtSpeed;
static CText *txtShown;

static  CColorButton *pOKButton = nullptr;                     // OKAY button on scroll
static  CColorButton *pCancelButton = nullptr;                 // Cancel button on scroll
static  CColorButton *pDefaultsButton = nullptr;               // Defaults button on scroll
static  CCheckButton *pFixedButton = nullptr;

CUserCfgDlg::CUserCfgDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
	: CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP") {

	m_pNamesButton = nullptr;
	DoModal();
}

void CUserCfgDlg::DoDataExchange(CDataExchange *pDX) {
	CBmpDialog::DoDataExchange(pDX);
}


void CUserCfgDlg::PutDlgData() {
	m_pSpeedScroll->SetScrollPos(m_nGameSpeed);
	m_pShownScroll->SetScrollPos(m_nShown);

	pFixedButton->SetCheck(!m_bRandomLetters);
	m_pNamesButton->SetCheck(m_bShowNames);
}


void CUserCfgDlg::GetDlgData() {
	m_nGameSpeed = m_pSpeedScroll->GetScrollPos();
	m_nShown = m_pShownScroll->GetScrollPos();

	m_bRandomLetters = true;
	if (pFixedButton->GetCheck() == 1)
		m_bRandomLetters = false;

	m_bShowNames = false;
	if (m_pNamesButton->GetCheck() == 1) {
		m_bShowNames = true;
	}
}


bool CUserCfgDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
	/*
	* respond to user
	*/
	if (HIWORD(lParam) == BN_CLICKED) {

		switch (wParam) {

		case IDOK:
			m_bShouldSave = true;
			ClearDialogImage();
			EndDialog(IDOK);
			return false;

		case IDCANCEL:
			ClearDialogImage();
			EndDialog(IDCANCEL);
			return false;

		/*
		* reset params to default
		*/
		case ID_RESET:

			m_bRandomLetters = false;
			m_nShown = SHOWN_DEF;
			m_nGameSpeed = SPEED_DEF;
			m_bShowNames = true;

			PutDlgData();
			DispSpeed();
			DispShown();
			break;

		case ID_NAMES:
			m_bShowNames = !m_bShowNames;
			PutDlgData();
			break;

		case ID_FIXED:
			m_bRandomLetters = !m_bRandomLetters;
			PutDlgData();
			break;

		default:
			break;
		}
	}

	return CBmpDialog::OnCommand(wParam, lParam);
}


void CUserCfgDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar *pScroll) {
	switch (pScroll->GetDlgCtrlID()) {

	case ID_SPEED:

		switch (nSBCode) {

		case SB_LEFT:
			m_nGameSpeed = SPEED_MIN;
			break;

		case SB_LINELEFT:
		case SB_PAGELEFT:
			if (m_nGameSpeed > SPEED_MIN)
				m_nGameSpeed--;
			break;


		case SB_RIGHT:
			m_nGameSpeed = SPEED_MAX;
			break;

		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			if (m_nGameSpeed < SPEED_MAX)
				m_nGameSpeed++;
			break;

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			m_nGameSpeed = nPos;
			break;

		default:
			break;
		}

		assert(m_nGameSpeed >= SPEED_MIN && m_nGameSpeed <= SPEED_MAX);

		if (m_nGameSpeed < SPEED_MIN)
			m_nGameSpeed = SPEED_MIN;
		if (m_nGameSpeed > SPEED_MAX)
			m_nGameSpeed = SPEED_MAX;

		/* can't access a null pointers */
		assert(pScroll != nullptr);

		pScroll->SetScrollPos(m_nGameSpeed);

		DispSpeed();
		break;

	case ID_SHOWN:

		switch (nSBCode) {

		case SB_LEFT:
			m_nShown = SHOWN_MIN;
			break;

		case SB_LINELEFT:
		case SB_PAGELEFT:
			if (m_nShown > SHOWN_MIN)
				m_nShown--;
			break;


		case SB_RIGHT:
			m_nShown = SHOWN_MAX;
			break;

		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			if (m_nShown < SHOWN_MAX)
				m_nShown++;
			break;

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			m_nShown = nPos;
			break;

		default:
			break;
		}

		assert(m_nShown >= SHOWN_MIN && m_nShown <= SHOWN_MAX);

		if (m_nShown < SHOWN_MIN)
			m_nShown = SHOWN_MIN;
		if (m_nShown > SHOWN_MAX)
			m_nShown = SHOWN_MAX;

		/* can't access a null pointers */
		assert(pScroll != nullptr);

		pScroll->SetScrollPos(m_nShown);

		DispShown();
		break;

	default:
		break;

	} // end Switch ID
}


bool CUserCfgDlg::OnInitDialog() {
	CRect tmpRect;
	char buf[10];
	CDC *pDC;

	CBmpDialog::OnInitDialog();

	tmpRect.SetRect(22, 100, 122, 120);
	m_pShownScroll = new CScrollBar;
	m_pShownScroll->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_SHOWN);
	m_pShownScroll->SetScrollRange(SHOWN_MIN, SHOWN_MAX, true);

	tmpRect.SetRect(22, 138, 122, 158);
	m_pSpeedScroll = new CScrollBar;
	m_pSpeedScroll->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_SPEED);
	m_pSpeedScroll->SetScrollRange(SPEED_MIN, SPEED_MAX, true);

	pDC = GetDC();

	tmpRect.SetRect(22, 125, 60, 140);
	if ((txtGameSpeed = new CText) != nullptr) {
		txtGameSpeed->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
	}

	tmpRect.SetRect(65, 125, 170, 140);
	if ((txtSpeed = new CText) != nullptr) {
		txtSpeed->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
	}

	tmpRect.SetRect(22, 87, 80, 102);
	if ((txtRevealed = new CText) != nullptr) {
		txtRevealed->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
	}

	tmpRect.SetRect(85, 87, 146, 102);
	if ((txtShown = new CText) != nullptr) {
		txtShown->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
	}

	ReleaseDC(pDC);

	if ((pOKButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pOKButton).SetPalette(m_pPalette);                        // set the palette to use
		(*pOKButton).SetControl(IDOK, this);            // tie to the dialog control
	}

	if ((pCancelButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pCancelButton).SetPalette(m_pPalette);                        // set the palette to use
		(*pCancelButton).SetControl(IDCANCEL, this);            // tie to the dialog control
	}

	if ((pDefaultsButton = new CColorButton) != nullptr) {                 // build a color QUIT button to let us exit
		(*pDefaultsButton).SetPalette(m_pPalette);                      // set the palette to use
		(*pDefaultsButton).SetControl(ID_RESET, this);              // tie to the dialog control
	}

	if ((pFixedButton = new CCheckButton) != nullptr) {        // build a color QUIT button to let us exit
		(*pFixedButton).SetPalette(m_pPalette);             // set the palette to use
		(*pFixedButton).SetControl(ID_FIXED, this);         // tie to the dialog control
	}

	if ((m_pNamesButton = new CCheckButton) != nullptr) {
		m_pNamesButton->SetPalette(m_pPalette);
		m_pNamesButton->SetControl(ID_NAMES, this);
	}

	m_bShouldSave = false;

	/*
	* User can specify if he/she wants the letters to appear in a random order
	* or in the predefined fixed order set by the MetaGame
	*/
	GetPrivateProfileString(INI_SECTION, "RandomLetters", "No", buf, 10, INI_FILENAME);
	m_bRandomLetters = false;
	if (!scumm_stricmp(buf, "Yes"))
		m_bRandomLetters = true;

	/*
	* This will determine how many letters are intially displayed (default is SHOWN_DEF)
	*/
	m_nShown = GetPrivateProfileInt(INI_SECTION, "NumStartingLetters", SHOWN_DEF, INI_FILENAME);
	if ((m_nShown < SHOWN_MIN) || (m_nShown > SHOWN_MAX))
		m_nShown = SHOWN_DEF;

	/*
	* This will determine the speed of the letters being displayed:
	* Range is 1..10 with 1 = 5000ms, 10 = 500ms
	*/
	m_nGameSpeed = GetPrivateProfileInt(INI_SECTION, "GameSpeed", SPEED_DEF, INI_FILENAME);
	if ((m_nGameSpeed < SPEED_MIN) || (m_nGameSpeed > SPEED_MAX))
		m_nGameSpeed = SPEED_DEF;

	GetPrivateProfileString(INI_SECTION, "ShowCategoryNames", "Yes", buf, 10, INI_FILENAME);
	assert(strlen(buf) < 10);
	m_bShowNames = false;
	if (!scumm_stricmp(buf, "Yes"))
		m_bShowNames = true;

	PutDlgData();
	return true;
}


void CUserCfgDlg::OnPaint() {
	CDC *pDC;

	CBmpDialog::OnPaint();

	pDC = GetDC();

	txtGameSpeed->DisplayString(pDC, "Speed:", 14, TEXT_BOLD, RGB(0, 0, 0));
	txtRevealed->DisplayString(pDC, "Revealed:", 14, TEXT_BOLD, RGB(0, 0, 0));
	DispSpeed();
	DispShown();

	ReleaseDC(pDC);
}


BEGIN_MESSAGE_MAP(CUserCfgDlg, CBmpDialog)
	ON_WM_HSCROLL()
	ON_WM_PAINT()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CUserCfgDlg::OnDestroy() {
	if (m_pSpeedScroll != nullptr) {
		delete m_pSpeedScroll;
		m_pSpeedScroll = nullptr;
	}

	if (m_pShownScroll != nullptr) {
		delete m_pShownScroll;
		m_pShownScroll = nullptr;
	}

	CBmpDialog::OnDestroy();
}


void CUserCfgDlg::DispSpeed() {
	CDC *pDC;

	if ((pDC = GetDC()) != nullptr) {
		txtSpeed->DisplayString(pDC, apszSpeeds[m_nGameSpeed - 1], 14, TEXT_BOLD, RGB(0, 0, 0));
		ReleaseDC(pDC);
	}
}


void CUserCfgDlg::DispShown() {
	CDC *pDC;
	char    msg[5];

	Common::sprintf_s(msg, "%d", m_nShown);

	if ((pDC = GetDC()) != nullptr) {
		txtShown->DisplayString(pDC, msg, 14, TEXT_BOLD, RGB(0, 0, 0));
		ReleaseDC(pDC);
	}
}


void CUserCfgDlg::ClearDialogImage() {
	CDC *pDC;

	if (m_bShouldSave) {
		GetDlgData();

		WritePrivateProfileString(INI_SECTION, "RandomLetters",
		                          m_bRandomLetters ? "Yes" : "No", INI_FILENAME);
		WritePrivateProfileString(INI_SECTION, "NumStartingLetters",
		                          Common::String::format("%d", m_nShown).c_str(),
		                          INI_FILENAME);
		WritePrivateProfileString(INI_SECTION, "GameSpeed",
		                          Common::String::format("%d", m_nGameSpeed).c_str(),
		                          INI_FILENAME);
		WritePrivateProfileString(INI_SECTION, "ShowCategoryNames",
		                          m_bShowNames ? "Yes" : "No", INI_FILENAME);
	}

	pDC = GetDC();

	if (txtSpeed != nullptr) {
		txtSpeed->RestoreBackground(pDC);
		delete txtSpeed;
		txtSpeed = nullptr;
	}

	if (txtShown != nullptr) {
		txtShown->RestoreBackground(pDC);
		delete txtShown;
		txtShown = nullptr;
	}

	if (txtRevealed != nullptr) {
		txtRevealed->RestoreBackground(pDC);
		delete txtRevealed;
		txtRevealed = nullptr;
	}

	if (txtGameSpeed != nullptr) {
		txtGameSpeed->RestoreBackground(pDC);
		delete txtGameSpeed;
		txtGameSpeed = nullptr;
	}

	ReleaseDC(pDC);

	if (pOKButton != nullptr) {                          // release the button
		delete pOKButton;
		pOKButton = nullptr;
	}

	if (pCancelButton != nullptr) {                        // release the button
		delete pCancelButton;
		pCancelButton = nullptr;
	}

	if (pDefaultsButton != nullptr) {                    // release the button
		delete pDefaultsButton;
		pDefaultsButton = nullptr;
	}

	if (pFixedButton != nullptr) {                         // release the button
		delete pFixedButton;
		pFixedButton = nullptr;
	}

	if (m_pNamesButton != nullptr) {
		delete m_pNamesButton;
		m_pNamesButton = nullptr;
	}

	ValidateRect(nullptr);
}

} // namespace PDQ
} // namespace HodjNPodj
} // namespace Bagel
