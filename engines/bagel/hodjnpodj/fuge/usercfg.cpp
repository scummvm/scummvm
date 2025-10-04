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
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/fuge/usercfg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

#define ID_RESET     104

#define ID_SCROLL1   105
#define ID_SCROLL2   106
#define ID_SCROLL3   107
#define ID_SCROLL4   108

#define ID_WALLS     109

#define PAGE_SIZE    2

extern const char *INI_SECTION;

static  CColorButton *pOKButton = nullptr;                     // OKAY button on scroll
static  CColorButton *pCancelButton = nullptr;                 // Cancel button on scroll
static  CColorButton *pDefaultsButton = nullptr;               // Defaults button on scroll


CUserCfgDlg::CUserCfgDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
	: CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP") {
	// Inits
	//
	m_pScrollBar1 = nullptr;
	m_pScrollBar2 = nullptr;
	m_pScrollBar3 = nullptr;
	m_pScrollBar4 = nullptr;
	m_pTxtNumBalls = nullptr;
	m_pTxtStartLevel = nullptr;
	m_pTxtBallSpeed = nullptr;
	m_pTxtPaddleSize = nullptr;

	DoModal();
}

void CUserCfgDlg::DoDataExchange(CDataExchange *pDX) {
	CBmpDialog::DoDataExchange(pDX);
}


void CUserCfgDlg::PutDlgData() {
	m_pScrollBar1->SetScrollPos(m_nNumBalls);
	m_pScrollBar2->SetScrollPos(m_nStartLevel);
	m_pScrollBar3->SetScrollPos(m_nBallSpeed);
	m_pScrollBar4->SetScrollPos(m_nPaddleSize);

	m_pWallButton->SetCheck(m_bOutterWall);
}


void CUserCfgDlg::GetDlgData() {
	m_nNumBalls   = m_pScrollBar1->GetScrollPos();
	m_nStartLevel = m_pScrollBar2->GetScrollPos();
	m_nBallSpeed  = m_pScrollBar3->GetScrollPos();
	m_nPaddleSize = m_pScrollBar4->GetScrollPos();

	m_bOutterWall = false;
	if (m_pWallButton->GetCheck() == 1) {
		m_bOutterWall = true;
	}
}


bool CUserCfgDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
	/*
	* respond to user
	*/
	if (HIWORD(lParam) == BN_CLICKED) {

		switch (wParam) {

		case IDOK:
			m_bSave = true;
			PostMessage(WM_CLOSE, 0, 0);
			return false;

		case IDCANCEL:
			PostMessage(WM_CLOSE, 0, 0);
			return false;

		/*
		* reset params to default
		*/
		case ID_RESET:

			m_nNumBalls   = BALLS_DEF;
			m_nStartLevel = LEVEL_DEF;
			m_nBallSpeed  = SPEED_DEF;
			m_nPaddleSize = PSIZE_DEF;
			m_bOutterWall = false;

			PutDlgData();

			UpdateOptions();
			break;

		case ID_WALLS:
			m_bOutterWall = !m_bOutterWall;
			PutDlgData();
			break;

		default:
			break;
		}
	}

	return CBmpDialog::OnCommand(wParam, lParam);
}

void CUserCfgDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar *pScroll) {
	char buf[40];
	int nMin, nMax, nVal;
	CDC *pDC;

	// can't access a null pointer
	assert(pScroll != nullptr);

	if (pScroll == m_pScrollBar1) {

		nMin = BALLS_MIN;
		nMax = BALLS_MAX;
		nVal = m_nNumBalls;

	} else if (pScroll == m_pScrollBar2) {

		nMin = LEVEL_MIN;
		nMax = LEVEL_MAX;
		nVal = m_nStartLevel;

	} else if (pScroll == m_pScrollBar3) {

		nMin = SPEED_MIN;
		nMax = SPEED_MAX;
		nVal = m_nBallSpeed;

	} else {
		assert(pScroll == m_pScrollBar4);

		nMin = PSIZE_MIN;
		nMax = PSIZE_MAX;
		nVal = m_nPaddleSize;
	}

	switch (nSBCode) {

	case SB_LEFT:
		nVal = nMin;
		break;

	case SB_PAGELEFT:
		nVal -= PAGE_SIZE;
		break;

	case SB_LINELEFT:
		if (nVal > nMin)
			nVal--;
		break;

	case SB_RIGHT:
		nVal = nMax;
		break;

	case SB_PAGERIGHT:
		nVal += PAGE_SIZE;
		break;

	case SB_LINERIGHT:
		if (nVal < nMax)
			nVal++;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nVal = nPos;
		break;

	default:
		break;
	}

	if (nVal < nMin)
		nVal = nMin;
	if (nVal > nMax)
		nVal = nMax;

	pScroll->SetScrollPos(nVal);

	if ((pDC = GetDC()) != nullptr) {

		if (pScroll == m_pScrollBar1) {

			m_nNumBalls = nVal;
			if (m_pTxtNumBalls != nullptr) {
				Common::sprintf_s(buf, "Number of Balls: %d", m_nNumBalls);
				m_pTxtNumBalls->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB(0, 0, 0));
			}

		} else if (pScroll == m_pScrollBar2) {

			m_nStartLevel = nVal;
			if (m_pTxtStartLevel != nullptr) {
				Common::sprintf_s(buf, "Starting Level: %d", m_nStartLevel);
				m_pTxtStartLevel->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB(0, 0, 0));
			}

		} else if (pScroll == m_pScrollBar3) {

			m_nBallSpeed = nVal;
			if (m_pTxtBallSpeed != nullptr) {
				Common::sprintf_s(buf, "Ball Speed: %d", m_nBallSpeed);
				m_pTxtBallSpeed->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB(0, 0, 0));
			}

		} else if (pScroll == m_pScrollBar4) {

			m_nPaddleSize = nVal;
			if (m_pTxtPaddleSize != nullptr) {
				Common::sprintf_s(buf, "Paddle Size: %d", m_nPaddleSize);
				m_pTxtPaddleSize->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB(0, 0, 0));
			}

		} else {
			assert(0);
		}
		ReleaseDC(pDC);
	}
}


bool CUserCfgDlg::OnInitDialog() {
	CRect tmpRect;
	CDC *pDC;

	CBmpDialog::OnInitDialog();

	if ((pDC = GetDC()) != nullptr) {

		tmpRect.SetRect(22, 22, 135, 35);
		if ((m_pTxtNumBalls = new CText) != nullptr) {
			m_pTxtNumBalls->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
		}

		tmpRect.SetRect(22, 35, 92, 53);
		if ((m_pScrollBar1 = new CScrollBar) != nullptr) {
			m_pScrollBar1->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_SCROLL1);
			m_pScrollBar1->SetScrollRange(BALLS_MIN, BALLS_MAX, true);
		}

		tmpRect.SetRect(22, 57, 135, 70);
		if ((m_pTxtStartLevel = new CText) != nullptr) {
			m_pTxtStartLevel->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
		}

		tmpRect.SetRect(22, 70, 92, 88);
		if ((m_pScrollBar2 = new CScrollBar) != nullptr) {
			m_pScrollBar2->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_SCROLL2);
			m_pScrollBar2->SetScrollRange(LEVEL_MIN, LEVEL_MAX, true);
		}

		tmpRect.SetRect(22, 92, 135, 105);
		if ((m_pTxtBallSpeed = new CText) != nullptr) {
			m_pTxtBallSpeed->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
		}

		tmpRect.SetRect(22, 105, 92, 123);
		if ((m_pScrollBar3 = new CScrollBar) != nullptr) {
			m_pScrollBar3->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_SCROLL3);
			m_pScrollBar3->SetScrollRange(SPEED_MIN, SPEED_MAX, true);
		}

		tmpRect.SetRect(22, 127, 110, 140);
		if ((m_pTxtPaddleSize = new CText) != nullptr) {
			m_pTxtPaddleSize->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
		}

		tmpRect.SetRect(22, 140, 92, 158);
		if ((m_pScrollBar4 = new CScrollBar) != nullptr) {
			m_pScrollBar4->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_SCROLL4);
			m_pScrollBar4->SetScrollRange(PSIZE_MIN, PSIZE_MAX, true);
		}

		ReleaseDC(pDC);
	}

	if ((pOKButton = new CColorButton) != nullptr) {           // build a color QUIT button to let us exit
		pOKButton->SetPalette(m_pPalette);                  // set the palette to use
		pOKButton->SetControl(IDOK, this);                  // tie to the dialog control
	}

	if ((pCancelButton = new CColorButton) != nullptr) {       // build a color QUIT button to let us exit
		pCancelButton->SetPalette(m_pPalette);              // set the palette to use
		pCancelButton->SetControl(IDCANCEL, this);          // tie to the dialog control
	}

	if ((pDefaultsButton = new CColorButton) != nullptr) {     // build a color QUIT button to let us exit
		pDefaultsButton->SetPalette(m_pPalette);            // set the palette to use
		pDefaultsButton->SetControl(ID_RESET, this);        // tie to the dialog control
	}

	if ((m_pWallButton = new CCheckButton) != nullptr) {
		m_pWallButton->SetPalette(m_pPalette);
		m_pWallButton->SetControl(ID_WALLS, this);
	}

	m_bSave = false;

	LoadIniSettings();

	PutDlgData();

	return true;
}

void CUserCfgDlg::OnPaint() {
	CBmpDialog::OnPaint();

	UpdateOptions();
}

void CUserCfgDlg::UpdateOptions() {
	char buf[40];
	CDC *pDC;

	if ((pDC = GetDC()) != nullptr) {

		if (m_pTxtNumBalls != nullptr) {
			Common::sprintf_s(buf, "Number of Balls: %d", m_nNumBalls);
			m_pTxtNumBalls->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB(0, 0, 0));
		}

		if (m_pTxtStartLevel != nullptr) {
			Common::sprintf_s(buf, "Starting Level: %d", m_nStartLevel);
			m_pTxtStartLevel->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB(0, 0, 0));
		}

		if (m_pTxtBallSpeed != nullptr) {
			Common::sprintf_s(buf, "Ball Speed: %d", m_nBallSpeed);
			m_pTxtBallSpeed->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB(0, 0, 0));
		}

		if (m_pTxtPaddleSize != nullptr) {
			Common::sprintf_s(buf, "Paddle Size: %d", m_nPaddleSize);
			m_pTxtPaddleSize->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB(0, 0, 0));
		}

		ReleaseDC(pDC);
	}
}

bool CUserCfgDlg::OnEraseBkgnd(CDC *) {
	return true;
}


void CUserCfgDlg::OnClose() {
	if (m_bSave) {

		SaveIniSettings();
	}

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

	assert(m_pTxtPaddleSize != nullptr);
	if (m_pTxtPaddleSize != nullptr) {
		delete m_pTxtPaddleSize;
		m_pTxtPaddleSize = nullptr;
	}

	assert(m_pTxtBallSpeed != nullptr);
	if (m_pTxtBallSpeed != nullptr) {
		delete m_pTxtBallSpeed;
		m_pTxtBallSpeed = nullptr;
	}

	assert(m_pTxtStartLevel != nullptr);
	if (m_pTxtStartLevel != nullptr) {
		delete m_pTxtStartLevel;
		m_pTxtStartLevel = nullptr;
	}

	assert(m_pTxtNumBalls != nullptr);
	if (m_pTxtNumBalls != nullptr) {
		delete m_pTxtNumBalls;
		m_pTxtNumBalls = nullptr;
	}

	//
	// de-allocate the scroll bars
	//
	assert(m_pScrollBar4 != nullptr);
	if (m_pScrollBar4 != nullptr) {
		delete m_pScrollBar4;
		m_pScrollBar4 = nullptr;
	}
	assert(m_pScrollBar3 != nullptr);
	if (m_pScrollBar3 != nullptr) {
		delete m_pScrollBar3;
		m_pScrollBar3 = nullptr;
	}
	assert(m_pScrollBar2 != nullptr);
	if (m_pScrollBar2 != nullptr) {
		delete m_pScrollBar2;
		m_pScrollBar2 = nullptr;
	}
	assert(m_pScrollBar1 != nullptr);
	if (m_pScrollBar1 != nullptr) {
		delete m_pScrollBar1;
		m_pScrollBar1 = nullptr;
	}

	if (m_pWallButton != nullptr) {
		delete m_pWallButton;
		m_pWallButton = nullptr;
	}

	ClearDialogImage();
	EndDialog(0);
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


void CUserCfgDlg::LoadIniSettings() {
	m_nNumBalls = GetPrivateProfileInt(INI_SECTION, "NumberOfBalls", BALLS_DEF, INI_FILENAME);
	if ((m_nNumBalls < BALLS_MIN) || (m_nNumBalls > BALLS_MAX))
		m_nNumBalls = BALLS_DEF;

	m_nStartLevel = GetPrivateProfileInt(INI_SECTION, "StartingLevel", LEVEL_DEF, INI_FILENAME);
	if ((m_nStartLevel < LEVEL_MIN) || (m_nStartLevel > LEVEL_MAX))
		m_nStartLevel = LEVEL_DEF;

	m_nBallSpeed = GetPrivateProfileInt(INI_SECTION, "BallSpeed", SPEED_DEF, INI_FILENAME);
	if ((m_nBallSpeed < SPEED_MIN) || (m_nBallSpeed > SPEED_MAX))
		m_nBallSpeed = SPEED_DEF;

	m_nPaddleSize = GetPrivateProfileInt(INI_SECTION, "PaddleSize", PSIZE_DEF, INI_FILENAME);
	if ((m_nPaddleSize < PSIZE_MIN) || (m_nPaddleSize > PSIZE_MAX))
		m_nPaddleSize = PSIZE_DEF;

	int outerWall = GetPrivateProfileInt(INI_SECTION, "OutterWall", 0, INI_FILENAME);
	m_bOutterWall = outerWall != 0;
}

void CUserCfgDlg::SaveIniSettings() {
	WritePrivateProfileString(INI_SECTION, "NumberOfBalls", Common::String::format("%d", m_nNumBalls).c_str(), INI_FILENAME);
	WritePrivateProfileString(INI_SECTION, "StartingLevel", Common::String::format("%d", m_nStartLevel).c_str(), INI_FILENAME);
	WritePrivateProfileString(INI_SECTION, "BallSpeed", Common::String::format("%d", m_nBallSpeed).c_str(), INI_FILENAME);
	WritePrivateProfileString(INI_SECTION, "PaddleSize", Common::String::format("%d", m_nPaddleSize).c_str(), INI_FILENAME);
	WritePrivateProfileString(INI_SECTION, "OutterWall", Common::String::format("%d", m_bOutterWall ? 1 : 0).c_str(), INI_FILENAME);
}

BEGIN_MESSAGE_MAP(CUserCfgDlg, CBmpDialog)
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
	ON_WM_PAINT()
END_MESSAGE_MAP()

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel
