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

#ifndef BAGEL_METAGAME_GTL_STORE_H
#define BAGEL_METAGAME_GTL_STORE_H

#include "bagel/hodjnpodj/metagame/bgen/invent.h"
#include "bagel/hodjnpodj/metagame/bgen/item.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define STORE_SPEC  ".\\art\\mlscroll.bmp"      // path for store's DIB on disk
#define STORE_DX            502
#define STORE_DY            395
#define STORE_CURL_DX       500
#define STORE_CURL_DY       50
#define STORE_BORDER_DX     60
#define STORE_BORDER_DY     60

#define STORE_TITLEZONE_DX  100
#define STORE_TITLEZONE_DY  40
#define STORE_TITLEZONE_DDY -10

#define STORE_TEXTZONE_DX   75
#define STORE_TEXTZONE_DY   40
#define STORE_TEXTZONE_DDY  20
#define STORE_TEXTZONE_DDDY 5

#define STORE_COSTZONE_DX   75
#define STORE_COSTZONE_DY   20
#define STORE_COSTZONE_DDY  20
#define STORE_COSTZONE_DDDY 5

#define STORE_BITMAP_DX     59
#define STORE_BITMAP_DY     59
#define STORE_BITMAP_DDX    5
#define STORE_BITMAP_DDY    5

#define IDD_STORE_DIALOG    930
#define IDC_STORE_OKAY      930

#define IDC_STORE_DOLLAR    931

#define STORE_TEXT_COLOR    RGB(128,0,128)
#define STORE_MORE_COLOR    RGB(0,0,0)
#define STORE_BLURB_COLOR   RGB(0,0,255)


class CGeneralStore : public CDialog {
// Construction
public:
	CGeneralStore(CWnd* pParent, CPalette *pPalette, CInventory *pStore, CInventory *pInventory);

	int SelectedItem(CPoint point);

	static  void UpdateStore(CDC *pDC);
	static  void UpdateContent(CDC *pDC);
	static  void UpdateItem(CDC *pDC, CItem *pItem, int nX, int nY);
	static  void UpdatePage(CDC *pDC);
	static  void UpdateCrowns(CDC *pDC);
	static  void ClearDialogImage();
	static  void RefreshBackground();
	static  void DoWaitCursor();
	static  void DoArrowCursor();

private:
	bool SetupKeyboardHook();
	void RemoveKeyboardHook();

	bool CreateWorkAreas(CDC *pDC);

private:
	bool m_bKeyboardHook;		// whether keyboard hook present
	CRect OkayRect;				// rectangle bounding the OKAY button
	CRect StoreRect;			// x/y (left/right) and dx/dy (right/bottom) for the store window
	CRect ScrollTopRect,		// area spanned by upper scroll curl
		ScrollBotRect;			// area spanned by lower scroll curl

	// Dialog Data
	//{{AFX_DATA(CGeneralStore)
	enum { IDD = IDD_STORE_DIALOG };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange *pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CGeneralStore)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual bool OnInitDialog() override;
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	virtual void OnOK() override;
	virtual void OnCancel() override;
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(bool bShow, unsigned int nStatus);
	afx_msg void OnSize(unsigned int nType, int cx, int cy);
	afx_msg void OnMouseMove(unsigned int nFlags, CPoint point);
	afx_msg bool OnSetCursor(CWnd *pWnd, unsigned int nHitTest, unsigned int message);
	afx_msg void OnLButtonDown(unsigned int nFlags, CPoint point);
	afx_msg void OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized) override;
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
