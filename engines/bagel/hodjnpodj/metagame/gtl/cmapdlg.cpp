/*****************************************************************
*
*  cmapdlg.cpp
*
*  HISTORY
*
*       1.00      07/26/94     BAR     Initial Design
*
*  MODULE DESCRIPTION:
*
*       Dialog box showing Hodj and Podj on the zoom map, built off of CBmpDialog
*
*  CONSTRUCTORS:
*
*       cmapdlg
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
#include "cmapdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CMapDialog::CMapDialog( CWnd* pParent, CPalette *pPalette, POINT HodjLoc, POINT PodjLoc )
         : CBmpDialog( pParent, pPalette, IDD_ZOOMMAP, ".\\art\\minimap.bmp", 0, 0, FALSE)
{
    // Initialize all members
    //
    m_pPalette = pPalette;
	
	m_HodjLoc = HodjLoc;
	m_PodjLoc = PodjLoc;
		
    m_pTextMessage = NULL;

    DoModal();

    //{{AFX_DATA_INIT(CMapDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void CMapDialog::OnDestroy()
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

void CMapDialog::DoDataExchange(CDataExchange* pDX)
{
    CBmpDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMapDialog)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMapDialog, CBmpDialog)
    //{{AFX_MSG_MAP(CMapDialog)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMapDialog message handlers

BOOL CMapDialog::OnInitDialog()
{
	BOOL	bSuccess;
    CDC     *pDC;
    CRect   textRect;                  								// game stats display
    int     nText_col_offset;           							// game stats placement
    int     nText_row_offset;
    int     nTextWidth, nTextHeight;

    CBmpDialog::OnInitDialog();

    pDC = GetDC();

    nText_col_offset    = MAP_COL;
    nText_row_offset    = MAP_ROW;
    nTextWidth          = ZOOMMAP_WIDTH;
    nTextHeight         = MAP_ROW;
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
    (*m_pOKButton).SetPalette(m_pPalette);                       // set the palette to use
    bSuccess = (*m_pOKButton).SetControl((int) GetDefID(),this);  // tie to the dialog control
    ASSERT(bSuccess);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMapDialog::OnPaint()
{
    CDC     *pDC;
    BOOL    bSuccess;
    char	buf[64];

    CBmpDialog::OnPaint();
    // Do not call CDialog::OnPaint() for painting messages

    pDC = GetDC();

    Common::sprintf_s( buf, "" );
    bSuccess = (*m_pTextMessage).DisplayString(pDC, buf, 20, FW_BOLD, TEXT_COLOR);
    ASSERT(bSuccess);

    m_ptZoomHodj.x = (int)(((long)m_HodjLoc.x * ZOOMMAP_WIDTH) / BIGMAP_WIDTH);
    m_ptZoomHodj.y = (int)(((long)m_HodjLoc.y  * ZOOMMAP_HEIGHT) / BIGMAP_HEIGHT);

    m_ptZoomPodj.x = (int)(((long)m_PodjLoc.x * ZOOMMAP_WIDTH) / BIGMAP_WIDTH);
    m_ptZoomPodj.y = (int)(((long)m_PodjLoc.y * ZOOMMAP_HEIGHT)/ BIGMAP_HEIGHT);

    PaintMaskedDIB( pDC, m_pPalette, ".\\art\\hodj.bmp",
                    m_ptZoomHodj.x - PLAYER_OFFSET_X, m_ptZoomHodj.y - PLAYER_OFFSET_Y,
					PLAYER_BITMAP_DX, PLAYER_BITMAP_DY );

    PaintMaskedDIB( pDC, m_pPalette, ".\\art\\podj.bmp",
                    m_ptZoomPodj.x - PLAYER_OFFSET_X, m_ptZoomPodj.y - PLAYER_OFFSET_Y,
					PLAYER_BITMAP_DX, PLAYER_BITMAP_DY );

    ReleaseDC(pDC);
}

void CMapDialog::ClearDialogImage()
{
    if (m_pOKButton != NULL) {                        // release the button
        delete m_pOKButton;
        m_pOKButton = NULL;
    }

    ValidateRect(NULL);
}

void CMapDialog::OnOK()
{
    ClearDialogImage();
    EndDialog( IDOK );
}

void CMapDialog::OnCancel()
{
    ClearDialogImage();
    EndDialog( IDCANCEL );
}

BOOL CMapDialog::OnEraseBkgnd(CDC *)
{
    // Prevents refreshing of background
    return(TRUE);
}
