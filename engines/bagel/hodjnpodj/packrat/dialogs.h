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

#ifndef HODJNPODJ_PACKRAT_DIALOGS_H
#define HODJNPODJ_PACKRAT_DIALOGS_H

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/packrat/packrat.h"

namespace Bagel {
namespace HodjNPodj {
namespace Packrat {

#define LEVELMAX       36
#define LIVESMAX       5
#define LEVELMIN       1
#define LIVESMIN       1

//////////////////////////////////////////////////////////////////////////////
////////    Options Specific to Pack Rat
//////////////////////////////////////////////////////////////////////////////
class CPackRatOptDlg : public CBmpDialog {
private:
	int     m_nLives;
	int     m_nLevel;
	CRect   rectDisplayLevel;
	CRect   rectDisplayLives;

public:
	CPackRatOptDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_MINIOPTIONS_DIALOG);
	void SetInitialOptions(int = 1, int = 1);    // Sets the private members
	void ClearDialogImage();
	bool OnInitDialog() override;

protected:
	//{{AFX_MSG(COptions)
	virtual void OnCancel() override;
	virtual void OnOK() override;
	virtual bool OnCommand(WPARAM, LPARAM) override;
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
	long    m_lCurrentScore;
	int     m_nLevel;

public:
	CMsgDlg(CWnd* pParent = nullptr, CPalette *pPalette = nullptr, unsigned int = IDD_MESSAGEBOX);
	void SetInitialOptions(int, long, int = 0);  // Sets the private members
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

} // namespace Packrat
} // namespace HodjNPodj
} // namespace Bagel

#endif
