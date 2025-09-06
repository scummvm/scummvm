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
#include "bagel/hodjnpodj/crypt/optn.h"
#include "bagel/hodjnpodj/crypt/resource.h"

namespace Bagel {
namespace HodjNPodj {
namespace Crypt {

static  CColorButton *pOKButton = nullptr;                     // OKAY button on scroll
static  CColorButton *pCancelButton = nullptr;                 // Cancel button on scroll
/////////////////////////////////////////////////////////////////////////////
// COptn dialog


COptn::COptn(
    CWnd* pParent /*=nullptr*/,
    CPalette *pPalette /*=nullptr*/,
    int nLttrsSlvd /*LSLVD_DEFAULT*/,
    int nTime /*TIME_DEFAULT*/
)
	: CBmpDialog(pParent, pPalette, IDD_USER_OPTIONS, ".\\ART\\SSCROLL.BMP")

{
	//{{AFX_DATA_INIT(COptn)
	m_nTimeIndex[0] = 15;
	m_nTimeIndex[1] = 20;
	m_nTimeIndex[2] = 25;
	m_nTimeIndex[3] = 30;
	m_nTimeIndex[4] = 35;
	m_nTimeIndex[5] = 40;
	m_nTimeIndex[6] = 45;
	m_nTimeIndex[7] = 50;
	m_nTimeIndex[8] = 60;
	m_nTimeIndex[9] = 70;
	m_nTimeIndex[10] = 80;
	m_nTimeIndex[11] = 90;
	m_nTimeIndex[12] = 120;
	m_nTimeIndex[13] = 150;
	m_nTimeIndex[14] = 180;
	m_nTimeIndex[15] = 240;
	m_nTimeIndex[16] = 300;
	m_nTimeIndex[17] = 589;
	m_nTimeIndex[18] = 601;

	pGamePalette = pPalette;

	m_nTime = TimeToIndex(nTime);
	m_nLttrsSlvd = nLttrsSlvd;

	m_pLttrsSlvd = nullptr;
	m_pTime = nullptr;
	//}}AFX_DATA_INIT
}

COptn::~COptn() {
	if (m_pLttrsSlvd != nullptr) {
		delete m_pLttrsSlvd;
		m_pLttrsSlvd = nullptr;
	}

	if (m_pTime != nullptr) {
		delete m_pTime;
		m_pLttrsSlvd = nullptr;
	}

	CBmpDialog::OnDestroy();
}

int COptn::TimeToIndex(int nTime) {
	int nLow;

	/***********************************
	* Seaches ordered array in n time. *
	***********************************/
	for (nLow = 0; nLow < TIME_TABLE ; nLow++) {
		if (m_nTimeIndex[nLow] == nTime)
			return (int)nLow;
	}

	return false;
}

void COptn::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptn)
	DDX_Control(pDX, IDC_LTTRSSLVD, m_LttrsSlvd);
	DDX_Control(pDX, IDC_TIME, m_Time);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptn, CDialog)
	//{{AFX_MSG_MAP(COptn)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptn message handlers

bool COptn::OnInitDialog() {
	CRect   statsRect;                      // game stats displays
	int     nStat_col_offset;               // game stats placement
	int     nStat_row_offset;
	int     nStatWidth, nStatHeight;
	bool    bAssertCheck;
	CDC     *pDC;

	CBmpDialog::OnInitDialog();

	// TODO: Add extra initialization here
	m_chTime[0] = "15 Seconds";
	m_chTime[1] = "20 Seconds";
	m_chTime[2] = "25 Seconds";
	m_chTime[3] = "30 Seconds";
	m_chTime[4] = "35 Seconds";
	m_chTime[5] = "40 Seconds";
	m_chTime[6] = "45 Seconds";
	m_chTime[7] = "50 Seconds";
	m_chTime[8] = "60 Seconds";
	m_chTime[9] = "70 Seconds";
	m_chTime[10] = "80 Seconds";
	m_chTime[11] = "90 Seconds";
	m_chTime[12] = "120 Seconds";
	m_chTime[13] = "150 Seconds";
	m_chTime[14] = "3 Minutes";
	m_chTime[15] = "4 Minutes";
	m_chTime[16] = "5 Minutes";
	m_chTime[17] = "10 Minutes";
	m_chTime[18] = "None";

	pDC = GetDC();

	nStat_col_offset    = STAT_BOX_COL;     // setup the Letters Solved
	nStat_row_offset    = STAT_BOX_ROW;     //  stat display box
	nStatWidth          = STAT_BOX_WIDTH;
	nStatHeight         = STAT_BOX_HEIGHT;
	statsRect.SetRect(
	    nStat_col_offset,
	    nStat_row_offset,
	    nStat_col_offset + nStatWidth,
	    nStat_row_offset + nStatHeight
	);

	if ((m_pLttrsSlvd = new CText()) != nullptr) {
		bAssertCheck = (*m_pLttrsSlvd).SetupText(pDC, pGamePalette, &statsRect, JUSTIFY_LEFT);
		ASSERT(bAssertCheck);               // initialize the text objext
	}
	m_LttrsSlvd.SetScrollRange(MIN_LTTRSSLVD, MAX_LTTRSSLVD, false);    // setup Letters Solved defaults
	m_LttrsSlvd.SetScrollPos(m_nLttrsSlvd, true);

	nStat_row_offset += BOX_ROW_OFFSET;     // Time stat display box
	statsRect.SetRect(
	    nStat_col_offset,
	    nStat_row_offset,
	    nStat_col_offset + nStatWidth,
	    nStat_row_offset + nStatHeight
	);

	if ((m_pTime = new CText()) != nullptr) {
		bAssertCheck = (*m_pTime).SetupText(pDC, pGamePalette, &statsRect, JUSTIFY_LEFT);
		ASSERT(bAssertCheck);               // initialize the text objext
	}

	m_Time.SetScrollRange(MIN_INDEX_TIME, MAX_INDEX_TIME, false);               // Time scroll bar defaults
	m_Time.SetScrollPos(m_nTime, true);

	if ((pOKButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pOKButton).SetPalette(pGamePalette);                      // set the palette to use
		(*pOKButton).SetControl(IDOK, this);            // tie to the dialog control
	}

	if ((pCancelButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pCancelButton).SetPalette(pGamePalette);                      // set the palette to use
		(*pCancelButton).SetControl(IDCANCEL, this);            // tie to the dialog control
	}

	ReleaseDC(pDC);
	return true;  // return true  unless you set the focus to a control
}

void COptn::OnDestroy() {
	CBmpDialog::OnDestroy();
}


void COptn::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar) {
	// TODO: Add your message handler code here and/or call default
	CDC     *pDC;
	bool    bAssertCheck;
	char    msg[64];

	pDC = GetDC();

	if (pScrollBar->GetDlgCtrlID() == IDC_LTTRSSLVD) {
		switch (nSBCode) {

		case SB_LINERIGHT:
			m_nLttrsSlvd++;
			break;

		case SB_PAGERIGHT:
			m_nLttrsSlvd += (int) MAX_LTTRSSLVD / 5; // magic # 5 = five divisions
			break;

		case SB_RIGHT:
			m_nLttrsSlvd = MAX_LTTRSSLVD;
			break;

		case SB_LINELEFT:
			m_nLttrsSlvd--;
			break;

		case SB_PAGELEFT:
			m_nLttrsSlvd -= (int) MAX_LTTRSSLVD / 5; // magic # 5 = five divisions
			break;

		case SB_LEFT:
			m_nLttrsSlvd = MIN_LTTRSSLVD;
			break;

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			m_nLttrsSlvd = (int) nPos;
			break;

		} // end switch

		if (m_nLttrsSlvd < MIN_LTTRSSLVD)
			m_nLttrsSlvd = MIN_LTTRSSLVD;
		if (m_nLttrsSlvd > MAX_LTTRSSLVD)
			m_nLttrsSlvd = MAX_LTTRSSLVD;

		Common::sprintf_s(msg, "Letters Solved: %d", m_nLttrsSlvd);
		bAssertCheck = (*m_pLttrsSlvd).DisplayString(pDC, msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
		ASSERT(bAssertCheck);
		pScrollBar->SetScrollPos(m_nLttrsSlvd, true);

	} else if (pScrollBar->GetDlgCtrlID() == IDC_TIME) {
		switch (nSBCode) {

		case SB_LINERIGHT:
			m_nTime++;
			break;
		case SB_PAGERIGHT:
			m_nTime += MAX_INDEX_TIME / 5; // want 5 pagerights end to end
			break;
		case SB_RIGHT:
			m_nTime = MAX_INDEX_TIME;
			break;
		case SB_LINELEFT:
			m_nTime--;
			break;
		case SB_PAGELEFT:
			m_nTime -= MAX_INDEX_TIME / 5; // want 5 pagerights end to end
			break;
		case SB_LEFT:
			m_nTime = 0;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			m_nTime = (int) nPos;
			break;
		} // end switch

		if (m_nTime < MIN_INDEX_TIME)
			m_nTime = MIN_INDEX_TIME;
		if (m_nTime > MAX_INDEX_TIME)
			m_nTime = MAX_INDEX_TIME;

		Common::sprintf_s(msg, "Time Limit: %s", m_chTime[m_nTime].c_str());

		bAssertCheck = (*m_pTime).DisplayString(pDC, msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
		ASSERT(bAssertCheck);   // paint the text

		pScrollBar->SetScrollPos(m_nTime, true);
	} // end if

	ReleaseDC(pDC);
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void COptn::OnPaint() {
	CDC     *pDC;
	bool    bAssertCheck;
	char    msg[64];

	CBmpDialog::OnPaint();
	// Do not call CDialog::OnPaint() for painting messages

	pDC = GetDC();

	Common::sprintf_s(msg, "Letters Solved: %d", m_nLttrsSlvd); // Display Letters Solved stats
	bAssertCheck = (*m_pLttrsSlvd).DisplayString(pDC, msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
	ASSERT(bAssertCheck);

	Common::sprintf_s(msg, "Time: %s", m_chTime[m_nTime].c_str());      // Display Time stats
	bAssertCheck = (*m_pTime).DisplayString(pDC, msg, FONT_SIZE, FW_BOLD, OPTIONS_COLOR);
	ASSERT(bAssertCheck);

	ReleaseDC(pDC);
}

bool COptn::OnEraseBkgnd(CDC *pDC) {
	return true;
}

void COptn::OnOK() {
	m_nTime = m_nTimeIndex[m_nTime];
	ClearDialogImage();
	EndDialog(IDOK);
//		CDialog::OnOK();
}

void COptn::OnCancel() {
	ClearDialogImage();
	EndDialog(0);
}

void COptn::ClearDialogImage() {
	if (pOKButton != nullptr) {                          // release the button
		delete pOKButton;
		pOKButton = nullptr;
	}

	if (pCancelButton != nullptr) {                        // release the button
		delete pCancelButton;
		pCancelButton = nullptr;
	}

	ValidateRect(nullptr);
}

} // namespace Crypt
} // namespace HodjNPodj
} // namespace Bagel
