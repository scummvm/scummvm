/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved.
 *
 *
 * c1btndlg.cpp
 *
 * HISTORY
 *
 *      1.1      08/19/94     BCW       Modified for 2 user defined buttons
 *      1.0      04/30/94     Jos       Creation
 *
 * MODULE DESCRIPTION:
 *
 *      Paints a scroll on the middle of the screen with
 *      two optional text strings displayed and 2 buttons
 *
 * CONSTRUCTORS:
 *
 *  C1ButtonDialog(CWnd *pParent, CPalette *pPalatte, char *msg1, char *msg2 = NULL);
 *
 *  If paramaters are not passed, assumes NULL. CWnd is the parent window
 *  used to get centering information.  msg2 is an optional messages
 *  that would be displayed, centered horizontally only, within the window. 
 *  For example, 
 *
 *      C1ButtonDialog dlgYesNo((CWnd *)this, pPalette, "Yes", "Are you", "really cool?");
 *
 * PUBLIC:
 *
 *
 *
 * PUBLIC GLOBAL:
 *
 *
 *
 * PROTECTED:
 *
 *
 * PRIVATE:
 *
 *
 * MEMBERS:
 *
 *  CText        *m_cTextMessage1;   // used to do the first message
 *  CText        *m_cTextMessage2;   // Used to display second message
 *  char         *m_pszMessage1;     // Message text string
 *  char         *m_pszMessage2;
 *  char         *m_pszButton1Text;
 *  CColorButton *m_pButton1;
 *
 * RELEVANT DOCUMENTATION:
 *
 *
 ****************************************************************/
#include "bagel/afxwin.h"


#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "wordlist.h"
#include "c1btndlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define TEXT_COLOR          RGB(0, 0, 0)    // displayed text color

#define MESSAGE_COL         16              // first message box positioning
#define MESSAGE_ROW         60

#define MESSAGE_WIDTH       175             // standard message box dims
#define MESSAGE_HEIGHT      20

#define MESSAGE2_ROW_OFFSET ( MESSAGE_HEIGHT + 4 )  // Row offset from first 
                                                    // message box

#define IDD_1BTNDLG 321

extern	CString	astrGameList[WORDSPERLIST];
extern	CString	astrGameListDisplay[WORDSPERLIST];
extern	CText	atxtDisplayWord[WORDSPERLIST];



C1ButtonDialog::C1ButtonDialog(CWnd *pParent, CPalette *pPalette, char *pszButton1Text, char *pszText1, char *pszText2)
         : CBmpDialog(pParent, pPalette, IDD_1BTNDLG, ".\\ART\\WLSTSCRL.BMP", 16, 25)
{
    assert(pParent != NULL);
    assert(pPalette != NULL);
    assert(pszText1 != NULL);
    assert(pszButton1Text != NULL);

    // Initialize all members
    //
    m_pPalette = pPalette;
    m_pszMessage1 = pszText1;
    m_pszMessage2 = pszText2;
    m_pszButton1Text = pszButton1Text;

    m_cTextMessage1 = NULL;
    m_cTextMessage2 = NULL;

    //{{AFX_DATA_INIT(C1ButtonDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void C1ButtonDialog::ClearDialogImage()
{
    if (m_pButton1 != NULL) {                          // release the 1st button
        delete m_pButton1;
        m_pButton1 = NULL;
    }

    ValidateRect(NULL);
}

void C1ButtonDialog::OnDestroy()
{
	int	i;
	
	for (i = 0; i < WORDSPERLIST; i++ ) {
		delete m_cWordList[i];
		m_cWordList[i] = NULL;
	}
	
    if (m_cTextMessage1 != NULL) {
        delete m_cTextMessage1;
        m_cTextMessage1 = NULL;
    }

    if (m_cTextMessage2 != NULL) {
        delete m_cTextMessage2;
        m_cTextMessage2 = NULL;
    }    

    CBmpDialog::OnDestroy();
}

void C1ButtonDialog::DoDataExchange(CDataExchange* pDX)
{
    CBmpDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(C1ButtonDialog)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(C1ButtonDialog, CBmpDialog)
    //{{AFX_MSG_MAP(C1ButtonDialog)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// C1ButtonDialog message handlers

BOOL C1ButtonDialog::OnInitDialog()
{
    CRect   statsRect;                  // game stats displays
    int     nStat_col_offset;           // game stats placement
    int     nStat_row_offset;
    int     nStatWidth, nStatHeight;
    BOOL    bAssertCheck;
    CDC     *pDC;
    int		i;

    CBmpDialog::OnInitDialog();

    pDC = GetDC();

    nStat_col_offset    = MESSAGE_COL;
    nStat_row_offset    = MESSAGE_ROW;
    nStatWidth          = MESSAGE_WIDTH;
    nStatHeight         = MESSAGE_HEIGHT;
    statsRect.SetRect   (
                         nStat_col_offset,
                         nStat_row_offset,
                         nStat_col_offset + nStatWidth,
                         nStat_row_offset + nStatHeight
                        );

    if ((m_cTextMessage1 = new CText()) != NULL) {
        bAssertCheck = (*m_cTextMessage1).SetupText(pDC, m_pPalette, &statsRect, JUSTIFY_CENTER);
        ASSERT(bAssertCheck);   // initialize the text objext
    }

    nStat_row_offset += MESSAGE2_ROW_OFFSET;
    statsRect.SetRect(
                         nStat_col_offset,
                         nStat_row_offset,
                         nStat_col_offset + nStatWidth,
                         nStat_row_offset + nStatHeight
                     );

    if ((m_cTextMessage2 = new CText()) != NULL) {
        bAssertCheck = (*m_cTextMessage2).SetupText(pDC, m_pPalette, &statsRect, JUSTIFY_CENTER);
        ASSERT(bAssertCheck);   // initialize the text objext
    }

	nStat_col_offset = 0;
	nStat_row_offset = 30;    
    for (i=0; i < WORDSPERLIST; i++ ) {
	    statsRect.SetRect(
	                         nStat_col_offset,
	                         nStat_row_offset + (12 * i),
	                         nStat_col_offset + nStatWidth,
	                         nStat_row_offset + (12 * i) + nStatHeight
	                     );
	    if ((m_cWordList[i] = new CText()) != NULL) {
	        bAssertCheck = (*m_cWordList[i]).SetupText(pDC, m_pPalette, &statsRect, JUSTIFY_CENTER);
	        ASSERT(bAssertCheck);   // initialize the text objext
	    }
	}    	

    ReleaseDC(pDC);

    if ((m_pButton1 = new CColorButton) != NULL) {            // build the first color button
        (*m_pButton1).SetPalette(m_pPalette);                 // set the palette to use
        (*m_pButton1).SetControl(IDOK, this);                 // tie to the dialog control
        (*m_pButton1).SetWindowText(m_pszButton1Text);
    }
    
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void C1ButtonDialog::OnPaint()
{
    CDC     *pDC;
    BOOL    bAssertCheck;
    int		i;
    char	buf[30];

    CBmpDialog::OnPaint();
    // Do not call CDialog::OnPaint() for painting messages

    pDC = GetDC();
/*
    if ( m_pszMessage1 != NULL ) {
        bAssertCheck = (*m_cTextMessage1).DisplayString(pDC, m_pszMessage1, 21, FW_BOLD, TEXT_COLOR);
        ASSERT(bAssertCheck);
    }

    if ( m_pszMessage2 != NULL ) {
        bAssertCheck = (*m_cTextMessage2).DisplayString(pDC, m_pszMessage2, 21, FW_BOLD, TEXT_COLOR);
        ASSERT(bAssertCheck);
    }
*/
	for ( i = 0; i < WORDSPERLIST; i++ ) { 
		if ( astrGameList[i].IsEmpty() ) {
			Common::sprintf_s( buf, "%s", astrGameListDisplay[i] );
			m_cWordList[i]->DisplayString( pDC,  buf, 12, FW_BOLD );
		}
		else {
			Common::sprintf_s( buf, "%s", astrGameListDisplay[i] );
			m_cWordList[i]->DisplayShadowedString( pDC,  buf, 12, FW_BOLD, (COLORREF)RGB( 0, 255, 255) );
		}
	}

    ReleaseDC(pDC);
}

void C1ButtonDialog::OnOK()
{
    ClearDialogImage();
    EndDialog(IDOK);
}

void C1ButtonDialog::OnCancel()
{
    ClearDialogImage();
    EndDialog(IDOK);
}

BOOL C1ButtonDialog::OnEraseBkgnd(CDC *)
{
    // Prevents refreshing of background
    return(TRUE);
}
