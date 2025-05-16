/*****************************************************************
*
*  CTurnDialog.cpp
*
*  HISTORY
*
*       1.00      07/25/94     BAR     Initial Design
*
*  MODULE DESCRIPTION:
*
*       Message box for turn gain/loss information, built off of CBmpDialog
*
*  CONSTRUCTORS:
*
*       turndialog
*
*  PUBLIC:
*
*
*
*  PUBLIC GLOBAL:
*
*
*
*  PROTECTED:
*
*
*
*  PRIVATE:
*
*
*
*  MEMBERS:
*
*
*
*  RELEVANT DOCUMENTATION:
*
*
*
****************************************************************/
#include "bagel/afxwin.h"

#include "bagel/hodjnpodj/hnplibs/text.h"
#include "cturndlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


CTurnDialog::CTurnDialog( CWnd* pParent, CPalette *pPalette, BOOL bHodj, BOOL bGain, BOOL bTurn )
         : CBmpDialog( pParent, pPalette, IDD_TURNDLG, ".\\art\\msscroll.bmp")
{
    // Initialize all members
    //
    m_pPalette = pPalette;
    m_bHodj = bHodj;
    m_bGain = bGain;
    m_bTurn = bTurn;
    
    m_pTextMessage = NULL;

    DoModal();

    //{{AFX_DATA_INIT(CTurnDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void CTurnDialog::OnDestroy()
{
    if (m_pTextMessage != NULL) {
        delete m_pTextMessage;
        m_pTextMessage = NULL;
    }

    if (m_pOKButton != NULL) {                        // release the button
        delete m_pOKButton;
        m_pOKButton = NULL;
    }

    CBmpDialog::OnDestroy();
}

void CTurnDialog::DoDataExchange(CDataExchange* pDX)
{
    CBmpDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTurnDialog)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTurnDialog, CBmpDialog)
    //{{AFX_MSG_MAP(CTurnDialog)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTurnDialog message handlers

BOOL CTurnDialog::OnInitDialog()
{
    BOOL    bSuccess;
    CDC     *pDC;
    CRect   textRect;                                               // game stats display
    int     nText_col_offset;                                       // game stats placement
    int     nText_row_offset;
    int     nTextWidth, nTextHeight;

    CBmpDialog::OnInitDialog();

    pDC = GetDC();

    nText_col_offset    = ITEMDLG_COL;
    nText_row_offset    = ITEMDLG_DY - ITEMDLG_HEIGHT;  //ITEMDLG_ROW + 
    nTextWidth          = ITEMDLG_WIDTH;
    nTextHeight         = ITEMDLG_HEIGHT;
    textRect.SetRect( nText_col_offset, nText_row_offset,
                         nText_col_offset + nTextWidth,
                         nText_row_offset + nTextHeight );

    if ((m_pTextMessage = new CText()) != NULL) {
        bSuccess = (*m_pTextMessage).SetupText(pDC, m_pPalette, &textRect, JUSTIFY_CENTER);
        ASSERT(bSuccess);   // initialize the text objext
    }

    ReleaseDC(pDC);

    m_pOKButton = new CColorButton;                   // build a color QUIT button to let us exit
    ASSERT(m_pOKButton != NULL);
    (*m_pOKButton).SetPalette(m_pPalette);        // set the palette to use
    bSuccess = (*m_pOKButton).SetControl((int) GetDefID(),this);  // tie to the dialog control
    ASSERT(bSuccess);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTurnDialog::OnPaint()
{
    CDC     *pDC;
    BOOL    bSuccess;
    char    msgBuf[64],
            artBuf[64];

    CBmpDialog::OnPaint();
    // Do not call CDialog::OnPaint() for painting messages

    pDC = GetDC();

    if ( m_bTurn ) {
        sprintf( msgBuf, "%s %s", m_bHodj?"Hodj":"Podj", m_bGain?"gains a turn.":"loses a turn." );
        sprintf( artBuf, "%s", m_bGain?".\\art\\freeturn.bmp":".\\art\\loseturn.bmp" );
    }
    else {
        sprintf( msgBuf, "%s %s", m_bHodj?"Hodj":"Podj", m_bGain?"wins the game!":"loses the game." );
        sprintf( artBuf, "%s", m_bGain?".\\art\\wingame.bmp":".\\art\\losegame.bmp" );
    }

    bSuccess = (*m_pTextMessage).DisplayString(pDC, msgBuf, 20, FW_BOLD, TEXT_COLOR);
    ASSERT(bSuccess);
    
    PaintMaskedDIB( pDC, m_pPalette, artBuf,
                    (ITEMDLG_DX - ITEMDLG_BITMAP_DX / 2), ITEMDLG_ROW, //(ITEMDLG_DY - ITEMDLG_BITMAP_DY), 
                    ITEMDLG_BITMAP_DX, ITEMDLG_BITMAP_DY);

    ReleaseDC(pDC);
}

void CTurnDialog::OnOK()
{
    ClearDialogImage();
    EndDialog( IDOK );
}

void CTurnDialog::OnCancel()
{
    ClearDialogImage();
    EndDialog( IDCANCEL );
}

BOOL CTurnDialog::OnEraseBkgnd(CDC *)
{
    // Prevents refreshing of background
    return(TRUE);
}

void CTurnDialog::ClearDialogImage(void)
{
    if (m_pTextMessage != NULL) {
        delete m_pTextMessage;
        m_pTextMessage = NULL;
    }

    if (m_pOKButton != NULL) {                         // release the button
        delete m_pOKButton;
        m_pOKButton = NULL;
    }

    ValidateRect(NULL);
}
