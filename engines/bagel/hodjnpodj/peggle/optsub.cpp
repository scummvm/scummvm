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

#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/peggle/options.h"
#include "bagel/hodjnpodj/peggle/resource.h"

namespace Bagel {
namespace HodjNPodj {
namespace Peggle {

extern  LPGAMESTRUCT    pGameInfo;

extern  CBmpButton  *m_pTriButton;
extern  CBmpButton  *m_pTriPButton;
extern  CBmpButton  *m_pCroButton;
extern  CBmpButton  *m_pCroPButton;

CPalette    *pOptionsPalette = nullptr;

static  CColorButton *pRulesButton = nullptr;
static  CColorButton *pNewgameButton = nullptr;
static  CColorButton *pOptionsButton = nullptr;
static  CColorButton *pAudioButton = nullptr;
static  CColorButton *pReturnButton = nullptr;
static  CColorButton *pQuitButton = nullptr;

static  CColorButton *pOkayButton = nullptr;
static  CColorButton *pCancelButton = nullptr;


/////////////////////////////////////////////////////////////////////////////
// COptions dialog


COptions::COptions(CWnd* pParent, CPalette *pPalette, int iDlgId)
	: CDialog(iDlgId, pParent) {
	pOptionsPalette = pPalette;
	m_iDlgId = iDlgId ;     // dialog id
	if (pParent == nullptr)
		m_pDlgParentWnd = ((CWnd *)this)->GetParent();
	else
		m_pDlgParentWnd = pParent ;

	//{{AFX_DATA_INIT(COptions)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COptions::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptions)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptions, CDialog)
	//{{AFX_MSG_MAP(COptions)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_NCMOUSEMOVE()
	ON_BN_CLICKED(IDC_OPTIONS_RULES, COptions::OnClickedRules)
	ON_BN_CLICKED(IDC_OPTIONS_NEWGAME, COptions::OnClickedNewgame)
	ON_BN_CLICKED(IDC_OPTIONS_OPTIONS, COptions::OnClickedOptions)
	ON_BN_CLICKED(IDC_OPTIONS_AUDIO, COptions::OnClickedAudio)
	ON_BN_CLICKED(IDC_OPTIONS_RETURN, COptions::OnClickedReturn)
	ON_BN_CLICKED(IDC_OPTIONS_QUIT, COptions::OnClickedQuit)
	ON_WM_DESTROY()
//	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptions message handlers

void COptions::OnOK() {
	ClearDialogImage();
	CDialog::EndDialog(IDC_OPTIONS_QUIT);
}


bool COptions::OnInitDialog() {
	bool    bSuccess;
	CWnd    *pButton;
	CDC     *pDC;
	CDialog::OnInitDialog();                        // do basic dialog initialization
	CRect   cDlgRect, cWindRect ;

	int iDlgWidth, iDlgHeight ; // size of dialog box
	CWnd * pMainWnd ;       // main window for application

	pMainWnd = m_pDlgParentWnd ;        // get dialog box parent
	while (pMainWnd->GetParent()) { // while it has a parent
		pMainWnd = pMainWnd->GetParent();
	}  // use it

	// get the parent window's rectangular bounds
	(*pMainWnd).GetWindowRect(&cWindRect);  // get pos/size of parent
//	((CWnd *)this)->GetWindowRect(&cDlgRect);  // get pos/size of dialog
	cDlgRect.SetRect(0, 0, 222, 210);

	// center the dialog box on the screen
	iDlgWidth = cDlgRect.right - cDlgRect.left ;    // dlg box width
	cDlgRect.left = (cWindRect.right + cWindRect.left - iDlgWidth) / 2 ;
	cDlgRect.right = cDlgRect.left + iDlgWidth ;

	iDlgHeight = cDlgRect.bottom - cDlgRect.top ;   // dlg box height
	cDlgRect.top = (cWindRect.bottom + cWindRect.top - iDlgHeight) / 2 ;
	cDlgRect.bottom = cDlgRect.top + iDlgHeight ;

	MoveWindow(&cDlgRect, false) ;  // center window, don't repaint

	pDC = GetDC();
	ASSERT(pDC != nullptr);
	m_pDlgBackground = FetchScreenBitmap(pDC, pOptionsPalette, 0, 0, iDlgWidth, iDlgHeight);
	ASSERT(m_pDlgBackground != nullptr);
	ReleaseDC(pDC);

	if (m_iDlgId == IDD_OPTIONS_DIALOG) {
		pRulesButton = new CColorButton();
		ASSERT(pRulesButton != nullptr);
		(*pRulesButton).SetPalette(pOptionsPalette);
		bSuccess = (*pRulesButton).SetControl(IDC_OPTIONS_RULES, this);

		pNewgameButton = new CColorButton();
		ASSERT(pNewgameButton != nullptr);
		(*pNewgameButton).SetPalette(pOptionsPalette);
		bSuccess = (*pNewgameButton).SetControl(IDC_OPTIONS_NEWGAME, this);

		pOptionsButton = new CColorButton();
		ASSERT(pOptionsButton != nullptr);
		(*pOptionsButton).SetPalette(pOptionsPalette);
		bSuccess = (*pOptionsButton).SetControl(IDC_OPTIONS_OPTIONS, this);

		pAudioButton = new CColorButton();
		ASSERT(pAudioButton != nullptr);
		(*pAudioButton).SetPalette(pOptionsPalette);
		bSuccess = (*pAudioButton).SetControl(IDC_OPTIONS_AUDIO, this);

		pReturnButton = new CColorButton();
		ASSERT(pReturnButton != nullptr);
		(*pReturnButton).SetPalette(pOptionsPalette);
		bSuccess = (*pReturnButton).SetControl(IDC_OPTIONS_RETURN, this);

		pQuitButton = new CColorButton();
		ASSERT(pQuitButton != nullptr);
		(*pQuitButton).SetPalette(pOptionsPalette);
		bSuccess = (*pQuitButton).SetControl(IDC_OPTIONS_QUIT, this);
	} else if (m_iDlgId == IDD_SUBOPTIONS) {
		pOkayButton = new CColorButton();
		ASSERT(pOkayButton != nullptr);
		(*pOkayButton).SetPalette(pOptionsPalette);
		bSuccess = (*pOkayButton).SetControl(IDC_SUB_OKAY, this);

		pCancelButton = new CColorButton();
		ASSERT(pCancelButton != nullptr);
		(*pCancelButton).SetPalette(pOptionsPalette);
		bSuccess = (*pCancelButton).SetControl(IDC_SUB_CANCEL, this);
	}

	#if DEMO_FLAG
	if (m_iDlgId == IDD_OPTIONS_DIALOG) {
		pButton = GetDlgItem(IDC_OPTIONS_OPTIONS);      // get the window for the options button
		ASSERT(pButton != nullptr);                        // ... and verify we have it
		(*pButton).EnableWindow(false);
	}
	#else
	if ((*pGameInfo).bPlayingMetagame) {
		if (m_iDlgId == IDD_OPTIONS_DIALOG) {
			pButton = GetDlgItem(IDC_OPTIONS_OPTIONS);      // get the window for the options button
			ASSERT(pButton != nullptr);                        // ... and verify we have it
			(*pButton).EnableWindow(false);
			pButton = GetDlgItem(IDC_OPTIONS_NEWGAME);      // get the window for the options button
			ASSERT(pButton != nullptr);                        // ... and verify we have it
			(*pButton).EnableWindow(false);
		}
	}
	#endif

	(void)bSuccess;  // suppress unused variable warning

	return true;  // return true  unless focused on a control
}


void COptions::OnDestroy() {
	bool    bUpdateNeeded;

	if (m_iDlgId == IDD_OPTIONS_DIALOG) {
		if (pRulesButton != nullptr)
			delete pRulesButton;
		if (pNewgameButton != nullptr)
			delete pNewgameButton;
		if (pOptionsButton != nullptr)
			delete pOptionsButton;
		if (pAudioButton != nullptr)
			delete pAudioButton;
		if (pReturnButton != nullptr)
			delete pReturnButton;
		if (pQuitButton != nullptr)
			delete pQuitButton;
	} else if (m_iDlgId == IDD_SUBOPTIONS) {
		if (pOkayButton != nullptr)
			delete pOkayButton;
		if (pCancelButton != nullptr)
			delete pCancelButton;
		if (m_pTriButton != nullptr)
			delete m_pTriButton;
		if (m_pTriPButton != nullptr)
			delete m_pTriPButton;
		if (m_pCroButton != nullptr)
			delete m_pCroButton;
		if (m_pCroPButton != nullptr)
			delete m_pCroPButton;
	}

	if (m_pDlgBackground != nullptr) {
		delete m_pDlgBackground;
		m_pDlgBackground = nullptr;

		bUpdateNeeded = (*m_pDlgParentWnd).GetUpdateRect(nullptr, false);
		if (bUpdateNeeded)
			(*m_pDlgParentWnd).ValidateRect(nullptr);
	}

	CDialog::OnDestroy();
}

/*
void COptions::OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized)
{
if (!bMinimized)
    switch(nState) {
        case WA_ACTIVE:
        case WA_CLICKACTIVE:
                InvalidateRect(nullptr,false);
            break;
        }
}
*/

bool COptions::OnEraseBkgnd(CDC *pDC) {
	return true;
}


void COptions::OnPaint() {
	bool        bSuccess;
	CPalette    *pPalOld = nullptr;

	InvalidateRect(nullptr, false);

	CPaintDC    dc(this);                               // device context for painting

	RefreshBackground();

	if (pOptionsPalette != nullptr) {
		pPalOld = dc.SelectPalette(pOptionsPalette, false);
		dc.RealizePalette();
	}

	bSuccess = PaintMaskedDIB(&dc, pOptionsPalette, ".\\art\\oscroll.bmp", 0, 0);
	ASSERT(bSuccess);

	if (pOptionsPalette != nullptr)
		dc.SelectPalette(pPalOld, false);
}


void COptions::ClearDialogImage() {
	if (m_pDlgBackground != nullptr) {
		if (m_iDlgId == IDD_OPTIONS_DIALOG) {
			if (pRulesButton != nullptr)
				delete pRulesButton;
			if (pNewgameButton != nullptr)
				delete pNewgameButton;
			if (pOptionsButton != nullptr)
				delete pOptionsButton;
			if (pAudioButton != nullptr)
				delete pAudioButton;
			if (pReturnButton != nullptr)
				delete pReturnButton;
			if (pQuitButton != nullptr)
				delete pQuitButton;
			pRulesButton = nullptr;
			pNewgameButton = nullptr;
			pOptionsButton = nullptr;
			pReturnButton = nullptr;
			pQuitButton = nullptr;
			pAudioButton = nullptr;
		} else if (m_iDlgId == IDD_SUBOPTIONS) {
			if (pOkayButton != nullptr)
				delete pOkayButton;
			if (pCancelButton != nullptr)
				delete pCancelButton;
			if (m_pTriButton != nullptr)
				delete m_pTriButton;
			if (m_pTriPButton != nullptr)
				delete m_pTriPButton;
			if (m_pCroButton != nullptr)
				delete m_pCroButton;
			if (m_pCroPButton != nullptr)
				delete m_pCroPButton;
			pOkayButton = nullptr;
			pCancelButton = nullptr;
			m_pTriButton = nullptr;
			m_pTriPButton = nullptr;
			m_pCroButton = nullptr;
			m_pCroPButton = nullptr;
		}
		InvalidateRect(nullptr, false);
		RefreshBackground();
		ValidateRect(nullptr);
	}
}


void COptions::RefreshBackground() {
	bool        bSuccess;
	CDC         *pDC;
	CPalette    *pPalOld = nullptr;

	if (m_pDlgBackground == nullptr)
		return;

	pDC = GetDC();

	if (pOptionsPalette != nullptr) {
		pPalOld = (*pDC).SelectPalette(pOptionsPalette, false);
		(*pDC).RealizePalette();
	}

	bSuccess = PaintBitmap(pDC, pOptionsPalette, m_pDlgBackground, 0, 0);
	ASSERT(bSuccess);

	if (pOptionsPalette != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	ReleaseDC(pDC);
}


void COptions::OnShowWindow(bool bShow, unsigned int nStatus) {
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here

}


void COptions::OnSize(unsigned int nType, int cx, int cy) {
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

}


int COptions::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel
