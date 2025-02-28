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

#ifndef HODJNPODJ_MAZEDOOM_OPTION_DIALOG_H
#define HODJNPODJ_MAZEDOOM_OPTION_DIALOG_H

#include "bagel/mfc/afx.h"
#include "bagel/hodjnpodj/boflib/dialog.h"
#include "bagel/hodjnpodj/mazedoom/maze_doom.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

#define LEFT_SIDE		 30 
#define	OPTIONS_COLOR	RGB(0, 0, 0)	// Color of the stats info CText


class COptnDlg : public CBmpDialog {
	// Construction
public:
	COptnDlg(CWnd *pParent = NULL, CPalette *pPalette = NULL);	// standard constructor
	~COptnDlg();		// destructor
	void UpdateScrollbars();
	void ClearDialogImage(void);

	CScrollBar m_ScrollTime;
	CScrollBar m_ScrollDifficulty;
	int m_nDifficulty;
	int m_nTime;
	int nSeconds;
	int nMinutes;
	CString mDifficultyTable[MAX_DIFFICULTY];
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange *pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(COptnDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel

#endif
