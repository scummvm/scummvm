/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * uwondlg.cpp					private options dialog for poker
 *
 * HISTORY
 *
 *	1.0 5/9/94 GTB		
 *
 * MODULE DESCRIPTION:
 *
 *	The Set User Amount dialog specific to my game. It contains 
 *	a scroll bar for setting the amount, a "Set" button, and "Cancel" button
 *
 * LOCALS:
 *
 *   		n/a
 *
 * GLOBALS:
 *
 *			n/a      
 *
 * RELEVANT DOCUMENTATION:
 *
 *      n/a
 *
 * FILES USED:
 *
 * 			n/a
 ****************************************************************/
#include "stdafx.h"
#include "resource.h"
#include "dialogs.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

static CPalette *pUWonPalette;                                         

CColorButton    *pOKButton = NULL;

/*****************************************************************
 *
 * COptionsDlg
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * 	Constructor sends the input to the COptions constructor and 
 *	the intializes the private members 
 *   
 * FORMAL PARAMETERS:
 *
 *	Those needed to contruct a COptions dialog: pParent,pPalette, nID 
 *
 * IMPLICIT INPUT PARAMETERS:
 *  
 *	n/a
 *   
 * IMPLICIT OUTPUT PARAMETERS:
 *   
 *	private member m_nCurrentAmount
 *  globals	rectDisplayAmount and pSetAmountPalette		  
 *   
 * RETURN VALUE:
 *
 *	n/a
 *
 ****************************************************************/
CUserWonDlg::CUserWonDlg(CWnd *pParent, CPalette *pPalette, UINT nID)
			:  CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP" )
{
	pUWonPalette = pPalette;
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
BOOL CUserWonDlg::OnCommand(WPARAM wParam, LPARAM lParam)
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

void CUserWonDlg::OnOK(void)
{
    ClearDialogImage();
    EndDialog( 0 );
    return;
}

void CUserWonDlg::ClearDialogImage(void)
{

    if ( pOKButton != NULL ) {
        delete pOKButton;
        pOKButton = NULL;
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
 *	This sets the privates to the inputted values 
 *
 * FORMAL PARAMETERS:
 *
 *	lCurrentAmount = the current amount the user has  
 *
 * IMPLICIT INPUT PARAMETERS:
 *  
 *	m_nCurrentAmount = (int)min( AMOUNTMAX, lCurrentAmount)
 *   
 * IMPLICIT OUTPUT PARAMETERS:
 *   
 *	n/a
 *   
 * RETURN VALUE:
 *
 *
 ****************************************************************/
void CUserWonDlg::SetInitialOptions( int nWhichPhrase )
{        
  m_nWinPhrase = nWhichPhrase;
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
 * This uses the COptions Paint as its base, and displays the current
 * amount chosen from the scrollbar
 *
 * This routine is called whenever Windows sends a WM_PAINT message.
 * Note that creating a CPaintDC automatically does a BeginPaint and
 * an EndPaint call is done when it is destroyed at the end of this
 * function.  CPaintDC's constructor needs the window (this).
 *   
 * FORMAL PARAMETERS:
 *
 *	n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *  
 *	n/a
 *   
 * IMPLICIT OUTPUT PARAMETERS:
 *   
 *	n/a
 *   
 * RETURN VALUE:
 *
 *	n/a
 *
 ****************************************************************/
void CUserWonDlg::OnPaint(void)
{
CDC 			*pDC;
int		    nOldBkMode;

// 	call COptions onpaint, to paint the background
    CBmpDialog::OnPaint();

    pDC = GetDC();

// 	now paint in my text with a transparent background
    nOldBkMode = pDC->SetBkMode( TRANSPARENT );
    
    switch ( m_nWinPhrase ) {
      case 1:
		    pDC->TextOut( 62, 60, "You got a Pair!");
      	break;
      case 2:
		    pDC->TextOut( 63, 60, "You got a Pair");
		    pDC->TextOut( 57, 80, "of Jacks or higher!");
      	break;
      case 3:
		    pDC->TextOut( 52, 60, "You got Two Pairs!");
      	break;
      case 4:
		    pDC->TextOut( 61, 60, "You got a Three");
		    pDC->TextOut( 83, 80, "of a kind!");
      	break;
      case 5:
		    pDC->TextOut( 52, 60, "You got a Straight!");
      	break;
      case 6:
		    pDC->TextOut( 62, 60, "You got a Flush!");
      	break;
      case 7:
		    pDC->TextOut( 42, 60, "You got a Full House!");
      	break;
      case 8:
		    pDC->TextOut( 62, 60, "You got a Four");
		    pDC->TextOut( 83, 80, "of a kind!");
      	break;
      case 9:
		    pDC->TextOut( 33, 60, "You got a Straight Flush!");
      	break;
      case 10:
		    pDC->TextOut( 38, 60, "You got a Royal Flush!");
				break;
			default:
		    pDC->TextOut( 61, 60, "You got nothin'!");
				break;
    }

    pDC->SetBkMode( nOldBkMode );
    ReleaseDC(pDC);  
    return;
}

BOOL CUserWonDlg::OnInitDialog(void)
{                                    
	BOOL	bSuccess;
	
	CBmpDialog::OnInitDialog();
	
    pOKButton = new CColorButton();
    ASSERT( pOKButton != NULL );
    pOKButton->SetPalette( pUWonPalette );
    bSuccess = pOKButton->SetControl( IDOK, this );
    ASSERT( bSuccess );

	return(TRUE);
}
    
void CUserWonDlg::OnDestroy()
{
//  send a message to the calling app to tell it the user has quit the game
    if ( pOKButton != NULL ) {
        delete pOKButton;
        pOKButton = NULL;
    }

	CBmpDialog::OnDestroy();
} 


// Message Map
BEGIN_MESSAGE_MAP(CUserWonDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
