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

#ifndef HODJNPODJ_PEGGLE_OPTNDLG_H
#define HODJNPODJ_PEGGLE_OPTNDLG_H

#include "bagel/hodjnpodj/peggle/options.h"

namespace Bagel {
namespace HodjNPodj {
namespace Peggle {

#define OPTN_BUTTON_WIDTH       50
#define OPTN_BUTTON_HEIGHT      47
#define CROSS_X                 40
#define CROSS_Y                 27
#define TRIANGLE_X              40
#define TRIANGLE_Y              76
#define CROSSPLUS_X             125
#define CROSSPLUS_Y             27
#define TRIANGLEPLUS_X          125
#define TRIANGLEPLUS_Y          76

/////////////////////////////////////////////////////////////////////////////
// COptnDlg dialog

class COptnDlg : public COptions {
// Construction
public:
	COptnDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr);  // standard constructor

// Dialog Data
	//{{AFX_DATA(COptnDlg)
	enum { IDD = IDD_SUBOPTIONS };
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange *pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(COptnDlg)
	virtual bool OnInitDialog() override;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	virtual void OnOK() override;
	virtual void OnCancel() override;
	afx_msg void OnPaint();
	afx_msg void OnClickedOkay() ;
	afx_msg void OnClickedCancel() ;
	afx_msg void OnClickedCross() ;
	afx_msg void OnClickedCrossPlus() ;
	afx_msg void OnClickedTriangle() ;
	afx_msg void OnClickedTrianglePlus() ;
	afx_msg void OnRandom();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel

#endif
