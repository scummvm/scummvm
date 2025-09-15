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
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/packrat/resource.h"
#include "bagel/hodjnpodj/packrat/dialogs.h"

namespace Bagel {
namespace HodjNPodj {
namespace Packrat {

static CPalette         *pPackRatOptPalette;
CText                   *ptxtLevel = nullptr;
CText                   *ptxtLives = nullptr;

CColorButton    *pPROKButton = nullptr;
CColorButton    *pPRCancelButton = nullptr;

/*****************************************************************
 *
 * CPackRatOptDlg
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
CPackRatOptDlg::CPackRatOptDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID) :
		CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP"),
		rectDisplayLevel(43, 60, 174, 73),
		rectDisplayLives(43, 101, 174, 114) {
	CDC     *pDC = GetDC();
	pPackRatOptPalette = pPalette;
	ptxtLevel = new CText;
	ptxtLives = new CText;
	ptxtLevel->SetupText(pDC, pPackRatOptPalette, &rectDisplayLevel, JUSTIFY_CENTER);
	ptxtLives->SetupText(pDC, pPackRatOptPalette, &rectDisplayLives, JUSTIFY_CENTER);
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
bool CPackRatOptDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
//int     nLevel = 1;
// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog
	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
		/*
		        case IDC_TOGGLEJOY:
		            if ( m_bUseJoy ) {
		                SetDlgItemText( IDC_TOGGLEJOY, "Turn Off Joystick" );
		                m_bUseJoy = false;
		            }
		            else {
		                SetDlgItemText( IDC_TOGGLEJOY, "Turn On Joystick" );
		                m_bUseJoy = true;
		            }
		            break;
		*/
		case IDC_MINI_OK:
			// calculate current level from
			ClearDialogImage();
			EndDialog((m_nLevel * 10) + m_nLives);
			return 1;
		case IDC_MINI_CANCEL:
			ClearDialogImage();
			EndDialog(0);
			return 1;
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

void CPackRatOptDlg::OnCancel() {
	ClearDialogImage();
	EndDialog(0);
	return;
}

void CPackRatOptDlg::OnOK() {
	if (pPROKButton->GetState()  & 0x0008)
		SendMessage(WM_COMMAND, IDC_MINI_OK, BN_CLICKED);
	else
		SendMessage(WM_COMMAND, IDC_MINI_CANCEL, BN_CLICKED);
	return;
}

void CPackRatOptDlg::ClearDialogImage() {

	if (pPROKButton != nullptr) {
		delete pPROKButton;
		pPROKButton = nullptr;
	}
	if (pPRCancelButton != nullptr) {
		delete pPRCancelButton;
		pPRCancelButton = nullptr;
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
void CPackRatOptDlg::SetInitialOptions(int nGameLevel, int nNumOfLives) {
	m_nLevel = nGameLevel;
	m_nLives = nNumOfLives;
	return;
}


bool CPackRatOptDlg::OnInitDialog() {
	bool    bSuccess;

	CBmpDialog::OnInitDialog();

	pPROKButton = new CColorButton();
	ASSERT(pPROKButton != nullptr);
	pPROKButton->SetPalette(pPackRatOptPalette);
	bSuccess = pPROKButton->SetControl(IDC_MINI_OK, this);
	ASSERT(bSuccess);

	pPRCancelButton = new CColorButton();
	ASSERT(pPRCancelButton != nullptr);
	pPRCancelButton->SetPalette(pPackRatOptPalette);
	bSuccess = pPRCancelButton->SetControl(IDC_MINI_CANCEL, this);
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
void CPackRatOptDlg::OnPaint() {
	CDC             *pDC;
	CString         strLevel = "Game Level :";
	CString         strLives = "Number of Lives :";
	int             nOldBkMode;

	CScrollBar      *pLevel;
	CScrollBar      *pLives;

	char    cDisplay[5];

//      call CBmpDialog onpaint, to paint the background
	CBmpDialog::OnPaint();

	pDC = GetDC();

//      now paint in my text with a transparent background
	nOldBkMode = pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(43, 47, strLevel);
	pDC->TextOut(43, 88, strLives);
	pDC->SetBkMode(nOldBkMode);
	Common::sprintf_s(cDisplay, "%i", m_nLevel);
	ptxtLevel->DisplayString(pDC, cDisplay, 14, FW_BOLD, RGB(0, 0, 100));
	Common::sprintf_s(cDisplay, "%i", m_nLives);
	ptxtLives->DisplayString(pDC, cDisplay, 14, FW_BOLD, RGB(0, 0, 100));

	ReleaseDC(pDC);

	pLevel = new CScrollBar;
	pLives = new CScrollBar;

	pLevel = (CScrollBar *)GetDlgItem(IDC_GAMELEVEL);
	pLives = (CScrollBar *)GetDlgItem(IDC_NUMBEROFLIVES);

	pLevel->SetScrollRange(LEVELMIN, LEVELMAX, true);
	pLives->SetScrollRange(LIVESMIN, LIVESMAX, true);
	pLevel->SetScrollPos(m_nLevel, true);
	pLives->SetScrollPos(m_nLives, true);

	if (pLevel != nullptr) {
		pLevel = nullptr;
	}
	if (pLevel != nullptr) {
		pLives = nullptr;
	}
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
void CPackRatOptDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar) {
	int     oldAmnt = 0;
	int     newAmnt = 0;
	CDC     *pDC;
	//int     nJunk = 0;
	char    cDisplay[5];

	//nJunk = pScrollBar->GetDlgCtrlID();

// first set the range of the scoll bar
	switch (pScrollBar->GetDlgCtrlID()) {
	case IDC_GAMELEVEL:
		pScrollBar->SetScrollRange(LEVELMIN, LEVELMAX, true);
		break;
	case IDC_NUMBEROFLIVES:
		pScrollBar->SetScrollRange(LIVESMIN, LIVESMAX, true);
		break;
	}

// get the scroll bar's current position, i.e. the current amount set
	oldAmnt = pScrollBar->GetScrollPos();
	newAmnt = oldAmnt;

// switching off of what the scroll bar wants to do, act accordingly.
	switch (nSBCode) {
	case SB_LEFT:
		switch (pScrollBar->GetDlgCtrlID()) {
		case IDC_GAMELEVEL:
			newAmnt = LEVELMIN;
			break;
		case IDC_NUMBEROFLIVES:
			newAmnt = LIVESMIN;
			break;
		}
		break;
	case SB_RIGHT:
		switch (pScrollBar->GetDlgCtrlID()) {
		case IDC_GAMELEVEL:
			newAmnt = LEVELMAX;
			break;
		case IDC_NUMBEROFLIVES:
			newAmnt = LIVESMAX;
			break;
		}
		break;
	case SB_LINELEFT:
	case SB_PAGELEFT:
		switch (pScrollBar->GetDlgCtrlID()) {
		case IDC_GAMELEVEL:
			if (oldAmnt != LEVELMIN)
				newAmnt--;
			else
				newAmnt = LEVELMIN;
			break;
		case IDC_NUMBEROFLIVES:
			if (oldAmnt != LIVESMIN)
				newAmnt--;
			else
				newAmnt = LIVESMIN;
			break;
		}
		break;
	case SB_LINERIGHT:
	case SB_PAGERIGHT:
		switch (pScrollBar->GetDlgCtrlID()) {
		case IDC_GAMELEVEL:
			if (oldAmnt != LEVELMAX)
				newAmnt++;
			else
				newAmnt = LEVELMAX;
			break;
		case IDC_NUMBEROFLIVES:
			if (oldAmnt != LIVESMAX)
				newAmnt++;
			else
				newAmnt = LIVESMAX;
			break;
		}
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
	switch (pScrollBar->GetDlgCtrlID()) {
	case IDC_GAMELEVEL:
		m_nLevel = newAmnt;
		Common::sprintf_s(cDisplay, "%i", m_nLevel);
		ptxtLevel->DisplayString(pDC, cDisplay, 14, FW_BOLD, RGB(0, 0, 100));
		break;
	case IDC_NUMBEROFLIVES:
		m_nLives = newAmnt;
		Common::sprintf_s(cDisplay, "%i", m_nLives);
		ptxtLives->DisplayString(pDC, cDisplay, 14, FW_BOLD, RGB(0, 0, 100));
		break;
	}
	ReleaseDC(pDC);
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPackRatOptDlg::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	if (pPROKButton != nullptr) {
		delete pPROKButton;
		pPROKButton = nullptr;
	}
	if (pPRCancelButton != nullptr) {
		delete pPRCancelButton;
		pPRCancelButton = nullptr;
	}

	delete ptxtLevel;
	delete ptxtLives;
	CBmpDialog::OnDestroy();
}

// Message Map
BEGIN_MESSAGE_MAP(CPackRatOptDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace Packrat
} // namespace HodjNPodj
} // namespace Bagel
