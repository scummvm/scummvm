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

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/metagame/frame/resource.h"
#include "bagel/hodjnpodj/metagame/frame/dialogs.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Frame {

static CPalette     *pMainPalette;

static CColorButton	*pPlayButton = NULL;
static CColorButton	*pCancelButton = NULL;

static CRadioButton *pHSHButton = NULL;	// Hodj Skill High Radio Button
static CRadioButton *pHSMButton = NULL;	// Hodj Skill Medium Radio Button
static CRadioButton *pHSLButton = NULL;	// Hodj Skill Low Radio Button

static CRadioButton *pPSHButton = NULL;	// Podj Skill High Radio Button
static CRadioButton *pPSMButton = NULL;	// Podj Skill Medium Radio Button
static CRadioButton *pPSLButton = NULL;	// Podj Skill Low Radio Button

static CRadioButton *pGTLButton = NULL;	// Game Time Long Radio Button
static CRadioButton *pGTMButton = NULL;	// Game Time Medium Radio Button
static CRadioButton *pGTSButton = NULL;	// Game Time Short Radio Button

static CRadioButton *pPCButton = NULL;	// Podj is Computer High Radio Button
static CRadioButton *pPHButton = NULL;	// Podj is Human High Radio Button

/*****************************************************************
 *
 * CMetaSetupDlg
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
CMetaSetupDlg::CMetaSetupDlg(CWnd *pParent, CPalette *pPalette ) :  CBmpDialog(pParent, pPalette, IDD_META_SETUP, ".\\ART\\MLSCROLL.BMP",-1,-1,FALSE)
{
        pMainPalette = pPalette;                  
		m_lpMetaGameStruct = NULL;
		m_nHodjSkillLevel = SKILLLEVEL_LOW;
		m_nPodjSkillLevel = SKILLLEVEL_LOW;
		m_nGameTime = MEDIUM_GAME;
		m_bPodjIsComputer = TRUE;
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
BOOL CMetaSetupDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{

// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog

if (HIWORD(lParam) == BN_CLICKED) {
    switch (wParam) {
			case IDC_HODJ_SKILL_HARD: 
				pHSHButton->SetCheck( 1 );
				m_nHodjSkillLevel = SKILLLEVEL_HIGH;
				break;
			case IDC_HODJ_SKILL_MEDIUM:
				pHSMButton->SetCheck( 1 );
				m_nHodjSkillLevel = SKILLLEVEL_MEDIUM;
				break;
			case IDC_HODJ_SKILL_EASY:
				pHSLButton->SetCheck( 1 );
				m_nHodjSkillLevel = SKILLLEVEL_LOW;
				break;

			case IDC_PODJ_SKILL_HARD:
				pPSHButton->SetCheck( 1 );
				m_nPodjSkillLevel = SKILLLEVEL_HIGH;
				break;
			case IDC_PODJ_SKILL_MEDIUM:
				pPSMButton->SetCheck( 1 );
				m_nPodjSkillLevel = SKILLLEVEL_MEDIUM;
				break;
			case IDC_PODJ_SKILL_EASY:
				pPSLButton->SetCheck( 1 );
				m_nPodjSkillLevel = SKILLLEVEL_LOW;
				break;

			case IDC_GAMETIME_LONG:
				pGTLButton->SetCheck( 1 );
				m_nGameTime = LONG_GAME;
				break;
			case IDC_GAMETIME_MEDIUM:
				pGTMButton->SetCheck( 1 );
				m_nGameTime = MEDIUM_GAME;
				break;
			case IDC_GAMETIME_SHORT:
				pGTSButton->SetCheck( 1 );
				m_nGameTime = SHORT_GAME;
				break;             

			case IDC_PODJ_COMPUTER:
				pPCButton->SetCheck( 1 );
				SetDlgItemText(IDC_PODJ_SKILL_HARD,"Tough Opponent");
				SetDlgItemText(IDC_PODJ_SKILL_MEDIUM,"Average Opponent");
				SetDlgItemText(IDC_PODJ_SKILL_EASY,"Unskilled Opponent");
				m_bPodjIsComputer = TRUE;
				break;
			case IDC_PODJ_HUMAN:
				pPHButton->SetCheck( 1 );
				SetDlgItemText(IDC_PODJ_SKILL_HARD,"Hard");
				SetDlgItemText(IDC_PODJ_SKILL_MEDIUM,"Medium");
				SetDlgItemText(IDC_PODJ_SKILL_EASY,"Easy");
				m_bPodjIsComputer = FALSE;
				break;
			

            case IDC_PLAY:
// fill in the appropriate settings for the game structure   
                m_lpMetaGameStruct->m_cHodj.m_iSkillLevel = m_nHodjSkillLevel;
                m_lpMetaGameStruct->m_cPodj.m_iSkillLevel = m_nPodjSkillLevel;
                m_lpMetaGameStruct->m_cHodj.m_bComputer = FALSE;
                m_lpMetaGameStruct->m_cPodj.m_bComputer = m_bPodjIsComputer;
                m_lpMetaGameStruct->m_iGameTime = m_nGameTime;
			    ClearDialogImage();
                EndDialog( 1 );
                return(TRUE);

            case IDC_CANCEL_PLAY:
			    ClearDialogImage();
                EndDialog( 0 );
                return(TRUE);
        }
    }

    return(CBmpDialog::OnCommand(wParam, lParam));
}

void CMetaSetupDlg::OnCancel(void)
{
    ClearDialogImage();
	EndDialog( 0 );
	return;
}

void CMetaSetupDlg::OnOK(void)
{
	
	if ( pPlayButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_PLAY, (LPARAM)BN_CLICKED );
		return;
	}          
	if ( pCancelButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_CANCEL_PLAY, (LPARAM)BN_CLICKED );
		return;
	}
	if ( pHSHButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_HODJ_SKILL_HARD, (LPARAM)BN_CLICKED );
		return;
	}
	if ( pHSMButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_HODJ_SKILL_MEDIUM, (LPARAM)BN_CLICKED );
		return;
	}
	if ( pHSLButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_HODJ_SKILL_EASY, (LPARAM)BN_CLICKED );
		return;
	}
	if ( pPSHButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_PODJ_SKILL_HARD, (LPARAM)BN_CLICKED );
		return;
	}
	if ( pPSMButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_PODJ_SKILL_MEDIUM, (LPARAM)BN_CLICKED );
		return;
	}
	if ( pPSLButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_PODJ_SKILL_EASY, (LPARAM)BN_CLICKED );
		return;
	}
	if ( pGTLButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_GAMETIME_LONG, (LPARAM)BN_CLICKED );
		return;
	}
	if ( pGTMButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_GAMETIME_MEDIUM, (LPARAM)BN_CLICKED );
		return;
	}
	if ( pGTSButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_GAMETIME_SHORT, (LPARAM)BN_CLICKED );
		return;
	}
	if ( pPCButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_PODJ_COMPUTER, (LPARAM)BN_CLICKED );
		return;
	}
	if ( pPHButton->GetState() & 0x0008 ) {
		SendMessage( WM_COMMAND, IDC_PODJ_HUMAN, (LPARAM)BN_CLICKED );
		return;
	}

	return;
}

void CMetaSetupDlg::ClearDialogImage(void)
{
/*
	if ( pPlayButton != NULL )
		delete pPlayButton;
		
	if ( pCancelButton != NULL )
		delete pCancelButton;

	if ( pHSHButton != NULL )
		delete pHSHButton;
		
	if ( pHSMButton != NULL )
		delete pHSMButton;
		
	if ( pHSLButton != NULL )
		delete pHSLButton;
		
	if ( pPSHButton != NULL )
		delete pPSHButton;
		
	if ( pPSMButton != NULL )
		delete pPSMButton;
		
	if ( pPSLButton != NULL )
		delete pPSLButton;

	if ( pGTLButton != NULL )
		delete pGTLButton;
		
	if ( pGTMButton != NULL )
		delete pGTMButton;
		
	if ( pGTSButton != NULL )
		delete pGTSButton;

	if ( pPCButton != NULL )
		delete pPCButton;
		
	if ( pPHButton != NULL )
		delete pPHButton;

    pPlayButton = NULL;
    pCancelButton = NULL;

	pHSHButton = NULL;
	pHSMButton = NULL;
	pHSLButton = NULL;
	pPSHButton = NULL;
	pPSMButton = NULL;
	pPSLButton = NULL;
	pGTLButton = NULL;
	pGTMButton = NULL;
	pGTSButton = NULL;
	pPCButton = NULL;
	pPHButton = NULL;

    ValidateRect(NULL);
*/
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
void CMetaSetupDlg::SetInitialOptions( CBfcMgr *lpMetaGameStruct )
{        
	m_lpMetaGameStruct = lpMetaGameStruct;
    m_nHodjSkillLevel = m_lpMetaGameStruct->m_cHodj.m_iSkillLevel;
    m_nPodjSkillLevel = m_lpMetaGameStruct->m_cPodj.m_iSkillLevel;
    m_bPodjIsComputer = m_lpMetaGameStruct->m_cPodj.m_bComputer;
    m_nGameTime = m_lpMetaGameStruct->m_iGameTime;
    return;
}  


BOOL CMetaSetupDlg::OnInitDialog()
{
BOOL	bSuccess;

    CBmpDialog::OnInitDialog();

	pPlayButton = new CColorButton();
	ASSERT( pPlayButton != NULL );
	pPlayButton->SetPalette( pMainPalette );
	bSuccess = pPlayButton->SetControl( IDC_PLAY, this );
	ASSERT( bSuccess );
	
	pCancelButton = new CColorButton();
	ASSERT( pCancelButton != NULL );
	pCancelButton->SetPalette( pMainPalette );
	bSuccess = pCancelButton->SetControl( IDC_CANCEL_PLAY, this );
	ASSERT( bSuccess );
	
	pHSHButton = new CRadioButton();
	ASSERT( pHSHButton != NULL );
	pHSHButton->SetPalette( pMainPalette );
	bSuccess = pHSHButton->SetControl( IDC_HODJ_SKILL_HARD, this );
	ASSERT( bSuccess );
	
	pHSMButton = new CRadioButton();
	ASSERT( pHSMButton != NULL );
	pHSMButton->SetPalette( pMainPalette );
	bSuccess = pHSMButton->SetControl( IDC_HODJ_SKILL_MEDIUM, this );
	ASSERT( bSuccess );
	
	pHSLButton = new CRadioButton();
	ASSERT( pHSLButton != NULL );
	pHSLButton->SetPalette( pMainPalette );
	bSuccess = pHSLButton->SetControl( IDC_HODJ_SKILL_EASY, this );
	ASSERT( bSuccess );

	pPSHButton = new CRadioButton();
	ASSERT( pPSHButton != NULL );
	pPSHButton->SetPalette( pMainPalette );
	bSuccess = pPSHButton->SetControl( IDC_PODJ_SKILL_HARD, this );
	ASSERT( bSuccess );

	pPSMButton = new CRadioButton();
	ASSERT( pPSMButton != NULL );
	pPSMButton->SetPalette( pMainPalette );
	bSuccess = pPSMButton->SetControl( IDC_PODJ_SKILL_MEDIUM, this );
	ASSERT( bSuccess );

	pPSLButton = new CRadioButton();
	ASSERT( pPSLButton != NULL );
	pPSLButton->SetPalette( pMainPalette );
	bSuccess = pPSLButton->SetControl( IDC_PODJ_SKILL_EASY, this );
	ASSERT( bSuccess );

	pGTLButton = new CRadioButton();
	ASSERT( pGTLButton != NULL );
	pGTLButton->SetPalette( pMainPalette );
	bSuccess = pGTLButton->SetControl( IDC_GAMETIME_LONG, this );
	ASSERT( bSuccess );

	pGTMButton = new CRadioButton();
	ASSERT( pGTMButton != NULL );
	pGTMButton->SetPalette( pMainPalette );
	bSuccess = pGTMButton->SetControl( IDC_GAMETIME_MEDIUM, this );
	ASSERT( bSuccess );

	pGTSButton = new CRadioButton();
	ASSERT( pGTSButton != NULL );
	pGTSButton->SetPalette( pMainPalette );
	bSuccess = pGTSButton->SetControl( IDC_GAMETIME_SHORT, this );
	ASSERT( bSuccess );

	pPCButton = new CRadioButton();
	ASSERT( pPCButton != NULL );
	pPCButton->SetPalette( pMainPalette );
	bSuccess = pPCButton->SetControl( IDC_PODJ_COMPUTER, this );
	ASSERT( bSuccess );

	pPHButton = new CRadioButton();
	ASSERT( pPHButton != NULL );
	pPHButton->SetPalette( pMainPalette );
	bSuccess = pPHButton->SetControl( IDC_PODJ_HUMAN, this );
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
void CMetaSetupDlg::OnPaint(void)
{
//      call CBmpDialog onpaint, to paint the background
    CBmpDialog::OnPaint();

    // CPaintDC dc(this);
    PAINTSTRUCT lpPaint;
    Invalidate(FALSE);
    BeginPaint(&lpPaint);

    CDC     *pDC = GetDC();
    int     nOldBkMode = pDC->SetBkMode( TRANSPARENT );

	pDC->TextOut( 72, 63, "Number of Players", 17);
	pDC->TextOut( 262, 63, "Game Duration", 13);
	pDC->TextOut( 72, 184, "Hodj's Skill Level", 18);
	pDC->TextOut( 262, 184, "Podj's Skill Level", 18);

	pDC->SetBkMode( nOldBkMode );
    ReleaseDC(pDC);
	
    EndPaint(&lpPaint);

	if ( m_nHodjSkillLevel == SKILLLEVEL_HIGH ) {
		pHSHButton->SetCheck( 1 );
	}
	else {
		if ( m_nHodjSkillLevel == SKILLLEVEL_MEDIUM ) {
			pHSMButton->SetCheck( 1 );
		}
		else {
			pHSLButton->SetCheck( 1 );
		}
	}

	if ( m_nPodjSkillLevel == SKILLLEVEL_HIGH ) {
		pPSHButton->SetCheck( 1 );
	}
	else {
		if ( m_nPodjSkillLevel == SKILLLEVEL_MEDIUM ) {
			pPSMButton->SetCheck( 1 );
		}
		else {
			pPSLButton->SetCheck( 1 );
		}
	}


	if ( m_nGameTime == LONG_GAME ) {
		pGTLButton->SetCheck( 1 );
	}
	else {
		if ( m_nGameTime == MEDIUM_GAME ) {
			pGTMButton->SetCheck( 1 );
		}
		else {
			pGTSButton->SetCheck( 1 );
		}
	}

	if ( m_bPodjIsComputer == TRUE ) {
		pPCButton->SetCheck( 1 );
	}
	else {
		pPHButton->SetCheck( 1 );
	}

    return;
}

void CMetaSetupDlg::OnDestroy()
{
	if ( pPlayButton != NULL )
		delete pPlayButton;
		
	if ( pCancelButton != NULL )
		delete pCancelButton;
		
	if ( pHSHButton != NULL )
		delete pHSHButton;
		
	if ( pHSMButton != NULL )
		delete pHSMButton;
		
	if ( pHSLButton != NULL )
		delete pHSLButton;
		
	if ( pPSHButton != NULL )
		delete pPSHButton;
		
	if ( pPSMButton != NULL )
		delete pPSMButton;
		
	if ( pPSLButton != NULL )
		delete pPSLButton;

	if ( pGTLButton != NULL )
		delete pGTLButton;
		
	if ( pGTMButton != NULL )
		delete pGTMButton;
		
	if ( pGTSButton != NULL )
		delete pGTSButton;

	if ( pPCButton != NULL )
		delete pPCButton;
		
	if ( pPHButton != NULL )
		delete pPHButton;
		
    CBmpDialog::OnDestroy();
}

// Message Map
BEGIN_MESSAGE_MAP(CMetaSetupDlg, CBmpDialog)
    //{{AFX_MSG_MAP( CMainPokerWindow )
    ON_WM_PAINT()
	ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace Frame
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
