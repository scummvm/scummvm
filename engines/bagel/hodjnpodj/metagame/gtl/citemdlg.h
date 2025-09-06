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

#ifndef BAGEL_METAGAME_GTL_CITEMDLG_H
#define BAGEL_METAGAME_GTL_CITEMDLG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/metagame/bgen/item.h"

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

#define ITEMDLG2_ROW_OFFSET ( ITEMDLG_DY - ITEMDLG_HEIGHT ) // Row offset from first
// message box

#define IDD_ITEMMOVE        202
#define BUTTON_DY           10

class CItemDialog : public CBmpDialog {
private:
	CText *m_pTextDescription;
	CText *m_pTextMessage;
	CColorButton *m_pOKButton;          // OKAY button on scroll
	CItem *m_pItem;
	bool            m_bHodj;
	bool            m_bGain;
	long            m_lAmount;

public:

	// standard constructor
	CItemDialog(CWnd *pParent, CPalette *pPalette, CItem *pItem, bool bHodj, bool bGain, long lAmount);

	//  static  void UpdateItem(CDC *pDC, CItem *pItem, int nX, int nY);

	// Dialog Data
	//{{AFX_DATA(CItemDialog)
	enum {
		IDD = IDD_ITEMMOVE
	};
	// NOTE: the ClassWizard will add data members here
//}}AFX_DATA

private:
	void ClearDialogImage();

	// Implementation
protected:
	virtual void DoDataExchange(CDataExchange *pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CItemDialog)
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
