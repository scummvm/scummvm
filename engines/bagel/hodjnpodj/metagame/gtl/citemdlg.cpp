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
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/metagame/gtl/citemdlg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

CItemDialog::CItemDialog(CWnd* pParent, CPalette *pPalette, CItem *pItem, BOOL bHodj, BOOL bGain, long lAmount)
	: CBmpDialog(pParent, pPalette, IDD_ITEMMOVE, ".\\art\\msscroll.bmp") {
	// Initialize all members
	//
	m_pPalette = pPalette;
	m_pItem = pItem;
	m_bHodj = bHodj;
	m_bGain = bGain;
	m_lAmount = lAmount;

	m_pTextMessage = nullptr;
	m_pTextDescription = nullptr;

	DoModal();

	//{{AFX_DATA_INIT(CItemDialog)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CItemDialog::ClearDialogImage() {
	if (m_pOKButton != nullptr) {                        // release the button
		delete m_pOKButton;
		m_pOKButton = nullptr;
	}

	ValidateRect(nullptr);
}

void CItemDialog::OnDestroy() {
	if (m_pTextMessage != nullptr) {
		delete m_pTextMessage;
		m_pTextMessage = nullptr;
	}

	if (m_pTextDescription != nullptr) {
		delete m_pTextDescription;
		m_pTextDescription = nullptr;
	}

	if (m_pOKButton != nullptr) {                        // release the button
		delete m_pOKButton;
		m_pOKButton = nullptr;
	}

	CBmpDialog::OnDestroy();
}

void CItemDialog::DoDataExchange(CDataExchange* pDX) {
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

BOOL CItemDialog::OnInitDialog() {
	BOOL    bSuccess;
	CDC     *pDC;
	CRect   textRect;                                               // game stats display
	int     nText_col_offset;                                       // game stats placement
	int     nText_row_offset;
	int     nTextWidth, nTextHeight;

	CBmpDialog::OnInitDialog();

	pDC = GetDC();

	nText_col_offset    = ITEMDLG_COL;
	nText_row_offset    = ITEMDLG_ROW;
	nTextWidth          = ITEMDLG_WIDTH;
	nTextHeight         = ITEMDLG_HEIGHT;
	textRect.SetRect(nText_col_offset, nText_row_offset,
	                 nText_col_offset + nTextWidth,
	                 nText_row_offset + nTextHeight);

	if ((m_pTextMessage = new CText()) != nullptr) {
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

	if ((m_pTextDescription = new CText()) != nullptr) {
		bSuccess = (*m_pTextDescription).SetupText(pDC, m_pPalette, &textRect, JUSTIFY_CENTER);
		ASSERT(bSuccess);   // initialize the text objext
	}

	ReleaseDC(pDC);

	m_pOKButton = new CColorButton;                   // build a color QUIT button to let us exit
	ASSERT(m_pOKButton != nullptr);
	(*m_pOKButton).SetPalette(m_pPalette);        // set the palette to use
	bSuccess = (*m_pOKButton).SetControl((int) GetDefID(), this); // tie to the dialog control
	ASSERT(bSuccess);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CItemDialog::OnPaint() {
	CDC     *pDC;
	BOOL    bSuccess;
	char    buf[64];
	int     i = 0;

	CBmpDialog::OnPaint();
	// Do not call CDialog::OnPaint() for painting messages

	pDC = GetDC();

	Common::sprintf_s(buf, "%s %s", m_bHodj ? "Hodj" : "Podj", m_bGain ? "gains" : "loses");
	bSuccess = (*m_pTextMessage).DisplayString(pDC, buf, 20, FW_BOLD, TEXT_COLOR);
	ASSERT(bSuccess);

	Common::sprintf_s(buf, (*m_pItem).GetDescription((*m_pItem).GetID(), m_lAmount));
	while (buf[i] != 0) i++;
	if (i > 20)
		bSuccess = (*m_pTextDescription).DisplayString(pDC, buf, 13, FW_BOLD, ITEMDLG_TEXT_COLOR);
	else
		bSuccess = (*m_pTextDescription).DisplayString(pDC, buf, 16, FW_BOLD, ITEMDLG_TEXT_COLOR);
	ASSERT(bSuccess);

	PaintMaskedDIB(pDC, m_pPalette, (*m_pItem).GetArtSpec(),
	               (ITEMDLG_DX - ITEMDLG_BITMAP_DX / 2), (ITEMDLG_DY - ITEMDLG_BITMAP_DY),
	               ITEMDLG_BITMAP_DX, ITEMDLG_BITMAP_DY);

	ReleaseDC(pDC);
}

void CItemDialog::OnOK() {
	ClearDialogImage();
	EndDialog(IDOK);
}

void CItemDialog::OnCancel() {
	ClearDialogImage();
	EndDialog(IDCANCEL);
}

BOOL CItemDialog::OnEraseBkgnd(CDC *) {
	// Prevents refreshing of background
	return (TRUE);
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
