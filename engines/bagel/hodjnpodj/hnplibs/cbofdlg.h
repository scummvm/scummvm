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
	CBmpDialog(CWnd *, CPalette *, int, const char *, const int dx = -1, const int dy = -1, bool bSaveBackground = true);
	CBmpDialog(CWnd *, CPalette *, int, int, const int dx = -1, const int dy = -1, bool bSaveBackground = true);

	void RefreshBackground(CDC *pDC = nullptr);

private:
	// Dialog Data
	//{{AFX_DATA(CBmpDialog)
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

protected:
	// Implementation
	void EndDialog(int);
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CBmpDialog)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	bool OnInitDialog() override;
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(bool bShow, unsigned int nStatus);
	afx_msg void OnSize(unsigned int nType, int cx, int cy);
	afx_msg void OnMouseMove(unsigned int nFlags, CPoint point);
	afx_msg void OnLButtonDown(unsigned int nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBitmap    *m_pDlgBackground = nullptr;
	CPalette   *m_pPalette = nullptr;
	const char *m_pBmpFileName = nullptr;
	int         m_nBmpID = 0;
	int         m_nDx = 0;
	int         m_nDy = 0;
	bool        m_bSaveBackground = false;
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
