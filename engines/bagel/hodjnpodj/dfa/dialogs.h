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

#ifndef HODJNPODJ_DFA_DIALOGS_H
#define HODJNPODJ_DFA_DIALOGS_H

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/dfa/dfa.h"

namespace Bagel {
namespace HodjNPodj {
namespace DFA {

//////////////////////////////////////////////////////////////////////////////
////////    Options Specific to Word Search
//////////////////////////////////////////////////////////////////////////////
class CDFAOptDlg : public CBmpDialog {
private:
	int m_nGameTime = 0;
	int m_nBeaverTime = 0;
	CRect rDisplayGTime;
	CRect rDisplayBTime;

public:
	CDFAOptDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_MINIOPTIONS_DIALOG);
	void SetInitialOptions(int = 8, int = 1);    // Sets the private members
	void ClearDialogImage();
	bool OnInitDialog() override;

protected:
	//{{AFX_MSG(COptions)
	virtual void OnCancel() override;
	virtual void OnOK() override;
	virtual bool OnCommand(WPARAM, LPARAM) override;
	afx_msg bool OnEraseBkgnd(CDC *pDC);
	afx_msg void OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////
////////    Pack Rat Message Box
//////////////////////////////////////////////////////////////////////////////
class CMsgDlg : public CBmpDialog {
private:
	int     m_nWhichMsg;
	long    m_lScore;

public:
	CMsgDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_MESSAGEBOX);
	void SetInitialOptions(int, long);   // Sets the private members
	void ClearDialogImage();
	bool OnInitDialog() override;

protected:
	//{{AFX_MSG(COptions)
	virtual void OnCancel() override;
	virtual void OnOK() override;
	virtual bool OnCommand(WPARAM, LPARAM) override;
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace DFA
} // namespace HodjNPodj
} // namespace Bagel

#endif
