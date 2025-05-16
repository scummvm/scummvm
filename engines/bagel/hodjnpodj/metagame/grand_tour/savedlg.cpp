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

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace GrandTour {

#include "bagel/afxwin.h"

#include <assert.h>
#include "bagel/hodjnpodj/hnplibs/stdinc.h"

#include "savedlg.h"

#define IDC_SLOT1 2001
#define IDC_SLOT2 2002
#define IDC_SLOT3 2003
#define IDC_SLOT4 2004
#define IDC_SLOT5 2005
#define IDC_SLOT6 2006

#define IDC_TEXT1 2007
#define IDC_TEXT2 2008
#define IDC_TEXT3 2009
#define IDC_TEXT4 2010
#define IDC_TEXT5 2011
#define IDC_TEXT6 2012

#define MAX_BUTTON_TEXT 25

STATIC const CHAR *gpszTitle = "Save Game";
extern CHAR szDescBuf[40];

CSaveDlg::CSaveDlg(CHAR *pszDescriptions[], CWnd *pWnd, CPalette *pPalette)
	: CBmpDialog(pWnd, pPalette, 2000, ".\\ART\\MLSCROLL.BMP") {
	INT i;

	// Inits
	m_pTxtSave = NULL;
	m_nCurSlot = -1;

	m_pQuitButton = NULL;
	for (i = 0; i < MAX_SAVEGAMES; i++) {
		m_pSlotButtons[i] = NULL;
		m_pszDescriptions[i] = pszDescriptions[i];
	}

	// the order if these IDs matter
	//
	assert(IDC_SLOT1 + 1 == IDC_SLOT2);
	assert(IDC_SLOT1 + 2 == IDC_SLOT3);
	assert(IDC_SLOT1 + 3 == IDC_SLOT4);
	assert(IDC_SLOT1 + 4 == IDC_SLOT5);
	assert(IDC_SLOT1 + 5 == IDC_SLOT6);

	assert(IDC_TEXT1 + 1 == IDC_TEXT2);
	assert(IDC_TEXT1 + 2 == IDC_TEXT3);
	assert(IDC_TEXT1 + 3 == IDC_TEXT4);
	assert(IDC_TEXT1 + 4 == IDC_TEXT5);
	assert(IDC_TEXT1 + 5 == IDC_TEXT6);
}


BOOL CSaveDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
	if (HIWORD(lParam) == BN_CLICKED) {

		switch (wParam) {

		case IDC_SLOT1:
		case IDC_SLOT2:
		case IDC_SLOT3:
		case IDC_SLOT4:
		case IDC_SLOT5:
		case IDC_SLOT6:
			EditDescription((int)wParam - IDC_SLOT1);
			return (TRUE);

		case IDOK:
			m_pSlotText[m_nCurSlot]->GetWindowText(szDescBuf, MAX_BUTTON_TEXT);
			ClearDialogImage();
			assert(m_nCurSlot != -1);
			EndDialog(m_nCurSlot);
			return (TRUE);

		case IDCANCEL:
			ClearDialogImage();
			EndDialog(-1);
			return (TRUE);
		}
	}
	return (CBmpDialog::OnCommand(wParam, lParam));
}


void CSaveDlg::OnCancel(void) {
	ClearDialogImage();
	EndDialog(-1);
}

VOID CSaveDlg::EditDescription(INT nNewSlot) {
	CHAR szBuf[40];

	// abort any other edits the user was making
	//
	if (m_nCurSlot != -1) {

		// hide old text edit box
		m_pSlotText[m_nCurSlot]->ShowWindow(SW_HIDE);
		m_pSlotButtons[m_nCurSlot]->UpdateWindow();

		// re-enable old slot button
		m_pSlotButtons[m_nCurSlot]->EnableWindow(TRUE);
	}

	// enable the OK button
	m_pOkButton->EnableWindow(TRUE);

	// disable this slot button
	m_pSlotButtons[nNewSlot]->EnableWindow(FALSE);

	// show the new edit box
	m_pSlotText[nNewSlot]->ShowWindow(SW_SHOW);
	m_pSlotText[nNewSlot]->UpdateWindow();
	m_pSlotText[nNewSlot]->SetFocus();

	// put the current save game text into that box
	//
	szBuf[0] = '\0';
	if (m_pszDescriptions[nNewSlot] != NULL) {
		strcpy(szBuf, m_pszDescriptions[nNewSlot]);
		assert(strlen(szBuf) < MAX_BUTTON_TEXT);
	}
	m_pSlotText[nNewSlot]->SetWindowText(szBuf);
	m_pSlotText[nNewSlot]->SetSel(0, -1);

	// remember this slot
	m_nCurSlot = nNewSlot;
}


BOOL CSaveDlg::OnInitDialog(void) {
	CRect rect;
	CDC *pDC;
	INT i;
	BOOL bSuccess;

	CBmpDialog::OnInitDialog();            // do basic dialog initialization

	// create the buttons (each saved-game-slot is a button)
	//
	for (i = 0; i < MAX_SAVEGAMES; i++) {

		if ((m_pSlotButtons[i] = new CColorButton()) != NULL) {
			m_pSlotButtons[i]->SetPalette(m_pPalette);
			bSuccess = m_pSlotButtons[i]->SetControl(IDC_SLOT1 + i, this);
			assert(bSuccess);
		}

		// if a slot is empty
		//
		if (m_pszDescriptions[i] == NULL) {

			m_pSlotButtons[i]->SetWindowText("empty");

		} else {
			m_pSlotButtons[i]->SetWindowText(m_pszDescriptions[i]);
		}

		m_pSlotText[i] = (CEdit *)GetDlgItem(IDC_TEXT1 + i);
		if (m_pSlotText[i] != NULL) {
			m_pSlotText[i]->LimitText(MAX_BUTTON_TEXT);
			m_pSlotText[i]->ShowWindow(SW_HIDE);
		}

		// the text boxes need to be initially hidden
	}

	if ((m_pQuitButton = new CColorButton()) != NULL) {
		m_pQuitButton->SetPalette(m_pPalette);
		bSuccess = m_pQuitButton->SetControl(IDCANCEL, this);
		m_pQuitButton->SetFocus();
		assert(bSuccess);
	}

	if ((m_pOkButton = new CColorButton()) != NULL) {
		m_pOkButton->SetPalette(m_pPalette);
		bSuccess = m_pOkButton->SetControl(IDOK, this);
		assert(bSuccess);
	}

	if ((m_pTxtSave = new CText) != NULL) {
		rect.SetRect(198, 40, 367, 80);
		pDC = GetDC();
		m_pTxtSave->SetupText(pDC, m_pPalette, &rect, JUSTIFY_LEFT);
		ReleaseDC(pDC);
	}

	return (FALSE);
}


void CSaveDlg::OnPaint(void) {
	CDC *pDC;

	CBmpDialog::OnPaint();

	if (m_pTxtSave != NULL) {
		pDC = GetDC();
		m_pTxtSave->DisplayString(pDC, gpszTitle, 25, FW_BOLD, RGB(0, 0, 0));
		ReleaseDC(pDC);
	}
}


void CSaveDlg::ClearDialogImage(void) {
	INT i;

	if (m_pTxtSave != NULL) {
		delete m_pTxtSave;
		m_pTxtSave = NULL;
	}

	if (m_pOkButton != NULL) {
		delete m_pOkButton;
		m_pOkButton = NULL;
	}

	if (m_pQuitButton != NULL) {
		delete m_pQuitButton;
		m_pQuitButton = NULL;
	}

	for (i = 0; i < MAX_SAVEGAMES; i++) {
		if (m_pSlotButtons[i] != NULL) {
			delete m_pSlotButtons[i];
			m_pSlotButtons[i] = NULL;
		}
	}
}


void CSaveDlg::OnDestroy(void) {
	ClearDialogImage();

	CBmpDialog::OnDestroy();
}


// Message Map
BEGIN_MESSAGE_MAP(CSaveDlg, CBmpDialog)
	//{{AFX_MSG_MAP(CSaveDlg)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace GrandTour
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
