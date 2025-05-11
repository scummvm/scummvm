/*****************************************************************
*
*  itemmsg.cpp
*
*  HISTORY
*
*       1.00      07/25/94     BAR     Initial Design
*
*  MODULE DESCRIPTION:
*
*       Message box for item gain/loss information, built off of CBmpDialog
*
*  CONSTRUCTORS:
*
*       itemmsg
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
#include "citemdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


CItemDialog::CItemDialog( CWnd* pParent, CPalette *pPalette, CItem *pItem, BOOL bHodj, BOOL bGain, long lAmount )
         : CBmpDialog( pParent, pPalette, IDD_ITEMMOVE, ".\\art\\msscroll.bmp")
{
    // Initialize all members
    //
    m_pPalette = pPalette;
	m_pItem = pItem;
	m_bHodj = bHodj;
	m_bGain = bGain;      
    m_lAmount = lAmount;
	
    m_pTextMessage = NULL;
    m_pTextDescription = NULL;

    DoModal();

    //{{AFX_DATA_INIT(CItemDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void CItemDialog::ClearDialogImage()
{
    if (m_pOKButton != NULL) {                        // release the button
        delete m_pOKButton;
        m_pOKButton = NULL;
    }

    ValidateRect(NULL);
}

void CItemDialog::OnDestroy()
{
    if (m_pTextMessage != NULL) {
        delete m_pTextMessage;
        m_pTextMessage = NULL;
    }

    if (m_pTextDescription != NULL) {
        delete m_pTextDescription;
        m_pTextDescription = NULL;
    }

    if (m_pOKButton != NULL) {                        // release the button
        delete m_pOKButton;
        m_pOKButton = NULL;
    }

    CBmpDialog::OnDestroy();
}

void CItemDialog::DoDataExchange(CDataExchange* pDX)
{
    CBmpDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CItemDialog)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CItemDialog, CBmpDialog)
    //{{AFX_MSG_MAP(CItemDialog)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CItemDialog message handlers

BOOL CItemDialog::OnInitDialog()
{
	BOOL	bSuccess;
    CDC     *pDC;
    CRect   textRect;                  								// game stats display
    int     nText_col_offset;           							// game stats placement
    int     nText_row_offset;
    int     nTextWidth, nTextHeight;

    CBmpDialog::OnInitDialog();

    pDC = GetDC();

    nText_col_offset    = ITEMDLG_COL;
    nText_row_offset    = ITEMDLG_ROW;
    nTextWidth          = ITEMDLG_WIDTH;
    nTextHeight         = ITEMDLG_HEIGHT;
    textRect.SetRect( nText_col_offset, nText_row_offset,
                         nText_col_offset + nTextWidth,
                         nText_row_offset + nTextHeight );

    if ((m_pTextMessage = new CText()) != NULL) {
        bSuccess = (*m_pTextMessage).SetupText(pDC, m_pPalette, &textRect, JUSTIFY_CENTER);
        ASSERT(bSuccess);   // initialize the text objext
    }

    nText_row_offset += ITEMDLG2_ROW_OFFSET;
    textRect.SetRect(
                         nText_col_offset,
                         nText_row_offset,
                         nText_col_offset + nTextWidth,
                         nText_row_offset + nTextHeight
                     );

    if ((m_pTextDescription = new CText()) != NULL) {
        bSuccess = (*m_pTextDescription).SetupText(pDC, m_pPalette, &textRect, JUSTIFY_CENTER);
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

void CItemDialog::OnPaint()
{
    CDC     *pDC;
    BOOL    bSuccess;
	CFont   *pFontOld = NULL;               // font that was mapped to the context
    char	buf[64];
    int		i = 0;

    CBmpDialog::OnPaint();
    // Do not call CDialog::OnPaint() for painting messages

    pDC = GetDC();

    Common::sprintf_s(buf, "%s %s", m_bHodj ? "Hodj": "Podj", m_bGain ? "gains": "loses");
    bSuccess = (*m_pTextMessage).DisplayString(pDC, buf, 20, FW_BOLD, TEXT_COLOR);
    ASSERT(bSuccess);

    Common::sprintf_s( buf, (*m_pItem).GetDescription( (*m_pItem).GetID(), m_lAmount) );
	while( buf[i] != NULL ) i++;
	if ( i > 20 )
    	bSuccess = (*m_pTextDescription).DisplayString(pDC, buf, 13, FW_BOLD, ITEMDLG_TEXT_COLOR);
	else
    	bSuccess = (*m_pTextDescription).DisplayString(pDC, buf, 16, FW_BOLD, ITEMDLG_TEXT_COLOR);
    ASSERT(bSuccess);

    PaintMaskedDIB( pDC, m_pPalette, (*m_pItem).GetArtSpec(),
					(ITEMDLG_DX - ITEMDLG_BITMAP_DX / 2), (ITEMDLG_DY - ITEMDLG_BITMAP_DY), 
					ITEMDLG_BITMAP_DX, ITEMDLG_BITMAP_DY);

    ReleaseDC(pDC);
}

void CItemDialog::OnOK()
{
    ClearDialogImage();
    EndDialog( IDOK );
}

void CItemDialog::OnCancel()
{
    ClearDialogImage();
    EndDialog( IDCANCEL );
}

BOOL CItemDialog::OnEraseBkgnd(CDC *)
{
    // Prevents refreshing of background
    return(TRUE);
}
