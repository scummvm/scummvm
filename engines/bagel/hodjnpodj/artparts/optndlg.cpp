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
#include "bagel/hodjnpodj/artparts/globals.h"
#include "bagel/hodjnpodj/artparts/resource.h"
#include "bagel/hodjnpodj/artparts/artparts.h"
#include "bagel/hodjnpodj/artparts/optndlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

namespace Bagel {
namespace HodjNPodj {
namespace ArtParts {

static  CPalette *pSubOptionsPalette;
static  CColorButton *pOKButton = nullptr;                     // OKAY button on scroll
static  CColorButton *pCancelButton = nullptr;                 // Cancel button on scroll
static  CCheckButton *pFramedButton = nullptr;                 // Framed check box


CText   *m_pPartsText = nullptr;
CText   *m_pColumnText = nullptr;
CText   *m_pRowText = nullptr;
CText   *m_pTimerText = nullptr;

int m_nColumnFactors[14] =  {1, 2, 3, 4, 5, 6, 8, 10, 12, 15, 20, 24, 25, 30};  // 14 factors
int m_nRowFactors[11] =     {1, 2, 3, 4, 6, 8, 9, 12, 16, 18, 24};              // 11 factors
int m_nTimeScale[12] =  {15, 30, 45, 60, 75, 90, 120, 180, 240, 300, 600, 700};

/////////////////////////////////////////////////////////////////////////////
// COptnDlg dialog


COptnDlg::COptnDlg(CWnd* pParent, CPalette* pPalette)
	: CBmpDialog(pParent, pPalette, IDD_SUBOPTIONS, ".\\ART\\SSCROLL.BMP") {
	//{{AFX_DATA_INIT(COptnDlg)
	m_nTime = MIN_TIME;
	m_nColumns = MIN_COLUMNS;
	m_nRows = MIN_ROWS;
	m_nNumParts = m_nColumns * m_nRows;
	nCFacs = 14;
	nRFacs = 11;
	m_bFramed = false;
	pSubOptionsPalette = pPalette;
	//}}AFX_DATA_INIT
}

COptnDlg::~COptnDlg() {
	if (m_pPartsText != nullptr)
		delete m_pPartsText;
	if (m_pColumnText != nullptr)
		delete m_pColumnText;
	if (m_pRowText != nullptr)
		delete m_pRowText;
	if (m_pTimerText != nullptr)
		delete m_pTimerText;
//	if( m_pFramedText != nullptr )
//		delete m_pFramedText;

	CBmpDialog::OnDestroy();
}

void COptnDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptnDlg)
	DDX_Control(pDX, IDC_NUMCOLUMNS, m_ScrollColumns);
	DDX_Control(pDX, IDC_NUMROWS, m_ScrollRows);
	DDX_Control(pDX, IDC_TIMELIMIT, m_ScrollTime);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptnDlg, CDialog)
	//{{AFX_MSG_MAP(COptnDlg)
	ON_WM_HSCROLL()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_FRAMED, COptnDlg::OnFramed)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptnDlg message handlers

int COptnDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CBmpDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

bool COptnDlg::OnInitDialog() {
	CBmpDialog::OnInitDialog();
	int i;

	CDC     *pDC;
	CRect   statRect;

	pDC = GetDC();

	statRect.SetRect(LEFT_SIDE, 45, LEFT_SIDE + 100, 60);
	if ((m_pPartsText = new CText()) != nullptr) {
		(*m_pPartsText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}

	statRect.SetRect(LEFT_SIDE, 65, LEFT_SIDE + 100, 80);
	if ((m_pColumnText = new CText()) != nullptr) {
		(*m_pColumnText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}

	m_ScrollColumns.SetScrollRange(0, nCFacs - 1, 0);   // Array starts at zero, so
	for (i = 0; i < nCFacs; i++) {
		if (m_nColumnFactors[i] == m_nColumns)
			m_ScrollColumns.SetScrollPos(i, true);
	}

	statRect.SetRect(LEFT_SIDE, 99, LEFT_SIDE + 100, 114);
	if ((m_pRowText = new CText()) != nullptr) {
		(*m_pRowText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}

	m_ScrollRows.SetScrollRange(0, nRFacs - 1, 0);       //...last element is Max - 1
	for (i = 0; i < nRFacs; i++) {
		if (m_nRowFactors[i] == m_nRows)
			m_ScrollRows.SetScrollPos(i, true);
	}

	m_nNumParts = m_nColumns * m_nRows;

	statRect.SetRect(LEFT_SIDE, 132, LEFT_SIDE + 100, 146);
	if ((m_pTimerText = new CText()) != nullptr) {
		(*m_pTimerText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}

	m_ScrollTime.SetScrollRange(TIMER_START, TIMER_MAX - 1, 0);
	if (m_nTime == 0) m_nTime = m_nTimeScale[TIMER_MAX - 1];
	for (i = 0; i < TIMER_MAX; i++) {
		if (m_nTimeScale[i] == m_nTime)
			m_ScrollTime.SetScrollPos(i, true);
	}

	ReleaseDC(pDC);

	statRect.SetRect(155, 45, 205, 60);

	if ((pOKButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pOKButton).SetPalette(pSubOptionsPalette);                        // set the palette to use
		(*pOKButton).SetControl(IDOK, this);            // tie to the dialog control
	}

	if ((pCancelButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pCancelButton).SetPalette(pSubOptionsPalette);                        // set the palette to use
		(*pCancelButton).SetControl(IDCANCEL, this);            // tie to the dialog control
	}

	if ((pFramedButton = new CCheckButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pFramedButton).SetPalette(pSubOptionsPalette);                        // set the palette to use
		(*pFramedButton).SetControl(IDC_FRAMED, this);              // tie to the dialog control
	}
	((CWnd *)this)->CheckDlgButton(IDC_FRAMED, m_bFramed);           // Set the frame option box

	return true;  // return true  unless you set the focus to a control
}


bool COptnDlg::OnEraseBkgnd(CDC *pDC) {
	return true;
}

void COptnDlg::OnDestroy() {
	CBmpDialog::OnDestroy();
}


void COptnDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar) {
	int pMin,
	    pMax;
	int OldPos = pScrollBar->GetScrollPos();
	int NewPos = OldPos;

	pScrollBar->GetScrollRange(&pMin, &pMax);

	switch (nSBCode) {
	case SB_LINERIGHT:
	case SB_PAGERIGHT:
		NewPos ++;
		break;
	case SB_RIGHT:
		NewPos = pMax;
		break;
	case SB_LINELEFT:
	case SB_PAGELEFT:
		NewPos--;
		break;
	case SB_LEFT:
		NewPos = pMin;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		NewPos = nPos;
		break;
	}

	if (NewPos < pMin) NewPos = pMin;
	if (NewPos > pMax) NewPos = pMax;

	if (NewPos != OldPos) {                              //To prevent "flicker"
		(*pScrollBar).SetScrollPos(NewPos, true);        //...only update when
	}                                                   //...changed

	UpdateScrollbars();

	CDialog::OnHScroll(nSBCode, NewPos, pScrollBar);
}


/*****************************************************************
 *
 *  UpdateScrollbars
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Updates data adjusted with scrollbars
 *
 *  FORMAL PARAMETERS:
 *
 *      none
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      CScrollbar  pScrollTime, pScrollColumns, pScrollRows
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      int m_nTime, m_nNumParts, m_nColumns, m_nRows
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void COptnDlg::UpdateScrollbars() {
	int OldValue;
	CDC     *pDC;
	char    msg[64];
	int     nMins, nSecs;

	pDC = GetDC();

	OldValue = m_nTime;
	m_nTime = m_nTimeScale[m_ScrollTime.GetScrollPos()];
	if (OldValue != m_nTime) {
		if (m_nTime == m_nTimeScale[TIMER_MAX - 1])
			Common::sprintf_s(msg, "Time Limit: None");
		else {
			nMins = m_nTime / 60;
			nSecs = m_nTime % 60;

			Common::sprintf_s(msg, "Time Limit: %02d:%02d", nMins, nSecs);
		}
		(*m_pTimerText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);
	}

	OldValue = m_nColumns;
	m_nColumns = m_nColumnFactors[m_ScrollColumns.GetScrollPos()];
	if (OldValue != m_nColumns) {
		Common::sprintf_s(msg, "Columns: %d", m_nColumns);
		(*m_pColumnText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);
	}

	OldValue = m_nRows;
	m_nRows = m_nRowFactors[m_ScrollRows.GetScrollPos()];
	if (OldValue != m_nRows) {
		Common::sprintf_s(msg, "Rows: %d", m_nRows);
		(*m_pRowText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);
	}

	OldValue = m_nNumParts;
	m_nNumParts = m_nColumns * m_nRows;
	if (OldValue != m_nNumParts) {
		Common::sprintf_s(msg, "Parts: %d", m_nNumParts);
		(*m_pPartsText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);
	}

	ReleaseDC(pDC);
}


void COptnDlg::OnFramed() {
	m_bFramed = !m_bFramed;
	((CWnd *)this)->CheckDlgButton(IDC_FRAMED, m_bFramed);
}

void COptnDlg::OnOK() {
	if (m_nTime > MAX_TIME) m_nTime = MIN_TIME;
	ClearDialogImage();
	EndDialog(IDOK);
}

void COptnDlg::OnCancel() {
	ClearDialogImage();
	EndDialog(0);
}

void COptnDlg::OnPaint() {
	CDC     *pDC;
	char    msg[64];
	int     nMins, nSecs;

	CBmpDialog::OnPaint();

	pDC = GetDC();

	Common::sprintf_s(msg, "Parts: %d", m_nNumParts);
	(*m_pPartsText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);

	Common::sprintf_s(msg, "Columns: %d", m_nColumns);
	(*m_pColumnText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);

	Common::sprintf_s(msg, "Rows: %d", m_nRows);
	(*m_pRowText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);

	if (m_nTime == m_nTimeScale[TIMER_MAX - 1])
		Common::sprintf_s(msg, "Time Limit: None");
	else {
		nMins = m_nTime / 60;
		nSecs = m_nTime % 60;
		Common::sprintf_s(msg, "Time Limit: %02d:%02d", nMins, nSecs);
	}
	(*m_pTimerText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);

	ReleaseDC(pDC);

}

void COptnDlg::ClearDialogImage() {
	if (pOKButton != nullptr) {                          // release the button
		delete pOKButton;
		pOKButton = nullptr;
	}

	if (pCancelButton != nullptr) {                        // release the button
		delete pCancelButton;
		pCancelButton = nullptr;
	}

	if (pFramedButton != nullptr) {                        // release the button
		delete pFramedButton;
		pFramedButton = nullptr;
	}

	ValidateRect(nullptr);
}

} // namespace ArtParts
} // namespace HodjNPodj
} // namespace Bagel
