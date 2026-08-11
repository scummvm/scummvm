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

#ifndef HODJNPODJ_WORDSEARCH_CLONGDLG_H
#define HODJNPODJ_WORDSEARCH_CLONGDLG_H

#include "bagel/afxwin.h"

namespace Bagel {
namespace HodjNPodj {
namespace WordSearch {

#define CBUTTON1 IDOK
#define CBUTTON2 IDCANCEL

class CLongDialog : public CBmpDialog {
public:
	// standard constructor
	CLongDialog(CWnd *, CPalette *, const char *,
	            const char *, const char *pszText2 = nullptr);

private:
	void ClearDialogImage();

	// Dialog Data
	//{{AFX_DATA(C1ButtonDialog)
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(C1ButtonDialog)
	afx_msg void OnPaint();
	virtual void OnOK() override;
	virtual void OnCancel() override;
	virtual bool OnInitDialog() override;
	afx_msg void OnDestroy();
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CText *m_cTextMessage1 = nullptr;
	CText *m_cTextMessage2 = nullptr;
	CText *m_cWordList[WORDSPERLIST] = {};
	const char *m_pszMessage1 = nullptr;
	const char *m_pszMessage2 = nullptr;
	const char *m_pszButton1Text = nullptr;
	CColorButton *m_pButton1 = nullptr;
};

} // namespace Poker
} // namespace HodjNPodj
} // namespace Bagel

#endif
