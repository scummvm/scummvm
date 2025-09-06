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
#include "bagel/hodjnpodj/poker/resource.h"
#include "bagel/hodjnpodj/poker/dialogs.h"

namespace Bagel {
namespace HodjNPodj {
namespace Poker {

static CPalette *pUWonPalette;

CColorButton    *pOKButton = nullptr;

/*****************************************************************
 *
 * COptionsDlg
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  Constructor sends the input to the COptions constructor and
 *  the intializes the private members
 *
 * FORMAL PARAMETERS:
 *
 *  Those needed to contruct a COptions dialog: pParent,pPalette, nID
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  private member m_nCurrentAmount
 *  globals rectDisplayAmount and pSetAmountPalette
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
CUserWonDlg::CUserWonDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
	:  CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP") {
	pUWonPalette = pPalette;
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
bool CUserWonDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog
	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
		case IDOK:
			ClearDialogImage();
			EndDialog(0);
			return 1;
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

void CUserWonDlg::OnOK() {
	ClearDialogImage();
	EndDialog(0);
	return;
}

void CUserWonDlg::ClearDialogImage() {

	if (pOKButton != nullptr) {
		delete pOKButton;
		pOKButton = nullptr;
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
 *  This sets the privates to the inputted values
 *
 * FORMAL PARAMETERS:
 *
 *  lCurrentAmount = the current amount the user has
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_nCurrentAmount = (int)min( AMOUNTMAX, lCurrentAmount)
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *
 ****************************************************************/
void CUserWonDlg::SetInitialOptions(int nWhichPhrase) {
	m_nWinPhrase = nWhichPhrase;
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
 * This uses the COptions Paint as its base, and displays the current
 * amount chosen from the scrollbar
 *
 * This routine is called whenever Windows sends a WM_PAINT message.
 * Note that creating a CPaintDC automatically does a BeginPaint and
 * an EndPaint call is done when it is destroyed at the end of this
 * function.  CPaintDC's constructor needs the window (this).
 *
 * FORMAL PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/
void CUserWonDlg::OnPaint() {
	CDC             *pDC;
	int         nOldBkMode;

//  call COptions onpaint, to paint the background
	CBmpDialog::OnPaint();

	pDC = GetDC();

//  now paint in my text with a transparent background
	nOldBkMode = pDC->SetBkMode(TRANSPARENT);

	switch (m_nWinPhrase) {
	case 1:
		pDC->TextOut(62, 60, "You got a Pair!");
		break;
	case 2:
		pDC->TextOut(63, 60, "You got a Pair");
		pDC->TextOut(57, 80, "of Jacks or higher!");
		break;
	case 3:
		pDC->TextOut(52, 60, "You got Two Pairs!");
		break;
	case 4:
		pDC->TextOut(61, 60, "You got a Three");
		pDC->TextOut(83, 80, "of a kind!");
		break;
	case 5:
		pDC->TextOut(52, 60, "You got a Straight!");
		break;
	case 6:
		pDC->TextOut(62, 60, "You got a Flush!");
		break;
	case 7:
		pDC->TextOut(42, 60, "You got a Full House!");
		break;
	case 8:
		pDC->TextOut(62, 60, "You got a Four");
		pDC->TextOut(83, 80, "of a kind!");
		break;
	case 9:
		pDC->TextOut(33, 60, "You got a Straight Flush!");
		break;
	case 10:
		pDC->TextOut(38, 60, "You got a Royal Flush!");
		break;
	default:
		pDC->TextOut(61, 60, "You got nothin'!");
		break;
	}

	pDC->SetBkMode(nOldBkMode);
	ReleaseDC(pDC);
	return;
}

bool CUserWonDlg::OnInitDialog() {
	bool    bSuccess;

	CBmpDialog::OnInitDialog();

	pOKButton = new CColorButton();
	ASSERT(pOKButton != nullptr);
	pOKButton->SetPalette(pUWonPalette);
	bSuccess = pOKButton->SetControl(IDOK, this);
	ASSERT(bSuccess);

	return true;
}

void CUserWonDlg::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	if (pOKButton != nullptr) {
		delete pOKButton;
		pOKButton = nullptr;
	}

	CBmpDialog::OnDestroy();
}


// Message Map
BEGIN_MESSAGE_MAP(CUserWonDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace Poker
} // namespace HodjNPodj
} // namespace Bagel
