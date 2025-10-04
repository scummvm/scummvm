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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/garfunkle/resource.h"
#include "bagel/hodjnpodj/garfunkle/garfunkle.h"
#include "bagel/hodjnpodj/garfunkle/optndlg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Garkfunkle {

static  CPalette *pSubOptionsPalette;
static  CColorButton *pOKButton = nullptr;                     // OKAY button on scroll
static  CColorButton *pCancelButton = nullptr;                 // Cancel button on scroll
static  CRadioButton *pGameButton = nullptr;
static  CRadioButton *pMusicButton = nullptr;

CText   *m_pButtonsText = nullptr;
CText   *m_pSpeedText = nullptr;

/////////////////////////////////////////////////////////////////////////////
// COptnDlg dialog


COptnDlg::COptnDlg(CWnd* pParent, CPalette* pPalette)
	: CBmpDialog(pParent, pPalette, IDD_SUBOPTIONS, ".\\ART\\SSCROLL.BMP") {
	//{{AFX_DATA_INIT(COptnDlg)
	m_nSpeed = MIN_SPEED;
	m_nNumButtons = MAX_BUTTONS;
	m_bPlayGame = true;
	pSubOptionsPalette = pPalette;
	//}}AFX_DATA_INIT
}

COptnDlg::~COptnDlg() {
	if (m_pButtonsText != nullptr)
		delete m_pButtonsText;
	if (m_pSpeedText != nullptr)
		delete m_pSpeedText;

	CBmpDialog::OnDestroy();
}

void COptnDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptnDlg)
	DDX_Control(pDX, IDC_NUMBUTTONS, m_ScrollButtons);
	DDX_Control(pDX, IDC_SPEED, m_ScrollSpeed);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptnDlg, CDialog)
	//{{AFX_MSG_MAP(COptnDlg)
	ON_WM_HSCROLL()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptnDlg message handlers

int COptnDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CBmpDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

bool COptnDlg::OnInitDialog() {
	CBmpDialog::OnInitDialog();

	CDC     *pDC;
	CRect   statRect;

	pDC = GetDC();

	mSpeedTable[0] =  "Way Largo";
	mSpeedTable[1] =  "Largo";
	mSpeedTable[2] =  "Adagio";
	mSpeedTable[3] =  "Andante";
	mSpeedTable[4] =  "Andantino";
	mSpeedTable[5] =  "Moderato";
	mSpeedTable[6] =  "Allegretto";
	mSpeedTable[7] =  "Allegro";
	mSpeedTable[8] =  "Vivace";
	mSpeedTable[9] =  "Presto";
	mSpeedTable[10] = "Prestissimo";
	mSpeedTable[11] = "Way Prestissimo";

	statRect.SetRect(LEFT_SIDE, 26, LEFT_SIDE + 175, 41);
	if ((m_pButtonsText = new CText()) != nullptr) {
		(*m_pButtonsText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}

	m_ScrollButtons.SetScrollRange(MIN_BUTTONS, MAX_BUTTONS, 0);
	m_ScrollButtons.SetScrollPos(m_nNumButtons, true);

	statRect.SetRect(LEFT_SIDE, 65, LEFT_SIDE + 175, 80);
	if ((m_pSpeedText = new CText()) != nullptr) {
		(*m_pSpeedText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}

	m_ScrollSpeed.SetScrollRange(MIN_SPEED, MAX_SPEED, 0);
	m_ScrollSpeed.SetScrollPos(m_nSpeed, true);

	if ((pOKButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pOKButton).SetPalette(pSubOptionsPalette);                        // set the palette to use
		(*pOKButton).SetControl(IDOK, this);            // tie to the dialog control
	}

	if ((pCancelButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pCancelButton).SetPalette(pSubOptionsPalette);                        // set the palette to use
		(*pCancelButton).SetControl(IDCANCEL, this);            // tie to the dialog control
	}

	if ((pGameButton = new CRadioButton) != nullptr) {                 // build a color QUIT button to let us exit
		(*pGameButton).SetPalette(pSubOptionsPalette);                      // set the palette to use
		(*pGameButton).SetControl(IDC_PLAYGAME, this);              // tie to the dialog control
	}


	if ((pMusicButton = new CRadioButton) != nullptr) {                    // build a color QUIT button to let us exit
		(*pMusicButton).SetPalette(pSubOptionsPalette);                     // set the palette to use
		(*pMusicButton).SetControl(IDC_PLAYMUSIC, this);            // tie to the dialog control
	}


	ReleaseDC(pDC);

	return true;  // return true  unless you set the focus to a control
}

void COptnDlg::OnDestroy() {
	CBmpDialog::OnDestroy();
}


bool COptnDlg::OnCommand(WPARAM wParam, LPARAM lParam) {

	if (HIWORD(lParam) == BN_CLICKED) {

		switch (wParam) {

		case IDC_PLAYGAME:
			m_bPlayGame = true;
			(*pGameButton).SetCheck(m_bPlayGame);
			(*pMusicButton).SetCheck(!m_bPlayGame);
			break;

		case IDC_PLAYMUSIC:
			m_bPlayGame = false;
			(*pGameButton).SetCheck(m_bPlayGame);
			(*pMusicButton).SetCheck(!m_bPlayGame);
			break;

		case IDOK:
			ClearDialogImage();
			EndDialog(IDOK);
			break;

		case IDCANCEL:
			ClearDialogImage();
			EndDialog(0);
			break;

		default:
			break;
		} // end switch
	} // end if

	return true;

} // end OnCommand


bool COptnDlg::OnEraseBkgnd(CDC *pDC) {
	return true;
}


void COptnDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar) {
	int pMin,
	    pMax;
	int OldPos = pScrollBar->GetScrollPos();
	int NewPos = OldPos;

	pScrollBar->GetScrollRange(&pMin, &pMax);

	switch (nSBCode) {
	case SB_LINERIGHT:
	case SB_PAGERIGHT:
		NewPos++;
		break;
	case SB_RIGHT:
		NewPos = pMax;
		break;
	case SB_LINELEFT:
	case SB_PAGELEFT:
		NewPos--;
		break;
	case SB_LEFT:
		NewPos = pMin;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		NewPos = nPos;
		break;
	}

	if (NewPos < pMin) NewPos = pMin;
	if (NewPos > pMax) NewPos = pMax;

	if (NewPos != OldPos) {                              //To prevent "flicker"
		(*pScrollBar).SetScrollPos(NewPos, true);        //...only update when
	}                                                   //...changed

	UpdateScrollbars();

	CDialog::OnHScroll(nSBCode, NewPos, pScrollBar);
}


/*****************************************************************
 *
 *  UpdateScrollbars
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Updates data adjusted with scrollbars
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      CScrollbar  pScrollTime, pScrollColumns, pScrollRows
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      int m_nTime, m_nNumParts, m_nColumns, m_nRows
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void COptnDlg::UpdateScrollbars() {
	int OldValue;
	CDC     *pDC;
	char    msg[64];

	pDC = GetDC();

	OldValue = m_nNumButtons;
	m_nNumButtons = m_ScrollButtons.GetScrollPos();
	if (OldValue != m_nNumButtons) {
		Common::sprintf_s(msg, "Number of Musicians:  %d", m_nNumButtons);
		(*m_pButtonsText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);
	}

	OldValue = m_nSpeed;
	m_nSpeed = m_ScrollSpeed.GetScrollPos();
	if (OldValue != m_nSpeed) {
		Common::sprintf_s(msg, "Speed:  %s", mSpeedTable[m_nSpeed].c_str());
		(*m_pSpeedText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);
	}

	ReleaseDC(pDC);
}

void COptnDlg::OnOK() {
	ClearDialogImage();
	EndDialog(IDOK);
}

void COptnDlg::OnCancel() {
	ClearDialogImage();
	EndDialog(0);
}

void COptnDlg::OnPaint() {
	CDC     *pDC;
	char    msg[64];

	CBmpDialog::OnPaint();

	pDC = GetDC();

	Common::sprintf_s(msg, "Number of Musicians:  %d", m_nNumButtons);
	(*m_pButtonsText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);

	Common::sprintf_s(msg, "Speed:  %s", mSpeedTable[m_nSpeed].c_str());
	(*m_pSpeedText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);

	(*pGameButton).SetCheck(m_bPlayGame);
	(*pMusicButton).SetCheck(!m_bPlayGame);

	ReleaseDC(pDC);

}

void COptnDlg::ClearDialogImage() {
	if (pOKButton != nullptr) {                          // release the button
		delete pOKButton;
		pOKButton = nullptr;
	}

	if (pCancelButton != nullptr) {                        // release the button
		delete pCancelButton;
		pCancelButton = nullptr;
	}

	if (pGameButton != nullptr) {                      // release the button
		delete pGameButton;
		pGameButton = nullptr;
	}

	if (pMusicButton != nullptr) {                         // release the button
		delete pMusicButton;
		pMusicButton = nullptr;
	}

	ValidateRect(nullptr);
}

} // namespace Garfunkle
} // namespace HodjNPodj
} // namespace Bagel
