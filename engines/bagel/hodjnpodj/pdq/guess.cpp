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
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/pdq/main.h"
#include "bagel/hodjnpodj/pdq/guess.h"
#include "bagel/hodjnpodj/pdq/game.h"

namespace Bagel {
namespace HodjNPodj {
namespace PDQ {

#define IDC_EDIT 101

/////////////////////////////////////////////////////////////////////////////
// CGuessDlg dialog

CGuessDlg::CGuessDlg(CWnd *pParent, CPalette *pPalette)
	: CBmpDialog(pParent, pPalette, IDD_GUESS_DLG, ".\\ART\\GUESS.BMP", 36, 48) {
}

void CGuessDlg::DoDataExchange(CDataExchange* pDX) {
	CBmpDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT, text);
}

bool CGuessDlg::OnInitDialog() {
	CEdit *pEdit;

	CBmpDialog::OnInitDialog();

	pEdit = (CEdit *)GetDlgItem(IDC_EDIT);
	pEdit->LimitText(MAX_PLENGTH_S + 1);

	SetTimer(10, 50, nullptr);

	return true;
}

void CGuessDlg::OnTimer(uintptr nEventID) {
	char buf[MAX_PLENGTH_S + 2];
	CEdit *pEdit;

	KillTimer(nEventID);

	pEdit = (CEdit *)GetDlgItem(IDC_EDIT);
	pEdit->SetFocus();
	pEdit->GetWindowText(buf, MAX_PLENGTH + 1);
	pEdit->SetSel(-1, strlen(buf), true);
}

void CGuessDlg::OnPaint() {
	CBmpDialog::OnPaint();
}

void CGuessDlg::OnSetFocus(CWnd *) {
	SetTimer(10, 50, nullptr);
}

void CGuessDlg::OnActivate(unsigned int nState, CWnd *, bool bMinimized) {
	CEdit *pEdit;

	pEdit = (CEdit *)GetDlgItem(IDC_EDIT);

	if (!bMinimized) {
		switch (nState) {

		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			pEdit->SetSel(-1, 0, true);
			InvalidateRect(nullptr, false);
			break;
		}
	}
}

bool CGuessDlg::OnEraseBkgnd(CDC *) {
	return true;
}

BEGIN_MESSAGE_MAP(CGuessDlg, CBmpDialog)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_ACTIVATE()
	ON_WM_SETFOCUS()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

} // namespace PDQ
} // namespace HodjNPodj
} // namespace Bagel
