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
#include "bagel/hodjnpodj/beacon/globals.h"
#include "bagel/hodjnpodj/beacon/resource.h"
#include "bagel/hodjnpodj/beacon/beacon.h"
#include "bagel/hodjnpodj/beacon/optndlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

namespace Bagel {
namespace HodjNPodj {
namespace Beacon {

static  CPalette *pSubOptionsPalette;
static  CColorButton *pOKButton = nullptr;                     // OKAY button on scroll
static  CColorButton *pCancelButton = nullptr;                 // Cancel button on scroll
static  CCheckButton *pAutoButton = nullptr;                   // Framed check box
static  CCheckButton *pChangeButton = nullptr;                 // Framed check box

CText   *m_pScoreText = nullptr;
CText   *m_pSweepsText = nullptr;
CText   *m_pSpeedText = nullptr;

int     m_nSweepSettings[15] =  {MIN_SWEEPS, 10, 15, 20, 30, 40, 50, 60, 80, 100, 120, 150, 200, 250, MAX_SWEEPS};  // 14 factors

/////////////////////////////////////////////////////////////////////////////
// COptnDlg dialog


COptnDlg::COptnDlg(CWnd* pParent, CPalette* pPalette)
	: CBmpDialog(pParent, pPalette, IDD_SUBOPTIONS, ".\\ART\\SSCROLL.BMP") {
	//{{AFX_DATA_INIT(COptnDlg)
	m_bAutomatic = false;
	m_bChangeAtTwelve = false;
	m_nSweeps = 0;
	m_nSpeed = MIN_SPEED;
	nSweepSets = 15;
	pSubOptionsPalette = pPalette;
	//}}AFX_DATA_INIT
}

COptnDlg::~COptnDlg() {
	if (m_pScoreText != nullptr)
		delete m_pScoreText;
	if (m_pSweepsText != nullptr)
		delete m_pSweepsText;
	if (m_pSpeedText != nullptr)
		delete m_pSpeedText;

	CBmpDialog::OnDestroy();
}

void COptnDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptnDlg)
	DDX_Control(pDX, IDC_NUMSWEEPS, m_ScrollSweeps);
	DDX_Control(pDX, IDC_SPEED, m_ScrollSpeed);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptnDlg, CDialog)
	//{{AFX_MSG_MAP(COptnDlg)
	ON_WM_HSCROLL()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_AUTOMATIC, COptnDlg::OnAutomatic)
	ON_BN_CLICKED(IDC_CHANGE, COptnDlg::OnChangeAtTwelve)
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
	int     i;

	pDC = GetDC();

//	mSpeedTable[0] =  "Leaky Rowboat";
//	mSpeedTable[1] =  "Kayak";
//	mSpeedTable[2] =  "Fishing Trawler";
	mSpeedTable[0] =  "Rowboat";
	mSpeedTable[1] =  "Dinghy";
	mSpeedTable[2] =  "Hovercraft";

	statRect.SetRect(LEFT_SIDE, 35, LEFT_SIDE + 185, 50);
	if ((m_pScoreText = new CText()) != nullptr) {
		(*m_pScoreText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}

	statRect.SetRect(LEFT_SIDE, 20, LEFT_SIDE + 185, 35);
	if ((m_pSweepsText = new CText()) != nullptr) {
		(*m_pSweepsText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}

	m_ScrollSweeps.SetScrollRange(0, nSweepSets - 1, 0);
	for (i = 0; i < (int)nSweepSets; i++) {
		if (m_nSweepSettings[i] == (int)m_nSweeps)
			m_ScrollSweeps.SetScrollPos(i, true);
	}

	statRect.SetRect(LEFT_SIDE, 70, LEFT_SIDE + 115, 88);
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

	if ((pAutoButton = new CCheckButton) != nullptr) {                 // build a color QUIT button to let us exit
		(*pAutoButton).SetPalette(pSubOptionsPalette);                      // set the palette to use
		(*pAutoButton).SetControl(IDC_AUTOMATIC, this);             // tie to the dialog control
	}
	((CWnd *)this)->CheckDlgButton(IDC_AUTOMATIC, m_bAutomatic);         // Set the Auto option box

	if ((pChangeButton = new CCheckButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pChangeButton).SetPalette(pSubOptionsPalette);                        // set the palette to use
		(*pChangeButton).SetControl(IDC_CHANGE, this);              // tie to the dialog control
	}
	((CWnd *)this)->CheckDlgButton(IDC_CHANGE, m_bChangeAtTwelve);       // Set the Auto option box

	ReleaseDC(pDC);

	return true;  // return true  unless you set the focus to a control
}


bool COptnDlg::OnEraseBkgnd(CDC *pDC) {
	return true;
}


void COptnDlg::OnDestroy() {
	CBmpDialog::OnDestroy();
}


bool COptnDlg::OnCommand(WPARAM wParam, LPARAM lParam) {

	if (HIWORD(lParam) == BN_CLICKED) {

		switch (wParam) {

		case IDC_AUTOMATIC:
			m_bAutomatic = !m_bAutomatic;
			((CWnd *)this)->CheckDlgButton(IDC_AUTOMATIC, m_bAutomatic);
			break;

		case IDC_CHANGE:
			m_bChangeAtTwelve = !m_bChangeAtTwelve;
			((CWnd *)this)->CheckDlgButton(IDC_CHANGE, m_bChangeAtTwelve);
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


void COptnDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar) {
	int pMin,
	    pMax;
	int OldPos = pScrollBar->GetScrollPos();
	int NewPos = OldPos;

	pScrollBar->GetScrollRange(&pMin, &pMax);

	switch (nSBCode) {
	case SB_LINERIGHT:
		NewPos++;
		break;
	case SB_PAGERIGHT:
		NewPos += NUM_BUTTONS;
		break;
	case SB_RIGHT:
		NewPos = pMax;
		break;
	case SB_LINELEFT:
		NewPos--;
		break;
	case SB_PAGELEFT:
		NewPos -= NUM_BUTTONS;
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
	unsigned int    OldValue;
	CDC     *pDC;
	char    msg[64];

	pDC = GetDC();

	OldValue = m_nSweeps;
	m_nSweeps = m_nSweepSettings[m_ScrollSweeps.GetScrollPos()];
	if (OldValue != m_nSweeps) {
		if (m_nSweeps == MAX_SWEEPS)
			Common::sprintf_s(msg, "Number of Sweeps: Unlimited");
		else
			Common::sprintf_s(msg, "Number of Sweeps: %d", m_nSweeps);
		(*m_pSweepsText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);
	}

	OldValue = m_nSpeed;
	m_nSpeed = m_ScrollSpeed.GetScrollPos();
	if (OldValue != m_nSpeed) {
		Common::sprintf_s(msg, "Speed:  %s", mSpeedTable[m_nSpeed].c_str());
		(*m_pSpeedText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);
	}

	ReleaseDC(pDC);
}


void COptnDlg::OnAutomatic() {
	m_bAutomatic = !m_bAutomatic;
	((CWnd *)this)->CheckDlgButton(IDC_AUTOMATIC, m_bAutomatic);
}

void COptnDlg::OnChangeAtTwelve() {
	m_bChangeAtTwelve = !m_bChangeAtTwelve;
	((CWnd *)this)->CheckDlgButton(IDC_CHANGE, m_bChangeAtTwelve);
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

	Common::sprintf_s(msg, "(Effective on New Game)");
	(*m_pScoreText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);

	if (m_nSweeps == MAX_SWEEPS)
		Common::sprintf_s(msg, "Number of Sweeps: Unlimited");
	else
		Common::sprintf_s(msg, "Number of Sweeps: %d", m_nSweeps);
	(*m_pSweepsText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);

	Common::sprintf_s(msg, "Speed:  %s", mSpeedTable[m_nSpeed].c_str());
	(*m_pSpeedText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);

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

	if (pAutoButton != nullptr) {                      // release the button
		delete pAutoButton;
		pAutoButton = nullptr;
	}

	if (pChangeButton != nullptr) {                        // release the button
		delete pChangeButton;
		pChangeButton = nullptr;
	}

	ValidateRect(nullptr);
}

} // namespace Beacon
} // namespace HodjNPodj
} // namespace Bagel
