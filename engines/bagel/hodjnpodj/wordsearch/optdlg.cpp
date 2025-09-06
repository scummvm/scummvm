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

#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/wordsearch/resource.h"
#include "bagel/hodjnpodj/wordsearch/dialogs.h"

namespace Bagel {
namespace HodjNPodj {
namespace WordSearch {

#define GAMETIMEMAX         12
#define GAMETIMEMIN         1
#define GAMESTEP            1
#define GAMEJUMP            1

static CPalette     *pPackRatOptPalette;
CText               *ptxtTime;

CColorButton    *pOKButton = nullptr;
CColorButton    *pCancelButton = nullptr;
CCheckButton    *pWordForwardButton = nullptr;

/*****************************************************************
 *
 * CWSOptDlg
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
CWSOptDlg::CWSOptDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
		:  CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP"),
		rDisplayTime(35, 90, 200, 110) {
	CDC     *pDC = GetDC();

	pPackRatOptPalette = pPalette;

	ptxtTime = new CText(pDC, pPackRatOptPalette, &rDisplayTime, JUSTIFY_LEFT);

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
bool CWSOptDlg::OnCommand(WPARAM wParam, LPARAM lParam) {

	int nReturn = 0;
// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog
	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {

		case IDC_MINI_BWTOGGLE:
			if (m_bWordForwardOnly) {
				m_bWordForwardOnly = false;
				pWordForwardButton->SetCheck(false);
			} else {
				m_bWordForwardOnly = true;
				pWordForwardButton->SetCheck(true);
			}
			break;

		case IDC_MINI_OK:
			// calculate current level from

			switch (m_nGameTime) {
			case 1:
				nReturn = 15;
				break;
			case 2:
				nReturn = 30;
				break;
			case 3:
				nReturn = 45;
				break;
			case 4:
				nReturn = 60;
				break;
			case 5:
				nReturn = 75;
				break;
			case 6:
				nReturn = 90;
				break;
			case 7:
				nReturn = 120;
				break;
			case 8:
				nReturn = 180;
				break;
			case 9:
				nReturn = 240;
				break;
			case 10:
				nReturn = 300;
				break;
			case 11:
				nReturn = 600;
				break;
			default:
				nReturn = 0;
				break;
			}

			if (m_bShowWord) {
				nReturn += 1000;
			} else {
				nReturn += 2000;
			}

			if (m_bWordForwardOnly) {
				nReturn += 10000;
			} else {
				nReturn += 20000;
			}

			ClearDialogImage();
			EndDialog(nReturn);
			return 1;

		case IDC_MINI_CANCEL:
			ClearDialogImage();
			EndDialog(-1);
			return 1;
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}


void CWSOptDlg::OnCancel() {
	ClearDialogImage();
	EndDialog(-1);
	return;
}

void CWSOptDlg::OnOK() {
	ClearDialogImage();
	EndDialog(-1);
	return;
}

void CWSOptDlg::ClearDialogImage() {

	if (pOKButton != nullptr) {
		delete pOKButton;
		pOKButton = nullptr;
	}
	if (pCancelButton != nullptr) {
		delete pCancelButton;
		pCancelButton = nullptr;
	}
	if (pWordForwardButton != nullptr) {
		delete pWordForwardButton;
		pWordForwardButton = nullptr;
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
void CWSOptDlg::SetInitialOptions(int nTime, bool bShowWords, bool bWFO) {
	switch (nTime) {
	case 15:
		m_nGameTime = 1;
		break;
	case 30:
		m_nGameTime = 2;
		break;
	case 45:
		m_nGameTime = 3;
		break;
	case 60:
		m_nGameTime = 4;
		break;
	case 75:
		m_nGameTime = 5;
		break;
	case 90:
		m_nGameTime = 6;
		break;
	case 120:
		m_nGameTime = 7;
		break;
	case 180:
		m_nGameTime = 8;
		break;
	case 240:
		m_nGameTime = 9;
		break;
	case 300:
		m_nGameTime = 10;
		break;
	case 600:
		m_nGameTime = 11;
		break;
	default:
		m_nGameTime = 12;
		break;
	}
	m_bShowWord = bShowWords;
	m_bWordForwardOnly = bWFO;

	return;
}


bool CWSOptDlg::OnInitDialog() {
	bool    bSuccess;

	CBmpDialog::OnInitDialog();

	pOKButton = new CColorButton();
	ASSERT(pOKButton != nullptr);
	pOKButton->SetPalette(pPackRatOptPalette);
	bSuccess = pOKButton->SetControl(IDC_MINI_OK, this);
	ASSERT(bSuccess);

	pCancelButton = new CColorButton();
	ASSERT(pCancelButton != nullptr);
	pCancelButton->SetPalette(pPackRatOptPalette);
	bSuccess = pCancelButton->SetControl(IDC_MINI_CANCEL, this);
	ASSERT(bSuccess);

	pWordForwardButton = new CCheckButton();
	ASSERT(pWordForwardButton != nullptr);
	pWordForwardButton->SetPalette(pPackRatOptPalette);
	bSuccess = pWordForwardButton->SetControl(IDC_MINI_BWTOGGLE, this);
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
void CWSOptDlg::OnPaint() {
	CDC             *pDC;
	CString         strTime = "Time Limit:  ";
//int             nOldBkMode;

	CScrollBar      *pTime;

	char    cDisplay[64];

//      call CBmpDialog onpaint, to paint the background
	CBmpDialog::OnPaint();

	if (m_bWordForwardOnly) {
		pWordForwardButton->SetCheck(1);
	} else {
		pWordForwardButton->SetCheck(0);
	}

	pDC = GetDC();

//      now paint in my text with a transparent background
//    nOldBkMode = pDC->SetBkMode( TRANSPARENT );
//    pDC->TextOut( 43, 73, strTime.c_str());
//    pDC->SetBkMode( nOldBkMode );

	switch (m_nGameTime) {
	case 1:
		Common::sprintf_s(cDisplay, "%s15 Seconds", strTime.c_str());
		break;
	case 2:
		Common::sprintf_s(cDisplay, "%s30 Seconds", strTime.c_str());
		break;
	case 3:
		Common::sprintf_s(cDisplay, "%s45 Seconds", strTime.c_str());
		break;
	case 4:
		Common::sprintf_s(cDisplay, "%s60 Seconds", strTime.c_str());
		break;
	case 5:
		Common::sprintf_s(cDisplay, "%s75 Seconds", strTime.c_str());
		break;
	case 6:
		Common::sprintf_s(cDisplay, "%s90 Seconds", strTime.c_str());
		break;
	case 7:
		Common::sprintf_s(cDisplay, "%s2 Minutes", strTime.c_str());
		break;
	case 8:
		Common::sprintf_s(cDisplay, "%s3 Minutes", strTime.c_str());
		break;
	case 9:
		Common::sprintf_s(cDisplay, "%s4 Minutes", strTime.c_str());
		break;
	case 10:
		Common::sprintf_s(cDisplay, "%s5 Minutes", strTime.c_str());
		break;
	case 11:
		Common::sprintf_s(cDisplay, "%s10 Minutes", strTime.c_str());
		break;
	default:
		Common::sprintf_s(cDisplay, "%sUnlimited", strTime.c_str());
		break;
	}

	ptxtTime->DisplayString(pDC, cDisplay, 14, FW_BOLD, RGB(0, 0, 0));

	ReleaseDC(pDC);

	pTime = new CScrollBar;

	pTime = (CScrollBar *)GetDlgItem(IDC_GAMETIME);

	pTime->SetScrollRange(GAMETIMEMIN, GAMETIMEMAX, true);
	if (m_nGameTime == 0)
		pTime->SetScrollPos(GAMETIMEMAX, true);
	else
		pTime->SetScrollPos(m_nGameTime, true);


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
void CWSOptDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar) {
	int     oldAmnt = 0;
	int     newAmnt = 0;
	CDC     *pDC;
	//int     nJunk = 0;
	char    cDisplay[64];
	CString strTime = "Time Limit:  ";

	//nJunk = pScrollBar->GetDlgCtrlID();

// first set the range of the scoll bar
	if (pScrollBar->GetDlgCtrlID() == IDC_GAMETIME)
		pScrollBar->SetScrollRange(GAMETIMEMIN, GAMETIMEMAX, true);

// get the scroll bar's current position, i.e. the current amount set
	oldAmnt = pScrollBar->GetScrollPos();
	newAmnt = oldAmnt;

// switching off of what the scroll bar wants to do, act accordingly.
	switch (nSBCode) {
	case SB_LINELEFT:
	case SB_PAGELEFT:
	case SB_LEFT:
		newAmnt --;
		if (newAmnt < GAMETIMEMIN)
			newAmnt = GAMETIMEMIN;
		break;
	case SB_LINERIGHT:
	case SB_PAGERIGHT:
	case SB_RIGHT:
		newAmnt ++;
		if (newAmnt > GAMETIMEMAX)
			newAmnt = GAMETIMEMAX;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		newAmnt = nPos;
		break;
	}

// set the scroll bar to the new position
	pScrollBar->SetScrollPos(newAmnt, true);

// set the current amount to the new amount just set
// and paint this new amount onto the screen

	pDC = GetDC();
	if (pScrollBar->GetDlgCtrlID() == IDC_GAMETIME) {
		m_nGameTime = newAmnt;
		switch (m_nGameTime) {
		case 1:
			Common::sprintf_s(cDisplay, "%s15 Seconds", strTime.c_str());
			break;
		case 2:
			Common::sprintf_s(cDisplay, "%s30 Seconds", strTime.c_str());
			break;
		case 3:
			Common::sprintf_s(cDisplay, "%s45 Seconds", strTime.c_str());
			break;
		case 4:
			Common::sprintf_s(cDisplay, "%s60 Seconds", strTime.c_str());
			break;
		case 5:
			Common::sprintf_s(cDisplay, "%s75 Seconds", strTime.c_str());
			break;
		case 6:
			Common::sprintf_s(cDisplay, "%s90 Seconds", strTime.c_str());
			break;
		case 7:
			Common::sprintf_s(cDisplay, "%s2 Minutes", strTime.c_str());
			break;
		case 8:
			Common::sprintf_s(cDisplay, "%s3 Minutes", strTime.c_str());
			break;
		case 9:
			Common::sprintf_s(cDisplay, "%s4 Minutes", strTime.c_str());
			break;
		case 10:
			Common::sprintf_s(cDisplay, "%s5 Minutes", strTime.c_str());
			break;
		case 11:
			Common::sprintf_s(cDisplay, "%s10 Minutes", strTime.c_str());
			break;
		default:
			Common::sprintf_s(cDisplay, "%sUnlimited", strTime.c_str());
			break;
		}

		ptxtTime->DisplayString(pDC, cDisplay, 14, FW_BOLD, RGB(0, 0, 0));
	}
	ReleaseDC(pDC);
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CWSOptDlg::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	if (pOKButton != nullptr)
		delete pOKButton;

	if (pCancelButton != nullptr)
		delete pCancelButton;

	if (pWordForwardButton != nullptr)
		delete pWordForwardButton;

	if (ptxtTime != nullptr)
		delete ptxtTime;

	CBmpDialog::OnDestroy();
}

// Message Map
BEGIN_MESSAGE_MAP(CWSOptDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace WordSearch
} // namespace HodjNPodj
} // namespace Bagel
