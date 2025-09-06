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

#ifndef HODJNPODJ_ARTPARTS_OPTNDLG_H
#define HODJNPODJ_ARTPARTS_OPTNDLG_H

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

namespace Bagel {
namespace HodjNPodj {
namespace ArtParts {

#define LEFT_SIDE        30
#define OPTIONS_COLOR   RGB(0, 0, 0)    // Color of the stats info CText

/////////////////////////////////////////////////////////////////////////////
// COptnDlg dialog

class COptnDlg : public CBmpDialog {
// Construction
public:
	COptnDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr);  // standard constructor
	~COptnDlg();        // destructor
	void UpdateScrollbars();
	void ClearDialogImage();

	int m_nTime;
	int m_nRows;
	int m_nColumns;
	int m_nMins;
	int m_nSecs;
	float   m_nScore;
	bool    m_bFramed;
	int m_nNumParts;

	int nCFacs;
	int nRFacs;

// Dialog Data
	//{{AFX_DATA(COptnDlg)
	enum { IDD = IDD_SUBOPTIONS };
	CScrollBar m_ScrollTime;
	CScrollBar m_ScrollColumns;
	CScrollBar m_ScrollRows;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(COptnDlg)
	virtual bool OnInitDialog() override;
	afx_msg void OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	virtual void OnOK() override;
	virtual void OnCancel() override;
	afx_msg void OnPaint();
	afx_msg void OnFramed();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace ArtParts
} // namespace HodjNPodj
} // namespace Bagel

#endif
