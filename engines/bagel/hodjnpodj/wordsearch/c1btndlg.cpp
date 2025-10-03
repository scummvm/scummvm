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
#include "bagel/hodjnpodj/wordsearch/wordlist.h"
#include "bagel/hodjnpodj/wordsearch/c1btndlg.h"

namespace Bagel {
namespace HodjNPodj {
namespace WordSearch {

#define TEXT_COLOR          RGB(0, 0, 0)    // displayed text color

#define MESSAGE_COL         16              // first message box positioning
#define MESSAGE_ROW         60

#define MESSAGE_WIDTH       175             // standard message box dims
#define MESSAGE_HEIGHT      20

#define MESSAGE2_ROW_OFFSET ( MESSAGE_HEIGHT + 4 )  // Row offset from first 
// message box

#define IDD_1BTNDLG 321

extern  CString *astrGameList[WORDSPERLIST];
extern  CString *astrGameListDisplay[WORDSPERLIST];
extern  CText   *atxtDisplayWord[WORDSPERLIST];



C1ButtonDialog::C1ButtonDialog(CWnd *pParent, CPalette *pPalette, char *pszButton1Text, char *pszText1, char *pszText2)
	: CBmpDialog(pParent, pPalette, IDD_1BTNDLG, ".\\ART\\WLSTSCRL.BMP", 16, 25) {
	assert(pParent != nullptr);
	assert(pPalette != nullptr);
	assert(pszText1 != nullptr);
	assert(pszButton1Text != nullptr);

	// Initialize all members
	//
	m_pPalette = pPalette;
	m_pszMessage1 = pszText1;
	m_pszMessage2 = pszText2;
	m_pszButton1Text = pszButton1Text;

	m_cTextMessage1 = nullptr;
	m_cTextMessage2 = nullptr;

	//{{AFX_DATA_INIT(C1ButtonDialog)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void C1ButtonDialog::ClearDialogImage() {
	if (m_pButton1 != nullptr) {                          // release the 1st button
		delete m_pButton1;
		m_pButton1 = nullptr;
	}

	ValidateRect(nullptr);
}

void C1ButtonDialog::OnDestroy() {
	int i;

	for (i = 0; i < WORDSPERLIST; i++) {
		delete m_cWordList[i];
		m_cWordList[i] = nullptr;
	}

	if (m_cTextMessage1 != nullptr) {
		delete m_cTextMessage1;
		m_cTextMessage1 = nullptr;
	}

	if (m_cTextMessage2 != nullptr) {
		delete m_cTextMessage2;
		m_cTextMessage2 = nullptr;
	}

	CBmpDialog::OnDestroy();
}

void C1ButtonDialog::DoDataExchange(CDataExchange* pDX) {
	CBmpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(C1ButtonDialog)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(C1ButtonDialog, CBmpDialog)
	//{{AFX_MSG_MAP(C1ButtonDialog)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// C1ButtonDialog message handlers

bool C1ButtonDialog::OnInitDialog() {
	CRect   statsRect;                  // game stats displays
	int     nStat_col_offset;           // game stats placement
	int     nStat_row_offset;
	int     nStatWidth, nStatHeight;
	bool    bAssertCheck;
	CDC     *pDC;
	int     i;

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

	nStat_col_offset = 0;
	nStat_row_offset = 30;
	for (i = 0; i < WORDSPERLIST; i++) {
		statsRect.SetRect(
		    nStat_col_offset,
		    nStat_row_offset + (12 * i),
		    nStat_col_offset + nStatWidth,
		    nStat_row_offset + (12 * i) + nStatHeight
		);
		if ((m_cWordList[i] = new CText()) != nullptr) {
			bAssertCheck = (*m_cWordList[i]).SetupText(pDC, m_pPalette, &statsRect, JUSTIFY_CENTER);
			ASSERT(bAssertCheck);   // initialize the text objext
		}
	}

	ReleaseDC(pDC);

	if ((m_pButton1 = new CColorButton) != nullptr) {            // build the first color button
		(*m_pButton1).SetPalette(m_pPalette);                 // set the palette to use
		(*m_pButton1).SetControl(IDOK, this);                 // tie to the dialog control
		(*m_pButton1).SetWindowText(m_pszButton1Text);
	}

	return true;  // return true  unless you set the focus to a control
}

void C1ButtonDialog::OnPaint() {
	CDC     *pDC;
	int     i;
	char    buf[30];

	CBmpDialog::OnPaint();
	// Do not call CDialog::OnPaint() for painting messages

	pDC = GetDC();
	/*
	    if ( m_pszMessage1 != nullptr ) {
	        bAssertCheck = (*m_cTextMessage1).DisplayString(pDC, m_pszMessage1, 21, FW_BOLD, TEXT_COLOR);
	        ASSERT(bAssertCheck);
	    }

	    if ( m_pszMessage2 != nullptr ) {
	        bAssertCheck = (*m_cTextMessage2).DisplayString(pDC, m_pszMessage2, 21, FW_BOLD, TEXT_COLOR);
	        ASSERT(bAssertCheck);
	    }
	*/
	for (i = 0; i < WORDSPERLIST; i++) {
		if ((*astrGameList)[i].IsEmpty()) {
			Common::sprintf_s(buf, "%s", (*astrGameListDisplay)[i].c_str());
			m_cWordList[i]->DisplayString(pDC,  buf, 12, FW_BOLD);
		} else {
			Common::sprintf_s(buf, "%s", (*astrGameListDisplay)[i].c_str());
			m_cWordList[i]->DisplayShadowedString(pDC,  buf, 12, FW_BOLD, (COLORREF)RGB(0, 255, 255));
		}
	}

	ReleaseDC(pDC);
}

void C1ButtonDialog::OnOK() {
	ClearDialogImage();
	EndDialog(IDOK);
}

void C1ButtonDialog::OnCancel() {
	ClearDialogImage();
	EndDialog(IDCANCEL);
}

bool C1ButtonDialog::OnEraseBkgnd(CDC *) {
	// Prevents refreshing of background
	return true;
}

} // namespace WordSearch
} // namespace HodjNPodj
} // namespace Bagel
