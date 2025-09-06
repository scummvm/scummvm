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
#include "bagel/hodjnpodj/mazedoom/globals.h"
#include "bagel/hodjnpodj/mazedoom/resource.h"
#include "bagel/hodjnpodj/mazedoom/mod.h"
#include "bagel/hodjnpodj/mazedoom/optndlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

static  CPalette *pSubOptionsPalette;
static  CColorButton *pOKButton = nullptr;                     // OKAY button on scroll
static  CColorButton *pCancelButton = nullptr;                 // Cancel button on scroll

CText   *m_pTimeLeft = nullptr;
CText   *m_pDiffTitleText = nullptr;
CText   *m_pDifficultyText = nullptr;
CText   *m_pTimerText = nullptr;

int m_nTimeScale[10] =  {30, 45, 60, 75, 90, 120, 180, 240, 300, 600};

/////////////////////////////////////////////////////////////////////////////
// COptnDlg dialog


COptnDlg::COptnDlg(CWnd* pParent, CPalette* pPalette)
	: CBmpDialog(pParent, pPalette, IDD_SUBOPTIONS, ".\\ART\\SSCROLL.BMP") {
	//{{AFX_DATA_INIT(COptnDlg)
	_time = MIN_TIME;
	_difficulty = MIN_DIFFICULTY;
	m_pTimeLeft = nullptr;
	m_pTimerText = nullptr;
	m_pDifficultyText = nullptr;
	m_pDiffTitleText = nullptr;

	pSubOptionsPalette = pPalette;
	//}}AFX_DATA_INIT
}

COptnDlg::~COptnDlg() {
	if (m_pTimeLeft != nullptr)
		delete m_pTimeLeft;
	if (m_pTimerText != nullptr)
		delete m_pTimerText;
	if (m_pDiffTitleText != nullptr)
		delete m_pDiffTitleText;
	if (m_pDifficultyText != nullptr)
		delete m_pDifficultyText;

	CBmpDialog::OnDestroy();
}

void COptnDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptnDlg)
	DDX_Control(pDX, IDC_DIFFICULTY, m_ScrollDifficulty);
	DDX_Control(pDX, IDC_TIMELIMIT, m_ScrollTime);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptnDlg, CDialog)
	//{{AFX_MSG_MAP(COptnDlg)
	ON_WM_HSCROLL()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
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

	CDC     *pDC;
	CRect   statRect;
	int     i;

	mDifficultyTable[0] =  "Total Wussy";
	mDifficultyTable[1] =  "Big Sissy";
	mDifficultyTable[2] =  "Major Wimp";
	mDifficultyTable[3] =  "Minor Wimp";
	mDifficultyTable[4] =  "Majorly Minor Miner";
	mDifficultyTable[5] =  "Minor Miner";
	mDifficultyTable[6] =  "Miner";
	mDifficultyTable[7] =  "Major Miner";
	mDifficultyTable[8] =  "Enchantemite Miner";
	mDifficultyTable[9] =  "Studly Enchantemite Miner";

	pDC = GetDC();

	statRect.SetRect(LEFT_SIDE, 25, LEFT_SIDE + 70, 40);
	if ((m_pTimeLeft = new CText()) != nullptr) {
		(*m_pTimeLeft).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}

	statRect.SetRect(LEFT_SIDE, 45, LEFT_SIDE + 80, 70);
	if ((m_pDiffTitleText = new CText()) != nullptr) {
		(*m_pDiffTitleText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}

	statRect.SetRect(LEFT_SIDE, 65, LEFT_SIDE + 170, 80);
	if ((m_pDifficultyText = new CText()) != nullptr) {
		(*m_pDifficultyText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}

	m_ScrollDifficulty.SetScrollRange(MIN_DIFFICULTY, MAX_DIFFICULTY, 0);       //...last element is Max - 1
	m_ScrollDifficulty.SetScrollPos(_difficulty, true);

	statRect.SetRect(LEFT_SIDE, 115, LEFT_SIDE + 100, 130);
	if ((m_pTimerText = new CText()) != nullptr) {
		(*m_pTimerText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}

	m_ScrollTime.SetScrollRange(TIMER_MIN, TIMER_MAX - 1, 0);
	if (_time == 0) _time = m_nTimeScale[TIMER_MAX - 1];
	for (i = 0; i < TIMER_MAX; i++) {
		if (m_nTimeScale[i] == _time)
			m_ScrollTime.SetScrollPos(i, true);
	}

	ReleaseDC(pDC);

	if ((pOKButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pOKButton).SetPalette(pSubOptionsPalette);                        // set the palette to use
		(*pOKButton).SetControl(IDOK, this);            // tie to the dialog control
	}

	if ((pCancelButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pCancelButton).SetPalette(pSubOptionsPalette);                        // set the palette to use
		(*pCancelButton).SetControl(IDCANCEL, this);            // tie to the dialog control
	}

	return true;  // return true  unless you set the focus to a control
}


bool COptnDlg::OnEraseBkgnd(CDC *pDC) {
	return true;
}


void COptnDlg::OnDestroy() {
	CBmpDialog::OnDestroy();
}


void COptnDlg::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar) {
	int     pMin,
	        pMax;
	int     OldPos = pScrollBar->GetScrollPos();
	int     NewPos = OldPos;

	pScrollBar->GetScrollRange(&pMin, &pMax);

	switch (nSBCode) {
	case SB_LINERIGHT:
	case SB_PAGERIGHT:
		NewPos++;                           // Increment by one
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
 *      int _time, m_nNumParts, m_nColumns, m_nRows
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
	int     m_nMins, m_nSecs;

	pDC = GetDC();

	OldValue = _time;
	_time = m_nTimeScale[m_ScrollTime.GetScrollPos()];
	if (OldValue != _time) {
		if (_time == m_nTimeScale[TIMER_MAX - 1])
			Common::sprintf_s(msg, "Time Limit: None");
		else {
			m_nMins = _time / 60;
			m_nSecs = _time % 60;

			Common::sprintf_s(msg, "Time Limit: %02d:%02d", m_nMins, m_nSecs);
		}
		(*m_pTimerText).DisplayString(pDC, msg, 14, TEXT_BOLD, RGB(0, 0, 0));
	}

	OldValue = _difficulty;
	_difficulty = m_ScrollDifficulty.GetScrollPos();
	if (OldValue != _difficulty) {
		Common::sprintf_s(msg, "%s", mDifficultyTable[_difficulty - 1].c_str());
		(*m_pDifficultyText).DisplayString(pDC, msg, 14, TEXT_BOLD, RGB(0, 0, 0));
	}

	ReleaseDC(pDC);
}


void COptnDlg::OnOK() {
	if (_time == m_nTimeScale[TIMER_MAX - 1]) _time = 0;
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
	int     m_nMins, m_nSecs;

	CBmpDialog::OnPaint();

	pDC = GetDC();

	Common::sprintf_s(msg, "Time: %02d:%02d", _minutes, _seconds);
	(*m_pTimeLeft).DisplayString(pDC, msg, 14, TEXT_BOLD, RGB(0, 0, 0));

	Common::sprintf_s(msg, "Level:");
	(*m_pDiffTitleText).DisplayString(pDC, msg, 14, TEXT_BOLD, RGB(0, 0, 0));

	Common::sprintf_s(msg, "%s", mDifficultyTable[_difficulty - 1].c_str());
	(*m_pDifficultyText).DisplayString(pDC, msg, 14, TEXT_BOLD, RGB(0, 0, 0));

	if (_time == m_nTimeScale[TIMER_MAX - 1])
		Common::sprintf_s(msg, "Time Limit: None");
	else {
		m_nMins = _time / 60;
		m_nSecs = _time % 60;

		Common::sprintf_s(msg, "Time Limit: %02d:%02d", m_nMins, m_nSecs);
	}
	(*m_pTimerText).DisplayString(pDC, msg, 14, TEXT_BOLD, RGB(0, 0, 0));

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

	ValidateRect(nullptr);
}

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel
