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

#ifndef HODJNPODJ_HNPLIBS_RULES_H
#define HODJNPODJ_HNPLIBS_RULES_H

#include "bagel/hodjnpodj/hnplibs/dibdoc.h"

namespace Bagel {
namespace HodjNPodj {

#define IDD_RULES_DIALOG    900
#define IDC_RULES_OKAY      900

#define IDC_RULES_ARROWDN   901
#define IDC_RULES_ARROWUP   902
#define IDC_RULES_INVALID   903


class CRules : public CDialog {
// Construction
public:
	CRules(CWnd *pParent = nullptr, const char *pszPathName = nullptr,
	       CPalette *pPalette = nullptr, const char *pszSoundPath = nullptr);

	void UpdateScroll(int nPage);
	void ClearDialogImage();
	void RefreshBackground();

private:

	bool SetupKeyboardHook();
	void RemoveKeyboardHook();

	static  CDibDoc *LoadScrollDIB(const char *pSpec, CRect *pRect);
	static  bool PaintScrollDIB(CDC *pDC, CDibDoc *pDibDoc);
	static  CBitmap *CreateScrollBitmap(CDC *pDC, CDibDoc *pDibDoc, CPalette *pPalette);
	static  CDC *SetupMask(CDC *pDC, CDC *pBitmapDC, CBitmap *pMask, CBitmap * &pMaskOld, CRect *pRect);
	static  CDC *SetupCompatibleContext(CDC *pDC, CBitmap *pBitmap, CBitmap * &pBitmapOld, CPalette *pPalette, CPalette * &pPalOld);
	static  void ReleaseCompatibleContext(CDC * &pDC, CBitmap * &pBitmap, CBitmap *pBitmapOld, CPalette *pPalOld);

	bool CreateWorkAreas(CDC *pDC);
	void UnfurlScroll(CDC *pDC);
	void UpdateMore(CDC *pDC);
	void WritePage(CDC *pDC, int nPage);
	void DoWaitCursor();
	void DoWaitCursor(int nCode) override {
		CDialog::DoWaitCursor(nCode);
	}
	void DoArrowCursor();
	void Sleep(clock_t wait);

private:
	bool m_bKeyboardHook = false;	// whether keyboard hook present
	CRect OkayRect;					// rectangle bounding the OKAY button
	CRect ScrollRect,				// x/y (left/right) and dx/dy (right/bottom) for the scroll window
		ScrollTopRect,				// rectangle bounding the scroll top section
		ScrollBotRect,				// rectangle bounding the scroll bottom section
		ScrollMidRect;				// rectangle bounding the scroll middle section

	CRect ScrollTopCurlRect,		// current location of top curl for mouse clicks
		ScrollBotCurlRect;			// current location of bottom curl for mouse clicks

// Dialog Data
	//{{AFX_DATA(CRules)
	enum { IDD = IDD_RULES_DIALOG };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CRules)
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

} // namespace HodjNPodj
} // namespace Bagel

#endif
