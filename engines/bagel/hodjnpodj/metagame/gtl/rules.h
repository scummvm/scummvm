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

#ifndef BAGEL_METAGAME_GTL_RULES_H
#define BAGEL_METAGAME_GTL_RULES_H

#include "bagel/hodjnpodj/hnplibs/dibdoc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define IDD_RULES_DIALOG    900
#define IDC_RULES_OKAY      900

#define IDC_RULES_ARROWDN   901
#define IDC_RULES_ARROWUP   902
#define IDC_RULES_INVALID   903


class CRules : public CDialog {
// Construction
public:
	CRules(CWnd* pParent = nullptr, char *pszPathName = nullptr, CPalette *pPalette = nullptr, char *pszSoundPath = nullptr);   // standard constructor

	void UpdateScroll(int nPage);
	void ClearDialogImage(void);
	void RefreshBackground(void);

private:

	BOOL SetupKeyboardHook(void);
	void RemoveKeyboardHook(void);

	static  CDibDoc *LoadScrollDIB(const char *pSpec, CRect *pRect);
	static  BOOL PaintScrollDIB(CDC *pDC, CDibDoc *pDibDoc);
	static  CBitmap *CreateScrollBitmap(CDC *pDC, CDibDoc *pDibDoc, CPalette *pPalette);
	static  CDC *SetupMask(CDC *pDC, CDC *pBitmapDC, CBitmap *pMask, CBitmap * &pMaskOld, CRect *pRect);
	static  CDC *SetupCompatibleContext(CDC *pDC, CBitmap *pBitmap, CBitmap * &pBitmapOld, CPalette *pPalette, CPalette * &pPalOld);
	static  void ReleaseCompatibleContext(CDC * &pDC, CBitmap * &pBitmap, CBitmap *pBitmapOld, CPalette *pPalOld);

	static  BOOL CreateWorkAreas(CDC *pDC);
	static  void UnfurlScroll(CDC *pDC);
	static  void UpdateMore(CDC *pDC);
	static  void WritePage(CDC *pDC, int nPage);
	static  void DoWaitCursor(void);
	static  void DoArrowCursor(void);
	static  void Sleep(clock_t wait);

private:
	BOOL    m_bKeyboardHook;                // whether keyboard hook present

// Dialog Data
	//{{AFX_DATA(CRules)
	enum { IDD = IDD_RULES_DIALOG };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CRules)
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

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
