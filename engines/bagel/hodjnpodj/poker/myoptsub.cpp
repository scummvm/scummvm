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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/poker/options.h"

namespace Bagel {
namespace HodjNPodj {
namespace Poker {

extern CFont *pFont;

CPalette	*pOptionsPalette = NULL;
static	int 		tabstop = 20;

/////////////////////////////////////////////////////////////////////////////
// COptions dialog


COptions::COptions(CWnd* pParent, CPalette *pPalette, int iDlgId)
        : CDialog(iDlgId, pParent)
{
	pOptionsPalette = pPalette;
	if (!pParent)
          pParent = ((CWnd *)this)->GetParent();
    m_pParentWnd = pParent;

    pFont = NULL;

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
	delete pFont;
    pFont = NULL;

    CDialog::EndDialog(IDC_OPTIONS_QUIT);
}


void COptions::EndDialog(int nResult) {
    if (pFont != NULL) {
        delete pFont;
        pFont = NULL;
    }

    CDialog::EndDialog(nResult);
}

BOOL COptions::OnInitDialog()
{
	CDialog::OnInitDialog();						// do basic dialog initialization
	CRect		cDlgRect, cWindRect ;
	 int iDlgWidth, iDlgHeight ;	// size of dialog box
				 
// get the button's position and size

	(*m_pParentWnd).GetWindowRect(&cWindRect);  // get pos/size of parent
	((CWnd *)this)->GetWindowRect(&cDlgRect);  // get pos/size of dialog
// modify code below to center dialog box based on GAME_WIDTH and GAME_HEIGHT
// center the dialog box on the screen
	 iDlgWidth = cDlgRect.right - cDlgRect.left ;	// dlg box width
	 cDlgRect.left = (cWindRect.right + cWindRect.left - iDlgWidth)/2 ;
				// center the dialog box horizontally
	 cDlgRect.right = cDlgRect.left + iDlgWidth ;


	 iDlgHeight = cDlgRect.bottom - cDlgRect.top ;	// dlg box height
	 cDlgRect.top = (cWindRect.bottom + cWindRect.top - iDlgHeight)/2 ;
				// center the dialog box vertically
	 cDlgRect.bottom = cDlgRect.top + iDlgHeight ;


	 MoveWindow(&cDlgRect, FALSE) ;	// center window, don't repaint

		
	return(TRUE);  	// return TRUE  unless focused on a control
}


BOOL COptions::OnEraseBkgnd(CDC *pDC)
{
return(TRUE);
}


void COptions::OnPaint()
{
BOOL		bSuccess;
CPaintDC	dc(this); 								// device context for painting
CPalette	*pPalOld = NULL;

	if (pOptionsPalette != NULL) {
		pPalOld = dc.SelectPalette(pOptionsPalette,FALSE);
		(void) dc.RealizePalette();
	}


// BOOL PaintMaskedBitmap(CDC *pDC, CPalette *pPalette,
//    const int resId, const int x, const int y)

   bSuccess = PaintMaskedBitmap(&dc, pOptionsPalette, ".\\art\\sscroll.bmp", 0, 0);

	
	if (pOptionsPalette != NULL)
		(void) dc.SelectPalette(pPalOld,FALSE);
}


void COptions::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
}


void COptions::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}


int COptions::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	MFC::AddFontResource("msserif.fon");
	pFont = new CFont();
	ASSERT(pFont != NULL);
	ASSERT((*pFont).CreateFont(16,0,0,0,FW_BOLD,0,0,0,0,OUT_RASTER_PRECIS,0,PROOF_QUALITY,FF_ROMAN,"MS Sans Serif"));

	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel
