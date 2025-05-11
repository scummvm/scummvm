// optndlg.cpp : implementation file
//

#include "stdafx.h"
#include "globals.h"
#include "resource.h"
#include "options.h"
#include "optndlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "game2.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CPalette 	*pSubOptionsPalette;
CBmpButton	*m_pTriButton = NULL;
CBmpButton	*m_pTriPButton = NULL;
CBmpButton	*m_pCroButton = NULL;
CBmpButton	*m_pCroPButton = NULL;

static  CCheckButton *pRandomButton = NULL;					// Framed check box

extern 	char 		BoardSelected;
extern	BOOL		bRandomBoard;
static	char		chNewBoard;
static	BOOL		m_bRandom;
	
/////////////////////////////////////////////////////////////////////////////
// COptnDlg dialog


COptnDlg::COptnDlg(CWnd* pParent, CPalette* pPalette)
		:COptions(pParent, pPalette, IDD_SUBOPTIONS)
//	: CDialog(COptnDlg::IDD, pParent)
{                                                         
	//{{AFX_DATA_INIT(COptnDlg)
	pSubOptionsPalette = pPalette;
	//}}AFX_DATA_INIT
}

void COptnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptnDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptnDlg, CDialog)
	//{{AFX_MSG_MAP(COptnDlg)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_SUB_OKAY, OnClickedOkay)
	ON_BN_CLICKED(IDC_SUB_CANCEL, OnClickedCancel)
	ON_BN_CLICKED(IDC_SUB_CROSS, OnClickedCross)
	ON_BN_CLICKED(IDC_SUB_CROSSPLUS, OnClickedCrossPlus)
	ON_BN_CLICKED(IDC_SUB_TRIANGLE, OnClickedTriangle)
	ON_BN_CLICKED(IDC_SUB_TRIANGLEPLUS, OnClickedTrianglePlus)
	ON_BN_CLICKED(IDC_RANDOM, OnRandom)
    ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptnDlg message handlers

int COptnDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (COptions::OnCreate(lpCreateStruct) == -1)
		return -1;
	
 	return 0;
}

BOOL COptnDlg::OnInitDialog()
{
	BOOL	bSuccess;
	CRect	myRect;

	COptions::OnInitDialog();

	chNewBoard = BoardSelected;
	m_bRandom = bRandomBoard;

	m_pTriButton = new CBmpButton;
	ASSERT( m_pTriButton != NULL);
	myRect.SetRect( TRIANGLE_X, TRIANGLE_Y, TRIANGLE_X + OPTN_BUTTON_WIDTH, TRIANGLE_Y + OPTN_BUTTON_HEIGHT); 
	bSuccess = ( *m_pTriButton).Create( NULL, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, myRect, this, IDC_SUB_TRIANGLE );
	ASSERT( bSuccess );
	bSuccess = (*m_pTriButton).LoadBitmaps( "TRIUP", "TRIDOWN", NULL, NULL );
	ASSERT( bSuccess );

	m_pCroButton = new CBmpButton;
	ASSERT( m_pCroButton != NULL);
	myRect.SetRect( CROSS_X, CROSS_Y, CROSS_X + OPTN_BUTTON_WIDTH, CROSS_Y + OPTN_BUTTON_HEIGHT); 
	bSuccess = ( *m_pCroButton).Create( NULL, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, myRect, this, IDC_SUB_CROSS );
	ASSERT( bSuccess );
	bSuccess = (*m_pCroButton).LoadBitmaps( "CROUP", "CRODOWN", NULL, NULL );
	ASSERT( bSuccess );

	m_pTriPButton = new CBmpButton;
	ASSERT( m_pTriPButton != NULL);
	myRect.SetRect( TRIANGLEPLUS_X, TRIANGLEPLUS_Y, 
					TRIANGLEPLUS_X + OPTN_BUTTON_WIDTH, TRIANGLEPLUS_Y + OPTN_BUTTON_HEIGHT); 
	bSuccess = ( *m_pTriPButton).Create( NULL, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, myRect, this, IDC_SUB_TRIANGLEPLUS );
	ASSERT( bSuccess );
	bSuccess = (*m_pTriPButton).LoadBitmaps( "TRIPUP", "TRIPDOWN", NULL, NULL );
	ASSERT( bSuccess );

	m_pCroPButton = new CBmpButton;
	ASSERT( m_pCroPButton != NULL);
	myRect.SetRect( CROSSPLUS_X, CROSSPLUS_Y, CROSSPLUS_X + OPTN_BUTTON_WIDTH, CROSSPLUS_Y + OPTN_BUTTON_HEIGHT); 
	bSuccess = ( *m_pCroPButton).Create( NULL, BS_OWNERDRAW | WS_CHILD | WS_VISIBLE, myRect, this, IDC_SUB_CROSSPLUS );
	ASSERT( bSuccess );
	bSuccess = (*m_pCroPButton).LoadBitmaps( "CROPUP", "CROPDOWN", NULL, NULL );
	ASSERT( bSuccess );

	if ((pRandomButton = new CCheckButton) != NULL) {					// build a color QUIT button to let us exit
		(*pRandomButton).SetPalette(pSubOptionsPalette);						// set the palette to use
		(*pRandomButton).SetControl(IDC_RANDOM,this);				// tie to the dialog control
	}
	((CWnd *)this)->CheckDlgButton( IDC_RANDOM, m_bRandom );		// Set the Auto option box

	if ( !m_bRandom ) {												// If not randomly selecting board	
		switch ( chNewBoard ) {
			case CROSS:
				(*m_pCroButton).SendMessage( BM_SETSTATE, TRUE, 0L );
				break;
				
			case CROSS_PLUS:
				(*m_pCroPButton).SendMessage( BM_SETSTATE, TRUE, 0L );
				break;
				
			case TRIANGLE:
				(*m_pTriButton).SendMessage( BM_SETSTATE, TRUE, 0L );
				break;
				
			default:										//case TRIANGLE_PLUS:
				(*m_pTriPButton).SendMessage( BM_SETSTATE, TRUE, 0L );
				break;
		} // end switch
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void COptnDlg::OnDestroy(void)
{
BOOL	bUpdateNeeded;
		
	if (m_pDlgBackground != NULL) {
		delete m_pDlgBackground;
		m_pDlgBackground = NULL;

		bUpdateNeeded = (*m_pDlgParentWnd).GetUpdateRect(NULL,FALSE);
		if (bUpdateNeeded)
	    	(*m_pDlgParentWnd).ValidateRect(NULL);
	}

	if (pRandomButton != NULL) {                     	// release the button
		delete pRandomButton;
		pRandomButton = NULL;
	}

    CDialog::OnDestroy();
}


BOOL COptnDlg::OnEraseBkgnd(CDC *pDC)
{
return(TRUE);
}

   
void COptnDlg::OnClickedOkay()
{
	COptnDlg::OnOK();
}

   
void COptnDlg::OnOK()
{
	if ((chNewBoard != BoardSelected) || (m_bRandom != bRandomBoard)) {
		BoardSelected = chNewBoard;
		bRandomBoard = m_bRandom;
		COptions::ClearDialogImage();
		EndDialog( IDOK );
	}
	else  {
		COptions::ClearDialogImage();
		EndDialog( 0 );
	}
}

   
void COptnDlg::OnClickedCancel()
{
	COptnDlg::OnCancel();
}


void COptnDlg::OnCancel()
{
	COptions::ClearDialogImage();
	EndDialog( 0 );
}


void COptnDlg::OnPaint()
{

COptions::OnPaint();

}

void COptnDlg::OnRandom()
{
	m_bRandom = !m_bRandom;
	((CWnd *)this)->CheckDlgButton( IDC_RANDOM, m_bRandom );

	(*m_pCroPButton).SendMessage( BM_SETSTATE, FALSE, 0L );
	(*m_pTriButton).SendMessage( BM_SETSTATE, FALSE, 0L );
	(*m_pTriPButton).SendMessage( BM_SETSTATE, FALSE, 0L );
	(*m_pCroButton).SendMessage( BM_SETSTATE, FALSE, 0L );
}
   
void COptnDlg::OnClickedCross()
{
	if (m_bRandom) {
		m_bRandom = FALSE;
		((CWnd *)this)->CheckDlgButton( IDC_RANDOM, m_bRandom );
	}
	chNewBoard = CROSS;

	(*m_pCroPButton).SendMessage( BM_SETSTATE, FALSE, 0L );
	(*m_pTriButton).SendMessage( BM_SETSTATE, FALSE, 0L );
	(*m_pTriPButton).SendMessage( BM_SETSTATE, FALSE, 0L );

	(*m_pCroButton).SendMessage( BM_SETSTATE, TRUE, 0L );
}


void COptnDlg::OnClickedCrossPlus()
{
	if (m_bRandom) {
		m_bRandom = FALSE;
		((CWnd *)this)->CheckDlgButton( IDC_RANDOM, m_bRandom );
	}
	chNewBoard = CROSS_PLUS;

	(*m_pCroButton).SendMessage( BM_SETSTATE, FALSE, 0L );
	(*m_pTriButton).SendMessage( BM_SETSTATE, FALSE, 0L );
	(*m_pTriPButton).SendMessage( BM_SETSTATE, FALSE, 0L );

	(*m_pCroPButton).SendMessage( BM_SETSTATE, TRUE, 0L );
}


void COptnDlg::OnClickedTriangle()
{
	if (m_bRandom) {
		m_bRandom = FALSE;
		((CWnd *)this)->CheckDlgButton( IDC_RANDOM, m_bRandom );
	}
	chNewBoard = TRIANGLE;

	(*m_pCroButton).SendMessage( BM_SETSTATE, FALSE, 0L );
	(*m_pCroPButton).SendMessage( BM_SETSTATE, FALSE, 0L );
	(*m_pTriPButton).SendMessage( BM_SETSTATE, FALSE, 0L );

	(*m_pTriButton).SendMessage( BM_SETSTATE, TRUE, 0L );
}


void COptnDlg::OnClickedTrianglePlus()
{
	if (m_bRandom) {
		m_bRandom = FALSE;
		((CWnd *)this)->CheckDlgButton( IDC_RANDOM, m_bRandom );
	}
	chNewBoard = TRIANGLE_PLUS;

	(*m_pCroButton).SendMessage( BM_SETSTATE, FALSE, 0L );
	(*m_pCroPButton).SendMessage( BM_SETSTATE, FALSE, 0L );
	(*m_pTriButton).SendMessage( BM_SETSTATE, FALSE, 0L );

	(*m_pTriPButton).SendMessage( BM_SETSTATE, TRUE, 0L );
}
