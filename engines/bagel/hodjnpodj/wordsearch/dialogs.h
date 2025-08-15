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

#ifndef HODJNPODJ_WORDSEARCH_DIALOGS_H
#define HODJNPODJ_WORDSEARCH_DIALOGS_H

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/wordsearch/wordsearch.h"

namespace Bagel {
namespace HodjNPodj {
namespace WordSearch {

//////////////////////////////////////////////////////////////////////////////
////////    Options Specific to Word Search
//////////////////////////////////////////////////////////////////////////////
class CWSOptDlg : public CBmpDialog {
private:
	int     m_nGameTime;
	BOOL    m_bShowWord;
	BOOL    m_bWordForwardOnly;
	CRect   rDisplayTime;

public:
	CWSOptDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr, UINT = IDD_MINIOPTIONS_DIALOG);
	void SetInitialOptions(int = 0, BOOL = TRUE, BOOL = FALSE);  // Sets the private members
	void ClearDialogImage();
	BOOL OnInitDialog() override;

protected:
	//{{AFX_MSG(COptions)
	virtual void OnCancel(void) override;
	virtual void OnOK(void) override;
	virtual BOOL OnCommand(WPARAM, LPARAM) override;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
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
	long    m_nWordsLeft;

public:
	CMsgDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr, UINT = IDD_MESSAGEBOX);
	void SetInitialOptions(int, int);    // Sets the private members
	void ClearDialogImage();
	BOOL OnInitDialog() override;

protected:
	//{{AFX_MSG(COptions)
	virtual void OnCancel(void) override;
	virtual void OnOK(void) override;
	virtual BOOL OnCommand(WPARAM, LPARAM) override;
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace Poker
} // namespace HodjNPodj
} // namespace Bagel

#endif
