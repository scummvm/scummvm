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

#ifndef HODJNPODJ_MANKALA_MNKOPT_H
#define HODJNPODJ_MANKALA_MNKOPT_H

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/mankala/mnk.h"
#include "bagel/hodjnpodj/mankala/resource.h"

namespace Bagel {
namespace HodjNPodj {
namespace Mankala {

#define     OPTIONS_COLOR   RGB(0, 0, 0)    // Color of the stats info CText
#define     LEFT_SIDE       25
#define     NUM_LEVELS      5// (MAXSTRENGTH - MINSTRENGTH + 1)     // 5- 1 + 1 = 5

/////////////////////////////////////////////////////////////////////////////
// CMnkOpt dialog

class CMnkOpt : public CDialog {
// Construction
public:
	CMnkOpt(CWnd* pParent = nullptr);  // standard constructor
	~CMnkOpt();     // destructor

// Dialog Data
	//{{AFX_DATA(CMnkOpt)
	enum { IDD = IDD_MNK_DIALOG };
	int     m_iLevel0;
	int     m_iLevel1;
	int     m_iPlayer0;
	int     m_iPlayer1;
	int     m_iStartStones;
	int     m_iTableStones;
	bool    m_bInitData;
	int     m_iMaxDepth0;
	int     m_iMaxDepth1;
	bool    m_bDumpMoves;
	bool    m_bDumpPopulate;
	bool    m_bDumpTree;
	int     m_iCapDepth0;
	int     m_iCapDepth1;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CMnkOpt)
	// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CMnkUsr dialog

class CMnkUsr : public CBmpDialog {
// Construction
public:
	CMnkUsr(CWnd *xpParent = nullptr,
	        CPalette *xpPalette = nullptr, unsigned int nID = IDD_MNK_USER) ;
	~CMnkUsr();     // destructor

	CPalette *m_xpGamePalette ;

	CScrollBar *m_xpUScrShell, *m_xpUScrStrength ;
	void UpdateScrollbars();

// Dialog Data
	//{{AFX_DATA(CMnkUsr)
	enum { IDD = IDD_MNK_USER };
	int     m_iUShells;
	int     m_iUStrength;
	CString mLevelTable [NUM_LEVELS];
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	virtual bool OnCommand(WPARAM, LPARAM) override;
	void ClearDialogImage();


	// Generated message map functions
	//{{AFX_MSG(CMnkUsr)
	afx_msg void OnHScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar* pScrollBar);
	virtual bool OnInitDialog() override;
	afx_msg void OnPaint();
//	afx_msg void OnKillfocusUShells();
//	afx_msg void OnKillfocusUStrength();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace Mankala
} // namespace HodjNPodj
} // namespace Bagel

#endif
