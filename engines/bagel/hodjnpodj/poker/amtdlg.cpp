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

static CPalette *pSetAmountPalette;
CText           *ptxtAmount = nullptr;
long            lCAmount;

static CColorButton    *pAmountButton = nullptr;
static CColorButton    *pCancelButton = nullptr;

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
CSetAmountDlg::CSetAmountDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
	:  CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP") {
	CDC *pDC = GetDC();

	pSetAmountPalette = pPalette;
	rectDisplayAmount.SetRect(95, 65, 150, 81);
	m_nCurrentAmount = 1000;

	ptxtAmount = new CText(pDC, pSetAmountPalette, &rectDisplayAmount, JUSTIFY_LEFT);

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
 *  wParam      identifier for the button to be processed
 *  lParam      type of message to be processed
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
bool CSetAmountDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog
	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
		case IDC_SETSTARTAMOUNT:
			ClearDialogImage();
			EndDialog(m_nCurrentAmount);
			return 1;
			break;
		case ID_CANCEL:
			ClearDialogImage();
			EndDialog(0);
			return 1;
			break;
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

/*****************************************************************
 *
 * OnInitDialog
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This initializes the options dialog to enable and disable
 *  buttons when necessary
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
 *  bool to tell windows that it has dealt this function
 *
 ****************************************************************/
bool CSetAmountDlg::OnInitDialog() {
	bool    bSuccess;

	CBmpDialog::OnInitDialog();

	pAmountButton = new CColorButton();
	ASSERT(pAmountButton != nullptr);
	pAmountButton->SetPalette(pSetAmountPalette);
	bSuccess = pAmountButton->SetControl(IDC_SETSTARTAMOUNT, this);
	ASSERT(bSuccess);

	pCancelButton = new CColorButton();
	ASSERT(pCancelButton != nullptr);
	pCancelButton->SetPalette(pSetAmountPalette);
	bSuccess = pCancelButton->SetControl(ID_CANCEL, this);
	ASSERT(bSuccess);

	return true;
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
void CSetAmountDlg::SetInitialOptions(long lCurrentAmount) {
	lCAmount = lCurrentAmount;
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
void CSetAmountDlg::OnPaint() {
	CDC             *pDC;
	CString     strHowMuch1 = "How much would you";
	CString     strHowMuch2 = "like (0 - 1000)?";
	char        cDisplay[40];
	int         nOldBkMode;
	CText       *ptxtCAmount = nullptr;
	CRect       rectCAmount(42, 102, 200, 117);
	CScrollBar  *pSetAmountSB = nullptr;
	int         nScrollPos = 0;

//  call COptions onpaint, to paint the background
	CBmpDialog::OnPaint();

	pDC = GetDC();
//  now paint in my text with a transparent background

	nOldBkMode = pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(42, 35, strHowMuch1);
	pDC->TextOut(42, 50, strHowMuch2);
	pDC->SetBkMode(nOldBkMode);

	Common::sprintf_s(m_cAmount, "%i", m_nCurrentAmount);
	ptxtAmount->DisplayString(pDC, m_cAmount, 16, FW_BOLD, RGB(0, 0, 0));


	ptxtCAmount = new CText(pDC, pSetAmountPalette, &rectCAmount, JUSTIFY_LEFT);
	Common::sprintf_s(cDisplay, "Current Amount: %li", lCAmount);
	ptxtCAmount->DisplayString(pDC, cDisplay, 14, FW_BOLD, RGB(0, 0, 0));

	switch (m_nCurrentAmount) {
	case 10:
		nScrollPos = 1;
		break;
	case 25:
		nScrollPos = 2;
		break;
	case 50:
		nScrollPos = 3;
		break;
	case 100:
		nScrollPos = 4;
		break;
	case 250:
		nScrollPos = 5;
		break;
	case 500:
		nScrollPos = 6;
		break;
	case 1000:
		nScrollPos = 7;
		break;
	case 2500:
		nScrollPos = 8;
		break;
	case 5000:
		nScrollPos = 9;
		break;
	case 10000:
		nScrollPos = 10;
		break;
	}

	pSetAmountSB = (CScrollBar *)GetDlgItem(IDC_SETAMT_BAR);
	pSetAmountSB->SetScrollRange(AMOUNTMIN, AMOUNTMAX, true);
	pSetAmountSB->SetScrollPos(nScrollPos, true);

	ReleaseDC(pDC);
}

/*****************************************************************
 *
 * OnHScroll
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *  This is the functionality of the scroll bar
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
void CSetAmountDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar) {
	int     oldAmnt = 0;
	int         newAmnt = 0;
	CDC         *pDC;

// first set the range of the scoll bar
	pScrollBar->SetScrollRange(AMOUNTMIN, AMOUNTMAX, true);

// get the scroll bar's current position, i.e. the current amount set
	oldAmnt = pScrollBar->GetScrollPos();
	newAmnt = oldAmnt;

// switching off of what the scroll bar wants to do, act accordingly.
	switch (nSBCode) {
	case SB_LEFT:
	case SB_LINELEFT:
	case SB_PAGELEFT:
		if (oldAmnt != 0)
			newAmnt--;
		else
			newAmnt = 0;
		break;
	case SB_RIGHT:
	case SB_LINERIGHT:
	case SB_PAGERIGHT:
		if (oldAmnt != AMOUNTMAX)
			newAmnt++;
		else
			newAmnt = AMOUNTMAX;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		newAmnt = nPos;
		break;
	}

// set the scroll bar to the new position
	pScrollBar->SetScrollPos(newAmnt, true);

// set the current amount to the new amount just set
	switch (newAmnt) {
	case 1:
		m_nCurrentAmount = 10;
		break;
	case 2:
		m_nCurrentAmount = 25;
		break;
	case 3:
		m_nCurrentAmount = 50;
		break;
	case 4:
		m_nCurrentAmount = 100;
		break;
	case 5:
		m_nCurrentAmount = 250;
		break;
	case 6:
		m_nCurrentAmount = 500;
		break;
	case 7:
		m_nCurrentAmount = 1000;
		break;
	case 8:
		m_nCurrentAmount = 2500;
		break;
	case 9:
		m_nCurrentAmount = 5000;
		break;
	case 10:
		m_nCurrentAmount = 10000;
		break;
	}

// paint this new amount onto the screen
	pDC = GetDC();
	Common::sprintf_s(m_cAmount, "%i", m_nCurrentAmount);
	ptxtAmount->DisplayString(pDC, m_cAmount, 16, FW_BOLD, RGB(0, 0, 0));
	ReleaseDC(pDC);
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

bool CSetAmountDlg::OnEraseBkgnd(CDC *pDC) {
	return true;
}

void CSetAmountDlg::ClearDialogImage() {

	if (pAmountButton != nullptr) {
		delete pAmountButton;
		pAmountButton = nullptr;
	}

	if (pCancelButton != nullptr) {
		delete pCancelButton;
		pCancelButton = nullptr;
	}

	ValidateRect(nullptr);

	return;
}

void CSetAmountDlg::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	delete ptxtAmount;
	CBmpDialog::OnDestroy();
}

// Message Map
BEGIN_MESSAGE_MAP(CSetAmountDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace Poker
} // namespace HodjNPodj
} // namespace Bagel
