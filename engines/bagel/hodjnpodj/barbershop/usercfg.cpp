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
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/barbershop/usercfg.h"
#include "bagel/hodjnpodj/hnplibs/menures.h"
#include "bagel/hodjnpodj/barbershop/paint.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

#define ID_RESET     104
#define ID_LIMIT     105

#define PAGE_SIZE    10

// globals!!
//
//
extern const char *INI_SECTION;
extern CPalette     *pGamePalette;
extern LPGAMESTRUCT pGameParams;
extern int          g_nCardBack;

CUserCfgDlg::CUserCfgDlg(CWnd *pParent, CPalette *pPalette, unsigned int nID)
	: CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP") {
	m_cRectCardBack1    = CRect(CBCK_RECT1_LEFT - DELTA, CBCK_RECT1_TOP - DELTA, CBCK_RECT1_RIG + DELTA, CBCK_RECT1_BOT + DELTA);
	m_cRectCardBack2    = CRect(CBCK_RECT2_LEFT - DELTA, CBCK_RECT2_TOP - DELTA, CBCK_RECT2_RIG + DELTA, CBCK_RECT2_BOT + DELTA);
	m_nCardBack         = g_nCardBack;
	DoModal();
}

void CUserCfgDlg::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);
}

bool CUserCfgDlg::OnInitDialog() {
	CDC     *pDC = GetDC();
	CRect   tmpRect;

	CBmpDialog::OnInitDialog();
	m_bSave = false;

	if ((m_ctextBox = new CText) != nullptr) {
		bool    bAssertCheck;

		tmpRect.SetRect(TEXT_LEFT, TEXT_TOP, TEXT_RIG, TEXT_BOT);
		bAssertCheck = (*m_ctextBox).SetupText(pDC, pGamePalette, &tmpRect, JUSTIFY_CENTER);
		ASSERT(bAssertCheck);   // initialize the text objext
	} // end if

	if ((m_pOKButton = new CColorButton) != nullptr) {     // build a color OK button
		(*m_pOKButton).SetPalette(pGamePalette);        // set the palette to use
		(*m_pOKButton).SetControl(IDOK, this);          // tie to the dialog control
	} // end if

	ReleaseDC(pDC);
	return true;
}

void CUserCfgDlg::OnPaint() {
	CDC     *pDC = nullptr;
	char    msg[64];
	bool    bAssertCheck;

	CBmpDialog::OnPaint();

	pDC = GetDC();

	if (m_nCardBack == CARD_BACK1) {         // card back painting
		PaintMaskedDIB(pDC, pGamePalette, CARD_BACK1B_BMP, CBCK_RECT1_LEFT, CBCK_RECT1_TOP, CBCK_RECT1_RIG - CBCK_RECT1_LEFT, CBCK_RECT1_BOT - CBCK_RECT1_TOP);
		PaintMaskedDIB(pDC, pGamePalette, CARD_BACK2A_BMP, CBCK_RECT2_LEFT, CBCK_RECT2_TOP, CBCK_RECT2_RIG - CBCK_RECT2_LEFT, CBCK_RECT2_BOT - CBCK_RECT2_TOP);
	} else {
		PaintMaskedDIB(pDC, pGamePalette, CARD_BACK1A_BMP, CBCK_RECT1_LEFT, CBCK_RECT1_TOP, CBCK_RECT1_RIG - CBCK_RECT1_LEFT, CBCK_RECT1_BOT - CBCK_RECT1_TOP);
		PaintMaskedDIB(pDC, pGamePalette, CARD_BACK2B_BMP, CBCK_RECT2_LEFT, CBCK_RECT2_TOP, CBCK_RECT2_RIG - CBCK_RECT2_LEFT, CBCK_RECT2_BOT - CBCK_RECT2_TOP);
	} // end if

	Common::sprintf_s(msg, "Select a card back");   // top message
	bAssertCheck = (*m_ctextBox).DisplayString(pDC, msg, FONT_SIZE, FW_BOLD, RGBCOLOR_BLACK);
	ASSERT(bAssertCheck);

	ReleaseDC(pDC);
}

void CUserCfgDlg::OnLButtonUp(unsigned int nFlags, CPoint point) {
	CDC     *pDC = GetDC();

	if (
	    m_cRectCardBack1.PtInRect(point) == true &&
	    m_nCardBack != CARD_BACK1
	) {
		// update visual image
		//
		PaintMaskedDIB(pDC, pGamePalette, CARD_BACK1B_BMP, CBCK_RECT1_LEFT, CBCK_RECT1_TOP, CBCK_RECT1_RIG - CBCK_RECT1_LEFT, CBCK_RECT1_BOT - CBCK_RECT1_TOP);
		PaintMaskedDIB(pDC, pGamePalette, CARD_BACK2A_BMP, CBCK_RECT2_LEFT, CBCK_RECT2_TOP, CBCK_RECT2_RIG - CBCK_RECT2_LEFT, CBCK_RECT2_BOT - CBCK_RECT2_TOP);

		m_nCardBack = CARD_BACK1;
	} else if (
	    m_cRectCardBack2.PtInRect(point) == true &&
	    m_nCardBack != CARD_BACK2
	) {
		// update visual image
		//
		PaintMaskedDIB(pDC, pGamePalette, CARD_BACK1A_BMP, CBCK_RECT1_LEFT, CBCK_RECT1_TOP, CBCK_RECT1_RIG - CBCK_RECT1_LEFT, CBCK_RECT1_BOT - CBCK_RECT1_TOP);
		PaintMaskedDIB(pDC, pGamePalette, CARD_BACK2B_BMP, CBCK_RECT2_LEFT, CBCK_RECT2_TOP, CBCK_RECT2_RIG - CBCK_RECT2_LEFT, CBCK_RECT2_BOT - CBCK_RECT2_TOP);

		m_nCardBack = CARD_BACK2;
	} // end if

	ReleaseDC(pDC);
}

void CUserCfgDlg::OnOK() {
	ValidateRect(nullptr);
	g_nCardBack = m_nCardBack;
	EndDialog(IDOK);
}

void CUserCfgDlg::OnCancel() {
	ValidateRect(nullptr);
	EndDialog(IDCANCEL);
}

void CUserCfgDlg::OnClose() {
	ValidateRect(nullptr);
	if (m_pOKButton != nullptr) {      // release button
		delete m_pOKButton;
		m_pOKButton = nullptr;
	}

	if (m_ctextBox != nullptr) {
		delete m_ctextBox;
		m_ctextBox = nullptr;
	} // end if
}

BEGIN_MESSAGE_MAP(CUserCfgDlg, CBmpDialog)
	ON_WM_LBUTTONUP()
	ON_WM_CLOSE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

/*****************************************************************
 *
 *  MyFocusRect( CDC *pDC, CRect rect, int nDrawMode )
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Draws a rectangle which inverts the current pixels,
 *          thereby delineating the current area of focus.
 *
 *  FORMAL PARAMETERS:
 *
 *      CDC *pDC    The Device context in which the FocusRect is to be drawn
 *      CRect rect  The CRect object holding the location of the FocusRect
 *
 *  IMPLICIT INPUT PARAMETERS:
 *
 *      pMyPen and pMyBrush, global pointers to the Pen and Brush used
 *
 *  IMPLICIT OUTPUT PARAMETERS:
 *
 *      none
 *
 *  RETURN VALUE:
 *
 *      void
 *
 ****************************************************************/
void CUserCfgDlg::MyFocusRect(CDC *pDC, CRect rect, int nDrawMode, COLORREF rgbColor) {
	CBrush      *pMyBrush = nullptr;                   // New Brush
	CBrush      *pOldBrush = nullptr;                  // Pointer to old brush
	CPen        *pMyPen = nullptr;                     // New Pen
	CPen        *pOldPen = nullptr;                    // Pointer to old pen
	CPalette    *pPalOld = nullptr;                    // Pointer to old palette
	int         OldDrawMode;                        // Holder for old draw mode

	pMyBrush = new CBrush();                        // Construct new brush
	pMyPen = new CPen();                            // Construct new pen

	LOGBRUSH lb;                                    // log brush type
	lb.lbStyle = BS_HOLLOW;                         // Don't fill in area
	pMyBrush->CreateBrushIndirect(&lb);              // Create a new brush
	pMyPen->CreatePen(PS_INSIDEFRAME, DELTA, rgbColor); // Create a new pen

	pPalOld = (*pDC).SelectPalette(pGamePalette, false);     // Select in game palette
	(*pDC).RealizePalette();                                // Use it

	pOldPen = pDC->SelectObject(pMyPen);         // Select the new pen & save old
	pOldBrush = pDC->SelectObject(pMyBrush);     // Select the new brush & save old
	OldDrawMode = pDC->SetROP2(nDrawMode);       // Set pen mode, saving old state

	pDC->Rectangle(rect);                        // Draw the Rectangle to the DC

	pDC->SelectObject(pOldPen);                  // Select the old pen
	pDC->SelectObject(pOldBrush);                // Select the old brush
	pDC->SetROP2(OldDrawMode);                   // Set pen mode back to old state
	(*pDC).SelectPalette(pPalOld, false);           // Select back the old palette

	if (pMyBrush != nullptr) {                         // If the brush was constructed, delete it
		pMyBrush->DeleteObject();
		delete pMyBrush;
	}

	if (pMyPen != nullptr) {                           // If the pen was constructed, delete it
		pMyPen->DeleteObject();
		delete pMyPen;
	}

} // End MyFocusRect()

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel
