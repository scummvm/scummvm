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

#ifndef BAGEL_METAGAME_GTL_PAWN_H
#define BAGEL_METAGAME_GTL_PAWN_H

#include "bagel/hodjnpodj/metagame/bgen/invent.h"
#include "bagel/hodjnpodj/metagame/bgen/item.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define PAWN_SPEC   ".\\art\\mlscroll.bmp"      // path for pawn's DIB on disk
#define PAWN_DX         502
#define PAWN_DY         395
#define PAWN_CURL_DX        500
#define PAWN_CURL_DY        50
#define PAWN_BORDER_DX      60
#define PAWN_BORDER_DY      60

#define PAWN_TITLEZONE_DX   100
#define PAWN_TITLEZONE_DY   40
#define PAWN_TITLEZONE_DDY  -10

#define PAWN_TEXTZONE_DX    75
#define PAWN_TEXTZONE_DY    40
#define PAWN_TEXTZONE_DDY   20
#define PAWN_TEXTZONE_DDDY  5

#define PAWN_COSTZONE_DX    75
#define PAWN_COSTZONE_DY    20
#define PAWN_COSTZONE_DDY   20
#define PAWN_COSTZONE_DDDY  5

#define PAWN_BITMAP_DX      59
#define PAWN_BITMAP_DY      59
#define PAWN_BITMAP_DDX     5
#define PAWN_BITMAP_DDY     5

#define IDD_PAWN_DIALOG     940
#define IDC_PAWN_OKAY       940

#define IDC_PAWN_DOLLAR     941

#define PAWN_TEXT_COLOR     RGB(128,0,128)
#define PAWN_MORE_COLOR     RGB(0,0,0)
#define PAWN_BLURB_COLOR    RGB(0,0,255)

class CPawnShop : public CDialog {
// Construction
public:
	CPawnShop(CWnd* pParent, CPalette *pPalette, CInventory *pGeneralStore, CInventory *pInventory);

	int SelectedItem(CPoint point);

	static  void UpdatePawn(CDC *pDC);
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
	bool m_bKeyboardHook = false;	// whether keyboard hook present
	CRect OkayRect;					// rectangle bounding the OKAY button
	CRect PawnRect;					// x/y (left/right) and dx/dy (right/bottom) for the pawn window
	CRect ScrollTopRect,			// area spanned by upper scroll curl
		ScrollBotRect;				// area spanned by lower scroll curl

// Dialog Data
	//{{AFX_DATA(CPawnShop)
	enum { IDD = IDD_PAWN_DIALOG };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange *pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CPawnShop)
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
