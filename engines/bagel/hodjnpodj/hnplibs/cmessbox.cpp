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
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/menures.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"

namespace Bagel {
namespace HodjNPodj {

static  CColorButton *pOKButton = nullptr;                     // OKAY button on scroll

CMessageBox::CMessageBox(CWnd* pParent, CPalette *pPalette, const char *msg1, const char *msg2, const int dx, const int dy)
	: CBmpDialog(pParent, pPalette, IDD_GAMEOVER, ".\\ART\\SSCROLL.BMP", dx, dy) {
	// Initialize all members
	//
	m_pPalette = pPalette;
	m_pMessage1 = msg1;
	m_pMessage2 = msg2;

	m_cTextMessage1 = nullptr;
	m_cTextMessage2 = nullptr;

	DoModal();

	//{{AFX_DATA_INIT(CMessageBox)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CMessageBox::ClearDialogImage() {
	if (pOKButton != nullptr) {                          // release the button
		delete pOKButton;
		pOKButton = nullptr;
	}

	ValidateRect(nullptr);
}

void CMessageBox::OnDestroy() {
	if (m_cTextMessage1 != nullptr) {
		delete m_cTextMessage1;
		m_cTextMessage1 = nullptr;
	}

	if (m_cTextMessage2 != nullptr) {
		delete m_cTextMessage2;
		m_cTextMessage2 = nullptr;
	}

	if (pOKButton != nullptr) {                          // release the button
		delete pOKButton;
		pOKButton = nullptr;
	}

	CBmpDialog::OnDestroy();
}

void CMessageBox::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessageBox)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMessageBox, CDialog)
	//{{AFX_MSG_MAP(CMessageBox)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMessageBox message handlers

bool CMessageBox::OnInitDialog() {
	CRect   statsRect;                  // game stats displays
	int     nStat_col_offset;           // game stats placement
	int     nStat_row_offset;
	int     nStatWidth, nStatHeight;
	bool    bAssertCheck;
	CDC     *pDC;

	CBmpDialog::OnInitDialog();

	// TODO: Add extra initialization here

	pDC = GetDC();
	// setup the Starting Villages stat display box
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
	statsRect.SetRect(
	    nStat_col_offset,
	    nStat_row_offset,
	    nStat_col_offset + nStatWidth,
	    nStat_row_offset + nStatHeight
	);

	if ((m_cTextMessage2 = new CText()) != nullptr) {
		bAssertCheck = (*m_cTextMessage2).SetupText(pDC, m_pPalette, &statsRect, JUSTIFY_CENTER);
		ASSERT(bAssertCheck);   // initialize the text objext
	}

	ReleaseDC(pDC);

	if ((pOKButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pOKButton).SetPalette(m_pPalette);                        // set the palette to use
		(*pOKButton).SetControl(IDOK, this);            // tie to the dialog control
	}

	return true;  // return true  unless you set the focus to a control
}

void CMessageBox::OnPaint() {
	// TODO: Add your message handler code here
	CDC     *pDC;
	bool    bAssertCheck;

	CBmpDialog::OnPaint();
	// Do not call CDialog::OnPaint() for painting messages

	pDC = GetDC();

	if (m_pMessage1 != nullptr) {
		bAssertCheck = (*m_cTextMessage1).DisplayString(pDC, m_pMessage1, 21, FW_BOLD, TEXT_COLOR);
		ASSERT(bAssertCheck);
	}

	if (m_pMessage2 != nullptr) {
		bAssertCheck = (*m_cTextMessage2).DisplayString(pDC, m_pMessage2, 21, FW_BOLD, TEXT_COLOR);
		ASSERT(bAssertCheck);
	}

	ReleaseDC(pDC);
}

void CMessageBox::OnOK() {
	ClearDialogImage();
	EndDialog(IDOK);
}

void CMessageBox::OnCancel() {
	ClearDialogImage();
	EndDialog(IDCANCEL);
}

bool CMessageBox::OnEraseBkgnd(CDC *pDC) {
	// Prevents refreshing of background
	return true;
}

} // namespace HodjNPodj
} // namespace Bagel
