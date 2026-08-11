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

#ifndef HODJNPODJ_GARFUNKLE_OPTNDLG_H
#define HODJNPODJ_GARFUNKLE_OPTNDLG_H

#include "bagel/hodjnpodj/garfunkle/resource.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

namespace Bagel {
namespace HodjNPodj {
namespace Garkfunkle {

#define LEFT_SIDE       26
#define OPTIONS_COLOR   RGB(0, 0, 0)    // Color of the stats info CText
#define NUM_SPEEDS      12

/////////////////////////////////////////////////////////////////////////////
// COptnDlg dialog

class COptnDlg : public CBmpDialog {
// Construction
public:
	COptnDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr);  // standard constructor
	~COptnDlg();        // destructor
	void UpdateScrollbars();
	void ClearDialogImage();

// Dialog Data
	//{{AFX_DATA(COptnDlg)
	enum { IDD = IDD_SUBOPTIONS };
	CScrollBar  m_ScrollButtons;
	CScrollBar  m_ScrollSpeed;
	bool        m_bPlayGame;
	int m_nNumButtons;
	int m_nSpeed;
	CString mSpeedTable [NUM_SPEEDS];
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(COptnDlg)
	virtual bool OnInitDialog() override;
	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;
	afx_msg void OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	virtual void OnOK() override;
	virtual void OnCancel() override;
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace Garfunkle
} // namespace HodjNPodj
} // namespace Bagel

#endif
