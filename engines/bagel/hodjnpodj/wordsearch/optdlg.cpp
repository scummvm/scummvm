/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * optdlg.cpp                                   private options dialog for poker
 *
 * HISTORY
 *
 *      1.0 5/9/94 GTB          
 *
 * MODULE DESCRIPTION:
 *
 *      The Set User Amount dialog specific to my game. It contains 
 *      a scroll bar for setting the amount, a "Set" button, and "Cancel" button
 *
 * LOCALS:
 *
 *              n/a
 *
 * GLOBALS:
 *
 *                      n/a      
 *
 * RELEVANT DOCUMENTATION:
 *
 *      n/a
 *
 * FILES USED:
 *
 *                      n/a
 ****************************************************************/
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "resource.h"
#include "dialogs.h"

#define GAMETIMEMAX			12
#define GAMETIMEMIN			1 
#define	GAMESTEP			1
#define	GAMEJUMP			1

static CPalette		*pPackRatOptPalette;
static CRect        rDisplayTime( 35, 90, 200, 110 );	//( 43, 90, 174, 110 );
CText				*ptxtTime;

CColorButton	*pOKButton = NULL;
CColorButton	*pCancelButton = NULL;
CCheckButton	*pWordForwardButton = NULL;

/*****************************************************************
 *
 * CWSOptDlg
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Constructor sends the input to the CBmpDialog constructor and 
 *      the intializes the private members 
 *   
 * FORMAL PARAMETERS:
 *
 *      Those needed to contruct a CBmpDialog dialog: pParent,pPalette, nID 
 *
 * IMPLICIT INPUT PARAMETERS:
 *  
 *      n/a
 *   
 * IMPLICIT OUTPUT PARAMETERS:
 *   
 *      private member m_nCurrenLEVEL
 *  globals     rectDisplayAmount and pSeLEVELPalette           
 *   
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
CWSOptDlg::CWSOptDlg(CWnd *pParent, CPalette *pPalette, UINT nID) 
:  CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP" )
{
CDC     *pDC = GetDC();            

        pPackRatOptPalette = pPalette;                  

		ptxtTime = new CText( pDC, pPackRatOptPalette, &rDisplayTime, JUSTIFY_LEFT );

        ReleaseDC( pDC );
}

/*****************************************************************
 *
 * OnCommand
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Process the "Set" and "Cancel" buttons 
 *
 * This function is called when a WM_COMMAND message is issued,
 * typically in order to process control related activities.
 *   
 * FORMAL PARAMETERS:
 *
 *      wParam          identifier for the button to be processed
 *      lParam          type of message to be processed
 *
 * IMPLICIT INPUT PARAMETERS:
 *  
 *      n/a
 *   
 * IMPLICIT OUTPUT PARAMETERS:
 *   
 *      n/a
 *   
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
BOOL CWSOptDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{

int nReturn = 0;
// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog
if (HIWORD(lParam) == BN_CLICKED) {
	switch (wParam) {

		case IDC_MINI_BWTOGGLE:
			if ( m_bWordForwardOnly ) {
				m_bWordForwardOnly = FALSE;
				pWordForwardButton->SetCheck( FALSE );
			}
			else {
				m_bWordForwardOnly = TRUE;
				pWordForwardButton->SetCheck( TRUE );
			}
			break;

		case IDC_MINI_OK:
			  // calculate current level from 

		    switch ( m_nGameTime ) {
		    	case 1:
		    		nReturn = 15;
		    		break;
		    	case 2:
		    		nReturn = 30;
		    		break;
		    	case 3:
		    		nReturn = 45;
		    		break;
		    	case 4:
		    		nReturn = 60;
		    		break;
		    	case 5:
		    		nReturn = 75;
		    		break;
		    	case 6:
		    		nReturn = 90;
		    		break;
		    	case 7:
		    		nReturn = 120;
		    		break;
		    	case 8:
		    		nReturn = 180;
		    		break;
		    	case 9:
		    		nReturn = 240;
		    		break;
		    	case 10:
		    		nReturn = 300;
		    		break;
		    	case 11:
		    		nReturn = 600;
		    		break;
		    	default:
		    		nReturn = 0;
		    		break;
		    }
            
			if ( m_bShowWord ) {
				nReturn += 1000;
			}
			else {
				nReturn += 2000;
            }

			if ( m_bWordForwardOnly ) {
				nReturn += 10000;
			}
			else {
				nReturn += 20000;
            }

			ClearDialogImage();
			EndDialog( nReturn );
			return 1; 
			
		case IDC_MINI_CANCEL:
			ClearDialogImage();
			EndDialog( -1 );
			return 1;
		}
	}
	return(CDialog::OnCommand(wParam, lParam));
}
                                        
                                        
void CWSOptDlg::OnCancel(void)
{
    ClearDialogImage();
    EndDialog( -1 );
    return;
}

void CWSOptDlg::OnOK(void)
{
    ClearDialogImage();
    EndDialog( -1 );
    return;
}

void CWSOptDlg::ClearDialogImage(void)
{

	if ( pOKButton != NULL ) {
		delete pOKButton;
		pOKButton = NULL;
    }
	if ( pCancelButton != NULL ) {
		delete pCancelButton;
		pCancelButton = NULL;
    }
	if ( pWordForwardButton != NULL ) {
		delete pWordForwardButton;
		pWordForwardButton = NULL;
    }

    ValidateRect(NULL);

    return;
}
                                        
/*****************************************************************
 *
 * SetInitialOptions
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This sets the privates to the inputted values 
 *
 * FORMAL PARAMETERS:
 *
 *      lCurrenLEVEL = the current amount the user has  
 *
 * IMPLICIT INPUT PARAMETERS:
 *  
 *      m_nCurrenLEVEL = (int)min( AMOUNTMAX, lCurrenLEVEL)
 *   
 * IMPLICIT OUTPUT PARAMETERS:
 *   
 *      n/a
 *   
 * RETURN VALUE:
 *
 *
 ****************************************************************/
void CWSOptDlg::SetInitialOptions( int nTime, BOOL bShowWords, BOOL bWFO )
{        
	switch ( nTime ) {
		case 15:
			m_nGameTime = 1;
			break;
		case 30:
			m_nGameTime = 2;
			break;
		case 45:
			m_nGameTime = 3;
			break;
		case 60:
			m_nGameTime = 4;
			break;
		case 75:
			m_nGameTime = 5;
			break;
		case 90:
			m_nGameTime = 6;
			break;
		case 120:
			m_nGameTime = 7;
			break;
		case 180:
			m_nGameTime = 8;
			break;
		case 240:
			m_nGameTime = 9;
			break;
		case 300:
			m_nGameTime = 10;
			break;
		case 600:
			m_nGameTime = 11;
			break;
		default:
			m_nGameTime = 12;
			break;
	}
	m_bShowWord = bShowWords;  
	m_bWordForwardOnly = bWFO;
	
	return;
}  


BOOL CWSOptDlg::OnInitDialog()
{
BOOL	bSuccess;

	CBmpDialog::OnInitDialog();

    pOKButton = new CColorButton();
    ASSERT( pOKButton != NULL );
    pOKButton->SetPalette( pPackRatOptPalette );
    bSuccess = pOKButton->SetControl( IDC_MINI_OK, this );
    ASSERT( bSuccess );
    
    pCancelButton = new CColorButton();
    ASSERT( pCancelButton != NULL );
    pCancelButton->SetPalette( pPackRatOptPalette );
    bSuccess = pCancelButton->SetControl( IDC_MINI_CANCEL, this );
    ASSERT( bSuccess );
    
    pWordForwardButton = new CCheckButton();
    ASSERT( pWordForwardButton != NULL );
    pWordForwardButton->SetPalette( pPackRatOptPalette );
    bSuccess = pWordForwardButton->SetControl( IDC_MINI_BWTOGGLE, this );
    ASSERT( bSuccess );
    
	return(TRUE);
}

/*****************************************************************
 *
 * OnPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the screen whenever needed; e.g. when uncovered by an
 * overlapping window, when maximized from an icon, and when it the
 * window is initially created.
 *
 * This uses the CBmpDialog Paint as its base, and displays the current
 * amount chosen from the scrollbar
 *
 * This routine is called whenever Windows sends a WM_PAINT message.
 * Note that creating a CPaintDC automatically does a BeginPaint and
 * an EndPaint call is done when it is destroyed at the end of this
 * function.  CPaintDC's constructor needs the window (this).
 *   
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *  
 *      n/a
 *   
 * IMPLICIT OUTPUT PARAMETERS:
 *   
 *      n/a
 *   
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
void CWSOptDlg::OnPaint(void)
{
CDC             *pDC;
CString         strTime = "Time Limit:  ";
//int             nOldBkMode;
                             
CScrollBar		*pTime;

char	cDisplay[64];
            
//      call CBmpDialog onpaint, to paint the background
    CBmpDialog::OnPaint();

	if ( m_bWordForwardOnly ) {
		pWordForwardButton->SetCheck( 1 );
	}
	else {
		pWordForwardButton->SetCheck( 0 );
    }

    pDC = GetDC();

//      now paint in my text with a transparent background
//    nOldBkMode = pDC->SetBkMode( TRANSPARENT );
//    pDC->TextOut( 43, 73, strTime );
//    pDC->SetBkMode( nOldBkMode );

    switch ( m_nGameTime ) {
    	case 1:
    		Common::sprintf_s( cDisplay, "%s15 Seconds", strTime );
    		break;
    	case 2:
    		Common::sprintf_s( cDisplay, "%s30 Seconds", strTime );
    		break;
    	case 3:
    		Common::sprintf_s( cDisplay, "%s45 Seconds", strTime );
    		break;
    	case 4:
    		Common::sprintf_s( cDisplay, "%s60 Seconds", strTime );
    		break;
    	case 5:
    		Common::sprintf_s( cDisplay, "%s75 Seconds", strTime );
    		break;
    	case 6:
    		Common::sprintf_s( cDisplay, "%s90 Seconds", strTime );
    		break;
    	case 7:
    		Common::sprintf_s( cDisplay, "%s2 Minutes", strTime );
    		break;
    	case 8:
    		Common::sprintf_s( cDisplay, "%s3 Minutes", strTime );
    		break;
    	case 9:
    		Common::sprintf_s( cDisplay, "%s4 Minutes", strTime );
    		break;
    	case 10:
    		Common::sprintf_s( cDisplay, "%s5 Minutes", strTime );
    		break;
    	case 11:
    		Common::sprintf_s( cDisplay, "%s10 Minutes", strTime );
    		break;
    	default:
    		Common::sprintf_s( cDisplay, "%sUnlimited", strTime );
    		break;
    }

    ptxtTime->DisplayString( pDC, cDisplay, 14, FW_BOLD, RGB(0, 0, 0) );

    ReleaseDC(pDC);  

	pTime = new CScrollBar;

	pTime = (CScrollBar *)GetDlgItem( IDC_GAMETIME );
    
    pTime->SetScrollRange( GAMETIMEMIN, GAMETIMEMAX, TRUE );
    if ( m_nGameTime == 0 )
    	pTime->SetScrollPos( GAMETIMEMAX, TRUE );
    else
    	pTime->SetScrollPos( m_nGameTime, TRUE );
    

    return;
}

/*****************************************************************
 *
 * OnHScroll
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      This is the functionality of the scroll bar
 *   
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *  
 *      n/a
 *   
 * IMPLICIT OUTPUT PARAMETERS:
 *   
 *      n/a
 *   
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
void CWSOptDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
int     oldAmnt = 0;
int     newAmnt = 0;
CDC     *pDC;
int 	nJunk = 0;
char	cDisplay[64];
CString	strTime = "Time Limit:  ";

	nJunk = pScrollBar->GetDlgCtrlID();	

// first set the range of the scoll bar
	if ( pScrollBar->GetDlgCtrlID() == IDC_GAMETIME)
		pScrollBar->SetScrollRange( GAMETIMEMIN, GAMETIMEMAX,TRUE);

// get the scroll bar's current position, i.e. the current amount set
	oldAmnt = pScrollBar->GetScrollPos();
    newAmnt = oldAmnt;

// switching off of what the scroll bar wants to do, act accordingly.
    switch (nSBCode) {
        case SB_LINELEFT:
        case SB_PAGELEFT:
        case SB_LEFT:
            newAmnt --;
            if ( newAmnt < GAMETIMEMIN )
            	newAmnt = GAMETIMEMIN;
            break;
        case SB_LINERIGHT:
        case SB_PAGERIGHT:
        case SB_RIGHT:
            newAmnt ++;
            if ( newAmnt > GAMETIMEMAX )
            	newAmnt = GAMETIMEMAX;
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            newAmnt = nPos;
            break;
    }
	
// set the scroll bar to the new position
    pScrollBar->SetScrollPos( newAmnt, TRUE );        
	
// set the current amount to the new amount just set
// and paint this new amount onto the screen
			
    pDC = GetDC();
	if ( pScrollBar->GetDlgCtrlID() == IDC_GAMETIME) {
		m_nGameTime = newAmnt;
	    switch ( m_nGameTime ) {
	    	case 1:
	    		Common::sprintf_s( cDisplay, "%s15 Seconds", strTime );
	    		break;
	    	case 2:
	    		Common::sprintf_s( cDisplay, "%s30 Seconds", strTime );
	    		break;
	    	case 3:
	    		Common::sprintf_s( cDisplay, "%s45 Seconds", strTime );
	    		break;
	    	case 4:
	    		Common::sprintf_s( cDisplay, "%s60 Seconds", strTime );
	    		break;
	    	case 5:
	    		Common::sprintf_s( cDisplay, "%s75 Seconds", strTime );
	    		break;
	    	case 6:
	    		Common::sprintf_s( cDisplay, "%s90 Seconds", strTime );
	    		break;
	    	case 7:
	    		Common::sprintf_s( cDisplay, "%s2 Minutes", strTime );
	    		break;
	    	case 8:
	    		Common::sprintf_s( cDisplay, "%s3 Minutes", strTime );
	    		break;
	    	case 9:
	    		Common::sprintf_s( cDisplay, "%s4 Minutes", strTime );
	    		break;
	    	case 10:
	    		Common::sprintf_s( cDisplay, "%s5 Minutes", strTime );
	    		break;
	    	case 11:
	    		Common::sprintf_s( cDisplay, "%s10 Minutes", strTime );
	    		break;
	    	default:
	    		Common::sprintf_s( cDisplay, "%sUnlimited", strTime );
	    		break;
	    }

		ptxtTime->DisplayString( pDC, cDisplay, 14, FW_BOLD, RGB(0, 0, 0) );
	}
    ReleaseDC( pDC );
    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CWSOptDlg::OnDestroy()
{
//  send a message to the calling app to tell it the user has quit the game
	if ( pOKButton != NULL )
		delete pOKButton;

	if ( pCancelButton != NULL )
		delete pCancelButton;

	if ( pWordForwardButton != NULL )
		delete pWordForwardButton;

	if ( ptxtTime != NULL )
		delete ptxtTime;

	CBmpDialog::OnDestroy();
} 

// Message Map
BEGIN_MESSAGE_MAP(CWSOptDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
