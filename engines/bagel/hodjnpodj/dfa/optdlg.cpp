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
#include "bagel/hodjnpodj/dfa/resource.h"
#include "bagel/hodjnpodj/dfa/dialogs.h"

namespace Bagel {
namespace HodjNPodj {
namespace DFA {

constexpr int GAMETIMEMAX         = 5;
constexpr int GAMETIMEMIN         = 1;
constexpr int GAMESTEP            = 1;

constexpr int BEAVERTIMEMAX       = 4;
constexpr int BEAVERTIMEMIN       = 1;
constexpr int BEAVERSTEP = 1;

static CPalette     *pPackRatOptPalette;
CText               *ptxtGTime;
CText               *ptxtBTime;

CColorButton    *pDFAOKButton = nullptr;
CColorButton    *pDFACancelButton = nullptr;

/*****************************************************************
 *
 * CDFAOptDlg
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Constructor sends the input to the CBmpDialog constructor and
 *      the intializes the private members
 *
 * FORMAL PARAMETERS:
 *
 *      Those needed to contruct a CBmpDialog dialog: pParent,pPalette, nID
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      private member m_nCurrenLEVEL
 *  globals     rectDisplayAmount and pSeLEVELPalette
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
CDFAOptDlg::CDFAOptDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
	:  CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP"),
		rDisplayGTime(43, 85, 174, 98),
		rDisplayBTime(43, 35, 174, 48) {
	CDC     *pDC = GetDC();

	pPackRatOptPalette = pPalette;

	ptxtGTime = new CText(pDC, pPackRatOptPalette, &rDisplayGTime, JUSTIFY_CENTER);
	ptxtBTime = new CText(pDC, pPackRatOptPalette, &rDisplayBTime, JUSTIFY_CENTER);

	ReleaseDC(pDC);
}

/*****************************************************************
 *
 * OnCommand
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Process the "Set" and "Cancel" buttons
 *
 * This function is called when a WM_COMMAND message is issued,
 * typically in order to process control related activities.
 *
 * FORMAL PARAMETERS:
 *
 *      wParam          identifier for the button to be processed
 *      lParam          type of message to be processed
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
bool CDFAOptDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog
	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
		case IDC_MINI_OK:
			// calculate current level from
			ClearDialogImage();
			EndDialog(m_nGameTime + (m_nBeaverTime * 1000));
			return 1;
		case IDC_MINI_CANCEL:
			ClearDialogImage();
			EndDialog(-1);
			return 1;
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}

void CDFAOptDlg::OnCancel() {
	ClearDialogImage();
	EndDialog(-1);
	return;
}

void CDFAOptDlg::OnOK() {
	if (pDFAOKButton->GetState()  & 0x0008)
		SendMessage(WM_COMMAND, IDC_MINI_OK, BN_CLICKED);
	else
		SendMessage(WM_COMMAND, IDC_MINI_CANCEL, BN_CLICKED);
	return;
}

void CDFAOptDlg::ClearDialogImage() {

	if (pDFAOKButton != nullptr) {
		delete pDFAOKButton;
		pDFAOKButton = nullptr;
	}
	if (pDFACancelButton != nullptr) {
		delete pDFACancelButton;
		pDFACancelButton = nullptr;
	}

	ValidateRect(nullptr);

	return;
}
/*****************************************************************
 *
 * SetInitialOptions
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This sets the privates to the inputted values
 *
 * FORMAL PARAMETERS:
 *
 *      lCurrenLEVEL = the current amount the user has
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      m_nCurrenLEVEL = (int)min( AMOUNTMAX, lCurrenLEVEL)
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *
 ****************************************************************/
void CDFAOptDlg::SetInitialOptions(int nGTime, int nBTime) {
	m_nGameTime = nGTime / 15;          // get a value of 0 to 4
	m_nBeaverTime = nBTime;
	return;
}


bool CDFAOptDlg::OnInitDialog() {
	bool    bSuccess;

	CBmpDialog::OnInitDialog();

	pDFAOKButton = new CColorButton();
	ASSERT(pDFAOKButton != nullptr);
	pDFAOKButton->SetPalette(pPackRatOptPalette);
	bSuccess = pDFAOKButton->SetControl(IDC_MINI_OK, this);
	ASSERT(bSuccess);

	pDFACancelButton = new CColorButton();
	ASSERT(pDFACancelButton != nullptr);
	pDFACancelButton->SetPalette(pPackRatOptPalette);
	bSuccess = pDFACancelButton->SetControl(IDC_MINI_CANCEL, this);
	ASSERT(bSuccess);

	return true;
}

/*****************************************************************
 *
 * OnPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the screen whenever needed; e.g. when uncovered by an
 * overlapping window, when maximized from an icon, and when it the
 * window is initially created.
 *
 * This uses the CBmpDialog Paint as its base, and displays the current
 * amount chosen from the scrollbar
 *
 * This routine is called whenever Windows sends a WM_PAINT message.
 * Note that creating a CPaintDC automatically does a BeginPaint and
 * an EndPaint call is done when it is destroyed at the end of this
 * function.  CPaintDC's constructor needs the window (this).
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
void CDFAOptDlg::OnPaint() {
	CDC             *pDC;
	CString         strBTime = "Beaver Time (Secs) :";
	CString         strGTime = "Game Time (Secs) :";
	int             nOldBkMode;

	CScrollBar      *pBTime = nullptr;
	CScrollBar      *pGTime = nullptr;

	char    cGDisplay[12];
	char    cBDisplay[12];

//      call CBmpDialog onpaint, to paint the background
	CBmpDialog::OnPaint();

	pDC = GetDC();

//      now paint in my text with a transparent background
	nOldBkMode = pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(43, 24, strBTime);
	pDC->TextOut(43, 69, strGTime);
	pDC->SetBkMode(nOldBkMode);

	switch (m_nGameTime) {
	case 1:
		Common::sprintf_s(cGDisplay, "15");
		break;
	case 2:
		Common::sprintf_s(cGDisplay, "30");
		break;
	case 3:
		Common::sprintf_s(cGDisplay, "45");
		break;
	case 4:
		Common::sprintf_s(cGDisplay, "60");
		break;
	default:
		Common::sprintf_s(cGDisplay, "Unlimited");
		break;
	}
	ptxtGTime->DisplayString(pDC, cGDisplay, 14, FW_BOLD, RGB(0, 0, 0));

	pGTime = new CScrollBar;
	pGTime = (CScrollBar *)GetDlgItem(IDC_GAMETIME);
	pGTime->SetScrollRange(GAMETIMEMIN, GAMETIMEMAX, true);
	if (m_nGameTime == 0)
		pGTime->SetScrollPos(GAMETIMEMAX, true);
	else
		pGTime->SetScrollPos(m_nGameTime, true);


	switch (m_nBeaverTime) {
	case 1:
		Common::sprintf_s(cBDisplay, ".5");
		break;
	case 2:
		Common::sprintf_s(cBDisplay, "1");
		break;
	case 3:
		Common::sprintf_s(cBDisplay, "1.5");
		break;
	default:
		Common::sprintf_s(cBDisplay, "2");
		break;
	}
	ptxtBTime->DisplayString(pDC, cBDisplay, 14, FW_BOLD, RGB(0, 0, 0));

	pBTime = new CScrollBar;
	pBTime = (CScrollBar *)GetDlgItem(IDC_BEAVERDUR);
	pBTime->SetScrollRange(BEAVERTIMEMIN, BEAVERTIMEMAX, true);
	pBTime->SetScrollPos(m_nBeaverTime, true);

	if (pGTime != nullptr) {
		pGTime = nullptr;
	}

	if (pBTime != nullptr) {
		pBTime = nullptr;
	}

	ReleaseDC(pDC);
	return;
}

/*****************************************************************
 *
 * OnHScroll
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This is the functionality of the scroll bar
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
void CDFAOptDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar) {
	int     oldAmnt = 0;
	int     newAmnt = 0;
	CDC     *pDC;
	char    cGDisplay[12];
	char    cBDisplay[12];

// first set the range of the scoll bar
	if (pScrollBar->GetDlgCtrlID() == IDC_GAMETIME)
		pScrollBar->SetScrollRange(GAMETIMEMIN, GAMETIMEMAX, true);

	if (pScrollBar->GetDlgCtrlID() == IDC_BEAVERDUR)
		pScrollBar->SetScrollRange(BEAVERTIMEMIN, BEAVERTIMEMAX, true);

// get the scroll bar's current position, i.e. the current amount set
	oldAmnt = pScrollBar->GetScrollPos();
	newAmnt = oldAmnt;

// switching off of what the scroll bar wants to do, act accordingly.
	switch (nSBCode) {
	case SB_LINELEFT:
	case SB_PAGELEFT:
	case SB_LEFT:
		newAmnt --;
		if (pScrollBar->GetDlgCtrlID() == IDC_BEAVERDUR) {
			if (newAmnt < BEAVERTIMEMIN)
				newAmnt = BEAVERTIMEMIN;
		} else {
			if (newAmnt < GAMETIMEMIN)
				newAmnt = GAMETIMEMIN;
		}
		break;
	case SB_LINERIGHT:
	case SB_PAGERIGHT:
	case SB_RIGHT:
		newAmnt ++;
		if (pScrollBar->GetDlgCtrlID() == IDC_BEAVERDUR) {
			if (newAmnt > BEAVERTIMEMAX)
				newAmnt = BEAVERTIMEMAX;
		} else {
			if (newAmnt > GAMETIMEMAX)
				newAmnt = GAMETIMEMAX;
		}
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		newAmnt = nPos;
		break;
	}

// set the scroll bar to the new position
	if (pScrollBar->GetDlgCtrlID() == IDC_GAMETIME)
		m_nGameTime = newAmnt;

	if (pScrollBar->GetDlgCtrlID() == IDC_BEAVERDUR)
		m_nBeaverTime = newAmnt;

	pScrollBar->SetScrollPos(newAmnt, true);

// set the current amount to the new amount just set
// and paint this new amount onto the screen

	pDC = GetDC();

	if (pScrollBar->GetDlgCtrlID() == IDC_GAMETIME) {
		switch (m_nGameTime) {
		case 1:
			Common::sprintf_s(cGDisplay, "15");
			break;
		case 2:
			Common::sprintf_s(cGDisplay, "30");
			break;
		case 3:
			Common::sprintf_s(cGDisplay, "45");
			break;
		case 4:
			Common::sprintf_s(cGDisplay, "60");
			break;
		default:
			Common::sprintf_s(cGDisplay, "Unlimited");
			break;
		}
		ptxtGTime->DisplayString(pDC, cGDisplay, 14, FW_BOLD, RGB(0, 0, 0));
	}

	if (pScrollBar->GetDlgCtrlID() == IDC_BEAVERDUR) {
		switch (m_nBeaverTime) {
		case 1:
			Common::sprintf_s(cBDisplay, ".5");
			break;
		case 2:
			Common::sprintf_s(cBDisplay, "1");
			break;
		case 3:
			Common::sprintf_s(cBDisplay, "1.5");
			break;
		default:
			Common::sprintf_s(cBDisplay, "2");
			break;
		}
		ptxtBTime->DisplayString(pDC, cBDisplay, 14, FW_BOLD, RGB(0, 0, 0));
	}
	ReleaseDC(pDC);
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

bool CDFAOptDlg::OnEraseBkgnd(CDC *pDC) {
	return true;
}

void CDFAOptDlg::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	if (pDFAOKButton != nullptr) {
		delete pDFAOKButton;
		pDFAOKButton = nullptr;
	}
	if (pDFACancelButton != nullptr) {
		delete pDFACancelButton;
		pDFACancelButton = nullptr;
	}

	delete ptxtBTime;
	delete ptxtGTime;
	CBmpDialog::OnDestroy();
}

// Message Map
BEGIN_MESSAGE_MAP(CDFAOptDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace DFA
} // namespace HodjNPodj
} // namespace Bagel
