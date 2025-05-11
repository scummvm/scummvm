// optndlg.cpp : implementation file
//

#include "bagel/hodjnpodj/hnplibs/stdafx.h" 
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "globals.h"
#include "resource.h"
#include "mod.h" 
#include "optndlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static	CPalette *pSubOptionsPalette;
static	CColorButton *pOKButton = NULL;						// OKAY button on scroll
static	CColorButton *pCancelButton = NULL;					// Cancel button on scroll

CText	*m_pTimeLeft = NULL;
CText	*m_pDiffTitleText = NULL;
CText	*m_pDifficultyText = NULL;
CText	*m_pTimerText = NULL;

int m_nTimeScale[10] =	{30, 45, 60, 75, 90, 120, 180, 240, 300, 600};

/////////////////////////////////////////////////////////////////////////////
// COptnDlg dialog


COptnDlg::COptnDlg(CWnd* pParent, CPalette* pPalette)
		:CBmpDialog(pParent, pPalette, IDD_SUBOPTIONS, ".\\ART\\SSCROLL.BMP")
{                                                         
	//{{AFX_DATA_INIT(COptnDlg)
	m_nTime = MIN_TIME;
	m_nDifficulty = MIN_DIFFICULTY;
	m_pTimeLeft	= NULL;
	m_pTimerText = NULL;
	m_pDifficultyText = NULL;
	m_pDiffTitleText = NULL;

	pSubOptionsPalette = pPalette;
	//}}AFX_DATA_INIT
}

COptnDlg::~COptnDlg()
{
	if( m_pTimeLeft != NULL)
		delete m_pTimeLeft;
	if( m_pTimerText != NULL)
		delete m_pTimerText;
	if( m_pDiffTitleText != NULL)
		delete m_pDiffTitleText;
	if( m_pDifficultyText != NULL)
		delete m_pDifficultyText;

	CBmpDialog::OnDestroy();
}

void COptnDlg::DoDataExchange(CDataExchange* pDX)
{
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

int COptnDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBmpDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

 	return 0;
}

BOOL COptnDlg::OnInitDialog()
{
	CBmpDialog::OnInitDialog();

	CDC		*pDC;
	CRect	statRect;
	int		i;
	
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

	statRect.SetRect( LEFT_SIDE, 25, LEFT_SIDE + 70, 40 );
	if ((m_pTimeLeft = new CText()) != NULL) {
		(*m_pTimeLeft).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}
	
	statRect.SetRect( LEFT_SIDE, 45, LEFT_SIDE + 80, 70 );
	if ((m_pDiffTitleText = new CText()) != NULL) {
		(*m_pDiffTitleText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}
	
	statRect.SetRect( LEFT_SIDE, 65, LEFT_SIDE + 170, 80 );
	if ((m_pDifficultyText = new CText()) != NULL) {
		(*m_pDifficultyText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}
	
	m_ScrollDifficulty.SetScrollRange( MIN_DIFFICULTY, MAX_DIFFICULTY, 0 );     //...last element is Max - 1
	m_ScrollDifficulty.SetScrollPos( m_nDifficulty, TRUE );
	
	statRect.SetRect( LEFT_SIDE, 115, LEFT_SIDE + 100, 130 );
	if ((m_pTimerText = new CText()) != NULL) {
		(*m_pTimerText).SetupText(pDC, pSubOptionsPalette, &statRect, JUSTIFY_LEFT);
	}
	
	m_ScrollTime.SetScrollRange( TIMER_MIN, TIMER_MAX - 1, 0 );
	if (m_nTime == 0) m_nTime = m_nTimeScale[TIMER_MAX - 1];
	for (i = 0; i < TIMER_MAX; i++) {
		if (m_nTimeScale[i] == m_nTime)
			m_ScrollTime.SetScrollPos( i, TRUE );
	}
		
	ReleaseDC( pDC );
	
	if ((pOKButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pOKButton).SetPalette(pSubOptionsPalette);						// set the palette to use
		(*pOKButton).SetControl(IDOK,this);				// tie to the dialog control
	}
	
	if ((pCancelButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pCancelButton).SetPalette(pSubOptionsPalette);						// set the palette to use
		(*pCancelButton).SetControl(IDCANCEL,this);				// tie to the dialog control
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}


BOOL COptnDlg::OnEraseBkgnd(CDC *pDC)
{
	return(TRUE);
}


void COptnDlg::OnDestroy(void)
{
    CBmpDialog::OnDestroy();
}


void COptnDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{       
	int 	pMin, 
			pMax;
	int 	OldPos = pScrollBar->GetScrollPos();
	int 	NewPos = OldPos;
	
	pScrollBar->GetScrollRange( &pMin, &pMax );
		
	switch (nSBCode) {
		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			NewPos++;							// Increment by one
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
		  
	if ( NewPos < pMin ) NewPos = pMin;
	if ( NewPos > pMax ) NewPos = pMax;
	
	if ( NewPos != OldPos ) {							//To prevent "flicker"  
		(*pScrollBar).SetScrollPos( NewPos, TRUE );		//...only update when
	}                                                   //...changed
	
	UpdateScrollbars();
	
	CDialog::OnHScroll( nSBCode, NewPos, pScrollBar );
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
 *      CScrollbar	pScrollTime, pScrollColumns, pScrollRows
 *   
 *  IMPLICIT OUTPUT PARAMETERS:
 *   
 *      int	m_nTime, m_nNumParts, m_nColumns, m_nRows
 *   
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void COptnDlg::UpdateScrollbars()
{
	int OldValue;
	CDC 	*pDC;
	char	msg[64];
	int		m_nMins, m_nSecs;
		
    pDC = GetDC();

	OldValue = m_nTime;
	m_nTime = m_nTimeScale[m_ScrollTime.GetScrollPos()];
	if ( OldValue != m_nTime ){
		if ( m_nTime == m_nTimeScale[TIMER_MAX - 1] ) 
			Common::sprintf_s( msg, "Time Limit: None" );
		else {  
			m_nMins = m_nTime / 60;
	    	m_nSecs = m_nTime % 60;

			Common::sprintf_s( msg, "Time Limit: %02d:%02d", m_nMins, m_nSecs );
		}
        (*m_pTimerText).DisplayString( pDC, msg, 14, TEXT_BOLD, RGB( 0, 0, 0));
	} 

	OldValue = m_nDifficulty;
	m_nDifficulty = m_ScrollDifficulty.GetScrollPos();
	if ( OldValue != m_nDifficulty ) {
		Common::sprintf_s( msg, "%s", mDifficultyTable[m_nDifficulty - 1] );
        (*m_pDifficultyText).DisplayString( pDC, msg, 14, TEXT_BOLD, RGB( 0, 0, 0));
    }
	
	ReleaseDC( pDC );
}


void COptnDlg::OnOK()
{
	if (m_nTime == m_nTimeScale[TIMER_MAX - 1]) m_nTime = 0;
	ClearDialogImage();
	EndDialog( IDOK );
}

void COptnDlg::OnCancel()
{
	ClearDialogImage();
	EndDialog( 0 );
}

void COptnDlg::OnPaint()
{
	CDC 	*pDC;
	char	msg[64];
	int		m_nMins, m_nSecs;
		
	CBmpDialog::OnPaint();
    
    pDC = GetDC();
    
	Common::sprintf_s( msg, "Time: %02d:%02d", nMinutes, nSeconds );
    (*m_pTimeLeft).DisplayString( pDC, msg, 14, TEXT_BOLD, RGB( 0, 0, 0));
	
	Common::sprintf_s( msg, "Level:");
    (*m_pDiffTitleText).DisplayString( pDC, msg, 14, TEXT_BOLD, RGB( 0, 0, 0));

	Common::sprintf_s( msg, "%s", mDifficultyTable[m_nDifficulty - 1] );
    (*m_pDifficultyText).DisplayString( pDC, msg, 14, TEXT_BOLD, RGB( 0, 0, 0));

	if ( m_nTime == m_nTimeScale[TIMER_MAX - 1] ) 
		Common::sprintf_s( msg, "Time Limit: None" );
	else {
		m_nMins = m_nTime / 60;
	   	m_nSecs = m_nTime % 60;
    
		Common::sprintf_s( msg, "Time Limit: %02d:%02d", m_nMins, m_nSecs );
    }
    (*m_pTimerText).DisplayString( pDC, msg, 14, TEXT_BOLD, RGB( 0, 0, 0));
	
	ReleaseDC( pDC );

}

void COptnDlg::ClearDialogImage(void)
{
	if (pOKButton != NULL) {                          // release the button
		delete pOKButton;
		pOKButton = NULL;
	}

	if (pCancelButton != NULL) {                     	// release the button
		delete pCancelButton;
		pCancelButton = NULL;
	}

	ValidateRect(NULL);
}

