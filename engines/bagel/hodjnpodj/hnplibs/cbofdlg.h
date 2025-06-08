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

#ifndef HODJNPODJ_HNPLIBS_CBOFDLG_H
#define HODJNPODJ_HNPLIBS_CBOFDLG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"

namespace Bagel {
namespace HodjNPodj {

class CBmpDialog : public CDialog {
public:
	// Construction
	CBmpDialog(CWnd *, CPalette *, int, const char *, const int dx = -1, const int dy = -1, BOOL bSaveBackground = TRUE);
	CBmpDialog(CWnd *, CPalette *, int, int, const int dx = -1, const int dy = -1, BOOL bSaveBackground = TRUE);

	void RefreshBackground(CDC *pDC = nullptr);

private:
	// Dialog Data
	//{{AFX_DATA(CBmpDialog)
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

protected:
	// Implementation
	void EndDialog(int);
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CBmpDialog)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog() override;
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBitmap    *m_pDlgBackground;
	CPalette   *m_pPalette;
	const char *m_pBmpFileName;
	int         m_nBmpID;
	int         m_nDx;
	int         m_nDy;
	BOOL        m_bSaveBackground;
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
