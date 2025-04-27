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

#ifndef HODJNPODJ_METAGAME_GTL_INFDLG_H
#define HODJNPODJ_METAGAME_GTL_INFDLG_H

#include <afxwin.h>
#include <afxext.h>

#include "gtl.h"

#include "gtldoc.h"
#include "gtlview.h"
#include "gtlfrm.h"

#include "bsutl.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

/////////////////////////////////////////////////////////////////////////////
// CInfDlg dialog

class CInfDlg : public CDialog
{
public:
//    CWnd * m_xpParentWnd ;		// parent window = frame window
    CGtlFrame * m_xpFrameWnd ;	// parent window = frame window
    class CGtlDoc * m_xpDocument ;
    class CGtlData * m_xpGtlData ;
    BOOL m_bClose ;		// close window flag

// Construction
public:
	CInfDlg(CWnd* pParent = NULL);	// standard constructor
	void Create(void) {CDialog::Create(CInfDlg::IDD,
			(CWnd *)m_xpFrameWnd);}
	~CInfDlg(void) {TRACEDESTRUCTOR(CInfDlg) ;}

// Dialog Data
	//{{AFX_DATA(CInfDlg)
	enum { IDD = IDD_INFO_DLG };
	int		m_iXCoord;
	int		m_iYCoord;
	int		m_iLogScrollPosX;
	int		m_iLogScrollPosY;
	int		m_iTotalSizeX;
	int		m_iTotalSizeY;
	int		m_iScrollRangeX;
	int		m_iViewSizeX;
	int		m_iScrollRangeY;
	int		m_iViewSizeY;
	int		m_iScrollCount;
	//}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    virtual void CInfDlg::PostNcDestroy() ;
	virtual void CInfDlg::OnCancel() ;

	// Generated message map functions
	//{{AFX_MSG(CInfDlg)
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd) ;
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



/////////////////////////////////////////////////////////////////////////////
// CCtlDlg dialog

class CCtlDlg : public CDialog
{
public:
//  CWnd * m_xpParentWnd ;		// parent window
    CGtlFrame * m_xpFrameWnd ;	// parent window = frame window
//    class CGtlFrame * m_xpFrameWnd ;		// frame window
    class CGtlDoc * m_xpDocument ;
    class CGtlData * m_xpGtlData ;
    class CGtlView * m_xpView ;	// view that this box controls
    class CBsuSet m_cDlgBsuSet ;	// scroll bar set for dialog box

    CScrollBar * m_xpVScrholl, * m_xpHScroll ;	// scroll bars
    int m_iHScrollMin, m_iHScrollMax, m_iHScrollCurrent ;
    int m_iVScrollMin, m_iVScrollMax, m_iVScrollCurrent ;
    BOOL m_bClose ;		// close window flag

// Construction
public:
	CCtlDlg(CWnd* pParent = NULL);	// standard constructor
	void Create(void) {CDialog::Create(CCtlDlg::IDD,
			(CWnd *)m_xpFrameWnd);}
	~CCtlDlg(void) {TRACEDESTRUCTOR(CCtlDlg) ;}

// Dialog Data
	//{{AFX_DATA(CCtlDlg)
	enum { IDD = IDD_CTL_DLG };
	BOOL	m_bPaintBackground;
	BOOL	m_bShowNodes;
	BOOL	m_bMetaGame;
	BOOL	m_bShowSensitivity;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void PostNcDestroy(void) ;
	virtual void CCtlDlg::OnCancel() ;

	// Generated message map functions
	//{{AFX_MSG(CCtlDlg)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnClickedCtlPaintBkg();
	afx_msg void OnClickedCtlShowNodes();
	afx_msg void OnClickedCtlMetaGame();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnClickedCtlShowSensitivity();
	afx_msg void OnCtlDump();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// infdlg.cpp : implementation file

//- CCtlDlg::EffectControl -- effect control command on document
private: BOOL CCtlDlg::EffectControl(BOOL bUpdate PDFT(TRUE)) ;
//- CCtlDlg::InitDialogBox -- set scroll bars
public: BOOL CCtlDlg::InitDialogBox(void) ;
};


/////////////////////////////////////////////////////////////////////////////
// CNodeDlg dialog

class CNodeDlg : public CDialog
{
public:
    CGtlFrame * m_xpFrameWnd ;	// parent window = frame window
//  CWnd * m_xpParentWnd ;		// parent window
//    class CGtlFrame * m_xpFrameWnd ;		// frame window
    class CGtlDoc * m_xpDocument ;
    class CGtlData * m_xpGtlData ;
    class CGtlView * m_xpView ;	// view that this box controls
    BOOL m_bApply ;		// apply button pressed
    BOOL m_bClose ;		// close window flag

// Dialog Data
	//{{AFX_DATA(CNodeDlg)
	enum { IDD = IDD_NODE_DLG };
	CString	m_stSector;
	CString	m_stLoc;
	CString	m_stName;
	int		m_iX;
	int		m_iY;
	int		m_iNodeIndex;
	BOOL	m_bRelocatable;
	int		m_iWeight;
	int		m_iSensitivity;
//	int		m_iSector;
	int		m_iSectorButton;
	//}}AFX_DATA

// Construction
public:
	CNodeDlg(CWnd* pParent = NULL);	// standard constructor
	void Create(void) {CDialog::Create(CNodeDlg::IDD,
			(CWnd *)m_xpFrameWnd);}
	~CNodeDlg(void) {TRACEDESTRUCTOR(CNodeDlg) ;}

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void PostNcDestroy(void) ;
	virtual void OnCancel() ;
//	BOOL EffectControl(BOOL bUpdate PDFT(TRUE)) ;

	// Generated message map functions
	//{{AFX_MSG(CNodeDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
//	afx_msg void OnNodeReloc();
	afx_msg void OnNodeReset();
	afx_msg void OnNodeApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CActDlg dialog

class CActDlg : public CDialog
{
// Construction
public:
	CActDlg(CWnd* pParent = NULL);	// standard constructor
	~CActDlg(void) {TRACEDESTRUCTOR(CActDlg) ;}

// Dialog Data
	//{{AFX_DATA(CActDlg)
	enum { IDD = IDD_ACTION_DLG };
	CString	m_stGame;
	int		m_iMoney;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CActDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CMenuDlg dialog

class CMenuDlg : public CDialog
{
public:

    CGtlFrame * m_xpFrameWnd ;	// parent window = frame window
//  CWnd * m_xpParentWnd ;		// parent window = frame
    class CGtlDoc * m_xpDocument ;
    class CGtlData * m_xpGtlData ;
    class CGtlView * m_xpView ;	// view that this box controls

    BOOL m_bNew ;		// new file
    BOOL m_bOpen ;		// open file
    BOOL m_bSave ;		// save file
    BOOL m_bClose ;		// close window flag


// Construction
public:
    CMenuDlg(CWnd* pParent = NULL);	// standard constructor
	void Create(void) {CDialog::Create(CMenuDlg::IDD,
			(CWnd *)m_xpFrameWnd);}
	~CMenuDlg(void) {TRACEDESTRUCTOR(CMenuDlg) ;}

// Dialog Data
	//{{AFX_DATA(CMenuDlg)
	enum { IDD = IDD_MENU_DLG };
	BOOL	m_bCtlDlg;
	BOOL	m_bInfoDlg;
	BOOL	m_bNodeDlg;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
				// DDX/DDV support

    virtual void PostNcDestroy(void) ;
////- CMenuDlg::EffectControl -- effect control command on document
private: BOOL CMenuDlg::EffectControl(BOOL bUpdate PDFT(TRUE)) ;

	// Generated message map functions
	//{{AFX_MSG(CMenuDlg)
	afx_msg void OnMenuCtl();
	afx_msg void OnMenuInfo();
	afx_msg void OnMenuNew();
	afx_msg void OnMenuNode();
	afx_msg void OnMenuOpen();
	afx_msg void OnMenuSave();
	virtual void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
} ;

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
