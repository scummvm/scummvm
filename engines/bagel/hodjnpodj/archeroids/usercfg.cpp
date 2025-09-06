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
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/menures.h"
#include "bagel/hodjnpodj/archeroids/usercfg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Archeroids {

#define ID_OK           101
#define ID_CANCEL       102
#define ID_RESET        103
#define ID_GAMESPEED    112
#define ID_LIVES        110
#define ID_ARCHER_LEVEL 107
#define ID_BADGUYS      113

#define IDS_GAMESPEED    104
#define IDS_LIVES        111
#define IDS_ARCHER_LEVEL 108

extern const char *INI_SECTION;

static const char *apszSpeeds[10] = {
	"Injured Snail",
	"Snail",
	"Turtle",
	"Old Gray Mare",
	"Ferret",
	"Rabbit",
	"Race Horse",
	"Cheetah",
	"Scared Cheetah",
	"Cheetah on Steroids"
};

static  CColorButton *pOKButton = nullptr;                     // OKAY button on scroll
static  CColorButton *pCancelButton = nullptr;                 // Cancel button on scroll
static  CColorButton *pDefaultsButton = nullptr;               // Defaults button on scroll

CUserCfgDlg::CUserCfgDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
	: CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP") {
	m_nInitGameSpeed = DEFAULT_GAME_SPEED;
	m_nInitArcherLevel = DEFAULT_ARCHER_LEVEL;
	m_nInitNumLives = DEFAULT_LIVES;
	m_nInitNumBadGuys = DEFAULT_BADGUYS;

	DoModal();
}


void CUserCfgDlg::PutDlgData() {
	char buf[20];
	CDC *pDC;

	pDC = GetDC();
	m_pTxtSpeedSetting->DisplayString(pDC, apszSpeeds[m_nInitGameSpeed - 1], 14, TEXT_BOLD, RGB(0, 0, 0));

	Common::sprintf_s(buf, "Level:  %d", m_nInitArcherLevel);
	m_pTxtLevel->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB(0, 0, 0));

	Common::sprintf_s(buf, "Lives:  %d", m_nInitNumLives);
	m_pTxtLives->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB(0, 0, 0));

	ReleaseDC(pDC);

	MFC::SetScrollPos(GetDlgItem(IDS_GAMESPEED)->m_hWnd, SB_CTL, m_nInitGameSpeed, true);
	MFC::SetScrollPos(GetDlgItem(IDS_LIVES)->m_hWnd, SB_CTL, m_nInitNumLives, true);
	MFC::SetScrollPos(GetDlgItem(IDS_ARCHER_LEVEL)->m_hWnd, SB_CTL, m_nInitArcherLevel, true);
}


bool CUserCfgDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
	/*
	* respond to user
	*/
	if (HIWORD(lParam) == BN_CLICKED) {

		switch (wParam) {

		case ID_OK:
			m_bShouldSave = true;
			ClearDialogImage();
			EndDialog(IDOK);
			return false;

		case ID_CANCEL:
			ClearDialogImage();
			EndDialog(IDCANCEL);
			return false;

		/*
		* reset params to default
		*/
		case ID_RESET:

			m_nInitGameSpeed = DEFAULT_GAME_SPEED;
			m_nInitArcherLevel = DEFAULT_ARCHER_LEVEL;
			m_nInitNumLives = DEFAULT_LIVES;
			m_nInitNumBadGuys = DEFAULT_BADGUYS;

			PutDlgData();

			break;

		default:
			break;
		}
	}

	return CBmpDialog::OnCommand(wParam, lParam);
}


void CUserCfgDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar *pScroll) {
	char szBuf[40];
	CDC *pDC;

	// can't access a null pointer
	assert(pScroll != nullptr);

	switch (pScroll->GetDlgCtrlID()) {

	case IDS_GAMESPEED:

		switch (nSBCode) {

		case SB_LEFT:
			m_nInitGameSpeed = SPEED_MIN;
			break;

		case SB_LINELEFT:
		case SB_PAGELEFT:
			if (m_nInitGameSpeed > SPEED_MIN)
				m_nInitGameSpeed--;
			break;

		case SB_RIGHT:
			m_nInitGameSpeed = SPEED_MAX;
			break;

		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			if (m_nInitGameSpeed < SPEED_MAX)
				m_nInitGameSpeed++;
			break;

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			m_nInitGameSpeed = nPos;
			break;

		default:
			break;
		}

		assert(m_nInitGameSpeed >= SPEED_MIN && m_nInitGameSpeed <= SPEED_MAX);

		pScroll->SetScrollPos(m_nInitGameSpeed);

		pDC = GetDC();
		m_pTxtSpeedSetting->DisplayString(pDC, apszSpeeds[m_nInitGameSpeed - 1], 14, TEXT_BOLD, RGB(0, 0, 0));
		ReleaseDC(pDC);
		break;

	case IDS_LIVES:

		switch (nSBCode) {

		case SB_BOTTOM:
			m_nInitNumLives = LIVES_MIN;
			break;

		case SB_LINELEFT:
		case SB_PAGELEFT:
			if (m_nInitNumLives > LIVES_MIN)
				m_nInitNumLives--;
			break;

		case SB_TOP:
			m_nInitNumLives = LIVES_MAX;
			break;

		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			if (m_nInitNumLives < LIVES_MAX)
				m_nInitNumLives++;
			break;

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			m_nInitNumLives = nPos;
			break;

		default:
			break;
		}

		assert(m_nInitNumLives >= LIVES_MIN && m_nInitNumLives <= LIVES_MAX);

		pScroll->SetScrollPos(m_nInitNumLives);

		pDC = GetDC();
		Common::sprintf_s(szBuf, "Lives:  %d", m_nInitNumLives);
		m_pTxtLives->DisplayString(pDC, szBuf, 14, TEXT_BOLD, RGB(0, 0, 0));
		ReleaseDC(pDC);

		break;

	case IDS_ARCHER_LEVEL:

		switch (nSBCode) {

		case SB_BOTTOM:
			m_nInitArcherLevel = LEVEL_MIN;
			break;

		case SB_LINELEFT:
		case SB_PAGELEFT:
			if (m_nInitArcherLevel > LEVEL_MIN)
				m_nInitArcherLevel--;
			break;

		case SB_TOP:
			m_nInitArcherLevel = LEVEL_MAX;
			break;

		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			if (m_nInitArcherLevel < LEVEL_MAX)
				m_nInitArcherLevel++;
			break;

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			m_nInitArcherLevel = nPos;
			break;

		default:
			break;
		}

		assert(m_nInitArcherLevel >= LEVEL_MIN && m_nInitArcherLevel <= LEVEL_MAX);

		pScroll->SetScrollPos(m_nInitArcherLevel);

		pDC = GetDC();
		Common::sprintf_s(szBuf, "Level:  %d", m_nInitArcherLevel);
		m_pTxtLevel->DisplayString(pDC, szBuf, 14, TEXT_BOLD, RGB(0, 0, 0));
		ReleaseDC(pDC);

		break;

	default:
		assert(0);
		break;
	}
}


bool CUserCfgDlg::OnInitDialog() {
	CRect tmpRect;
	CDC *pDC;
	int nVal;

	CBmpDialog::OnInitDialog();

	m_bShouldSave = false;

	// Get the game speed (1..10)
	//
	nVal = GetPrivateProfileInt(INI_SECTION, "GameSpeed", DEFAULT_GAME_SPEED, INI_FILENAME);
	m_nInitGameSpeed = nVal;
	if (nVal < SPEED_MIN || nVal > SPEED_MAX)
		m_nInitGameSpeed = DEFAULT_GAME_SPEED;

	// Get the Archer level (1..8)
	//
	nVal = GetPrivateProfileInt(INI_SECTION, "ArcherLevel", DEFAULT_ARCHER_LEVEL, INI_FILENAME);
	m_nInitArcherLevel = nVal;
	if (nVal < LEVEL_MIN || nVal > LEVEL_MAX)
		m_nInitArcherLevel = DEFAULT_ARCHER_LEVEL;

	// Get initial number of lives (1..5)
	//
	nVal = GetPrivateProfileInt(INI_SECTION, "NumberOfLives", DEFAULT_LIVES, INI_FILENAME);
	m_nInitNumLives = nVal;
	if (nVal < LIVES_MIN || nVal > LIVES_MAX)
		m_nInitNumLives = DEFAULT_LIVES;

	// Get initial number of badguys
	//
	nVal = GetPrivateProfileInt(INI_SECTION, "NumberOfBadGuys", DEFAULT_BADGUYS, INI_FILENAME);
	m_nInitNumBadGuys = nVal;
	if (nVal < BADGUYS_MIN || nVal > BADGUYS_MAX)
		m_nInitNumBadGuys = DEFAULT_BADGUYS;

	MFC::SetScrollRange(GetDlgItem(IDS_GAMESPEED)->m_hWnd, SB_CTL, SPEED_MIN, SPEED_MAX, true);
	MFC::SetScrollRange(GetDlgItem(IDS_LIVES)->m_hWnd, SB_CTL, LIVES_MIN, LIVES_MAX, true);
	MFC::SetScrollRange(GetDlgItem(IDS_ARCHER_LEVEL)->m_hWnd, SB_CTL, LEVEL_MIN, LEVEL_MAX, true);

	pDC = GetDC();
	tmpRect.SetRect(18, 113, 65, 132);
	m_pTxtSpeed = new CText;
	m_pTxtSpeed->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);

	tmpRect.SetRect(66, 113, 200, 132);
	m_pTxtSpeedSetting = new CText;
	m_pTxtSpeedSetting->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);

	tmpRect.SetRect(18, 25, 68, 42);
	m_pTxtLevel = new CText;
	m_pTxtLevel->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);

	tmpRect.SetRect(18, 66, 68, 83);
	m_pTxtLives = new CText;
	m_pTxtLives->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);

	ReleaseDC(pDC);

	if ((pOKButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pOKButton).SetPalette(m_pPalette);                        // set the palette to use
		(*pOKButton).SetControl(ID_OK, this);               // tie to the dialog control
	}

	if ((pCancelButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pCancelButton).SetPalette(m_pPalette);                        // set the palette to use
		(*pCancelButton).SetControl(ID_CANCEL, this);               // tie to the dialog control
	}

	if ((pDefaultsButton = new CColorButton) != nullptr) {                 // build a color QUIT button to let us exit
		(*pDefaultsButton).SetPalette(m_pPalette);                      // set the palette to use
		(*pDefaultsButton).SetControl(ID_RESET, this);              // tie to the dialog control
	}


	return true;
}

void CUserCfgDlg::OnPaint() {
	CDC *pDC;

	CBmpDialog::OnPaint();

	PutDlgData();

	pDC = GetDC();

	m_pTxtSpeed->DisplayString(pDC, "Speed:", 14, TEXT_BOLD, RGB(0, 0, 0));

	ReleaseDC(pDC);
}

void CUserCfgDlg::OnDestroy() {
	ClearDialogImage();

	assert(m_pTxtSpeed != nullptr);
	if (m_pTxtSpeed != nullptr) {
		delete m_pTxtSpeed;
		m_pTxtSpeed = nullptr;
	}

	assert(m_pTxtSpeedSetting != nullptr);
	if (m_pTxtSpeedSetting != nullptr) {
		delete m_pTxtSpeedSetting;
	}

	assert(m_pTxtLevel != nullptr);
	if (m_pTxtLevel != nullptr) {
		delete m_pTxtLevel;
	}

	assert(m_pTxtLives != nullptr);
	if (m_pTxtLives != nullptr) {
		delete m_pTxtLives;
	}

	if (m_bShouldSave) {

		WritePrivateProfileString(INI_SECTION, "GameSpeed", Common::String::format("%d", m_nInitGameSpeed).c_str(), INI_FILENAME);
		WritePrivateProfileString(INI_SECTION, "ArcherLevel", Common::String::format("%d", m_nInitArcherLevel).c_str(), INI_FILENAME);
		WritePrivateProfileString(INI_SECTION, "NumberOfLives", Common::String::format("%d", m_nInitNumLives).c_str(), INI_FILENAME);
		WritePrivateProfileString(INI_SECTION, "NumberOfBadGuys", Common::String::format("%d", m_nInitNumBadGuys).c_str(), INI_FILENAME);
	}

	CBmpDialog::OnDestroy();
}

void CUserCfgDlg::ClearDialogImage() {
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
	ValidateRect(nullptr);
}


BEGIN_MESSAGE_MAP(CUserCfgDlg, CBmpDialog)
	ON_WM_CLOSE()
	ON_WM_HSCROLL()
	ON_WM_PAINT()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

} // namespace Archeroids
} // namespace HodjNPodj
} // namespace Bagel
