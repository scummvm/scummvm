#include "bagel/hodjnpodj/hnplibs/stdafx.h" 
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "resource.h"
#include "dialogs.h"

CPalette	*pPayoffPalette = NULL;
CRect			crectRedraw( 10, 140, 44, 290);

CText			*ptxtPair = NULL; 
CText			*ptxtPairJacks = NULL; 
CText			*ptxtTwoPair = NULL; 
CText			*ptxtThreeOAK = NULL; 
CText			*ptxtStraight = NULL; 
CText			*ptxtFlush = NULL; 
CText			*ptxtFullHouse = NULL; 
CText			*ptxtFourOAK = NULL; 
CText			*ptxtStraightFlush = NULL; 
CText			*ptxtRoyalFlush = NULL; 

CRect			rectPair( 			10, 131, 80, 146);
CRect			rectPairJacks( 		10, 145, 80, 160);
CRect			rectTwoPair( 		10, 159, 80, 174);
CRect			rectThree( 			10, 173, 80, 188);
CRect			rectStraight( 		10, 187, 80, 202);
CRect			rectFlush( 			10, 201, 80, 216);
CRect			rectFullHouse( 		10, 215, 80, 230);
CRect			rectFour( 			10, 229, 80, 244);
CRect			rectStraightFlush( 	10, 243, 80, 258);
CRect			rectRoyalFlush( 	10, 257, 80, 272);

static CColorButton    *pSetPayButton = NULL;
static CColorButton    *pCancelButton = NULL;
static CColorButton    *pOKButton = NULL;

static CRadioButton    *pKlingonButton = NULL;
static CRadioButton    *pVegasButton = NULL;
static CRadioButton    *pPoPorButton = NULL;
static CRadioButton    *pKuwaitiButton = NULL;
static CRadioButton    *pMartianButton = NULL;

CSetPayoffsDlg::CSetPayoffsDlg(CWnd *pParent, CPalette *pPalette, UINT nID, int nOdds, BOOL bJustDisplay ) 
		 :  CBmpDialog(pParent, pPalette, nID, ".\\ART\\MSCROLL.BMP" )
{
	ptxtPair = new CText;
	ptxtPairJacks = new CText;
	ptxtTwoPair = new CText;
	ptxtThreeOAK = new CText;
	ptxtStraight = new CText;
	ptxtFlush = new CText;
	ptxtFullHouse = new CText;
	ptxtFourOAK = new CText;
	ptxtStraightFlush = new CText;
	ptxtRoyalFlush = new CText;
	
	m_bJustDisplay = bJustDisplay;
	m_nSetOfOdds = nOdds;
	
	pPayoffPalette = pPalette;
  
}

void CSetPayoffsDlg::DoDataExchange(CDataExchange *pDX)
{
    CDialog::DoDataExchange(pDX);
}

BOOL CSetPayoffsDlg::OnInitDialog(void)
{
	BOOL	bSuccess = FALSE;
	
	CBmpDialog::OnInitDialog();
	
    pOKButton = new CColorButton();
    ASSERT( pOKButton != NULL );
    pOKButton->SetPalette( pPayoffPalette );
    bSuccess = pOKButton->SetControl( ID_CANCEL_VIEW, this );
    ASSERT( bSuccess );

    pSetPayButton = new CColorButton();
    ASSERT( pSetPayButton != NULL );
    pSetPayButton->SetPalette( pPayoffPalette );
    bSuccess = pSetPayButton->SetControl( ID_SETPAYOFFS, this );
    ASSERT( bSuccess );
	    
    pCancelButton = new CColorButton();
    ASSERT( pCancelButton != NULL );
    pCancelButton->SetPalette( pPayoffPalette );
    bSuccess = pCancelButton->SetControl( ID_CANCEL, this );
    ASSERT( bSuccess );
    
    pKlingonButton = new CRadioButton();
    ASSERT( pKlingonButton != NULL );
    pKlingonButton->SetPalette( pPayoffPalette );
    bSuccess = pKlingonButton->SetControl( ID_KLINGON, this );
    ASSERT( bSuccess );
    
    pVegasButton = new CRadioButton();
    ASSERT( pVegasButton != NULL );
    pVegasButton->SetPalette( pPayoffPalette );
    bSuccess = pVegasButton->SetControl( ID_VEGAS, this );
    ASSERT( bSuccess );
    
    pPoPorButton = new CRadioButton();
    ASSERT( pPoPorButton != NULL );
    pPoPorButton->SetPalette( pPayoffPalette );
    bSuccess = pPoPorButton->SetControl( ID_POPO, this );
    ASSERT( bSuccess );
    
    pKuwaitiButton = new CRadioButton();
    ASSERT( pKuwaitiButton != NULL );
    pKuwaitiButton->SetPalette( pPayoffPalette );
    bSuccess = pKuwaitiButton->SetControl( ID_KUWAITI, this );
    ASSERT( bSuccess );
    
    pMartianButton = new CRadioButton();
    ASSERT( pMartianButton != NULL );
    pMartianButton->SetPalette( pPayoffPalette );
    bSuccess = pMartianButton->SetControl( ID_MARTIAN, this );
    ASSERT( bSuccess );
    
	switch ( m_nSetOfOdds ) {
		case IDC_KLINGON:
			OnKlingon( FALSE );
			break;
		case IDC_VEGAS:
			OnVegas( FALSE );
			break;
		case IDC_MARTIAN:
			OnMartian( FALSE );
			break;
		case IDC_KUWAITI:
			OnKuwaiti( FALSE );
			break;
		case IDC_POPO:
		default:
			OnPopo( FALSE );
			break;
	}

	if ( m_bJustDisplay ) {
		pKlingonButton->EnableWindow( FALSE );
		pVegasButton->EnableWindow( FALSE );
		pPoPorButton->EnableWindow( FALSE );
		pKuwaitiButton->EnableWindow( FALSE );
		pMartianButton->EnableWindow( FALSE );
		pSetPayButton->ShowWindow( SW_HIDE );
		pCancelButton->ShowWindow( SW_HIDE );
		pOKButton->SetFocus();
	}
	else {
		pOKButton->ShowWindow( SW_HIDE );
		pSetPayButton->SetFocus();
	}
	
//	SetDefID( ID_POPO );
	return(TRUE);
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
BOOL CSetPayoffsDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{

// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog

	if (HIWORD(lParam) == BN_CLICKED) {
		switch ( wParam ) {
			case IDC_KLINGON:
				OnKlingon();
				break;
			case IDC_VEGAS:
				OnVegas();
				break;
			case IDC_POPO:
				OnPopo();
				break;
			case IDC_MARTIAN:
				OnMartian();
				break;
			case IDC_KUWAITI:
				OnKuwaiti();
				break;
			case ID_SETPAYOFFS:
			case IDC_SETPAYOFFS:
				OnSetpayoffs();
				break;
			case IDC_CANCEL:
			case ID_CANCEL:
			case ID_CANCEL_VIEW:
				OnCancel();
				break;
			default:
				CBmpDialog::OnCommand(wParam, lParam);
				break;
		}
    }
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
void CSetPayoffsDlg::OnPaint(void)
{
CDC 			*pDC;
int		    nOldBkMode;

// 	call COptions onpaint, to paint the background
    CBmpDialog::OnPaint();

    pDC = GetDC();

// 	now paint in my text with a transparent background
    ptxtPair->SetupText( pDC, pPayoffPalette, &rectPair, JUSTIFY_RIGHT );
    ptxtPairJacks->SetupText( pDC, pPayoffPalette, &rectPairJacks, JUSTIFY_RIGHT );
    ptxtTwoPair->SetupText( pDC, pPayoffPalette, &rectTwoPair, JUSTIFY_RIGHT );
    ptxtThreeOAK->SetupText( pDC, pPayoffPalette, &rectThree, JUSTIFY_RIGHT );
    ptxtStraight->SetupText( pDC, pPayoffPalette, &rectStraight, JUSTIFY_RIGHT );
    ptxtFlush->SetupText( pDC, pPayoffPalette, &rectFlush, JUSTIFY_RIGHT );
    ptxtFullHouse->SetupText( pDC, pPayoffPalette, &rectFullHouse, JUSTIFY_RIGHT );
    ptxtFourOAK->SetupText( pDC, pPayoffPalette, &rectFour, JUSTIFY_RIGHT );
    ptxtStraightFlush->SetupText( pDC, pPayoffPalette, &rectStraightFlush, JUSTIFY_RIGHT );
    ptxtRoyalFlush->SetupText( pDC, pPayoffPalette, &rectRoyalFlush, JUSTIFY_RIGHT );
    
    nOldBkMode = pDC->SetBkMode( TRANSPARENT );
    pDC->TextOut( 65, 24, "Choose a set of payoffs." );
    pDC->TextOut( 85, 130, "Pair" );
    pDC->TextOut( 85, 144, "Pair (Jacks  or higher)" );
    pDC->TextOut( 85, 158, "Two Pairs" );
    pDC->TextOut( 85, 172, "Three of a kind" );
    pDC->TextOut( 85, 186, "Straight" );
    pDC->TextOut( 85, 200, "Flush" );
    pDC->TextOut( 85, 214, "Full House" );
    pDC->TextOut( 85, 228, "Four of a Kind" );
    pDC->TextOut( 85, 242, "Straight Flush" );
    pDC->TextOut( 85, 256, "Royal Flush" ); 
    
	ptxtPair->DisplayString( pDC, m_cPair, 12, FW_BOLD, RGB(0,0,200) );
	ptxtPairJacks->DisplayString( pDC, m_cPairJacksPlus, 12, FW_BOLD, RGB(0,0,200) );
	ptxtTwoPair->DisplayString( pDC, m_cTwoPair, 12, FW_BOLD, RGB(0,0,200) );
	ptxtThreeOAK->DisplayString( pDC, m_cThreeOAK, 12, FW_BOLD, RGB(0,0,200) );
	ptxtStraight->DisplayString( pDC, m_cStriaght, 12, FW_BOLD, RGB(0,0,200) );
	ptxtFlush->DisplayString( pDC, m_cFlush, 12, FW_BOLD, RGB(0,0,200) );
	ptxtFullHouse->DisplayString( pDC, m_cFullHouse, 12, FW_BOLD, RGB(0,0,200) );
	ptxtFourOAK->DisplayString( pDC, m_cFourOAK, 12, FW_BOLD, RGB(0,0,200) );
	ptxtStraightFlush->DisplayString( pDC, m_cStraightFlush, 12, FW_BOLD, RGB(0,0,200) );
	ptxtRoyalFlush->DisplayString( pDC, m_cRoyalFlush, 12, FW_BOLD, RGB(0,0,200) ); 
		
    pDC->SetBkMode( nOldBkMode );
    ReleaseDC(pDC);  
    return;
}

//////////////////////////////////////////////////////////////////
void CSetPayoffsDlg::OnSetpayoffs()
{
//	RefreshBackground();
	ClearDialogImage();
	EndDialog( m_nSetOfOdds );
	return;
}

void CSetPayoffsDlg::OnCancel()
{
//	RefreshBackground();
	ClearDialogImage();
	EndDialog( 0 );
	return;
}

void CSetPayoffsDlg::OnKlingon( BOOL bDisplay )
{
CDC 			*pDC;
	pDC = GetDC();
	
	pKlingonButton->SetCheck( 1 );
	lstrcpy( (LPSTR)m_cPair, "0");
	lstrcpy( (LPSTR)m_cPairJacksPlus, "0");
	lstrcpy( (LPSTR)m_cTwoPair, "1");
	lstrcpy( (LPSTR)m_cThreeOAK, "2");
	lstrcpy( (LPSTR)m_cStriaght, "3");
	lstrcpy( (LPSTR)m_cFlush, "4");
	lstrcpy( (LPSTR)m_cFullHouse, "5");
	lstrcpy( (LPSTR)m_cFourOAK, "10");
	lstrcpy( (LPSTR)m_cStraightFlush, "20");
	lstrcpy( (LPSTR)m_cRoyalFlush, "50");
	m_nSetOfOdds = IDC_KLINGON; 
	if ( bDisplay ) {
		ptxtPair->DisplayString( pDC, m_cPair, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtPairJacks->DisplayString( pDC, m_cPairJacksPlus, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtTwoPair->DisplayString( pDC, m_cTwoPair, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtThreeOAK->DisplayString( pDC, m_cThreeOAK, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtStraight->DisplayString( pDC, m_cStriaght, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFlush->DisplayString( pDC, m_cFlush, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFullHouse->DisplayString( pDC, m_cFullHouse, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFourOAK->DisplayString( pDC, m_cFourOAK, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtStraightFlush->DisplayString( pDC, m_cStraightFlush, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtRoyalFlush->DisplayString( pDC, m_cRoyalFlush, 12, FW_BOLD, RGB( 0, 0, 200) ); 
	}
	ReleaseDC( pDC );
	return;
}

void CSetPayoffsDlg::OnKuwaiti( BOOL bDisplay )
{
CDC 			*pDC;
	pDC = GetDC();

	pKuwaitiButton->SetCheck( 1 );
	lstrcpy( (LPSTR)m_cPair, "2");
	lstrcpy( (LPSTR)m_cPairJacksPlus, "3");
	lstrcpy( (LPSTR)m_cTwoPair, "5");
	lstrcpy( (LPSTR)m_cThreeOAK, "10");
	lstrcpy( (LPSTR)m_cStriaght, "15");
	lstrcpy( (LPSTR)m_cFlush, "20");
	lstrcpy( (LPSTR)m_cFullHouse, "50");
	lstrcpy( (LPSTR)m_cFourOAK, "100");
	lstrcpy( (LPSTR)m_cStraightFlush, "250");
	lstrcpy( (LPSTR)m_cRoyalFlush, "1000");
	m_nSetOfOdds = IDC_KUWAITI;
	if ( bDisplay ) {
		ptxtPair->DisplayString( pDC, m_cPair, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtPairJacks->DisplayString( pDC, m_cPairJacksPlus, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtTwoPair->DisplayString( pDC, m_cTwoPair, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtThreeOAK->DisplayString( pDC, m_cThreeOAK, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtStraight->DisplayString( pDC, m_cStriaght, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFlush->DisplayString( pDC, m_cFlush, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFullHouse->DisplayString( pDC, m_cFullHouse, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFourOAK->DisplayString( pDC, m_cFourOAK, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtStraightFlush->DisplayString( pDC, m_cStraightFlush, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtRoyalFlush->DisplayString( pDC, m_cRoyalFlush, 12, FW_BOLD, RGB( 0, 0, 200) ); 
	}
	ReleaseDC( pDC );
	return;
}

void CSetPayoffsDlg::OnMartian( BOOL bDisplay )
{
CDC 			*pDC;
	pDC = GetDC();

	pMartianButton->SetCheck( 1 );
	lstrcpy( (LPSTR)m_cPair, "2");
	lstrcpy( (LPSTR)m_cPairJacksPlus, "2");
	lstrcpy( (LPSTR)m_cTwoPair, "13");
	lstrcpy( (LPSTR)m_cThreeOAK, "3");
	lstrcpy( (LPSTR)m_cStriaght, "89");
	lstrcpy( (LPSTR)m_cFlush, "98");
	lstrcpy( (LPSTR)m_cFullHouse, "177");
	lstrcpy( (LPSTR)m_cFourOAK, "4");
	lstrcpy( (LPSTR)m_cStraightFlush, "23");
	lstrcpy( (LPSTR)m_cRoyalFlush, "11");
	m_nSetOfOdds = IDC_MARTIAN;
	if ( bDisplay ) {
		ptxtPair->DisplayString( pDC, m_cPair, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtPairJacks->DisplayString( pDC, m_cPairJacksPlus, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtTwoPair->DisplayString( pDC, m_cTwoPair, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtThreeOAK->DisplayString( pDC, m_cThreeOAK, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtStraight->DisplayString( pDC, m_cStriaght, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFlush->DisplayString( pDC, m_cFlush, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFullHouse->DisplayString( pDC, m_cFullHouse, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFourOAK->DisplayString( pDC, m_cFourOAK, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtStraightFlush->DisplayString( pDC, m_cStraightFlush, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtRoyalFlush->DisplayString( pDC, m_cRoyalFlush, 12, FW_BOLD, RGB( 0, 0, 200) ); 
	}
	ReleaseDC( pDC );
	return;
}

void CSetPayoffsDlg::OnPopo( BOOL bDisplay )
{
CDC 			*pDC;
	pDC = GetDC();

	pPoPorButton->SetCheck( 1 );
	lstrcpy( (LPSTR)m_cPair, "1");
	lstrcpy( (LPSTR)m_cPairJacksPlus, "2");
	lstrcpy( (LPSTR)m_cTwoPair, "3");
	lstrcpy( (LPSTR)m_cThreeOAK, "5");
	lstrcpy( (LPSTR)m_cStriaght, "8");
	lstrcpy( (LPSTR)m_cFlush, "10");
	lstrcpy( (LPSTR)m_cFullHouse, "15");
	lstrcpy( (LPSTR)m_cFourOAK, "50");
	lstrcpy( (LPSTR)m_cStraightFlush, "100");
	lstrcpy( (LPSTR)m_cRoyalFlush, "500");
	m_nSetOfOdds = IDC_POPO;
	if ( bDisplay ) {
		ptxtPair->DisplayString( pDC, m_cPair, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtPairJacks->DisplayString( pDC, m_cPairJacksPlus, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtTwoPair->DisplayString( pDC, m_cTwoPair, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtThreeOAK->DisplayString( pDC, m_cThreeOAK, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtStraight->DisplayString( pDC, m_cStriaght, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFlush->DisplayString( pDC, m_cFlush, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFullHouse->DisplayString( pDC, m_cFullHouse, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFourOAK->DisplayString( pDC, m_cFourOAK, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtStraightFlush->DisplayString( pDC, m_cStraightFlush, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtRoyalFlush->DisplayString( pDC, m_cRoyalFlush, 12, FW_BOLD, RGB( 0, 0, 200) ); 
	}
	ReleaseDC( pDC );
	return;
}

void CSetPayoffsDlg::OnVegas( BOOL bDisplay )
{
CDC 			*pDC;
	pDC = GetDC();

	pVegasButton->SetCheck( 1 );
	lstrcpy( (LPSTR)m_cPair, "0");
	lstrcpy( (LPSTR)m_cPairJacksPlus, "1");
	lstrcpy( (LPSTR)m_cTwoPair, "2");
	lstrcpy( (LPSTR)m_cThreeOAK, "3");
	lstrcpy( (LPSTR)m_cStriaght, "4");
	lstrcpy( (LPSTR)m_cFlush, "6");
	lstrcpy( (LPSTR)m_cFullHouse, "9");
	lstrcpy( (LPSTR)m_cFourOAK, "25");
	lstrcpy( (LPSTR)m_cStraightFlush, "50");
	lstrcpy( (LPSTR)m_cRoyalFlush, "250");
	m_nSetOfOdds = IDC_VEGAS;
	if ( bDisplay ) {
		ptxtPair->DisplayString( pDC, m_cPair, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtPairJacks->DisplayString( pDC, m_cPairJacksPlus, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtTwoPair->DisplayString( pDC, m_cTwoPair, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtThreeOAK->DisplayString( pDC, m_cThreeOAK, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtStraight->DisplayString( pDC, m_cStriaght, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFlush->DisplayString( pDC, m_cFlush, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFullHouse->DisplayString( pDC, m_cFullHouse, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtFourOAK->DisplayString( pDC, m_cFourOAK, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtStraightFlush->DisplayString( pDC, m_cStraightFlush, 12, FW_BOLD, RGB( 0, 0, 200) );
		ptxtRoyalFlush->DisplayString( pDC, m_cRoyalFlush, 12, FW_BOLD, RGB( 0, 0, 200) ); 
	}
	ReleaseDC( pDC );
	return;
}

void CSetPayoffsDlg::OnDestroy()
{
//  send a message to the calling app to tell it the user has quit the game
	delete ptxtPair;
	delete ptxtPairJacks;
	delete ptxtTwoPair;
	delete ptxtThreeOAK;
	delete ptxtStraight;
	delete ptxtFlush;
	delete ptxtFullHouse;
	delete ptxtFourOAK;
	delete ptxtStraightFlush;
	delete ptxtRoyalFlush;

    if ( pSetPayButton != NULL ) {
        delete pSetPayButton;
        pSetPayButton = NULL;
    }
    if ( pCancelButton != NULL ) {
        delete pCancelButton;
        pCancelButton = NULL;
    }
    if ( pOKButton != NULL ) {
        delete pOKButton;
        pOKButton = NULL;
    }
    if ( pKlingonButton != NULL ) {
        delete pKlingonButton;
        pKlingonButton = NULL;
    }
    if ( pVegasButton != NULL ) {
        delete pVegasButton;
        pVegasButton = NULL;
    }
    if ( pPoPorButton != NULL ) {
        delete pPoPorButton;
        pPoPorButton = NULL;
    }
    if ( pKuwaitiButton != NULL ) {
        delete pKuwaitiButton;
        pKuwaitiButton = NULL;
    }
    if ( pMartianButton != NULL ) {
        delete pMartianButton;
        pMartianButton = NULL;
    }

	CBmpDialog::OnDestroy();
} 

BOOL CSetPayoffsDlg::OnEraseBkgnd(CDC *pDC)
{
	return(TRUE);
}

void CSetPayoffsDlg::ClearDialogImage(void)
{

    if ( pSetPayButton != NULL ) {
        delete pSetPayButton;
        pSetPayButton = NULL;
    }
    if ( pOKButton != NULL ) {
        delete pOKButton;
        pOKButton = NULL;
    }
    if ( pCancelButton != NULL ) {
        delete pCancelButton;
        pCancelButton = NULL;
    }
    if ( pKlingonButton != NULL ) {
        delete pKlingonButton;
        pKlingonButton = NULL;
    }
    if ( pVegasButton != NULL ) {
        delete pVegasButton;
        pVegasButton = NULL;
    }
    if ( pPoPorButton != NULL ) {
        delete pPoPorButton;
        pPoPorButton = NULL;
    }
    if ( pKuwaitiButton != NULL ) {
        delete pKuwaitiButton;
        pKuwaitiButton = NULL;
    }
    if ( pMartianButton != NULL ) {
        delete pMartianButton;
        pMartianButton = NULL;
    }

    ValidateRect(NULL);

    return;
}

BEGIN_MESSAGE_MAP(CSetPayoffsDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
