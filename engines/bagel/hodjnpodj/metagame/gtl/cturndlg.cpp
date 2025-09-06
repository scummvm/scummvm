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
#include "bagel/hodjnpodj/metagame/gtl/cturndlg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

CTurnDialog::CTurnDialog(CWnd* pParent, CPalette *pPalette, bool bHodj, bool bGain, bool bTurn)
	: CBmpDialog(pParent, pPalette, IDD_TURNDLG, ".\\art\\msscroll.bmp") {
	// Initialize all members
	//
	m_pPalette = pPalette;
	m_bHodj = bHodj;
	m_bGain = bGain;
	m_bTurn = bTurn;

	m_pTextMessage = nullptr;

	DoModal();

	//{{AFX_DATA_INIT(CTurnDialog)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CTurnDialog::OnDestroy() {
	if (m_pTextMessage != nullptr) {
		delete m_pTextMessage;
		m_pTextMessage = nullptr;
	}

	if (m_pOKButton != nullptr) {                        // release the button
		delete m_pOKButton;
		m_pOKButton = nullptr;
	}

	CBmpDialog::OnDestroy();
}

void CTurnDialog::DoDataExchange(CDataExchange* pDX) {
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

bool CTurnDialog::OnInitDialog() {
	bool    bSuccess;
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
	textRect.SetRect(nText_col_offset, nText_row_offset,
	                 nText_col_offset + nTextWidth,
	                 nText_row_offset + nTextHeight);

	if ((m_pTextMessage = new CText()) != nullptr) {
		bSuccess = (*m_pTextMessage).SetupText(pDC, m_pPalette, &textRect, JUSTIFY_CENTER);
		ASSERT(bSuccess);   // initialize the text objext
	}

	ReleaseDC(pDC);

	m_pOKButton = new CColorButton();                   // build a color QUIT button to let us exit
	ASSERT(m_pOKButton != nullptr);
	(*m_pOKButton).SetPalette(m_pPalette);        // set the palette to use
	bSuccess = (*m_pOKButton).SetControl((int) GetDefID(), this); // tie to the dialog control
	ASSERT(bSuccess);

	return true;  // return true  unless you set the focus to a control
}

void CTurnDialog::OnPaint() {
	CDC     *pDC;
	bool    bSuccess;
	char    msgBuf[64],
	        artBuf[64];

	CBmpDialog::OnPaint();
	// Do not call CDialog::OnPaint() for painting messages

	pDC = GetDC();

	if (m_bTurn) {
		Common::sprintf_s(msgBuf, "%s %s", m_bHodj ? "Hodj" : "Podj", m_bGain ? "gains a turn." : "loses a turn.");
		Common::sprintf_s(artBuf, "%s", m_bGain ? ".\\art\\freeturn.bmp" : ".\\art\\loseturn.bmp");
	} else {
		Common::sprintf_s(msgBuf, "%s %s", m_bHodj ? "Hodj" : "Podj", m_bGain ? "wins the game!" : "loses the game.");
		Common::sprintf_s(artBuf, "%s", m_bGain ? ".\\art\\wingame.bmp" : ".\\art\\losegame.bmp");
	}

	bSuccess = (*m_pTextMessage).DisplayString(pDC, msgBuf, 20, FW_BOLD, TEXT_COLOR);
	ASSERT(bSuccess);

	PaintMaskedDIB(pDC, m_pPalette, artBuf,
	               (ITEMDLG_DX - ITEMDLG_BITMAP_DX / 2), ITEMDLG_ROW, //(ITEMDLG_DY - ITEMDLG_BITMAP_DY),
	               ITEMDLG_BITMAP_DX, ITEMDLG_BITMAP_DY);

	ReleaseDC(pDC);
}

void CTurnDialog::OnOK() {
	ClearDialogImage();
	EndDialog(IDOK);
}

void CTurnDialog::OnCancel() {
	ClearDialogImage();
	EndDialog(IDCANCEL);
}

bool CTurnDialog::OnEraseBkgnd(CDC *) {
	// Prevents refreshing of background
	return true;
}

void CTurnDialog::ClearDialogImage() {
	if (m_pTextMessage != nullptr) {
		delete m_pTextMessage;
		m_pTextMessage = nullptr;
	}

	if (m_pOKButton != nullptr) {                         // release the button
		delete m_pOKButton;
		m_pOKButton = nullptr;
	}

	ValidateRect(nullptr);
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
