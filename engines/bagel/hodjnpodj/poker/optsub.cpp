// optsub.cpp -- subroutines for options.cpp
// Written by John J. Xenakis for Boffo Games Inc., 1994

// Note to programmers:
//  Please do modify this file.  Instead, modify the
//	file options.cpp.


#include "stdafx.h"

//#include "options.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define 	SMALL_SCROLL_DX	222
#define 	SMALL_SCROLL_DY	182
#define 	MEDIUM_SCROLL_DX	280
#define 	MEDIUM_SCROLL_DY	326

CPalette	*pOptionsPalette = NULL;


static	int 		tabstop = 20;

/////////////////////////////////////////////////////////////////////////////
// COptions dialog


COptions::COptions(CWnd* pParent, CPalette *pPalette, int iDlgId)
        : CDialog(iDlgId, pParent)
{
	m_iDlgId = iDlgId;
	pOptionsPalette = pPalette;
	if (!pParent)
          pParent = ((CWnd *)this)->GetParent();
    m_pParentWnd = pParent;

    // Inits
    m_pFont = NULL;
    m_pDlgBackground = NULL;

	//{{AFX_DATA_INIT(COptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptions, CDialog)
	//{{AFX_MSG_MAP(COptions)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_NCMOUSEMOVE()
    ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_OPTIONS_RULES, OnClickedRules)
	ON_BN_CLICKED(IDC_OPTIONS_NEWGAME, OnClickedNewgame)
	ON_BN_CLICKED(IDC_OPTIONS_OPTIONS, OnClickedOptions)
	ON_BN_CLICKED(IDC_OPTIONS_RETURN, OnClickedReturn)
	ON_BN_CLICKED(IDC_OPTIONS_QUIT, OnClickedQuit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptions message handlers

void COptions::OnOK()
{
    RefreshBackground();
    CDialog::EndDialog(IDC_OPTIONS_QUIT);
}


void COptions::OnDestroy()
{
    // make sure the font is always deleted
    //
    if (m_pFont != NULL) {
        delete m_pFont;
        m_pFont = NULL;
    }

    //
    // delete the bitmap created with FetchScreenBitmap
    //
    if (m_pDlgBackground != NULL) {
        m_pDlgBackground->DeleteObject();
        delete m_pDlgBackground;
        m_pDlgBackground = NULL;
    }

    CDialog::OnDestroy();
}

BOOL COptions::OnInitDialog()
{
    CRect cDlgRect, cWindRect;
    int iDlgWidth, iDlgHeight;          // size of dialog box
    CDC *pDC;
				 
    CDialog::OnInitDialog();            // do basic dialog initialization

    // get the button's position and size

	(*m_pParentWnd).GetWindowRect(&cWindRect);  // get pos/size of parent
	((CWnd *)this)->GetWindowRect(&cDlgRect);  // get pos/size of dialog

    // modify code below to center dialog box based on GAME_WIDTH and GAME_HEIGHT
    // center the dialog box on the screen

		if ( m_iDlgId == IDD_WINRATIO ) {
			iDlgWidth = MEDIUM_SCROLL_DX;
			iDlgHeight = MEDIUM_SCROLL_DY;
		}
		else {
			iDlgWidth = SMALL_SCROLL_DX;
			iDlgHeight = SMALL_SCROLL_DY;
		}
	
//    iDlgWidth = cDlgRect.right - cDlgRect.left ;   // dlg box width
    cDlgRect.left = (cWindRect.right + cWindRect.left - iDlgWidth)/2 ;

    // center the dialog box horizontally
    cDlgRect.right = cDlgRect.left + iDlgWidth ;


//    iDlgHeight = cDlgRect.bottom - cDlgRect.top ;  // dlg box height
    cDlgRect.top = (cWindRect.bottom + cWindRect.top - iDlgHeight)/2 ;

    // center the dialog box vertically
    cDlgRect.bottom = cDlgRect.top + iDlgHeight ;

    MoveWindow(&cDlgRect, FALSE) ; // center window, don't repaint

    pDC = GetDC();
    assert(pDC != NULL);
    m_pDlgBackground = FetchScreenBitmap(pDC, pOptionsPalette, 0, 0, iDlgWidth, iDlgHeight);
    assert(m_pDlgBackground != NULL);
    ReleaseDC(pDC);
		
	return(TRUE);  	// return TRUE  unless focused on a control
}


BOOL COptions::OnEraseBkgnd(CDC *pDC)
{
    return(TRUE);
}


void COptions::OnPaint()
{
    InvalidateRect( NULL, FALSE );
    
    CPaintDC    dc(this);                    // device context for painting
    CPalette    *pPalOld = NULL;
    BOOL        bSuccess;
    char				*pFileSpec;

	RefreshBackground();
	
	if (pOptionsPalette != NULL) {
		pPalOld = dc.SelectPalette(pOptionsPalette,FALSE);
        dc.RealizePalette();
	}

    // BOOL PaintMaskedBitmap(CDC *pDC, CPalette *pPalette,
    //    const int resId, const int x, const int y)


    
	if ( m_iDlgId == IDD_WINRATIO ) {
    	pFileSpec = ".\\art\\mscroll.bmp";
    }
    else {
    	pFileSpec = ".\\art\\sscroll.bmp";
    }
    bSuccess = PaintMaskedDIB(&dc, pOptionsPalette, pFileSpec, 0, 0);
    assert(bSuccess);
	
	if (pOptionsPalette != NULL)
        dc.SelectPalette(pPalOld, FALSE);
}

void COptions::RefreshBackground() 
{
	CDC			*pDC = GetDC();
    CPalette    *pPalOld = NULL;
    BOOL        bSuccess;

	if (pOptionsPalette != NULL) {
		pPalOld = pDC->SelectPalette(pOptionsPalette,FALSE);
        pDC->RealizePalette();
	}

    // BOOL PaintMaskedBitmap(CDC *pDC, CPalette *pPalette,
    //    const int resId, const int x, const int y)

    bSuccess = PaintBitmap( pDC, pOptionsPalette, m_pDlgBackground, 0, 0);
    assert(bSuccess);      
    
	if (pOptionsPalette != NULL)
        pDC->SelectPalette(pPalOld, FALSE);
	ReleaseDC( pDC );
	return;	
}

void COptions::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);
}


void COptions::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
}


int COptions::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	::AddFontResource("msserif.fon");
    m_pFont = new CFont();
    ASSERT(m_pFont != NULL);
    m_pFont->CreateFont(16,0,0,0,FW_BOLD,0,0,0,0,OUT_RASTER_PRECIS,0,PROOF_QUALITY,FF_ROMAN,"MS Sans Serif");

	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}
