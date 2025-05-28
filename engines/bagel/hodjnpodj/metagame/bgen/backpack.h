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

#ifndef BAGEL_METAGAME_BGEN_BACKPACK_H
#define BAGEL_METAGAME_BGEN_BACKPACK_H

#include "bagel/hodjnpodj/metagame/bgen/invent.h"
#include "bagel/hodjnpodj/metagame/bgen/item.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define BACKPACK_SPEC   ".\\art\\mlscroll.bmp"      // path for backpack's DIB on disk
#define BACKPACK_DX             502
#define BACKPACK_DY             395
#define BACKPACK_CURL_DX        500
#define BACKPACK_CURL_DY        50
#define BACKPACK_BORDER_DX      60
#define BACKPACK_BORDER_DY      60

#define BACKPACK_TITLEZONE_DX   100
#define BACKPACK_TITLEZONE_DY   40
#define BACKPACK_TITLEZONE_DDY  -5

#define BACKPACK_TEXTZONE_DX    75
#define BACKPACK_TEXTZONE_DY    30
#define BACKPACK_TEXTZONE_DDY   10

#define BACKPACK_BITMAP_DX      59
#define BACKPACK_BITMAP_DY      59
#define BACKPACK_BITMAP_DDX     10
#define BACKPACK_BITMAP_DDY     10

#define IDD_BACKPACK_DIALOG    910
#define IDC_BACKPACK_OKAY      910

#define BACKPACK_FONT_SIZE      -14
#define BACKPACK_TEXT_COLOR     RGB(128,0,128)
#define BACKPACK_BLURB_COLOR    RGB(0,0,255)
#define BACKPACK_MORE_COLOR     RGB(0,0,0)


class CBackpack : public CDialog {
// Construction
public:
	CBackpack(CWnd *pParent, CPalette *pPalette,
	          CInventory *pInventory);

	int SelectedItem(CPoint point);

	static  void UpdateBackpack(CDC *pDC);
	static  void UpdateContent(CDC *pDC);
	static  void UpdateItem(CDC *pDC, CItem *pItem, int nX, int nY);
	static  void UpdatePage(CDC *pDC);
	static  void ClearDialogImage(void);
	static  void RefreshBackground(void);
	static  void DoWaitCursor(void);
	static  void DoArrowCursor(void);

private:
	BOOL SetupKeyboardHook(void);
	void RemoveKeyboardHook(void);

	BOOL CreateWorkAreas(CDC *pDC);

private:
	BOOL    m_bKeyboardHook;                // whether keyboard hook present

// Dialog Data
	//{{AFX_DATA(CBackpack)
	enum { IDD = IDD_BACKPACK_DIALOG };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CBackpack)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog(void);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	virtual void OnOK(void);
	virtual void OnCancel(void);
	afx_msg void OnDestroy(void);
	afx_msg void OnPaint(void);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif //!_INC_BACKPACK
