// mnkopt.h : header file
//

#ifndef __mnkopt_H__
#define __mnkopt_H__

#include "mnk.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/cmessbox.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"

#define		OPTIONS_COLOR	RGB(0, 0, 0)	// Color of the stats info CText
#define		LEFT_SIDE		25
#define		NUM_LEVELS		5// (MAXSTRENGTH - MINSTRENGTH + 1)		// 5- 1 + 1 = 5

/////////////////////////////////////////////////////////////////////////////
// CMnkOpt dialog

class CMnkOpt : public CDialog
{
// Construction
public:
	CMnkOpt(CWnd* pParent = NULL);	// standard constructor
	~CMnkOpt();		// destructor

// Dialog Data
	//{{AFX_DATA(CMnkOpt)
	enum { IDD = IDD_MNK_DIALOG };
	int		m_iLevel0;
	int		m_iLevel1;
	int		m_iPlayer0;
	int		m_iPlayer1;
	int		m_iStartStones;
	int		m_iTableStones;
	BOOL	m_bInitData;
	int		m_iMaxDepth0;
	int		m_iMaxDepth1;
	BOOL	m_bDumpMoves;
	BOOL	m_bDumpPopulate;
	BOOL	m_bDumpTree;
	int		m_iCapDepth0;
	int		m_iCapDepth1;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CMnkOpt)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CMnkUsr dialog

class CMnkUsr : public CBmpDialog
{
// Construction
public:
	CMnkUsr::CMnkUsr(CWnd *xpParent = NULL,
		CPalette *xpPalette = NULL, UINT nID = IDD_MNK_USER) ;
	~CMnkUsr();		// destructor

	CPalette * m_xpGamePalette ;

	CScrollBar * m_xpUScrShell, * m_xpUScrStrength ;
	void UpdateScrollbars();

// Dialog Data
	//{{AFX_DATA(CMnkUsr)
	enum { IDD = IDD_MNK_USER };
	int		m_iUShells;
	int		m_iUStrength;
	CString mLevelTable [NUM_LEVELS];
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM, LPARAM);	
	void ClearDialogImage(void);


	// Generated message map functions
	//{{AFX_MSG(CMnkUsr)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
//	afx_msg void OnKillfocusUShells();
//	afx_msg void OnKillfocusUStrength();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif // __mnkopt_H__

