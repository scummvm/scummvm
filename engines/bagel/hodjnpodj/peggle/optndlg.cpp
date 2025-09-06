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
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/peggle/resource.h"
#include "bagel/hodjnpodj/peggle/options.h"
#include "bagel/hodjnpodj/peggle/optndlg.h"
#include "bagel/hodjnpodj/peggle/game2.h"

namespace Bagel {
namespace HodjNPodj {
namespace Peggle {

CPalette    *pSubOptionsPalette;
CBmpButton  *m_pTriButton = nullptr;
CBmpButton  *m_pTriPButton = nullptr;
CBmpButton  *m_pCroButton = nullptr;
CBmpButton  *m_pCroPButton = nullptr;

static  CCheckButton *pRandomButton = nullptr;                 // Framed check box

extern  int8        BoardSelected;
extern  bool        bRandomBoard;
static  char        chNewBoard;
static  bool        m_bRandom;

/////////////////////////////////////////////////////////////////////////////
// COptnDlg dialog


COptnDlg::COptnDlg(CWnd* pParent, CPalette* pPalette)
	: COptions(pParent, pPalette, IDD_SUBOPTIONS)
//	: CDialog(COptnDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptnDlg)
	pSubOptionsPalette = pPalette;
	//}}AFX_DATA_INIT
}

void COptnDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptnDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptnDlg, CDialog)
	//{{AFX_MSG_MAP(COptnDlg)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_SUB_OKAY, COptnDlg::OnClickedOkay)
	ON_BN_CLICKED(IDC_SUB_CANCEL, COptnDlg::OnClickedCancel)
	ON_BN_CLICKED(IDC_SUB_CROSS, COptnDlg::OnClickedCross)
	ON_BN_CLICKED(IDC_SUB_CROSSPLUS, COptnDlg::OnClickedCrossPlus)
	ON_BN_CLICKED(IDC_SUB_TRIANGLE, COptnDlg::OnClickedTriangle)
	ON_BN_CLICKED(IDC_SUB_TRIANGLEPLUS, COptnDlg::OnClickedTrianglePlus)
	ON_BN_CLICKED(IDC_RANDOM, COptnDlg::OnRandom)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptnDlg message handlers

int COptnDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (COptions::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

bool COptnDlg::OnInitDialog() {
	bool    bSuccess;
	CRect   myRect;

	COptions::OnInitDialog();

	chNewBoard = BoardSelected;
	m_bRandom = bRandomBoard;

	m_pTriButton = new CBmpButton;
	ASSERT(m_pTriButton != nullptr);
	myRect.SetRect(TRIANGLE_X, TRIANGLE_Y, TRIANGLE_X + OPTN_BUTTON_WIDTH, TRIANGLE_Y + OPTN_BUTTON_HEIGHT);
	bSuccess = (*m_pTriButton).Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, myRect, this, IDC_SUB_TRIANGLE);
	ASSERT(bSuccess);
	bSuccess = (*m_pTriButton).LoadBitmaps("TRIUP", "TRIDOWN", nullptr, nullptr);
	ASSERT(bSuccess);

	m_pCroButton = new CBmpButton;
	ASSERT(m_pCroButton != nullptr);
	myRect.SetRect(CROSS_X, CROSS_Y, CROSS_X + OPTN_BUTTON_WIDTH, CROSS_Y + OPTN_BUTTON_HEIGHT);
	bSuccess = (*m_pCroButton).Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, myRect, this, IDC_SUB_CROSS);
	ASSERT(bSuccess);
	bSuccess = (*m_pCroButton).LoadBitmaps("CROUP", "CRODOWN", nullptr, nullptr);
	ASSERT(bSuccess);

	m_pTriPButton = new CBmpButton;
	ASSERT(m_pTriPButton != nullptr);
	myRect.SetRect(TRIANGLEPLUS_X, TRIANGLEPLUS_Y,
	               TRIANGLEPLUS_X + OPTN_BUTTON_WIDTH, TRIANGLEPLUS_Y + OPTN_BUTTON_HEIGHT);
	bSuccess = (*m_pTriPButton).Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, myRect, this, IDC_SUB_TRIANGLEPLUS);
	ASSERT(bSuccess);
	bSuccess = (*m_pTriPButton).LoadBitmaps("TRIPUP", "TRIPDOWN", nullptr, nullptr);
	ASSERT(bSuccess);

	m_pCroPButton = new CBmpButton;
	ASSERT(m_pCroPButton != nullptr);
	myRect.SetRect(CROSSPLUS_X, CROSSPLUS_Y, CROSSPLUS_X + OPTN_BUTTON_WIDTH, CROSSPLUS_Y + OPTN_BUTTON_HEIGHT);
	bSuccess = (*m_pCroPButton).Create(nullptr, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, myRect, this, IDC_SUB_CROSSPLUS);
	ASSERT(bSuccess);
	bSuccess = (*m_pCroPButton).LoadBitmaps("CROPUP", "CROPDOWN", nullptr, nullptr);
	ASSERT(bSuccess);

	if ((pRandomButton = new CCheckButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*pRandomButton).SetPalette(pSubOptionsPalette);                        // set the palette to use
		(*pRandomButton).SetControl(IDC_RANDOM, this);              // tie to the dialog control
	}
	((CWnd *)this)->CheckDlgButton(IDC_RANDOM, m_bRandom);       // Set the Auto option box

	if (!m_bRandom) {                                                // If not randomly selecting board
		switch (chNewBoard) {
		case CROSS:
			(*m_pCroButton).SendMessage(BM_SETSTATE, true, 0L);
			break;

		case CROSS_PLUS:
			(*m_pCroPButton).SendMessage(BM_SETSTATE, true, 0L);
			break;

		case TRIANGLE:
			(*m_pTriButton).SendMessage(BM_SETSTATE, true, 0L);
			break;

		default:                                        //case TRIANGLE_PLUS:
			(*m_pTriPButton).SendMessage(BM_SETSTATE, true, 0L);
			break;
		} // end switch
	}

	return true;  // return true  unless you set the focus to a control
}


void COptnDlg::OnDestroy() {
	bool    bUpdateNeeded;

	if (m_pDlgBackground != nullptr) {
		delete m_pDlgBackground;
		m_pDlgBackground = nullptr;

		bUpdateNeeded = (*m_pDlgParentWnd).GetUpdateRect(nullptr, false);
		if (bUpdateNeeded)
			(*m_pDlgParentWnd).ValidateRect(nullptr);
	}

	if (pRandomButton != nullptr) {                        // release the button
		delete pRandomButton;
		pRandomButton = nullptr;
	}

	CDialog::OnDestroy();
}


bool COptnDlg::OnEraseBkgnd(CDC *pDC) {
	return true;
}


void COptnDlg::OnClickedOkay() {
	COptnDlg::OnOK();
}


void COptnDlg::OnOK() {
	if ((chNewBoard != BoardSelected) || (m_bRandom != bRandomBoard)) {
		BoardSelected = chNewBoard;
		bRandomBoard = m_bRandom;
		COptions::ClearDialogImage();
		EndDialog(IDOK);
	} else  {
		COptions::ClearDialogImage();
		EndDialog(0);
	}
}


void COptnDlg::OnClickedCancel() {
	COptnDlg::OnCancel();
}


void COptnDlg::OnCancel() {
	COptions::ClearDialogImage();
	EndDialog(0);
}


void COptnDlg::OnPaint() {

	COptions::OnPaint();

}

void COptnDlg::OnRandom() {
	m_bRandom = !m_bRandom;
	((CWnd *)this)->CheckDlgButton(IDC_RANDOM, m_bRandom);

	(*m_pCroPButton).SendMessage(BM_SETSTATE, false, 0L);
	(*m_pTriButton).SendMessage(BM_SETSTATE, false, 0L);
	(*m_pTriPButton).SendMessage(BM_SETSTATE, false, 0L);
	(*m_pCroButton).SendMessage(BM_SETSTATE, false, 0L);
}

void COptnDlg::OnClickedCross() {
	if (m_bRandom) {
		m_bRandom = false;
		((CWnd *)this)->CheckDlgButton(IDC_RANDOM, m_bRandom);
	}
	chNewBoard = CROSS;

	(*m_pCroPButton).SendMessage(BM_SETSTATE, false, 0L);
	(*m_pTriButton).SendMessage(BM_SETSTATE, false, 0L);
	(*m_pTriPButton).SendMessage(BM_SETSTATE, false, 0L);

	(*m_pCroButton).SendMessage(BM_SETSTATE, true, 0L);
}


void COptnDlg::OnClickedCrossPlus() {
	if (m_bRandom) {
		m_bRandom = false;
		((CWnd *)this)->CheckDlgButton(IDC_RANDOM, m_bRandom);
	}
	chNewBoard = CROSS_PLUS;

	(*m_pCroButton).SendMessage(BM_SETSTATE, false, 0L);
	(*m_pTriButton).SendMessage(BM_SETSTATE, false, 0L);
	(*m_pTriPButton).SendMessage(BM_SETSTATE, false, 0L);

	(*m_pCroPButton).SendMessage(BM_SETSTATE, true, 0L);
}


void COptnDlg::OnClickedTriangle() {
	if (m_bRandom) {
		m_bRandom = false;
		((CWnd *)this)->CheckDlgButton(IDC_RANDOM, m_bRandom);
	}
	chNewBoard = TRIANGLE;

	(*m_pCroButton).SendMessage(BM_SETSTATE, false, 0L);
	(*m_pCroPButton).SendMessage(BM_SETSTATE, false, 0L);
	(*m_pTriPButton).SendMessage(BM_SETSTATE, false, 0L);

	(*m_pTriButton).SendMessage(BM_SETSTATE, true, 0L);
}


void COptnDlg::OnClickedTrianglePlus() {
	if (m_bRandom) {
		m_bRandom = false;
		((CWnd *)this)->CheckDlgButton(IDC_RANDOM, m_bRandom);
	}
	chNewBoard = TRIANGLE_PLUS;

	(*m_pCroButton).SendMessage(BM_SETSTATE, false, 0L);
	(*m_pCroPButton).SendMessage(BM_SETSTATE, false, 0L);
	(*m_pTriButton).SendMessage(BM_SETSTATE, false, 0L);

	(*m_pTriPButton).SendMessage(BM_SETSTATE, true, 0L);
}

} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel
