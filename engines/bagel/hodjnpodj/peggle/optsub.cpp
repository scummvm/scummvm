// optsub.cpp -- subroutines for options.cpp
// Written by John J. Xenakis for Boffo Games Inc., 1994

// Note to programmers:
//	Please do not modify this file.  Instead, modify the
//	file options.cpp.


#include "stdafx.h"
#include "gamedll.h"
#include "globals.h"
#include "options.h"
#include "bitmaps.h"
#include "button.h"
#include "resource.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


extern	LPGAMESTRUCT 	pGameInfo;

extern	CBmpButton	*m_pTriButton;
extern	CBmpButton	*m_pTriPButton;
extern	CBmpButton	*m_pCroButton;
extern	CBmpButton	*m_pCroPButton;

CPalette	*pOptionsPalette = NULL;

static	CColorButton *pRulesButton = NULL;
static	CColorButton *pNewgameButton = NULL;
static	CColorButton *pOptionsButton = NULL;
static	CColorButton *pAudioButton = NULL;
static	CColorButton *pReturnButton = NULL;
static	CColorButton *pQuitButton = NULL;

static	CColorButton *pOkayButton = NULL;
static	CColorButton *pCancelButton = NULL;


/////////////////////////////////////////////////////////////////////////////
// COptions dialog


COptions::COptions(CWnd* pParent,CPalette *pPalette, int iDlgId)
        : CDialog(iDlgId, pParent)
{
	pOptionsPalette = pPalette;
   	m_iDlgId = iDlgId ;		// dialog id
	if (pParent == NULL)
		m_pDlgParentWnd = ((CWnd *)this)->GetParent();
	else
		m_pDlgParentWnd = pParent ;
	
	//{{AFX_DATA_INIT(COptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COptions::DoDataExchange(CDataExchange* pDX)
{
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
	ON_BN_CLICKED(IDC_OPTIONS_RULES, OnClickedRules)
	ON_BN_CLICKED(IDC_OPTIONS_NEWGAME, OnClickedNewgame)
	ON_BN_CLICKED(IDC_OPTIONS_OPTIONS, OnClickedOptions)
	ON_BN_CLICKED(IDC_OPTIONS_AUDIO, OnClickedAudio)
	ON_BN_CLICKED(IDC_OPTIONS_RETURN, OnClickedReturn)
	ON_BN_CLICKED(IDC_OPTIONS_QUIT, OnClickedQuit)
    ON_WM_DESTROY()
//	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptions message handlers

void COptions::OnOK(void)
{
	ClearDialogImage();
	CDialog::EndDialog(IDC_OPTIONS_QUIT);
}


BOOL COptions::OnInitDialog(void)
{
	BOOL	bSuccess;
	CWnd	*pButton;
	CDC		*pDC;
	CDialog::OnInitDialog();						// do basic dialog initialization
	CRect	cDlgRect, cWindRect ;

	int iDlgWidth, iDlgHeight ;	// size of dialog box
	CWnd * pMainWnd ;		// main window for application

	pMainWnd = m_pDlgParentWnd ;		// get dialog box parent
	while (pMainWnd->GetParent()) {	// while it has a parent
		pMainWnd = pMainWnd->GetParent();}	// use it

	// get the parent window's rectangular bounds
	(*pMainWnd).GetWindowRect(&cWindRect);  // get pos/size of parent
//	((CWnd *)this)->GetWindowRect(&cDlgRect);  // get pos/size of dialog
	cDlgRect.SetRect(0,0,222,210);

	// center the dialog box on the screen
	iDlgWidth = cDlgRect.right - cDlgRect.left ;	// dlg box width
	cDlgRect.left = (cWindRect.right + cWindRect.left - iDlgWidth)/2 ;
	cDlgRect.right = cDlgRect.left + iDlgWidth ;

	iDlgHeight = cDlgRect.bottom - cDlgRect.top ;	// dlg box height
	cDlgRect.top = (cWindRect.bottom + cWindRect.top - iDlgHeight)/2 ;
	cDlgRect.bottom = cDlgRect.top + iDlgHeight ;

	MoveWindow(&cDlgRect, FALSE) ;	// center window, don't repaint
        
    pDC = GetDC();
    ASSERT(pDC != NULL);
	m_pDlgBackground = FetchScreenBitmap(pDC,pOptionsPalette,0,0,iDlgWidth,iDlgHeight);
    ASSERT(m_pDlgBackground != NULL);
    ReleaseDC(pDC);

	if (m_iDlgId == IDD_OPTIONS_DIALOG) {
		pRulesButton = new CColorButton();
		ASSERT(pRulesButton != NULL);
		(*pRulesButton).SetPalette(pOptionsPalette);
		bSuccess = (*pRulesButton).SetControl(IDC_OPTIONS_RULES,this);

		pNewgameButton = new CColorButton();
		ASSERT(pNewgameButton != NULL);
		(*pNewgameButton).SetPalette(pOptionsPalette);
		bSuccess = (*pNewgameButton).SetControl(IDC_OPTIONS_NEWGAME,this);

		pOptionsButton = new CColorButton();
		ASSERT(pOptionsButton != NULL);
		(*pOptionsButton).SetPalette(pOptionsPalette);
		bSuccess = (*pOptionsButton).SetControl(IDC_OPTIONS_OPTIONS,this);

		pAudioButton = new CColorButton();
		ASSERT(pAudioButton != NULL);
		(*pAudioButton).SetPalette(pOptionsPalette);
		bSuccess = (*pAudioButton).SetControl(IDC_OPTIONS_AUDIO,this);

		pReturnButton = new CColorButton();
		ASSERT(pReturnButton != NULL);
		(*pReturnButton).SetPalette(pOptionsPalette);
		bSuccess = (*pReturnButton).SetControl(IDC_OPTIONS_RETURN,this);

		pQuitButton = new CColorButton();
		ASSERT(pQuitButton != NULL);
		(*pQuitButton).SetPalette(pOptionsPalette);
		bSuccess = (*pQuitButton).SetControl(IDC_OPTIONS_QUIT,this);
	}
else
	if (m_iDlgId == IDD_SUBOPTIONS) {
		pOkayButton = new CColorButton();
		ASSERT(pOkayButton != NULL);
		(*pOkayButton).SetPalette(pOptionsPalette);
		bSuccess = (*pOkayButton).SetControl(IDC_SUB_OKAY,this);

		pCancelButton = new CColorButton();
		ASSERT(pCancelButton != NULL);
		(*pCancelButton).SetPalette(pOptionsPalette);
		bSuccess = (*pCancelButton).SetControl(IDC_SUB_CANCEL,this);
	}

#if DEMO_FLAG
	if (m_iDlgId == IDD_OPTIONS_DIALOG) {
		pButton = GetDlgItem(IDC_OPTIONS_OPTIONS);		// get the window for the options button
		ASSERT(pButton != NULL);                        // ... and verify we have it
		(*pButton).EnableWindow(FALSE);
	}
#else
	if ((*pGameInfo).bPlayingMetagame) {
		if (m_iDlgId == IDD_OPTIONS_DIALOG) {
			pButton = GetDlgItem(IDC_OPTIONS_OPTIONS);		// get the window for the options button
			ASSERT(pButton != NULL);                        // ... and verify we have it
			(*pButton).EnableWindow(FALSE);
			pButton = GetDlgItem(IDC_OPTIONS_NEWGAME);		// get the window for the options button
			ASSERT(pButton != NULL);                        // ... and verify we have it
			(*pButton).EnableWindow(FALSE);
		}
	}
#endif
		
	return(TRUE);  	// return TRUE  unless focused on a control
}


void COptions::OnDestroy(void)
{
BOOL	bUpdateNeeded;

	if (m_iDlgId == IDD_OPTIONS_DIALOG) {
		if (pRulesButton != NULL)
			delete pRulesButton;
		if (pNewgameButton != NULL)
			delete pNewgameButton;
		if (pOptionsButton != NULL)
			delete pOptionsButton;
		if (pAudioButton != NULL)
			delete pAudioButton;
		if (pReturnButton != NULL)
			delete pReturnButton;
		if (pQuitButton != NULL)
			delete pQuitButton;
		}
	else
	if (m_iDlgId == IDD_SUBOPTIONS) {
		if (pOkayButton != NULL)
			delete pOkayButton;
		if (pCancelButton != NULL)
			delete pCancelButton;
		if (m_pTriButton != NULL)
			delete m_pTriButton;
		if (m_pTriPButton != NULL)
			delete m_pTriPButton;
		if (m_pCroButton != NULL)
			delete m_pCroButton;
		if (m_pCroPButton != NULL)
			delete m_pCroPButton;
	}
		
	if (m_pDlgBackground != NULL) {
		delete m_pDlgBackground;
		m_pDlgBackground = NULL;

		bUpdateNeeded = (*m_pDlgParentWnd).GetUpdateRect(NULL,FALSE);
		if (bUpdateNeeded)
	    	(*m_pDlgParentWnd).ValidateRect(NULL);
		}

    CDialog::OnDestroy();
}

/*
void COptions::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
if (!bMinimized)
	switch(nState) {
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
				InvalidateRect(NULL,FALSE);
			break;
		}
}
*/

BOOL COptions::OnEraseBkgnd(CDC *pDC)
{
return(TRUE);
}


void COptions::OnPaint(void)
{
BOOL		bSuccess;
CPalette	*pPalOld = NULL;

	InvalidateRect(NULL,FALSE);
	
CPaintDC	dc(this); 								// device context for painting

	RefreshBackground();

	if (pOptionsPalette != NULL) {
		pPalOld = dc.SelectPalette(pOptionsPalette,FALSE);
		(void) dc.RealizePalette();
	}

	bSuccess = PaintMaskedDIB(&dc, pOptionsPalette, ".\\art\\oscroll.bmp", 0, 0);
    ASSERT(bSuccess);
	
	if (pOptionsPalette != NULL)
		(void) dc.SelectPalette(pPalOld,FALSE);
}


void COptions::ClearDialogImage(void)
{
	if (m_pDlgBackground != NULL) {
		if (m_iDlgId == IDD_OPTIONS_DIALOG) {
			if (pRulesButton != NULL)
				delete pRulesButton;
			if (pNewgameButton != NULL)
				delete pNewgameButton;
			if (pOptionsButton != NULL)
				delete pOptionsButton;
			if (pAudioButton != NULL)
				delete pAudioButton;
			if (pReturnButton != NULL)
				delete pReturnButton;
			if (pQuitButton != NULL)
				delete pQuitButton;
			pRulesButton = NULL;
			pNewgameButton = NULL;
			pOptionsButton = NULL;
			pReturnButton = NULL;
			pQuitButton = NULL;
			pAudioButton = NULL;
			}
		else
		if (m_iDlgId == IDD_SUBOPTIONS) {
			if (pOkayButton != NULL)
				delete pOkayButton;
			if (pCancelButton != NULL)
				delete pCancelButton;
			if (m_pTriButton != NULL)
				delete m_pTriButton;
			if (m_pTriPButton != NULL)
				delete m_pTriPButton;
			if (m_pCroButton != NULL)
				delete m_pCroButton;
			if (m_pCroPButton != NULL)
				delete m_pCroPButton;
			pOkayButton = NULL;
			pCancelButton = NULL;
			m_pTriButton = NULL;
			m_pTriPButton = NULL;
			m_pCroButton = NULL;
			m_pCroPButton = NULL;
		}
		InvalidateRect(NULL,FALSE);
		RefreshBackground();
		ValidateRect(NULL);
	}
}


void COptions::RefreshBackground(void)
{
BOOL		bSuccess;
CDC			*pDC;
CPalette	*pPalOld = NULL;

	if (m_pDlgBackground == NULL)
		return;

	pDC = GetDC();
	
	if (pOptionsPalette != NULL) {
		pPalOld = (*pDC).SelectPalette(pOptionsPalette,FALSE);
		(void) (*pDC).RealizePalette();
	}

	bSuccess = PaintBitmap(pDC,pOptionsPalette,m_pDlgBackground,0,0);
    ASSERT(bSuccess);
	
	if (pOptionsPalette != NULL)
		(void) (*pDC).SelectPalette(pPalOld,FALSE);
	
	ReleaseDC(pDC);
}


void COptions::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
}


void COptions::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}


int COptions::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

