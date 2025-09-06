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
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/metagame/bgen/c2btndlg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define TEXT_COLOR          RGB(0, 0, 0)    // displayed text color

#define MESSAGE_COL         16              // first message box positioning
#define MESSAGE_ROW         40

#define MESSAGE_WIDTH       186             // standard message box dims
#define MESSAGE_HEIGHT      20

#define MESSAGE2_ROW_OFFSET ( MESSAGE_HEIGHT + 4 )  // Row offset from first
// message box

#define IDD_2BTNDLG 208


C2ButtonDialog::C2ButtonDialog(CWnd *pParent, CPalette *pPalette,
                               const char *pszButton1Text, const char *pszButton2Text,
                               const char *pszText1, const char *pszText2,
                               const char *pszText3)
	: CBmpDialog(pParent, pPalette, IDD_2BTNDLG, ".\\ART\\MSSCROLL.BMP") {
	assert(pParent != nullptr);
	assert(pPalette != nullptr);
	assert(pszText1 != nullptr);
	assert(pszButton1Text != nullptr);
	assert(pszButton2Text != nullptr);

	// Initialize all members
	//
	m_pPalette = pPalette;
	m_pszMessage1 = pszText1;
	m_pszMessage2 = pszText2;
	m_pszMessage3 = pszText3;
	m_pszButton1Text = pszButton1Text;
	m_pszButton2Text = pszButton2Text;

	m_cTextMessage1 = nullptr;
	m_cTextMessage2 = nullptr;
	m_cTextMessage3 = nullptr;

	//{{AFX_DATA_INIT(C2ButtonDialog)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void C2ButtonDialog::ClearDialogImage() {
	if (m_pButton1 != nullptr) {                          // release the 1st button
		delete m_pButton1;
		m_pButton1 = nullptr;
	}
	if (m_pButton2 != nullptr) {                          // release the 2nd button
		delete m_pButton2;
		m_pButton2 = nullptr;
	}

	ValidateRect(nullptr);
}

void C2ButtonDialog::OnDestroy() {
	if (m_cTextMessage1 != nullptr) {
		delete m_cTextMessage1;
		m_cTextMessage1 = nullptr;
	}

	if (m_cTextMessage2 != nullptr) {
		delete m_cTextMessage2;
		m_cTextMessage2 = nullptr;
	}

	if (m_cTextMessage3 != nullptr) {
		delete m_cTextMessage3;
		m_cTextMessage3 = nullptr;
	}

	CBmpDialog::OnDestroy();
}

void C2ButtonDialog::DoDataExchange(CDataExchange* pDX) {
	CBmpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(C2ButtonDialog)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(C2ButtonDialog, CBmpDialog)
	//{{AFX_MSG_MAP(C2ButtonDialog)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// C2ButtonDialog message handlers

bool C2ButtonDialog::OnInitDialog() {
	CRect   statsRect;                  // game stats displays
	int     nStat_col_offset;           // game stats placement
	int     nStat_row_offset;
	int     nStatWidth, nStatHeight;
	bool    bAssertCheck;
	CDC     *pDC;

	CBmpDialog::OnInitDialog();

	pDC = GetDC();

	nStat_col_offset    = MESSAGE_COL;
	nStat_row_offset    = MESSAGE_ROW;
	nStatWidth          = MESSAGE_WIDTH;
	nStatHeight         = MESSAGE_HEIGHT;
	statsRect.SetRect(
	    nStat_col_offset,
	    nStat_row_offset,
	    nStat_col_offset + nStatWidth,
	    nStat_row_offset + nStatHeight
	);

	if ((m_cTextMessage1 = new CText()) != nullptr) {
		bAssertCheck = (*m_cTextMessage1).SetupText(pDC, m_pPalette, &statsRect, JUSTIFY_CENTER);
		ASSERT(bAssertCheck);   // initialize the text objext
	}

	nStat_row_offset += MESSAGE2_ROW_OFFSET;
	statsRect.SetRect(nStat_col_offset,
	                  nStat_row_offset,
	                  nStat_col_offset + nStatWidth,
	                  nStat_row_offset + nStatHeight
	                 );

	if ((m_cTextMessage2 = new CText()) != nullptr) {
		bAssertCheck = (*m_cTextMessage2).SetupText(pDC, m_pPalette, &statsRect, JUSTIFY_CENTER);
		ASSERT(bAssertCheck);   // initialize the text objext
	}

	nStat_row_offset += MESSAGE2_ROW_OFFSET;
	statsRect.SetRect(nStat_col_offset,
	                  nStat_row_offset,
	                  nStat_col_offset + nStatWidth,
	                  nStat_row_offset + nStatHeight
	                 );

	if ((m_cTextMessage3 = new CText()) != nullptr) {
		bAssertCheck = m_cTextMessage3->SetupText(pDC, m_pPalette, &statsRect, JUSTIFY_CENTER);
		ASSERT(bAssertCheck);   // initialize the text objext
	}

	ReleaseDC(pDC);

	if ((m_pButton1 = new CColorButton) != nullptr) {            // build the first color button
		(*m_pButton1).SetPalette(m_pPalette);                 // set the palette to use
		(*m_pButton1).SetControl(IDOK, this);                 // tie to the dialog control
		(*m_pButton1).SetWindowText(m_pszButton1Text);
	}
	if ((m_pButton2 = new CColorButton) != nullptr) {             // build the second color button
		(*m_pButton2).SetPalette(m_pPalette);                  // set the palette to use
		(*m_pButton2).SetControl(IDCANCEL, this);              // tie to the dialog control
		(*m_pButton2).SetWindowText(m_pszButton2Text);
	}

	return true;  // return true  unless you set the focus to a control
}

void C2ButtonDialog::OnPaint() {
	// TODO: Add your message handler code here
	CDC     *pDC;
	bool    bAssertCheck;

	CBmpDialog::OnPaint();
	// Do not call CDialog::OnPaint() for painting messages

	pDC = GetDC();

	if (m_pszMessage1 != nullptr) {
		bAssertCheck = m_cTextMessage1->DisplayString(pDC, m_pszMessage1, 21, FW_BOLD, TEXT_COLOR);
		ASSERT(bAssertCheck);
	}

	if (m_pszMessage2 != nullptr) {
		bAssertCheck = m_cTextMessage2->DisplayString(pDC, m_pszMessage2, 21, FW_BOLD, TEXT_COLOR);
		ASSERT(bAssertCheck);
	}

	if (m_pszMessage3 != nullptr) {
		bAssertCheck = m_cTextMessage3->DisplayString(pDC, m_pszMessage3, 21, FW_BOLD, TEXT_COLOR);
		ASSERT(bAssertCheck);
	}

	ReleaseDC(pDC);
}

void C2ButtonDialog::OnOK() {
	ClearDialogImage();
	EndDialog(IDOK);
}

void C2ButtonDialog::OnCancel() {
	ClearDialogImage();
	EndDialog(IDCANCEL);
}

bool C2ButtonDialog::OnEraseBkgnd(CDC *) {
	// Prevents refreshing of background
	return true;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
