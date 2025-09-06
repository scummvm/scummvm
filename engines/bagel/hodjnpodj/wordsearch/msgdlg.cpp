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

static CPalette         *pPackRatOptPalette;

CColorButton    *pMsgOKButton = nullptr;

/*****************************************************************
 *
 * CMsgDlg
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
CMsgDlg::CMsgDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
	:  CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP") {
	pPackRatOptPalette = pPalette;
	m_nWhichMsg = 1;
	m_nWordsLeft = 0L;
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
bool CMsgDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog
	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
		case IDOK:
			ClearDialogImage();
			CDialog::EndDialog(0);
			return 1;
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

void CMsgDlg::OnCancel() {
	ClearDialogImage();
	EndDialog(0);
	return;
}

void CMsgDlg::OnOK() {
	ClearDialogImage();
	EndDialog(0);
	return;
}

void CMsgDlg::ClearDialogImage() {

	if (pMsgOKButton != nullptr) {
		delete pMsgOKButton;
		pMsgOKButton = nullptr;
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
void CMsgDlg::SetInitialOptions(int nWhichMsg, int nWords) {
	m_nWhichMsg = nWhichMsg;
	m_nWordsLeft = nWords;
	return;
}

bool CMsgDlg::OnInitDialog() {
	bool    bSuccess;

	CBmpDialog::OnInitDialog();

	pMsgOKButton = new CColorButton();
	ASSERT(pMsgOKButton != nullptr);
	pMsgOKButton->SetPalette(pPackRatOptPalette);
	bSuccess = pMsgOKButton->SetControl(IDOK, this);
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
void CMsgDlg::OnPaint() {
//      call CBmpDialog onpaint, to paint the background
	CBmpDialog::OnPaint();

	CDC     *pDC = GetDC();
//  CText(CDC *pDC, CPalette *pPalette, CRect *pRect, int nJustify = JUSTIFY_CENTER);
//  bool DisplayString(CDC *pDC, const char* pszText, const int nSize, const int nWeight, const COLORREF crColor = CTEXT_COLOR);

	CRect   rRect1(16, 36, 200, 56);
	CRect   rRect2(16, 60, 200, 80);
	CRect   rRect3(16, 84, 200, 104);
	CText   txtLine1(pDC, pPackRatOptPalette, &rRect1);
	CText   txtLine2(pDC, pPackRatOptPalette, &rRect2);
	CText   txtLine3(pDC, pPackRatOptPalette, &rRect3);
	char    cDisplay[30];

	switch (m_nWhichMsg) {
	case 1:
		txtLine1.DisplayString(pDC, "Congratulations!", 21, FW_BOLD);
		txtLine2.DisplayString(pDC, "You found every", 21, FW_BOLD);
		txtLine3.DisplayString(pDC, "word in the grid.", 21, FW_BOLD);
		break;
	case 2:
		if (m_nWordsLeft == 1)
			Common::sprintf_s(cDisplay, "You had %li word left.", m_nWordsLeft);
		else
			Common::sprintf_s(cDisplay, "You had %li words left.", m_nWordsLeft);

		txtLine2.DisplayString(pDC, "Game over.", 21, FW_BOLD);
		txtLine3.DisplayString(pDC, cDisplay, 21, FW_BOLD);
		break;
	}
	ReleaseDC(pDC);
	return;
}

void CMsgDlg::OnDestroy() {
//  send a message to the calling app to tell it the user has quit the game
	if (pMsgOKButton != nullptr) {
		delete pMsgOKButton;
		pMsgOKButton = nullptr;
	}

	CBmpDialog::OnDestroy();
}

// Message Map
BEGIN_MESSAGE_MAP(CMsgDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace WordSearch
} // namespace HodjNPodj
} // namespace Bagel
