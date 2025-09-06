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

#ifndef HODJNPODJ_HNPLIBS_CMESSBOX_H
#define HODJNPODJ_HNPLIBS_CMESSBOX_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

namespace Bagel {
namespace HodjNPodj {

#define TEXT_COLOR          RGB(0, 0, 0)    // displayed text color

#define MESSAGE_COL         16              // first message box positioning
#define MESSAGE_ROW         60

#define MESSAGE_WIDTH       186             // standard message box dims
#define MESSAGE_HEIGHT      20

#define MESSAGE2_ROW_OFFSET ( MESSAGE_HEIGHT + 4 )  // Row offset from first
// message box

#define IDD_GAMEOVER 201

class CMessageBox : public CBmpDialog {
private:
	CText *m_cTextMessage1, *m_cTextMessage2;
	const char *m_pMessage1, *m_pMessage2;

public:
	// standard constructor
	CMessageBox(CWnd *pParent, CPalette *pPalette, const char *msg1, const char *msg2 = nullptr, const int dx = -1, const int dy = -1);

	void ClearDialogImage();

	// Dialog Data
	//{{AFX_DATA(CMessageBox)
	enum { IDD = IDD_GAMEOVER };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

protected:
	// Implementation
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CMessageBox)
	afx_msg void OnPaint();
	virtual void OnOK() override;
	virtual void OnCancel() override;
	virtual bool OnInitDialog() override;
	afx_msg void OnDestroy();
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
