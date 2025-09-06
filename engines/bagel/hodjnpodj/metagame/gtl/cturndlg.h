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

#ifndef BAGEL_METAGAME_GTL_CTURNDLG_H
#define BAGEL_METAGAME_GTL_CTURNDLG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define TEXT_COLOR              RGB(0, 0, 0)    // displayed text color
#define ITEMDLG_FONT_SIZE       -14
#define ITEMDLG_TEXT_COLOR      RGB(128,0,128)
#define ITEMDLG_BLURB_COLOR     RGB(0,0,255)

#define ITEMDLG_COL         16              // first message box positioning
#define ITEMDLG_ROW         24

#define ITEMDLG_WIDTH       186             // TEXT dims
#define ITEMDLG_HEIGHT      20

#define ITEMDLG_DX          113             // Width of Dialog box
#define ITEMDLG_DY          110
#define ITEMDLG_BITMAP_DX   60
#define ITEMDLG_BITMAP_DY   60

#define IDD_TURNDLG         203

class CTurnDialog : public CBmpDialog {
private:
	CColorButton *m_pOKButton;      // OKAY button on scroll
	CText        *m_pTextMessage;
	bool          m_bHodj;
	bool          m_bGain;
	bool          m_bTurn;          // True: win/lose turn, False: win/lose Game

public:

	// standard constructor
	CTurnDialog(CWnd *pParent, CPalette *pPalette, bool bHodj, bool bGain, bool bTurn);
	void ClearDialogImage();

	// Dialog Data
	//{{AFX_DATA(CTurnDialog)
	enum { IDD = IDD_TURNDLG };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CTurnDialog)
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
