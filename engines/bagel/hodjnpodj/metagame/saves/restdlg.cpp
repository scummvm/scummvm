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

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/metagame/saves/restdlg.h"
#include "bagel/hodjnpodj/metagame/frame/resource.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Saves {

static const char *gpszTitle = "Restore Game";

CRestoreDlg::CRestoreDlg(char *pszDescriptions[], CWnd *pWnd, CPalette *pPalette)
	: CBmpDialog(pWnd, pPalette, IDD_RESTORE, ".\\ART\\MLSCROLL.BMP", -1, -1, true) {
	int i;

	// Inits
	m_pTxtRestore = nullptr;

	m_pQuitButton = nullptr;
	for (i = 0; i < MAX_SAVEGAMES; i++) {
		m_pSlotButtons[i] = nullptr;
		m_pszDescriptions[i] = pszDescriptions[i];
	}

	// the order if these IDs matter
	//
	assert(IDC_RSLOT1 + 1 == IDC_RSLOT2);
	assert(IDC_RSLOT1 + 2 == IDC_RSLOT3);
	assert(IDC_RSLOT1 + 3 == IDC_RSLOT4);
	assert(IDC_RSLOT1 + 4 == IDC_RSLOT5);
	assert(IDC_RSLOT1 + 5 == IDC_RSLOT6);
}


bool CRestoreDlg::OnCommand(WPARAM wParam, LPARAM lParam) {
	if (HIWORD(lParam) == BN_CLICKED) {

		switch (wParam) {

		case IDC_RSLOT1:
		case IDC_RSLOT2:
		case IDC_RSLOT3:
		case IDC_RSLOT4:
		case IDC_RSLOT5:
		case IDC_RSLOT6:
			ClearDialogImage();
			EndDialog((int)wParam - IDC_RSLOT1);
			return true;

		case IDCANCEL:
			ClearDialogImage();
			EndDialog(-1);
			return true;
		}
	}
	return CBmpDialog::OnCommand(wParam, lParam);
}


void CRestoreDlg::OnCancel() {
	ClearDialogImage();
	EndDialog(-1);
}


bool CRestoreDlg::OnInitDialog() {
	CRect rect;
	CDC *pDC;
	int i;
	bool bSuccess;

	CBmpDialog::OnInitDialog();            // do basic dialog initialization

	// create the buttons (each restore-game-slot is a button)
	//
	for (i = 0; i < MAX_SAVEGAMES; i++) {

		if ((m_pSlotButtons[i] = new CColorButton()) != nullptr) {
			m_pSlotButtons[i]->SetPalette(m_pPalette);
			bSuccess = m_pSlotButtons[i]->SetControl(IDC_RSLOT1 + i, this);
			assert(bSuccess);
		}

		// if a slot is empty
		//
		if (m_pszDescriptions[i] == nullptr) {

			m_pSlotButtons[i]->SetWindowText("empty");

			// then disable the button
			//
			m_pSlotButtons[i]->EnableWindow(false);

		} else {
			m_pSlotButtons[i]->SetWindowText(m_pszDescriptions[i]);
		}
	}

	if ((m_pQuitButton = new CColorButton()) != nullptr) {
		m_pQuitButton->SetPalette(m_pPalette);
		bSuccess = m_pQuitButton->SetControl(IDCANCEL, this);
		m_pQuitButton->SetFocus();
		assert(bSuccess);
	}

	if ((m_pTxtRestore = new CText) != nullptr) {
		rect.SetRect(193, 40, 365, 80);
		pDC = GetDC();
		m_pTxtRestore->SetupText(pDC, m_pPalette, &rect, JUSTIFY_LEFT);
		ReleaseDC(pDC);
	}

	return false;
}


void CRestoreDlg::OnPaint() {
	CDC *pDC;

	CBmpDialog::OnPaint();

	if (m_pTxtRestore != nullptr) {
		pDC = GetDC();
		m_pTxtRestore->DisplayString(pDC, gpszTitle, 25, FW_BOLD, RGB(0, 0, 0));
		ReleaseDC(pDC);
	}
}


void CRestoreDlg::ClearDialogImage() {
	int i;
	if (m_pTxtRestore != nullptr) {
		delete m_pTxtRestore;
		m_pTxtRestore = nullptr;
	}

	if (m_pQuitButton != nullptr) {
		delete m_pQuitButton;
		m_pQuitButton = nullptr;
	}

	for (i = 0; i < MAX_SAVEGAMES; i++) {
		if (m_pSlotButtons[i] != nullptr) {
			delete m_pSlotButtons[i];
			m_pSlotButtons[i] = nullptr;
		}
	}
}


void CRestoreDlg::OnDestroy() {
	int i;

	if (m_pTxtRestore != nullptr) {
		delete m_pTxtRestore;
		m_pTxtRestore = nullptr;
	}

	if (m_pQuitButton != nullptr) {
		delete m_pQuitButton;
		m_pQuitButton = nullptr;
	}

	for (i = 0; i < MAX_SAVEGAMES; i++) {
		if (m_pSlotButtons[i] != nullptr) {
			delete m_pSlotButtons[i];
			m_pSlotButtons[i] = nullptr;
		}
	}

	CBmpDialog::OnDestroy();
}

// Message Map
BEGIN_MESSAGE_MAP(CRestoreDlg, CBmpDialog)
	//{{AFX_MSG_MAP(CRestoreDlg)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace Saves
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
