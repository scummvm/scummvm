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
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/mankala/mnk.h"
#include "bagel/hodjnpodj/mankala/mnkopt.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace Mankala {

static  CColorButton *pOKButton = nullptr;                     // OKAY button on scroll
static  CColorButton *pCancelButton = nullptr;                 // Cancel button on scroll


CText   *m_pShellText = nullptr;
CText   *m_pStrengthText = nullptr;

/////////////////////////////////////////////////////////////////////////////
// CMnkOpt dialog

CMnkOpt::CMnkOpt(CWnd* pParent /*=nullptr*/)
	: CDialog(CMnkOpt::IDD, pParent) {
	//{{AFX_DATA_INIT(CMnkOpt)
	m_iLevel0 = -1;
	m_iLevel1 = -1;
	m_iPlayer0 = -1;
	m_iPlayer1 = -1;
	m_iStartStones = 0;
	m_iTableStones = 0;
	m_bInitData = false;
	m_iMaxDepth0 = 0;
	m_iMaxDepth1 = 0;
	m_bDumpMoves = false;
	m_bDumpPopulate = false;
	m_bDumpTree = false;
	m_iCapDepth0 = 0;
	m_iCapDepth1 = 0;
	//}}AFX_DATA_INIT
}

CMnkOpt::~CMnkOpt() {
}


void CMnkOpt::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMnkOpt)
	DDX_Radio(pDX, IDC_ALG_0, m_iLevel0);
	DDX_Radio(pDX, IDC_ALG_1, m_iLevel1);
	DDX_Radio(pDX, IDC_PLAY_0, m_iPlayer0);
	DDX_Radio(pDX, IDC_PLAY_1, m_iPlayer1);
	DDX_Text(pDX, IDC_STARTSTONES, m_iStartStones);
	DDV_MinMaxInt(pDX, m_iStartStones, 1, 12);
	DDX_Text(pDX, IDC_TABLESTONES, m_iTableStones);
	DDV_MinMaxInt(pDX, m_iTableStones, 0, 12);
	DDX_Check(pDX, IDC_INITDATA, m_bInitData);
	DDX_Text(pDX, IDC_MAXDEPTH0, m_iMaxDepth0);
	DDV_MinMaxInt(pDX, m_iMaxDepth0, 0, 30);
	DDX_Text(pDX, IDC_MAXDEPTH1, m_iMaxDepth1);
	DDV_MinMaxInt(pDX, m_iMaxDepth1, 0, 30);
	DDX_Check(pDX, IDC_DUMPMOVES, m_bDumpMoves);
	DDX_Check(pDX, IDC_DUMPPOP, m_bDumpPopulate);
	DDX_Check(pDX, IDC_DUMPTREE, m_bDumpTree);
	DDX_Text(pDX, IDC_CAPDEPTH0, m_iCapDepth0);
	DDV_MinMaxInt(pDX, m_iCapDepth0, 0, 30);
	DDX_Text(pDX, IDC_CAPDEPTH1, m_iCapDepth1);
	DDV_MinMaxInt(pDX, m_iCapDepth1, 0, 30);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMnkOpt, CDialog)
	//{{AFX_MSG_MAP(CMnkOpt)
	// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMnkOpt message handlers



/////////////////////////////////////////////////////////////////////////////
// CMnkUsr dialog

CMnkUsr::CMnkUsr(CWnd *xpParent, CPalette *xpPalette, unsigned int nID)
	:  CBmpDialog(xpParent, xpPalette, nID, ".\\ART\\SSCROLL.BMP") {
	//{{AFX_DATA_INIT(CMnkUsr)
	m_iUShells = 0;
	m_iUStrength = 0;
	//}}AFX_DATA_INIT

	m_xpGamePalette = xpPalette;
//    DoModal();
}

CMnkUsr::~CMnkUsr() {
	if (m_pShellText != nullptr)
		delete m_pShellText;
	if (m_pStrengthText != nullptr)
		delete m_pStrengthText;
}

void CMnkUsr::DoDataExchange(CDataExchange* pDX) {
	CBmpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMnkUsr)
//	DDX_Text(pDX, IDC_USHELLS, m_iUShells);
//	DDV_MinMaxInt(pDX, m_iUShells, 1, 12);
//	DDX_Text(pDX, IDC_USTRENGTH, m_iUStrength);
//	DDV_MinMaxInt(pDX, m_iUStrength, 1, 5);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMnkUsr, CBmpDialog)
	//{{AFX_MSG_MAP(CMnkUsr)
	ON_WM_HSCROLL()
	ON_WM_PAINT()
//	ON_EN_KILLFOCUS(IDC_USHELLS, OnKillfocusUShells)
//	ON_EN_KILLFOCUS(IDC_USTRENGTH, OnKillfocusUStrength)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMnkUsr message handlers

bool CMnkUsr::OnInitDialog() {
	CBmpDialog::OnInitDialog();

	CDC     *pDC;
	CRect   statRect;

	pDC = GetDC();

	mLevelTable[0] =  "CrabCakes";
	mLevelTable[1] =  "Soft-shell";
	mLevelTable[2] =  "Tough'n'Chewy";
	mLevelTable[3] =  "King Crab";
	mLevelTable[4] =  "Invincible";         /*nish added 08/04/94*/
	statRect.SetRect(LEFT_SIDE, 25, LEFT_SIDE + 100, 40);
	if ((m_pShellText = new CText()) != nullptr) {
		(*m_pShellText).SetupText(pDC, m_xpGamePalette, &statRect, JUSTIFY_LEFT);
	}

	statRect.SetRect(LEFT_SIDE, 65, LEFT_SIDE + 185, 80);
	if ((m_pStrengthText = new CText()) != nullptr) {
		(*m_pStrengthText).SetupText(pDC, m_xpGamePalette, &statRect, JUSTIFY_LEFT);
	}

	m_xpUScrShell = (CScrollBar *)GetDlgItem(IDC_USCRSHELL) ;
	m_xpUScrShell->SetScrollRange(MINSTONES, MAXSTONES, true) ;
	m_xpUScrShell->SetScrollPos(m_iUShells) ;

	m_xpUScrStrength = (CScrollBar *)GetDlgItem(IDC_USCRSTRENGTH) ;
	m_xpUScrStrength->SetScrollRange(MINSTRENGTH, MAXSTRENGTH, true) ;
	m_xpUScrStrength->SetScrollPos(m_iUStrength) ;

	if ((pOKButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pOKButton).SetPalette(m_pPalette);                        // set the palette to use
		(*pOKButton).SetControl(IDOK, this);            // tie to the dialog control
	}

	if ((pCancelButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pCancelButton).SetPalette(m_pPalette);                        // set the palette to use
		(*pCancelButton).SetControl(IDCANCEL, this);            // tie to the dialog control
	}


	ReleaseDC(pDC);
	return true;  // return true  unless you set the focus to a control
}

void CMnkUsr::ClearDialogImage() {
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

bool CMnkUsr::OnCommand(WPARAM wParam, LPARAM lParam) {
	/*
	* respond to user
	*/
	if (HIWORD(lParam) == BN_CLICKED) {

		switch (wParam) {

		case IDOK:
			ClearDialogImage();
			EndDialog(IDOK);
			return false;

		case IDCANCEL:
			ClearDialogImage();
			EndDialog(IDCANCEL);
			return false;

		default:
			break;
		}
	}

	return CBmpDialog::OnCommand(wParam, lParam);
}

void CMnkUsr::OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* xpScrollBar) {
//    int iId = xpScrollBar->GetDlgCtrlID() ;   // get id of control bar
	int iValMin, iValMax, iValCur ; // min, max, current values

	xpScrollBar->GetScrollRange(&iValMin, &iValMax) ;
	iValCur = xpScrollBar->GetScrollPos() ;

//    int *xpiVariable = (iId == IDC_USCRSHELL) ? &m_iUShells
//		: (iId == IDC_USCRSTRENGTH) ? &m_iUStrength : nullptr ;

	switch (nSBCode) {
	case SB_LEFT:
		iValCur = iValMin ;
		break;

	case SB_PAGELEFT:
		iValCur -= (iValMax - iValMin) / 3 ;
		break;

	case SB_LINELEFT:
		iValCur-- ;
		break;

	case SB_RIGHT:
		iValCur = iValMax ;
		break;

	case SB_PAGERIGHT:
		iValCur += (iValMax - iValMin) / 3 ;
		break;

	case SB_LINERIGHT:
		iValCur++ ;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		iValCur = nPos;
		break;

	default:
		break;
	}

	if (iValCur < iValMin)
		iValCur = iValMin ;

	if (iValCur > iValMax)
		iValCur = iValMax ;

	xpScrollBar->SetScrollPos(iValCur);

//    if (xpiVariable)
//	*xpiVariable = iValCur ;

	//UpdateData(false) ;

	UpdateScrollbars();

	CBmpDialog::OnHScroll(nSBCode, nPos, xpScrollBar);
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
 *      CScrollbar  pScrollShells, pScrollStrength
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
void CMnkUsr::UpdateScrollbars() {
	int     OldValue;
	CDC     *pDC;
	char    msg[64];

	pDC = GetDC();

	OldValue = m_iUShells;
	m_iUShells = m_xpUScrShell->GetScrollPos();
	if (OldValue != m_iUShells) {
		Common::sprintf_s(msg, "Shells per Pit: %d", m_iUShells);
		(*m_pShellText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);
	}

	OldValue = m_iUStrength;
	m_iUStrength = m_xpUScrStrength->GetScrollPos();
	if (OldValue != m_iUStrength) {
		Common::sprintf_s(msg, "Crab's Ability: %s", mLevelTable[m_iUStrength - 1].c_str());
		(*m_pStrengthText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);
	}

	ReleaseDC(pDC);
}



void CMnkUsr::OnPaint() {
	CDC     *pDC;
	char    msg[64];

	CBmpDialog::OnPaint();

	pDC = GetDC();

	Common::sprintf_s(msg, "Shells per Pit: %d", m_iUShells);
	(*m_pShellText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);

	Common::sprintf_s(msg, "Crab's Ability: %s", mLevelTable[m_iUStrength - 1].c_str());
	(*m_pStrengthText).DisplayString(pDC, msg, 14, FW_BOLD, OPTIONS_COLOR);

	ReleaseDC(pDC);

}

} // namespace Mankala
} // namespace HodjNPodj
} // namespace Bagel
