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
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"

namespace Bagel {
namespace HodjNPodj {

CBmpDialog::CBmpDialog(CWnd *pParent, CPalette *pPalette, int nID, const char *pFileName, const int dx, const int dy, bool bSaveBackground)
	: CDialog(nID, pParent) {
	// can't access null pointers
	//
	assert(pParent != nullptr);
	assert(pPalette != nullptr);
	assert(pFileName != nullptr);

	// Inits
	//
	m_pDlgBackground = nullptr;
	m_pParentWnd = pParent;
	m_pPalette = pPalette;
	m_pBmpFileName = pFileName;
	m_nDx = dx;
	m_nDy = dy;
	m_bSaveBackground = bSaveBackground;

	//{{AFX_DATA_INIT(CBmpDialog)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CBmpDialog::CBmpDialog(CWnd *pParent, CPalette *pPalette, int nID, int nBmpID, const int dx, const int dy, bool bSaveBackground)
	: CDialog(nID, pParent) {
	// can't access null pointers
	//
	assert(pParent != nullptr);
	assert(pPalette != nullptr);

	// Inits
	//
	m_pDlgBackground = nullptr;
	m_pBmpFileName = nullptr;
	m_pParentWnd = pParent;
	m_pPalette = pPalette;
	m_nBmpID = nBmpID;
	m_nDx = dx;
	m_nDy = dy;
	m_bSaveBackground = bSaveBackground;

	//{{AFX_DATA_INIT(CBmpDialog)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBmpDialog::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBmpDialog)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


void CBmpDialog::EndDialog(int nResult) {
	if (m_pDlgBackground != nullptr) {
		RefreshBackground();
		ValidateRect(nullptr);
	}
	CDialog::EndDialog(nResult);
}


void CBmpDialog::OnDestroy() {
	bool    bUpdateNeeded;

	//
	// delete the bitmap created with FetchScreenBitmap
	//
	if (m_pDlgBackground != nullptr) {
		m_pDlgBackground->DeleteObject();
		delete m_pDlgBackground;
		m_pDlgBackground = nullptr;

		bUpdateNeeded = (*m_pParentWnd).GetUpdateRect(nullptr, false);
		if (bUpdateNeeded)
			(*m_pParentWnd).ValidateRect(nullptr);
	}

	CDialog::OnDestroy();
}


bool CBmpDialog::OnInitDialog() {
	CRect cDlgRect, cWindRect;
	int iDlgWidth, iDlgHeight;          // size of dialog box
	CDC *pDC;
	CBitmap *pBitmap;
	CSize cSize;

	CDialog::OnInitDialog();            // do basic dialog initialization

	// get the button's position and size

	(*m_pParentWnd).GetWindowRect(&cWindRect);          // get pos/size of parent
	((CWnd *)this)->GetWindowRect(&cDlgRect);           // get pos/size of dialog

	pDC = GetDC();
	assert(pDC != nullptr);

	if (m_pBmpFileName == nullptr)                         // try to fetch the bitmap
		pBitmap = FetchResourceBitmap(pDC, nullptr, m_nBmpID);
	else
		pBitmap = FetchBitmap(pDC, nullptr, m_pBmpFileName);
	if (pBitmap == nullptr) {                              // no luck, so use what we know
		iDlgWidth = cDlgRect.right - cDlgRect.left;     // dlg box width
		iDlgHeight = cDlgRect.bottom - cDlgRect.top;    // dlg box height
	} else {
		cSize = GetBitmapSize(pBitmap);                 // actual width and height from bitmap
		iDlgWidth = cSize.cx;
		iDlgHeight = cSize.cy;
		delete pBitmap;
	}

	// modify code below to center dialog box based on GAME_WIDTH and GAME_HEIGHT
	// center the dialog box on the screen

	if (m_nDx == -1) {                                  // no horizontal location, so center
		//...the dialog box horizontally
		cDlgRect.left = cWindRect.left + (cWindRect.right - cWindRect.left - iDlgWidth) / 2;
	} else {                                                // want it in a specific horiz.loc.
		cDlgRect.left = cWindRect.left + m_nDx;
	}
	cDlgRect.right = cDlgRect.left + iDlgWidth;         // set the right side

	if (m_nDy == -1) {                                  // no vertical location, so center
		//...the dialog box vertically
		cDlgRect.top = cWindRect.top + (cWindRect.bottom - cWindRect.top - iDlgHeight) / 2;
	} else {                                                // want it in a specific vertical loc.
		cDlgRect.top = cWindRect.top + m_nDy;
	}
	cDlgRect.bottom = cDlgRect.top + iDlgHeight;

	MoveWindow(&cDlgRect, false);                       // position window, don't repaint

	// if we are saving the background
	//
	if (m_bSaveBackground) {
		// save a copy of the background
		m_pDlgBackground = FetchScreenBitmap(pDC, m_pPalette, 0, 0, iDlgWidth, iDlgHeight);
	}

	ReleaseDC(pDC);

	return true;  // return true  unless focused on a control
}


bool CBmpDialog::OnEraseBkgnd(CDC *pDC) {
	return true;
}


void CBmpDialog::RefreshBackground(CDC *pDC) {
	CDC *pMyDC;

	// paint back the background
	//
	if (m_pDlgBackground != nullptr) {
		if (pDC == nullptr) {
			if ((pMyDC = GetDC()) != nullptr) {
				PaintBitmap(pMyDC, m_pPalette, m_pDlgBackground, 0, 0);
				ReleaseDC(pMyDC);
			}
		} else
			PaintBitmap(pDC, m_pPalette, m_pDlgBackground, 0, 0);
	}
}


void CBmpDialog::OnPaint() {
	InvalidateRect(nullptr, false);

	CPaintDC    dc(this);                    // device context for painting
	CPalette *pPalOld = nullptr;
	bool        bSuccess;

	if (m_pPalette != nullptr) {
		pPalOld = dc.SelectPalette(m_pPalette, false);
		dc.RealizePalette();
	}

	// repaint the background behind the dialog
	//
	RefreshBackground(&dc);

	if (m_pDlgBackground != nullptr) {
		bSuccess = PaintBitmap(&dc, m_pPalette, m_pDlgBackground, 0, 0);
		assert(bSuccess);
	}

	// paint the dialog (uses bitmap instead of standard windows dialog)
	//
	if (m_pBmpFileName != nullptr) {
		bSuccess = PaintMaskedDIB(&dc, m_pPalette, m_pBmpFileName, 0, 0);
	} else {
		bSuccess = PaintMaskedResource(&dc, m_pPalette, m_nBmpID, 0, 0);
	}

	assert(bSuccess);

	if (m_pPalette != nullptr)
		dc.SelectPalette(pPalOld, false);
}


void CBmpDialog::OnShowWindow(bool bShow, unsigned int nStatus) {
	CDialog::OnShowWindow(bShow, nStatus);
}


void CBmpDialog::OnSize(unsigned int nType, int cx, int cy) {
	CDialog::OnSize(nType, cx, cy);
}


int CBmpDialog::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


void CBmpDialog::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CDialog::OnLButtonDown(nFlags, point);
}


void CBmpDialog::OnMouseMove(unsigned int nFlags, CPoint point) {
	CDialog::OnMouseMove(nFlags, point);
}

BEGIN_MESSAGE_MAP(CBmpDialog, CDialog)
	//{{AFX_MSG_MAP(CBmpDialog)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_NCMOUSEMOVE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace HodjNPodj
} // namespace Bagel
