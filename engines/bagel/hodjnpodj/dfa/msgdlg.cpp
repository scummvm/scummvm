/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * amtdlg.cpp                                   private options dialog for poker
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
#include "stdafx.h"
#include "resource.h"
#include "dialogs.h"
#include "text.h"

static CPalette         *pPackRatOptPalette;

CColorButton    *pMsgOKButton = NULL;

/*****************************************************************
 *
 * CMsgDlg
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
CMsgDlg::CMsgDlg(CWnd *pParent, CPalette *pPalette, UINT nID) 
		:  CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP" )
{
    pPackRatOptPalette = pPalette;
    m_nWhichMsg = 1;
	m_lScore = 0L;
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
BOOL CMsgDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog
	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
			case IDOK:
				ClearDialogImage();
				EndDialog( 0 );
				return 1;
		}
	}
	return(CDialog::OnCommand(wParam, lParam));
}

void CMsgDlg::OnCancel(void)
{
    ClearDialogImage();
    EndDialog( 0 );
    return;
}

void CMsgDlg::OnOK(void)
{
    ClearDialogImage();
    EndDialog( 0 );
    return;
}

void CMsgDlg::ClearDialogImage(void)
{

    if ( pMsgOKButton != NULL ) {
        delete pMsgOKButton;
        pMsgOKButton = NULL;
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
void CMsgDlg::SetInitialOptions( int nWhichMsg, long lScore )
{        
    m_nWhichMsg = nWhichMsg;
	m_lScore = lScore;
	return;
}  

BOOL CMsgDlg::OnInitDialog()
{
BOOL    bSuccess;

    CBmpDialog::OnInitDialog();

    pMsgOKButton = new CColorButton();
    ASSERT( pMsgOKButton != NULL );
    pMsgOKButton->SetPalette( pPackRatOptPalette );
    bSuccess = pMsgOKButton->SetControl( IDOK, this );
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
void CMsgDlg::OnPaint(void)
{
//      call CBmpDialog onpaint, to paint the background
    CBmpDialog::OnPaint();

CDC		*pDC = GetDC();
// 	CText(CDC *pDC, CPalette *pPalette, CRect *pRect, int nJustify = JUSTIFY_CENTER);
// 	BOOL DisplayString(CDC *pDC, const char* pszText, const int nSize, const int nWeight, const COLORREF crColor = CTEXT_COLOR);

CRect	rRect1( 16, 40, 200, 50 ); 
CRect	rRect2( 16, 60, 200, 80 ); 
CRect	rRect3( 16, 84, 200, 104 ); 
CText	txtLine1( pDC, pPackRatOptPalette, &rRect1 );
CText	txtLine2( pDC, pPackRatOptPalette, &rRect2 );
CText	txtLine3( pDC, pPackRatOptPalette, &rRect3 );
char	cDisplay[30];

	switch ( m_nWhichMsg ) {
	    case 1:
    		Common::sprintf_s( cDisplay, "Score:  %li", m_lScore );
//	    	txtLine1.DisplayString( pDC, "", 21, FW_BOLD ); 
	    	txtLine2.DisplayString( pDC, "Game over.", 21, FW_BOLD ); 
	    	txtLine3.DisplayString( pDC, cDisplay, 21, FW_BOLD ); 
			break;
	}
	ReleaseDC( pDC );
    return;
}

void CMsgDlg::OnDestroy()
{
//  send a message to the calling app to tell it the user has quit the game
    if ( pMsgOKButton != NULL ) {
        delete pMsgOKButton;
        pMsgOKButton = NULL;
    }

	CBmpDialog::OnDestroy();
} 

// Message Map
BEGIN_MESSAGE_MAP(CMsgDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
