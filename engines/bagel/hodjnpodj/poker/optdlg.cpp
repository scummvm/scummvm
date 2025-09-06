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

static CPalette     *m_pOptionsPalette = nullptr;

static CColorButton    *pAmountButton = nullptr;
static CColorButton    *pPayoffButton = nullptr;
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
 *  m_bSoundOn, m_bDisableSets
 *
 * RETURN VALUE:
 *
 *  n/a
 *
 ****************************************************************/

COptionsDlg::COptionsDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
	:  CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP") {
	m_bSoundOn = true;
	m_bDisableSets = false;
	m_pOptionsPalette = pPalette;
}

/*****************************************************************
 *
 * OnCommand
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Process the Toggle Sound, "Set Amount", and "Set Payoffs" buttons
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

bool COptionsDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog
	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
		case ID_TOGGLESOUND:
			ClearDialogImage();
			EndDialog(ID_TOGGLESOUND);
			return 1;
			break;
		case ID_SETUSERAMT:
			ClearDialogImage();
			EndDialog(ID_SETUSERAMT);
			return 1;
			break;
		case ID_SETPAYOFFS:
			ClearDialogImage();
			EndDialog(ID_SETPAYOFFS);
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
bool COptionsDlg::OnInitDialog() {
	bool    bSuccess;

	CBmpDialog::OnInitDialog();

	pPayoffButton = new CColorButton();
	ASSERT(pPayoffButton != nullptr);
	pPayoffButton->SetPalette(m_pOptionsPalette);
	bSuccess = pPayoffButton->SetControl(ID_SETPAYOFFS, this);
	ASSERT(bSuccess);

	pAmountButton = new CColorButton();
	ASSERT(pAmountButton != nullptr);
	pAmountButton->SetPalette(m_pOptionsPalette);
	bSuccess = pAmountButton->SetControl(ID_SETUSERAMT, this);
	ASSERT(bSuccess);

	pCancelButton = new CColorButton();
	ASSERT(pCancelButton != nullptr);
	pCancelButton->SetPalette(m_pOptionsPalette);
	bSuccess = pCancelButton->SetControl(ID_CANCEL, this);
	ASSERT(bSuccess);

// if we are in the middle of a hand, or we are playing from the metagame
// then disable the "Set Amount" and "Set Payoffs" buttons
	if (m_bDisableSets) {
		GetDlgItem(ID_SETUSERAMT)->EnableWindow(false);
		GetDlgItem(ID_SETPAYOFFS)->EnableWindow(false);
	}

// if the sound is on, set the text of the Toggle Sound button to "Sound Off"
	if (m_bSoundOn)
		SetDlgItemText(ID_TOGGLESOUND, "Sound Off");
// otherwise set the text of the Toggle Sound button to "Sound On"
	else
		SetDlgItemText(ID_TOGGLESOUND, "Sound On");

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
 *  bDisableSets = Should I disable the bet buttons
 *  bSoundOn = Is the sound turned on
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *  m_bDisableSets = bDisableSets
 *  m_bSoundOn = bSoundOn
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *  n/a
 *
 * RETURN VALUE:
 *
 *
 ****************************************************************/
void COptionsDlg::SetInitialOptions(bool bDisableSets, bool bSoundOn) {
	m_bDisableSets = bDisableSets;
	m_bSoundOn = bSoundOn;
	return;
}

bool COptionsDlg::OnEraseBkgnd(CDC *pDC) {
	return true;
}

void COptionsDlg::OnDestroy() {
	CBmpDialog::OnDestroy();
}

void COptionsDlg::ClearDialogImage() {

	if (pAmountButton != nullptr) {
		delete pAmountButton;
		pAmountButton = nullptr;
	}
	if (pPayoffButton != nullptr) {
		delete pPayoffButton;
		pPayoffButton = nullptr;
	}
	if (pCancelButton != nullptr) {
		delete pCancelButton;
		pCancelButton = nullptr;
	}

	ValidateRect(nullptr);

	return;
}

/// Message Map
BEGIN_MESSAGE_MAP(COptionsDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace Poker
} // namespace HodjNPodj
} // namespace Bagel
