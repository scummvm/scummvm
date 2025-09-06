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

#ifndef BAGEL_METAGAME_GTL_CMAPDLG_H
#define BAGEL_METAGAME_GTL_CMAPDLG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define TEXT_COLOR              RGB(0, 0, 0)    // displayed text color
#define ITEMDLG_TEXT_COLOR      RGB(128,0,128)
#define ITEMDLG_BLURB_COLOR     RGB(0,0,255)

#define MAP_COL         20              // first message box positioning
#define MAP_ROW         10

#define ZOOMMAP_WIDTH       640             // pixel dims
#define ZOOMMAP_HEIGHT      480
#define BIGMAP_WIDTH        1925            // pixel dims
#define BIGMAP_HEIGHT       1415

#define MAP_DX              113             // Width of Dialog box
#define MAP_DY              110
#define PLAYER_BITMAP_DX    18
#define PLAYER_BITMAP_DY    28
#define PLAYER_OFFSET_X      9
#define PLAYER_OFFSET_Y     14

#define NUM_NODES           20
#define IDD_ZOOMMAP         204
#define BUTTON_DY           10

class CMapDialog : public CBmpDialog {
private:
	POINT         m_HodjLoc, m_PodjLoc;
	POINT         m_ptZoomHodj, m_ptZoomPodj;
	CText        *m_pTextMessage;
	CColorButton *m_pOKButton;          // OKAY button on scroll

public:

	// standard constructor
	CMapDialog(CWnd* pParent, CPalette *pPalette, POINT HodjLoc, POINT PodjLoc);

	void ClearDialogImage();

	// Dialog Data
	//{{AFX_DATA(CMapDialog)
	enum { IDD = IDD_ZOOMMAP };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CMapDialog)
	afx_msg void OnPaint();
	virtual void OnOK() override;
	virtual void OnCancel() override;
	virtual bool OnInitDialog() override;
	afx_msg void OnDestroy();
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
